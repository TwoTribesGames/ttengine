#include <algorithm>

#include <tt/code/bufferutils.h>
#include <tt/code/ErrorStatus.h>
#include <tt/code/helpers.h>
#include <tt/engine/anim2d/ColorAnimationStack2D.h>
#include <tt/fs/File.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>
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

static const u16 s_version = MAKE_VERSION(1, 0);
static const size_t s_size = 2 + 1 + 2; // version, initiallysuspended, number of animations


//--------------------------------------------------------------------------------------------------
// Public member functions

ColorAnimationStack2D::ColorAnimationStack2D()
:
StackBase<ColorAnimation2DPtr>(),
m_initiallySuspended(false)
{
	
}


void ColorAnimationStack2D::push_back(const ColorAnimation2DPtr& p_animation)
{
	TT_ASSERTMSG(p_animation != 0, "No animation specified.");
	m_animations.push_back(p_animation);
	
	if (m_initiallySuspended == false)
	{
		p_animation->start();
	}
}


renderer::ColorRGBA ColorAnimationStack2D::getColor(const renderer::ColorRGBA& p_source) const
{
	if (empty())
	{
		return p_source;
	}
	
	math::Vector4 v(0, 0, 0, 0);
	for (Stack::const_iterator it = m_animations.begin(); it != m_animations.end(); ++it)
	{
		if((*it)->isTagged())
		{
			v += (*it)->getColor();
		}
	}
	
	v *= 255.0f;
	v.x += p_source.r;
	v.y += p_source.g;
	v.z += p_source.b;
	v.w += p_source.a;
	
	tt::math::clamp(v.x, 0.0f, 255.0f);
	tt::math::clamp(v.y, 0.0f, 255.0f);
	tt::math::clamp(v.z, 0.0f, 255.0f);
	tt::math::clamp(v.w, 0.0f, 255.0f);
	
	return renderer::ColorRGBA(static_cast<u8>(v.x), static_cast<u8>(v.y), static_cast<u8>(v.z), static_cast<u8>(v.w));
}


void ColorAnimationStack2D::setInitiallySuspended(bool p_suspended)
{
	m_initiallySuspended = p_suspended;
}


bool ColorAnimationStack2D::load(const xml::XmlNode* p_node)
{
	Tags emptyTags;
	return load(p_node, emptyTags, emptyTags);
}


