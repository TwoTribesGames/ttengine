#include <tt/code/bufferutils.h>
#include <tt/code/ErrorStatus.h>
#include <tt/engine/anim2d/RotationAnimation2D.h>
#include <tt/fs/File.h>
#include <tt/str/parse.h>
#include <tt/str/toStr.h>
#include <tt/xml/XmlNode.h>


namespace tt {
namespace engine {
namespace anim2d {

#define MAKE_VERSION(major, minor) (((major) << 8) | (minor))
#define GET_MAJOR_VERSION(version) ((version) >> 8)
#define GET_MINOR_VERSION(version) ((version) & 0xFF)

static const u16 s_versionRotationAnimation2D = MAKE_VERSION(1, 3);
static const size_t s_sizeRotationAnimation2D = 2 + 8 + 8; // version, begin rotation, end rotation


RotationAnimation2D::RotationAnimation2D()
:
PositionAnimation2D(),
m_begin(0.0f),
m_end(0.0f),
m_delta(0.0f),
m_beginRange(),
m_endRange()
{
}


void RotationAnimation2D::applyTransform(Transform* p_transform) const
{
	p_transform->rotation += m_begin + (m_delta * getTime());
}


bool RotationAnimation2D::hasZAnimation() const
{
	return false;
}


int RotationAnimation2D::getSortWeight() const
{
	return 2;
}


void RotationAnimation2D::setBeginAndEndRange(const math::Range& p_begin, const math::Range& p_end)
{
	m_beginRange = p_begin;
	m_endRange = p_end;
	setRanges();
}


void RotationAnimation2D::setBeginAndEnd(real p_begin, real p_end)
{
	m_beginRange.setMinMax(p_begin);
	m_endRange.setMinMax(p_end);
	setRanges();
}


bool RotationAnimation2D::load(const xml::XmlNode* p_node)
{
	Tags empty;
	return load(p_node, empty, empty);
}


bool RotationAnimation2D::load( const xml::XmlNode* p_node, const Tags& p_applyTags, const Tags& p_acceptedTags )
{
	TT_NULL_ASSERT(p_node);
	
	TT_ERR_CREATE("load");
	
	if (p_node->getFirstChild("begin") == 0)
	{
		TT_PANIC("Expected child 'begin' in node '%s'",
		         p_node->getName().c_str());
		return false;
	}
	std::string beginValue = p_node->getFirstChild("begin")->getAttribute("rotation");
	if (beginValue.empty())
	{
		TT_PANIC("Expected attribute 'rotation' in node 'begin' in node '%s'",
		         p_node->getName().c_str());
		return false;
	}
	
	math::Range begin = str::parseRange(beginValue, &errStatus);
	if (errStatus.hasError())
	{
		TT_PANIC("Value '%s' of attribute 'rotation' from node 'begin' from node '%s' can't be converted to a Range.",
		         beginValue.c_str(), 
		         p_node->getName().c_str());
		return false;
	}
	
	if (p_node->getFirstChild("end") == 0)
	{
		TT_PANIC("Expected child 'end' in node '%s'",
		         p_node->getName().c_str());
		return false;
	}
	std::string endValue = p_node->getFirstChild("end")->getAttribute("rotation");
	if (endValue.empty())
	{
		TT_PANIC("Expected attribute 'rotation' in node 'end' in node '%s'",
		         p_node->getName().c_str());
		return false;
	}
	
	math::Range end = str::parseRange(endValue, &errStatus);
	if (errStatus.hasError())
	{
		TT_PANIC("Value '%s' of attribute 'rotation' from node 'end' from node '%s' can't be converted to a Range.",
		         endValue.c_str(), 
		         p_node->getName().c_str());
		return false;
	}
	
	if (Animation2D::load(p_node, p_applyTags, p_acceptedTags) == false)
	{
		return false;
	}
	
	begin.setMinMax(math::degToRad(begin.getMin()), math::degToRad(begin.getMax()));
	end.setMinMax(math::degToRad(end.getMin()), math::degToRad(end.getMax()));
	
	setBeginAndEndRange(begin, end);
	
	return true;
}


bool RotationAnimation2D::save(xml::XmlNode* p_node) const
{
	TT_NULL_ASSERT(p_node);
	
	if (Animation2D::save(p_node) == false)
	{
		return false;
	}
	
	xml::XmlNode* begin = new xml::XmlNode("begin");
	math::Range begRng(math::radToDeg(m_beginRange.getMin()), math::radToDeg(m_beginRange.getMax()));
	begin->setAttribute("rotation", str::toStr(begRng));
	p_node->addChild(begin);
	
	xml::XmlNode* end = new xml::XmlNode("end");
	math::Range endRng(math::radToDeg(m_endRange.getMin()), math::radToDeg(m_endRange.getMax()));
	end->setAttribute("rotation", str::toStr(endRng));
	p_node->addChild(end);
	
	return true;
}


bool RotationAnimation2D::load(const fs::FilePtr& p_file)
{
	size_t size = getBufferSize();
	u8* buffer = new u8[size];
	if (p_file->read(buffer, static_cast<fs::size_type>(size)) != static_cast<fs::size_type>(size))
	{
		TT_PANIC("File '%s' too small.", p_file->getPath());
		delete[] buffer;
		return false;
	}
	const u8* scratch = buffer;
	
	bool ret = load(scratch, size);
	delete[] buffer;
	
	return ret;
}


bool RotationAnimation2D::save(const fs::FilePtr& p_file) const
{
	size_t bufsize = getBufferSize();
	u8* buffer = new u8[bufsize];
	u8* scratch = buffer;
	size_t size = bufsize;
	if (save(scratch, size) == false)
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


bool RotationAnimation2D::load(const u8*& p_bufferOUT, size_t& p_sizeOUT)
{
	Tags empty;
	return load(p_bufferOUT, p_sizeOUT, empty, empty);
}


bool RotationAnimation2D::load( const u8*& p_bufferOUT, size_t& p_sizeOUT, 
                                const Tags& p_applyTags, const Tags& p_acceptedTags )
{
	if (p_sizeOUT < getBufferSize())
	{
		TT_PANIC("Buffer too small, got %d bytes, needs %d\n", p_sizeOUT, getBufferSize());
		return false;
	}
	
	using namespace code::bufferutils;
	
	u16 version = be_get<u16>(p_bufferOUT, p_sizeOUT);
	if (version != s_versionRotationAnimation2D)
	{
		TT_PANIC("Invalid version, code %d.%d, data %d.%d",
		         GET_MAJOR_VERSION(s_versionRotationAnimation2D), GET_MINOR_VERSION(s_versionRotationAnimation2D),
		         GET_MAJOR_VERSION(version), GET_MINOR_VERSION(version));
		return false;
	}
	
	real min = be_get<real>(p_bufferOUT, p_sizeOUT);
	real max = be_get<real>(p_bufferOUT, p_sizeOUT);
	math::Range begin(min, max);
	
	min = be_get<real>(p_bufferOUT, p_sizeOUT);
	max = be_get<real>(p_bufferOUT, p_sizeOUT);
	math::Range end(min, max);
	
	setBeginAndEndRange(begin, end);
	
	return Animation2D::load(p_bufferOUT, p_sizeOUT, p_applyTags, p_acceptedTags);
}


bool RotationAnimation2D::save(u8*& p_bufferOUT, size_t& p_sizeOUT) const
{
	if (p_sizeOUT < getBufferSize())
	{
		TT_PANIC("Buffer too small, got %d bytes, needs %d\n", p_sizeOUT, getBufferSize());
		return false;
	}
	
	using namespace code::bufferutils;
	be_put(s_versionRotationAnimation2D, p_bufferOUT, p_sizeOUT);
	
	be_put(m_beginRange.getMin(), p_bufferOUT, p_sizeOUT);
	be_put(m_beginRange.getMax(), p_bufferOUT, p_sizeOUT);
	be_put(m_endRange.getMin(), p_bufferOUT, p_sizeOUT);
	be_put(m_endRange.getMax(), p_bufferOUT, p_sizeOUT);
	
	return Animation2D::save(p_bufferOUT, p_sizeOUT);
}


size_t RotationAnimation2D::getBufferSize() const
{
	return s_sizeRotationAnimation2D + Animation2D::getBufferSize();
}


void RotationAnimation2D::setRanges()
{
	Animation2D::setRanges();
	m_begin = m_beginRange.getRandom(tt::math::Random::getEffects());
	m_end = m_endRange.getRandom    (tt::math::Random::getEffects());
	m_delta = m_end - m_begin;
}


RotationAnimation2D::RotationAnimation2D(const RotationAnimation2D& p_rhs)
:
PositionAnimation2D(p_rhs),
m_begin(p_rhs.m_begin),
m_end(p_rhs.m_end),
m_delta(p_rhs.m_delta),
m_beginRange(p_rhs.m_beginRange),
m_endRange(p_rhs.m_endRange)
{
}

//namespace end
}
}
}
