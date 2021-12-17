#include <algorithm>

#include <tt/code/bufferutils.h>
#include <tt/code/ErrorStatus.h>
#include <tt/code/helpers.h>
#include <tt/pres/anim2d/ColorAnimationStack2D.h>
#include <tt/pres/PresentationObject.h>
#include <tt/fs/File.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>
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

static const u16 s_version = MAKE_VERSION(1, 1);
static const size_t s_size = 2 + 2; // version, initiallysuspended, number of animations


ColorAnimationStack2D::ColorAnimationStack2D()
:
StackBase<ColorAnimation2D>()
{
	
}


void ColorAnimationStack2D::push_back(const ColorAnimation2DPtr& p_animation)
{
	TT_ASSERTMSG(p_animation != 0, "No animation specified.");
	m_allAnimations.push_back(p_animation);
	TT_ASSERT(m_activeAnimations.empty());
}


engine::renderer::ColorRGBA ColorAnimationStack2D::getColor(const engine::renderer::ColorRGBA& p_source) const
{
	if (activeEmpty())
	{
		return p_source;
	}
	
	math::Vector4 v(0, 0, 0, 0);
	for (Stack::const_iterator it = m_activeAnimations.begin(); it != m_activeAnimations.end(); ++it)
	{
		TT_ASSERT((*it)->hasNameOrTagMatch());
		v += (*it)->getColor();
	}
	
	v *= 255.0f;
	v.x += p_source.r;
	v.y += p_source.g;
	v.z += p_source.b;
	v.w += p_source.a;
	
	math::clamp(v.x, 0.0f, 255.0f);
	math::clamp(v.y, 0.0f, 255.0f);
	math::clamp(v.z, 0.0f, 255.0f);
	math::clamp(v.w, 0.0f, 255.0f);
	
	return engine::renderer::ColorRGBA(static_cast<u8>(v.x), static_cast<u8>(v.y), static_cast<u8>(v.z), static_cast<u8>(v.w));
}


bool ColorAnimationStack2D::load(const xml::XmlNode* p_node, 
                                 const DataTags& p_applyTags, 
                                 const Tags& p_acceptedTags, code::ErrorStatus* p_errStatus)
{
	TT_ERR_CHAIN(bool, false, "ColorAnimationStack2D load from xml");
	TT_ERR_NULL_ASSERT(p_node);
	
	clear();
	
	
	DataTags stackTags;
	
	stackTags.load(p_node, p_applyTags, p_acceptedTags, &errStatus);
	
	TT_ERR_RETURN_ON_ERROR();
	
	for (const xml::XmlNode* child = p_node->getChild(); child != 0; child = child->getSibling())
	{
		if(child->getName() == "color")
		{
			ColorAnimation2DPtr anim(new ColorAnimation2D);
			if (anim->load(child, stackTags, p_acceptedTags, &errStatus) == false)
			{
				clear();
				return false;
			}
			push_back(anim);
		}
	}
	
	return true;
}


bool ColorAnimationStack2D::load( const u8*& p_bufferOUT, size_t& p_sizeOUT, 
                                  const DataTags& p_applyTags, const Tags& p_acceptedTags, 
                                  code::ErrorStatus* p_errStatus )
{
	TT_ERR_CHAIN(bool, false, "Saving ColorAnimationStack2D Stack");
	
	loadHeader(p_bufferOUT, p_sizeOUT, &errStatus);
	TT_ERR_RETURN_ON_ERROR();
	
	for (Stack::iterator it = m_allAnimations.begin(); it != m_allAnimations.end(); ++it)
	{
		using namespace code::bufferutils;
		(*it).reset(new ColorAnimation2D);
		if ((*it)->load(p_bufferOUT, p_sizeOUT, p_applyTags, p_acceptedTags, &errStatus) == false)
		{
			clear();
			return false;
		}
	}
	
	return true;
}


bool ColorAnimationStack2D::save(u8*& p_bufferOUT, size_t& p_sizeOUT, code::ErrorStatus* p_errStatus ) const
{
	TT_ERR_CHAIN(bool, false, "Saving ColorAnimation Stack");
	TT_ERR_ASSERTMSG(p_sizeOUT >= getBufferSize(), "Buffer too small, got " << p_sizeOUT << 
	                 " bytes, needs " << getBufferSize() << "\n");
	
	using namespace code::bufferutils;
	be_put(s_version, p_bufferOUT, p_sizeOUT);
	
	be_put(static_cast<u16>(m_allAnimations.size()), p_bufferOUT, p_sizeOUT);
	for (Stack::const_iterator it = m_allAnimations.begin(); it != m_allAnimations.end(); ++it)
	{
		if ((*it)->save(p_bufferOUT, p_sizeOUT, &errStatus) == false)
		{
			return false;
		}
	}
	return true;
}


size_t ColorAnimationStack2D::getBufferSize() const
{
	size_t size = s_size;
	for (Stack::const_iterator it = m_allAnimations.begin(); it != m_allAnimations.end(); ++it)
	{
		size += (*it)->getBufferSize();
	}
	return size;
}


void ColorAnimationStack2D::appendStack(ColorAnimationStack2D& p_other)
{
	m_allAnimations.insert(m_allAnimations.end(), p_other.m_allAnimations.begin(), p_other.m_allAnimations.end());
	TT_ASSERT(m_activeAnimations.empty());
	
	p_other.m_allAnimations.clear();
	p_other.m_activeAnimations.clear();
}


ColorAnimationStack2D::ColorAnimationStack2D(const ColorAnimationStack2D& p_rhs)
:
StackBase<ColorAnimation2D>(p_rhs)
{
}


//-------------------------------------------------------------------------------------------------
// Private functions


bool ColorAnimationStack2D::loadHeader(const u8*& p_bufferOUT, size_t& p_sizeOUT,
                                       code::ErrorStatus* p_errStatus)
{
	TT_ERR_CHAIN(bool, false, "Loading AnimationStack2D header");
	
	clear();
	
	using namespace code::bufferutils;
	
	const u16 version = be_get<u16>(p_bufferOUT, p_sizeOUT);
	TT_ERR_ASSERTMSG(version == s_version, "Invalid ColorAnimationStack2D version: code "
			<< GET_MAJOR_VERSION(s_version) << "." <<GET_MINOR_VERSION(s_version) << ", data "
			<< GET_MAJOR_VERSION(version)   << "." <<GET_MINOR_VERSION(version)   <<
			" -- please update your presentation converter.");
	
	TT_ERR_ASSERTMSG(p_sizeOUT >= sizeof(u16), "Buffer too small.");
	const u16 animationCount   = be_get<u16>(p_bufferOUT, p_sizeOUT);
	
	m_allAnimations.resize(static_cast<Stack::size_type>(animationCount), ColorAnimation2DPtr());
	return true;
}


//namespace end
}
}
}