bool ColorAnimationStack2D::load(const xml::XmlNode* p_node, 
                                 const Tags& p_applyTags, 
                                 const Tags& p_acceptedTags)
{
	TT_NULL_ASSERT(p_node);
	
	TT_ERR_CREATE("load");
	
	clear();
	
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
			ColorAnimation2DPtr anim(new ColorAnimation2D);
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


bool ColorAnimationStack2D::save(xml::XmlNode* p_node) const
{
	TT_NULL_ASSERT(p_node);
	
	p_node->setAttribute("suspended", str::toStr(m_initiallySuspended));
	for (Stack::const_iterator it = m_animations.begin(); it != m_animations.end(); ++it)
	{
		xml::XmlNode* node = new xml::XmlNode("color");
		if ((*it)->save(node) == false)
		{
			delete node;
			return false;
		}
		p_node->addChild(node);
	}
	return true;
}


bool ColorAnimationStack2D::load(const fs::FilePtr& p_file)
{
	clear();
	
	size_t size = getBufferSize();
	u8* buffer = new u8[size];
	if (p_file->read(buffer, static_cast<fs::size_type>(size)) != static_cast<fs::size_type>(size))
	{
		TT_PANIC("File '%s' too small.", p_file->getPath());
		delete[] buffer;
		return false;
	}
	const u8* scratch = buffer;
	
	if (load(scratch, size) == false)
	{
		TT_PANIC("Failed to load header.");
		//delete[] buffer;
	}
	delete[] buffer;
	buffer  = 0;
	scratch = 0;
	
	for (Stack::iterator it = m_animations.begin(); it != m_animations.end(); ++it)
	{
		(*it).reset(new ColorAnimation2D);
		if ((*it)->load(p_file) == false)
		{
			clear();
			return false;
		}
	}
	return true;
}


bool ColorAnimationStack2D::save(const fs::FilePtr& p_file) const
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


bool ColorAnimationStack2D::load(const u8*& p_bufferOUT, size_t& p_sizeOUT)
{
	Tags emptyTags;
	return load(p_bufferOUT, p_sizeOUT, emptyTags, emptyTags);
}


bool ColorAnimationStack2D::load( const u8*& p_bufferOUT, size_t& p_sizeOUT, 
                                  const Tags& p_applyTags, const Tags& p_acceptedTags )
{
	if (loadHeader(p_bufferOUT, p_sizeOUT) == false)
	{
		TT_PANIC("Failed to load header.");
		return false;
	}
	
	for (Stack::iterator it = m_animations.begin(); it != m_animations.end(); ++it)
	{
		using namespace code::bufferutils;
		(*it).reset(new ColorAnimation2D);
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


bool ColorAnimationStack2D::save(u8*& p_bufferOUT, size_t& p_sizeOUT, code::ErrorStatus* p_errStatus ) const
{
	TT_ERR_CHAIN(bool, false, "Saving ColorAnimation Stack");
	
	if (p_sizeOUT < getBufferSize())
	{
		TT_ERR_AND_RETURN("Buffer too small, got " << p_sizeOUT << " bytes, needs " << getBufferSize());
	}
	
	using namespace code::bufferutils;
	be_put(s_version, p_bufferOUT, p_sizeOUT);
	
	be_put(static_cast<u8>(m_initiallySuspended ? 1 : 0), p_bufferOUT, p_sizeOUT);
	be_put(static_cast<u16>(m_animations.size()), p_bufferOUT, p_sizeOUT);
	for (Stack::const_iterator it = m_animations.begin(); it != m_animations.end(); ++it)
	{
		if ((*it)->save(p_bufferOUT, p_sizeOUT) == false)
		{
			return false;
		}
	}
	return true;
}


size_t ColorAnimationStack2D::getBufferSize() const
{
	size_t size = s_size;
	for (Stack::const_iterator it = m_animations.begin(); it != m_animations.end(); ++it)
	{
		size += (*it)->getBufferSize();
	}
	return size;
}


void ColorAnimationStack2D::appendStack( const ColorAnimationStack2DPtr& p_other )
{
	m_animations.insert(m_animations.end(), p_other->m_animations.begin(), p_other->m_animations.end());

	p_other->m_animations.clear();
}


ColorAnimationStack2DPtr ColorAnimationStack2D::clone() const
{
	return ColorAnimationStack2DPtr(new ColorAnimationStack2D(*this));
}


void ColorAnimationStack2D::setAnimationDirectionType(Animation2D::DirectionType p_type)
{
	for (Stack::iterator it = m_animations.begin(); it != m_animations.end(); ++it)
	{
		(*it)->setDirection(p_type);
	}
}


void ColorAnimationStack2D::setAnimationTimeType(Animation2D::TimeType p_type)
{
	for (Stack::iterator it = m_animations.begin(); it != m_animations.end(); ++it)
	{
		(*it)->setTimeType(p_type);
	}
}


void ColorAnimationStack2D::setAnimationTweenType(TweenType p_type)
{
	for (Stack::iterator it = m_animations.begin(); it != m_animations.end(); ++it)
	{
		(*it)->setTweenType(p_type);
	}
}


//--------------------------------------------------------------------------------------------------
// Private member functions

bool ColorAnimationStack2D::loadHeader(const u8*& p_bufferOUT, size_t& p_sizeOUT)
{
	clear();
	
	if (p_sizeOUT < getBufferSize())
	{
		TT_PANIC("Buffer too small, got %d bytes, needs %d\n", p_sizeOUT, getBufferSize());
		return false;
	}
	
	using namespace code::bufferutils;
	
	u16 version = be_get<u16>(p_bufferOUT, p_sizeOUT);
	if (version != s_version)
	{
		TT_PANIC("Invalid version, code %d.%d, data %d.%d",
		         GET_MAJOR_VERSION(s_version), GET_MINOR_VERSION(s_version),
		         GET_MAJOR_VERSION(version), GET_MINOR_VERSION(version));
		return false;
	}
	
	m_initiallySuspended = be_get<bool>(p_bufferOUT, p_sizeOUT);
	u16 animationCount   = be_get<u16 >(p_bufferOUT, p_sizeOUT);
	
	m_animations.resize(static_cast<Stack::size_type>(animationCount), ColorAnimation2DPtr());
	return true;
}


ColorAnimationStack2D::ColorAnimationStack2D(const ColorAnimationStack2D& p_rhs)
:
StackBase<ColorAnimation2DPtr>(p_rhs),
m_initiallySuspended(p_rhs.m_initiallySuspended)
{
}


//namespace end
}
}
}
