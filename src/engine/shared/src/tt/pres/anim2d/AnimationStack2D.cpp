#include <algorithm>

#include <tt/code/bufferutils.h>
#include <tt/code/ErrorStatus.h>
#include <tt/code/helpers.h>
#include <tt/pres/anim2d/AnimationFactory2D.h>
#include <tt/pres/anim2d/AnimationStack2D.h>
#include <tt/pres/anim2d/TranslationAnimation2D.h>
#include <tt/pres/PresentationObject.h>
#include <tt/math/Vector3.h>
#include <tt/platform/tt_error.h>
#include <tt/str/parse.h>
#include <tt/str/toStr.h>
#include <tt/xml/XmlNode.h>
#include <tt/xml/util/parse.h>


namespace tt {
namespace pres {
namespace anim2d {

#define MAKE_VERSION(major, minor) (((major) << 8) | (minor))
#define GET_MAJOR_VERSION(version) ((version) >> 8)
#define GET_MINOR_VERSION(version) ((version) & 0xFF)

static const u16 s_version = MAKE_VERSION(1, 3);
static const size_t s_size = 2 + 2 + 2; // version, animation count, origin translation count


//--------------------------------------------------------------------------------------------------
// Public member functions

AnimationStack2D::AnimationStack2D()
:
StackBase<PositionAnimation2D>(),
m_gameTranslations(),
m_originTranslations(),
m_originOffset(),
m_originOffsetInv(),
m_originOffsetIsIdentity(true),
m_customTransformUsed(false),
m_customTransform()
{
}


void AnimationStack2D::push_back(const PositionAnimation2DPtr& p_animation)
{
	TT_ASSERTMSG(p_animation != 0, "No animation specified.");
	m_allAnimations.push_back(p_animation);
	TT_ASSERT(m_activeAnimations.empty());
	
	sortAll();
}


void AnimationStack2D::clear()
{
	StackBase<PositionAnimation2D>::clear();
	code::helpers::freeContainer(m_gameTranslations);
	code::helpers::freeContainer(m_originTranslations);
	m_originOffset.setIdentity();
	m_originOffsetInv.setIdentity();
	m_originOffsetIsIdentity = true;
	resetCustomTransform();
}


void AnimationStack2D::updateTransform(math::Matrix44* p_mtx) const
{
	TT_NULL_ASSERT(p_mtx);
	
	if (activeEmpty() && m_customTransformUsed == false)
	{
		p_mtx->setIdentity();
		return;
	}
	
	Transform transform(m_customTransform);
	
	for (Stack::const_iterator it = m_activeAnimations.begin(); it != m_activeAnimations.end(); ++it)
	{
		TT_ASSERT((*it)->hasNameOrTagMatch());
		
		(*it)->applyTransform(&transform);
	}

	(*p_mtx) = transform.calcMatrix();
	if (m_originOffsetIsIdentity == false)
	{
		(*p_mtx) = m_originOffsetInv * (*p_mtx) * m_originOffset;
	}
}


bool AnimationStack2D::hasZAnimation() const
{
	for (Stack::const_iterator it = m_activeAnimations.begin(); it != m_activeAnimations.end(); ++it)
	{
		TT_ASSERT((*it)->hasNameOrTagMatch());
		if ((*it)->hasZAnimation())
		{
			return true;
		}
	}
	return false;
}


bool AnimationStack2D::hasRotationAnimation() const
{
	for (Stack::const_iterator it = m_activeAnimations.begin(); it != m_activeAnimations.end(); ++it)
	{
		TT_ASSERT((*it)->hasNameOrTagMatch());
		if ((*it)->hasRotationAnimation())
		{
			return true;
		}
	}
	return false;
}


void AnimationStack2D::sortAll()
{
	std::sort(m_allAnimations.begin(), m_allAnimations.end(), AnimationPredicate2D<PositionAnimation2DPtr>());
}


void AnimationStack2D::start(const Tags& p_tags, PresentationObject* p_presObj, const std::string& p_name)
{
	Transform transform;
	
	for(Translations::const_iterator it = m_originTranslations.begin();
		it != m_originTranslations.end(); ++it)
	{
		TranslationAnimation2DPtr ptr = (*it);
		ptr->start(p_tags, p_presObj, p_name);
		if (ptr->hasNameOrTagMatch())
		{
			(*it)->applyTransform(&transform);
		}
	}
	m_originOffset           = transform.calcMatrix();
	m_originOffsetInv        = m_originOffset.getInverse();
	m_originOffsetIsIdentity = m_originOffset.isIdentity();
	
	StackBase<PositionAnimation2D>::start(p_tags, p_presObj, p_name);
}


void AnimationStack2D::stop()
{
	StackBase<PositionAnimation2D>::stop();
}


void AnimationStack2D::pause()
{
	StackBase<PositionAnimation2D>::pause();
}


void AnimationStack2D::resume()
{
	StackBase<PositionAnimation2D>::resume();
}


void AnimationStack2D::reset()
{
	StackBase<PositionAnimation2D>::reset();
}


bool AnimationStack2D::load(const xml::XmlNode* p_node, 
                            const DataTags& p_applyTags, 
                            const Tags& p_acceptedTags,
                            code::ErrorStatus* p_errStatus )
{
	TT_ERR_CHAIN(bool, false, "loading AnimationStack2D xml ");
	TT_ERR_NULL_ASSERT(p_node);
	
	clear();
	
	DataTags stackTags;
	
	stackTags.load(p_node, p_applyTags, p_acceptedTags, &errStatus);
	
	TT_ERR_RETURN_ON_ERROR();
	
	for (const xml::XmlNode* child = p_node->getChild(); child != 0; child = child->getSibling())
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
			continue;
		}
		
