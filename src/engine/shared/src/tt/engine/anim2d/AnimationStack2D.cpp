#include <algorithm>

#include <tt/code/bufferutils.h>
#include <tt/code/ErrorStatus.h>
#include <tt/code/helpers.h>
#include <tt/engine/anim2d/AnimationFactory2D.h>
#include <tt/engine/anim2d/AnimationStack2D.h>
#include <tt/engine/anim2d/TranslationAnimation2D.h>
#include <tt/math/Vector3.h>
#include <tt/platform/tt_error.h>
#include <tt/str/parse.h>
#include <tt/str/toStr.h>
#include <tt/xml/XmlNode.h>
#include <tt/xml/util/parse.h>


namespace tt {
namespace engine {
namespace anim2d {

#define MAKE_VERSION(major, minor) (((major) << 8) | (minor))
#define GET_MAJOR_VERSION(version) ((version) >> 8)
#define GET_MINOR_VERSION(version) ((version) & 0xFF)

static const u16 s_versionAnimationStack2D = MAKE_VERSION(1, 1);
static const size_t s_sizeAnimationStack2D = 2 + 1 + 1 + 2; // version, autosort, initiallysuspended, number of animations


//--------------------------------------------------------------------------------------------------
// Public member functions

AnimationStack2D::AnimationStack2D()
:
StackBase<PositionAnimation2DPtr>(),
m_autoSort(false),
m_initiallySuspended(false),
m_gameTranslations()
{
}


void AnimationStack2D::push_back(const PositionAnimation2DPtr& p_animation)
{
	TT_ASSERTMSG(p_animation != 0, "No animation specified.");
	m_animations.push_back(p_animation);
	if (m_autoSort)
	{
		std::sort(m_animations.begin(), m_animations.end(), AnimationPredicate2D<PositionAnimation2DPtr>());
	}
	if (m_initiallySuspended == false)
	{
		p_animation->start();
	}
}


void AnimationStack2D::clear()
{
	code::helpers::freeContainer(m_animations);
	code::helpers::freeContainer(m_gameTranslations);
}


void AnimationStack2D::updateTransform(math::Matrix44* p_mtx) const
{
	TT_NULL_ASSERT(p_mtx);
	
	if (empty())
	{
		p_mtx->setIdentity();
		return;
	}
	
	PositionAnimation2D::Transform transform;
	
	for (Stack::const_iterator it = m_animations.begin(); it != m_animations.end(); ++it)
	{
		if((*it)->isTagged())
		{
			(*it)->applyTransform(&transform);
		}
	}
	
	(*p_mtx) = transform.calcMatrix();
}

bool AnimationStack2D::hasZAnimation() const
{
	for (Stack::const_iterator it = m_animations.begin(); it != m_animations.end(); ++it)
	{
		if ((*it)->hasZAnimation() && (*it)->isTagged())
		{
			return true;
		}
	}
	return false;
}


bool AnimationStack2D::hasRotationAnimation() const
{
	for (Stack::const_iterator it = m_animations.begin(); it != m_animations.end(); ++it)
	{
		if ((*it)->hasRotationAnimation() && (*it)->isTagged())
		{
			return true;
		}
	}
	return false;
}


void AnimationStack2D::setAutoSort(bool p_enabled)
{
	if (m_autoSort != p_enabled)
	{
		m_autoSort = p_enabled;
		if (m_autoSort)
		{
			std::sort(m_animations.begin(), m_animations.end(), AnimationPredicate2D<PositionAnimation2DPtr>());
		}
	}
}


void AnimationStack2D::setInitiallySuspended(bool p_suspended)
{
	m_initiallySuspended = p_suspended;
}


bool AnimationStack2D::load(const xml::XmlNode* p_node, bool p_invertTranslationAnimationY)
{
	Tags emptyTags;
	return load(p_node, p_invertTranslationAnimationY, emptyTags, emptyTags);
}


bool AnimationStack2D::load(const xml::XmlNode* p_node,
                            bool                p_invertTranslationAnimationY,
                            const Tags&         p_applyTags,
                            const Tags&         p_acceptedTags)
{
	TT_NULL_ASSERT(p_node);
	
	TT_ERR_CREATE("load");
	
	clear();
	
	if (p_node->getAttribute("autosort").empty() == false)
	{
		bool autosort = str::parseBool(p_node->getAttribute("autosort"), &errStatus);
		if (errStatus.hasError())
		{
			TT_PANIC("Value '%s' of attribute 'autosort' from node '%s' can't be converted to a bool.",
			         p_node->getAttribute("autosort").c_str(), 
			         p_node->getName().c_str());
			return false;
		}
		setAutoSort(autosort);
	}
	else
	{
		setAutoSort(true);
	}
	
	bool suspended = false;
	if (p_node->getAttribute("suspended").empty() == false)
	{
		suspended = str::parseBool(p_node->getAttribute("suspended"), &errStatus);
		if (errStatus.hasError())
		{
			TT_PANIC("Value '%s' of attribute 'suspended' from node '%s' can't be converted to a bool.",
			         p_node->getAttribute("suspended").c_str(), 
			         p_node->getName().c_str());
			return false;
		}
	}
	setInitiallySuspended(suspended);
	
	// tags for this stack
	Tags stackTags(p_applyTags);
	str::Strings strstackTags;
	
	for (const xml::XmlNode* child = p_node->getChild(); child != 0; child = child->getSibling())
	{
		if(child->getName() == "tag")
		{
			TT_ERR_SET_LOC("loading tags from animationstack");
			
			Tag tag(Animation2D::loadTag(child, p_acceptedTags, &errStatus));
			
			TT_ASSERTMSG(errStatus.hasError() == false, "Tag loading failed: %s", 
				         errStatus.getErrorMessage().c_str());
			if(errStatus.hasError()) return false;
			
			stackTags.insert(tag);
			strstackTags.push_back(child->getAttribute("name"));
		}
		else
		{
			PositionAnimation2DPtr anim;
			if ((child->getName() == "translation" && child->getFirstChild("begin") == 0) ||
				child->getName() == "gametranslation")
			{
				TranslationAnimation2DPtr translation = 
					TranslationAnimation2DPtr(new TranslationAnimation2D(true));
				
				anim = translation;
				m_gameTranslations.push_back(translation);
			}
			else
			{
				anim = AnimationFactory2D::create(child->getName());
			}
			
			if (anim == 0)
			{
				TT_PANIC("Unknown/unsupported animation type '%s' found in '%s'.",
					child->getName().c_str(),
					p_node->getName().c_str());
				return false;
			}
			
			// HACK: For TranslationAnimation2D, pass the "invert Y" setting along
			// FIXME: Do we even still need this "invert Y" behavior? Can't we just kill it?
			if (child->getName() == "translation" ||
			    child->getName() == "gametranslation")
			{
				tt_ptr_static_cast<TranslationAnimation2D>(anim)->setInvertYDuringLoad(p_invertTranslationAnimationY);
			}
			
			if (anim->load(child, p_applyTags, p_acceptedTags) == false)
			{
				clear();
				return false;
			}
			push_back(anim);
		}
	}
	
	// apply tags to the stack
	for (Stack::iterator it = m_animations.begin(); it != m_animations.end(); ++it)
	{
		(*it)->addTags(stackTags);
		(*it)->addTags(strstackTags);
	}

	return true;
}


bool AnimationStack2D::save(xml::XmlNode* p_node) const
{
	TT_NULL_ASSERT(p_node);
	
	p_node->setAttribute("autosort", str::toStr(m_autoSort));
	p_node->setAttribute("suspended", str::toStr(m_initiallySuspended));
	for (Stack::const_iterator it = m_animations.begin(); it != m_animations.end(); ++it)
	{
		xml::XmlNode* node = new xml::XmlNode;
		node->setName(AnimationFactory2D::getTextType(*it));
		if ((*it)->save(node) == false)
		{
			delete node;
			return false;
		}
		p_node->addChild(node);
	}
	return true;
}


bool AnimationStack2D::load(const fs::FilePtr& p_file, bool p_invertTranslationAnimationY)
{
	clear();
	
	tt::code::BufferPtr content = tt::fs::getFileContent(p_file);
	const u8* scratch = reinterpret_cast<const u8*>(content->getData());
	size_t size = static_cast<size_t>(content->getSize());

	return load(scratch, size, p_invertTranslationAnimationY);
}


bool AnimationStack2D::save(const fs::FilePtr& p_file) const
{
	TT_ERR_CREATE("save");
	
	size_t bufsize = getBufferSize();
	u8* buffer = new u8[bufsize];
	u8* scratch = buffer;
	size_t size = bufsize;
	if (save(scratch, size, &errStatus) == false)
	{
		delete[] buffer;
		return false;
	}
	
	if (p_file->write(buffer, static_cast<fs::size_type>(bufsize)) != static_cast<fs::size_type>(bufsize))
	{
		delete[] buffer;
		TT_PANIC("Failed to save %d bytes to '%s'", bufsize, p_file->getPath());
		return false;
	}
	delete[] buffer;
	return true;
}


bool AnimationStack2D::load(const u8*& p_bufferOUT, size_t& p_sizeOUT, bool p_invertTranslationAnimationY)
{
	Tags emptyTags;
	return load(p_bufferOUT, p_sizeOUT, p_invertTranslationAnimationY, emptyTags, emptyTags);
}


bool AnimationStack2D::load(const u8*& p_bufferOUT, size_t& p_sizeOUT, bool p_invertTranslationAnimationY,
                            const Tags& p_applyTags, const Tags& p_acceptedTags)
{
	if (loadHeader(p_bufferOUT, p_sizeOUT) == false)
	{
		TT_PANIC("Failed to load header.");
		return false;
	}
	
	for (Stack::iterator it = m_animations.begin(); it != m_animations.end(); ++it)
	{
		if (p_sizeOUT < 1)
		{
			TT_PANIC("Unexpected end of file.");
			clear();
			return false;
		}
		
		using namespace code::bufferutils;
		u8 type = be_get<u8>(p_bufferOUT, p_sizeOUT);

		if(type == PositionAnimation2D::AnimationType_GameTranslation)
		{
			TranslationAnimation2DPtr translation = 
				TranslationAnimation2DPtr(new TranslationAnimation2D(true));
			
			(*it) = translation;
			m_gameTranslations.push_back(translation);
		}
		else
		{
			(*it) = AnimationFactory2D::create(type);
		}

		if ((*it) == 0)
		{
			TT_PANIC("Unknown/unsupported animation type '%d' encountered.", type);
			return false;
		}
		
		// HACK: For TranslationAnimation2D, pass the "invert Y" setting along
		// FIXME: Do we even still need this "invert Y" behavior? Can't we just kill it?
		if (type == PositionAnimation2D::AnimationType_Translation ||
		    type == PositionAnimation2D::AnimationType_GameTranslation)
		{
			tt_ptr_static_cast<TranslationAnimation2D>(*it)->setInvertYDuringLoad(p_invertTranslationAnimationY);
		}
		
		if ((*it)->load(p_bufferOUT, p_sizeOUT, p_applyTags, p_acceptedTags) == false)
		{
			clear();
			return false;
		}
	}
	if (m_initiallySuspended == false)
	{
		start();
	}
	return true;
}


bool AnimationStack2D::save(u8*& p_bufferOUT, size_t& p_sizeOUT, code::ErrorStatus* p_errStatus ) const
{
	TT_ERR_CHAIN(bool, false, "Saving Animation Stack");

	if (p_sizeOUT < getBufferSize())
	{
		TT_ERR_AND_RETURN("Buffer too small, got " << p_sizeOUT << " bytes, needs " << getBufferSize());
	}
	
	using namespace code::bufferutils;
	be_put(s_versionAnimationStack2D, p_bufferOUT, p_sizeOUT);
	
	be_put(m_autoSort,                            p_bufferOUT, p_sizeOUT);
	be_put(m_initiallySuspended,                  p_bufferOUT, p_sizeOUT);
	be_put(static_cast<u16>(m_animations.size()), p_bufferOUT, p_sizeOUT);
	for (Stack::const_iterator it = m_animations.begin(); it != m_animations.end(); ++it)
	{
		be_put(static_cast<u8>((*it)->getAnimationType()), p_bufferOUT, p_sizeOUT);
		if ((*it)->save(p_bufferOUT, p_sizeOUT) == false)
		{
			return false;
		}
	}
	return true;
}


size_t AnimationStack2D::getBufferSize() const
{
	size_t size = s_sizeAnimationStack2D;
	for (Stack::const_iterator it = m_animations.begin(); it != m_animations.end(); ++it)
	{
		size += 1 + (*it)->getBufferSize(); // 1 byte needed for animation type
	}
	return size;
}


void AnimationStack2D::setGameTranslationBeginAndEnd( const math::Vector3& p_begin, const math::Vector3& p_end )
{
	for(Translations::iterator it(m_gameTranslations.begin()) ; it != m_gameTranslations.end() ; ++it)
	{
		(*it)->setBeginAndEnd(p_begin, p_end);
	}
}


void AnimationStack2D::appendStack( const AnimationStack2DPtr& p_other )
{
	// copy over the animations
	m_animations.insert(m_animations.end(), p_other->m_animations.begin(), p_other->m_animations.end());

	// clear the other list so they don't get deleted
	p_other->m_animations.clear();

	// add the gametranslations
	m_gameTranslations.insert(m_gameTranslations.end(), 
	                          p_other->m_gameTranslations.begin(), 
	                          p_other->m_gameTranslations.end());

	// sort the animations
	std::sort(m_animations.begin(), m_animations.end(), AnimationPredicate2D<PositionAnimation2DPtr>());
}


tt::engine::anim2d::AnimationStack2DPtr AnimationStack2D::clone() const
{
	return AnimationStack2DPtr(new AnimationStack2D(*this));
}


void AnimationStack2D::makeDefault()
{
	clear();
	m_autoSort = true;
	m_initiallySuspended = true;
	
	// default gameTranslation with a duration of 2 seconds
	TranslationAnimation2DPtr translation = 
		TranslationAnimation2DPtr(new TranslationAnimation2D(true));
	
	translation->setDuration(2);
	
	m_animations.push_back(translation);
	m_gameTranslations.push_back(translation);
}



void AnimationStack2D::setAnimationDirectionType(Animation2D::DirectionType p_type)
{
	for (Stack::iterator it = m_animations.begin(); it != m_animations.end(); ++it)
	{
		(*it)->setDirection(p_type);
	}
}


void AnimationStack2D::setAnimationTimeType(Animation2D::TimeType p_type)
{
	for (Stack::iterator it = m_animations.begin(); it != m_animations.end(); ++it)
	{
		(*it)->setTimeType(p_type);
	}
}


void AnimationStack2D::setAnimationTweenType(TweenType p_type)
{
	for (Stack::iterator it = m_animations.begin(); it != m_animations.end(); ++it)
	{
		(*it)->setTweenType(p_type);
	}
}


//--------------------------------------------------------------------------------------------------
// Private member functions

bool AnimationStack2D::loadHeader(const u8*& p_bufferOUT, size_t& p_sizeOUT)
{
	clear();
	
	if (p_sizeOUT < getBufferSize())
	{
		TT_PANIC("Buffer too small, got %d bytes, needs %d\n", p_sizeOUT, getBufferSize());
		return false;
	}
	
	using namespace code::bufferutils;
	
	u16 version = be_get<u16>(p_bufferOUT, p_sizeOUT);
	if (version != s_versionAnimationStack2D)
	{
		TT_PANIC("Invalid version, code %d.%d, data %d.%d",
		         GET_MAJOR_VERSION(s_versionAnimationStack2D), GET_MINOR_VERSION(s_versionAnimationStack2D),
		         GET_MAJOR_VERSION(version), GET_MINOR_VERSION(version));
		return false;
	}
	
	m_autoSort           = be_get<bool>(p_bufferOUT, p_sizeOUT);
	m_initiallySuspended = be_get<bool>(p_bufferOUT, p_sizeOUT);
	u16 animationCount   = be_get<u16 >(p_bufferOUT, p_sizeOUT);
	
	m_animations.resize(static_cast<Stack::size_type>(animationCount), PositionAnimation2DPtr());
	return true;
}


AnimationStack2D::AnimationStack2D(const AnimationStack2D& p_rhs)
:
StackBase<PositionAnimation2DPtr>(p_rhs),
m_autoSort(p_rhs.m_autoSort),
m_initiallySuspended(p_rhs.m_initiallySuspended),
m_gameTranslations()
{
	for (Stack::const_iterator it = m_animations.begin(); it != m_animations.end(); ++it)
	{
		if (std::find(p_rhs.m_gameTranslations.begin(), p_rhs.m_gameTranslations.end(), (*it)) !=
		              p_rhs.m_gameTranslations.end())
		{
#ifndef TT_BUILD_DEV
			m_gameTranslations.push_back(tt_ptr_static_cast<TranslationAnimation2D>(*it));
#else
			TranslationAnimation2DPtr t(tt_ptr_dynamic_cast<TranslationAnimation2D>(*it));
			TT_NULL_ASSERT(t);
			m_gameTranslations.push_back(t);
#endif
		}
	}
}

//namespace end
}
}
}