		if (anim->load(child, stackTags, p_acceptedTags, &errStatus) == false)
		{
			clear();
			return false;
		}
		push_back(anim);
		
	}
	return true;
}


bool AnimationStack2D::loadStartValues(const xml::XmlNode* p_node, 
                                       const DataTags& p_applyTags, 
                                       const Tags& p_acceptedTags,
                                       code::ErrorStatus* p_errStatus )
{
	TT_ERR_CHAIN(bool, false, "loading AnimationStack2D xml ");
	TT_ERR_NULL_ASSERT(p_node);
	
	clear();
	
	DataTags stackTags;
	
	stackTags.load(p_node, p_applyTags, p_acceptedTags, &errStatus);
	
	TT_ERR_RETURN_ON_ERROR();
	
	for (const xml::XmlNode* child = p_node->getChild(); child != 0; child = child->getSibling())
	{
		PositionAnimation2DPtr anim;
		if (child->getName() == "origin")
		{
			TranslationAnimation2DPtr translation = 
				TranslationAnimation2DPtr(new TranslationAnimation2D());
			
			anim = translation;
			m_originTranslations.push_back(translation);
		}
		else
		{
			anim = AnimationFactory2D::create(child->getName());
		}
		
		if (anim == 0)
		{
			continue;
		}
		
		if (anim->loadStatic(child, stackTags, p_acceptedTags, &errStatus) == false)
		{
			clear();
			return false;
		}
		
		if (child->getName() != "origin")
		{
			push_back(anim);
		}
	}
	return true;
}


bool AnimationStack2D::load( const u8*& p_bufferOUT, size_t& p_sizeOUT, const DataTags& p_applyTags, 
                             const Tags& p_acceptedTags,
                             code::ErrorStatus* p_errStatus)
{
	TT_ERR_CHAIN(bool, false, "loading AnimationStack2D binary ");
	
	loadHeader(p_bufferOUT, p_sizeOUT, &errStatus);
	TT_ERR_RETURN_ON_ERROR();
	
	for (Stack::iterator it = m_allAnimations.begin(); it != m_allAnimations.end(); ++it)
	{
		TT_ERR_ASSERTMSG(p_sizeOUT >= 1, "Unexpected end of file.");
		
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
		
		TT_ERR_ASSERTMSG((*it) != 0, "Unknown/unsupported animation type '" << type << "' encountered.");
		
		if ((*it)->load(p_bufferOUT, p_sizeOUT, p_applyTags, p_acceptedTags, &errStatus) == false)
		{
			clear();
			return false;
		}
	}
	
	// If these fail make them identity, but I think we don't need to do that. The asserts are to make sure.
	TT_ASSERT(m_originOffset.isIdentity());
	TT_ASSERT(m_originOffsetInv.isIdentity());
	TT_ASSERT(m_originOffsetIsIdentity);
	
	for (Translations::iterator it = m_originTranslations.begin(); it != m_originTranslations.end(); ++it)
	{
		TT_ERR_ASSERTMSG(p_sizeOUT >= 1, "Unexpected end of file.");
		
		using namespace code::bufferutils;
		
		(*it) = TranslationAnimation2DPtr(new TranslationAnimation2D());
		
		if ((*it)->load(p_bufferOUT, p_sizeOUT, p_applyTags, p_acceptedTags, &errStatus) == false)
		{
			clear();
			return false;
		}
	}
	return true;
}


bool AnimationStack2D::save(u8*& p_bufferOUT, size_t& p_sizeOUT, code::ErrorStatus* p_errStatus ) const
{
	TT_ERR_CHAIN(bool, false, "saving AnimationStack2D binary");
	TT_ERR_ASSERTMSG(p_sizeOUT >= getBufferSize(), "Buffer too small, got " << p_sizeOUT << 
	                 " bytes, needs " << getBufferSize() << "\n");
	
	using namespace code::bufferutils;
	be_put(s_version, p_bufferOUT, p_sizeOUT);
	
	be_put(static_cast<u16>(m_allAnimations.size()), p_bufferOUT, p_sizeOUT);
	be_put(static_cast<u16>(m_originTranslations.size()), p_bufferOUT, p_sizeOUT);
	
	
	for (Stack::const_iterator it = m_allAnimations.begin(); it != m_allAnimations.end(); ++it)
	{
		be_put(static_cast<u8>((*it)->getAnimationType()), p_bufferOUT, p_sizeOUT);
		if ((*it)->save(p_bufferOUT, p_sizeOUT, &errStatus) == false)
		{
			return false;
		}
	}
	
	for (Translations::const_iterator it = m_originTranslations.begin();
	     it != m_originTranslations.end(); ++it)
	{
		if ((*it)->save(p_bufferOUT, p_sizeOUT, &errStatus) == false)
		{
			return false;
		}
	}
	return true;
}


size_t AnimationStack2D::getBufferSize() const
{
	size_t size = s_size;
	for (Stack::const_iterator it = m_allAnimations.begin(); it != m_allAnimations.end(); ++it)
	{
		size += 1 + (*it)->getBufferSize(); // 1 byte needed for animation type
	}
	for (Translations::const_iterator it = m_originTranslations.begin(); 
	     it != m_originTranslations.end(); ++it)
	{
		size += (*it)->getBufferSize();
	}
	return size;
}


DataTags AnimationStack2D::getTags() const
{
	DataTags tags(StackBase<PositionAnimation2D>::getTags());
	
	for(Translations::const_iterator it = m_originTranslations.begin();
		it != m_originTranslations.end(); ++it)
	{
		tags.addDataTags((*it)->getTags());
	}
	
	return tags;
}


void AnimationStack2D::setGameTranslationBeginAndEnd( const math::Vector3& p_begin, const math::Vector3& p_end )
{
	for(Translations::iterator it(m_gameTranslations.begin()) ; it != m_gameTranslations.end() ; ++it)
	{
		(*it)->setBeginAndEnd(p_begin, p_end);
	}
}


void AnimationStack2D::appendStack( AnimationStack2D& p_other )
{
	// copy over the animations
	m_allAnimations.insert(m_allAnimations.end(), p_other.m_allAnimations.begin(), p_other.m_allAnimations.end());
	TT_ASSERT(m_activeAnimations.empty());
	m_originTranslations.insert(m_originTranslations.end(), p_other.m_originTranslations.begin(), p_other.m_originTranslations.end());
	
	// clear the other list so they don't get deleted
	p_other.m_allAnimations.clear();
	p_other.m_activeAnimations.clear();
	p_other.m_originTranslations.clear();
	
	// If these fail make them identity, but I think we don't need to do that. The asserts are to make sure.
	TT_ASSERT(m_originOffset.isIdentity());
	TT_ASSERT(m_originOffsetInv.isIdentity());
	TT_ASSERT(m_originOffsetIsIdentity);
	TT_ASSERT(p_other.m_originOffset.isIdentity());
	TT_ASSERT(p_other.m_originOffsetInv.isIdentity());
	TT_ASSERT(p_other.m_originOffsetIsIdentity);
	
	// add the gametranslations
	m_gameTranslations.insert(m_gameTranslations.end(), 
	                          p_other.m_gameTranslations.begin(), 
	                          p_other.m_gameTranslations.end());
	
	// sort the animations
	sortAll();
}


void AnimationStack2D::makeDefault()
{
	clear();
	
	// default gameTranslation with a duration of 2 seconds
	TranslationAnimation2DPtr translation = 
		TranslationAnimation2DPtr(new TranslationAnimation2D(true));
	
	translation->setDuration(2);
	
	m_allAnimations.push_back(translation);
	TT_ASSERT(m_activeAnimations.empty());
	m_gameTranslations.push_back(translation);
}


AnimationStack2D::AnimationStack2D(const AnimationStack2D& p_rhs)
:
StackBase<PositionAnimation2D>(p_rhs, true),		// make sure we call the contructor with NO animation copy
m_gameTranslations()
{
	for(Stack::const_iterator it = p_rhs.m_allAnimations.begin();
		it != p_rhs.m_allAnimations.end(); ++it)
	{
		PositionAnimation2DPtr anim = PositionAnimation2DPtr((*it)->clone());
		m_allAnimations.push_back(anim);
		TT_ASSERT(m_activeAnimations.empty());
		
		if (std::find(p_rhs.m_gameTranslations.begin(), p_rhs.m_gameTranslations.end(), (*it)) !=
		              p_rhs.m_gameTranslations.end())
		{
#ifndef TT_BUILD_DEV
			m_gameTranslations.push_back(tt_ptr_static_cast<TranslationAnimation2D>(anim));
#else
			TranslationAnimation2DPtr t(tt_ptr_dynamic_cast<TranslationAnimation2D>(anim));
			TT_NULL_ASSERT(t);
			m_gameTranslations.push_back(t);
#endif
		}
	}
	
	for(Translations::const_iterator it = p_rhs.m_originTranslations.begin();
		it != p_rhs.m_originTranslations.end(); ++it)
	{
		TranslationAnimation2DPtr originTranslation = TranslationAnimation2DPtr((*it)->clone());
		m_originTranslations.push_back(originTranslation);
	}
	
	m_originOffset           = p_rhs.m_originOffset;
	m_originOffsetInv        = p_rhs.m_originOffsetInv;
	m_originOffsetIsIdentity = p_rhs.m_originOffsetIsIdentity;
	
	m_customTransformUsed = p_rhs.m_customTransformUsed;
	m_customTransform     = p_rhs.m_customTransform;
}


//--------------------------------------------------------------------------------------------------
// Private member functions

bool AnimationStack2D::loadHeader(const u8*& p_bufferOUT, size_t& p_sizeOUT, 
                                  code::ErrorStatus* p_errStatus)
{
	TT_ERR_CHAIN(bool, false, "Loading AnimationStack2D header");
	
	clear();
	
	using namespace code::bufferutils;
	
	u16 version = be_get<u16>(p_bufferOUT, p_sizeOUT);
	TT_ERR_ASSERTMSG(version == s_version, "Invalid AnimationStack2D version: code "
			<< GET_MAJOR_VERSION(s_version) << "." <<GET_MINOR_VERSION(s_version)<<", data "
			<< GET_MAJOR_VERSION(version)   << "." <<GET_MINOR_VERSION(version)  <<
			" -- please update your presentation converter.");
	
	u16 animationCount   = be_get<u16>(p_bufferOUT, p_sizeOUT);
	
	m_allAnimations.resize(static_cast<Stack::size_type>(animationCount), PositionAnimation2DPtr());
	
	TT_ERR_ASSERTMSG(p_sizeOUT >= sizeof(u16), "Buffer too small.");
	animationCount   = be_get<u16>(p_bufferOUT, p_sizeOUT);
	
	m_originTranslations.resize(static_cast<Stack::size_type>(animationCount), TranslationAnimation2DPtr());
	return true;
}


//namespace end
}
}
}
