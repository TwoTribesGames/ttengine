#include <tt/code/bufferutils.h>
#include <tt/code/ErrorStatus.h>
#include <tt/engine/anim2d/ColorAnimation2D.h>
#include <tt/fs/File.h>
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

static const u16 s_versionColorAnimation2D = MAKE_VERSION(1, 2);
static const size_t s_sizeColorAnimation2D = 2 + 8 + 8 + 8 + 8 + 8 + 8 + 8 + 8; // version, begin r, begin g, begin b, begin a, end r, end g, end b, end a


ColorAnimation2D::ColorAnimation2D()
:
Animation2D(),
m_begin(0.0f, 0.0f, 0.0f, 0.0f),
m_end(0.0f, 0.0f, 0.0f, 0.0f),
m_delta(0.0f, 0.0f, 0.0f, 0.0f),
m_beginRRange(),
m_beginGRange(),
m_beginBRange(),
m_beginARange(),
m_endRRange(),
m_endGRange(),
m_endBRange(),
m_endARange()
{
}


ColorAnimation2D::~ColorAnimation2D()
{
}


int ColorAnimation2D::getSortWeight() const
{
	return 0;
}


math::Vector4 ColorAnimation2D::getColor() const
{
	return m_begin + (m_delta * getTime());
}


void ColorAnimation2D::setBeginAndEndRange(const math::Range& p_beginR, const math::Range& p_beginG, const math::Range& p_beginB, const math::Range& p_beginA,
	                                       const math::Range& p_endR,   const math::Range& p_endG,   const math::Range& p_endB,   const math::Range& p_endA)
{
	m_beginRRange = p_beginR;
	m_beginGRange = p_beginG;
	m_beginBRange = p_beginB;
	m_beginARange = p_beginA;
	m_endRRange = p_endR;
	m_endGRange = p_endG;
	m_endBRange = p_endB;
	m_endARange = p_endA;
	setRanges();
}


bool ColorAnimation2D::load(const xml::XmlNode* p_node)
{
	Tags empty;
	return load(p_node, empty, empty);
}


bool ColorAnimation2D::load( const xml::XmlNode* p_node, const Tags& p_applyTags, const Tags& p_acceptedTags )
{
	TT_NULL_ASSERT(p_node);
	TT_ERR_CREATE("ColorAnimation2D load from xml");
	
	const xml::XmlNode* begin = p_node->getFirstChild("begin");
	if (begin == 0)
	{
		TT_PANIC("Expected 'begin' node after '%s' node.", p_node->getName().c_str());
		return false;
	}
	
	math::Range beginR;
	math::Range beginG;
	math::Range beginB;
	math::Range beginA;
	
	if (begin->getAttribute("r").empty() == false) beginR = xml::util::parseRange(begin, "r", &errStatus);
	if (begin->getAttribute("g").empty() == false) beginG = xml::util::parseRange(begin, "g", &errStatus);
	if (begin->getAttribute("b").empty() == false) beginB = xml::util::parseRange(begin, "b", &errStatus);
	if (begin->getAttribute("a").empty() == false) beginA = xml::util::parseRange(begin, "a", &errStatus);
	
	const xml::XmlNode* end = p_node->getFirstChild("end");
	if (end == 0)
	{
		TT_PANIC("Expected 'end' node after '%s' node.", p_node->getName().c_str());
		return false;
	}
	
	math::Range endR;
	math::Range endG;
	math::Range endB;
	math::Range endA;
	
	if (end->getAttribute("r").empty() == false) endR = xml::util::parseRange(end, "r", &errStatus);
	if (end->getAttribute("g").empty() == false) endG = xml::util::parseRange(end, "g", &errStatus);
	if (end->getAttribute("b").empty() == false) endB = xml::util::parseRange(end, "b", &errStatus);
	if (end->getAttribute("a").empty() == false) endA = xml::util::parseRange(end, "a", &errStatus);
	
	if (errStatus.hasError())
	{
		TT_PANIC("%s", errStatus.getErrorMessage().c_str());
		return false;
	}
	
	if (Animation2D::load(p_node, p_applyTags, p_acceptedTags) == false)
	{
		return false;
	}
	
	beginR.setMinMax(beginR.getMin() / 255.0f, beginR.getMax() / 255.0f);
	beginG.setMinMax(beginG.getMin() / 255.0f, beginG.getMax() / 255.0f);
	beginB.setMinMax(beginB.getMin() / 255.0f, beginB.getMax() / 255.0f);
	beginA.setMinMax(beginA.getMin() / 255.0f, beginA.getMax() / 255.0f);
	endR.setMinMax(endR.getMin() / 255.0f, endR.getMax() / 255.0f);
	endG.setMinMax(endG.getMin() / 255.0f, endG.getMax() / 255.0f);
	endB.setMinMax(endB.getMin() / 255.0f, endB.getMax() / 255.0f);
	endA.setMinMax(endA.getMin() / 255.0f, endA.getMax() / 255.0f);
	
	setBeginAndEndRange(beginR, beginG, beginB, beginA, endR, endG, endB, endA);
	
	return true;
}


bool ColorAnimation2D::save(xml::XmlNode* p_node) const
{
	TT_NULL_ASSERT(p_node);
	
	if (Animation2D::save(p_node) == false)
	{
		return false;
	}
	
	
	math::Range beginR(m_beginRRange);
	math::Range beginG(m_beginGRange);
	math::Range beginB(m_beginBRange);
	math::Range beginA(m_beginARange);
	math::Range endR(m_endRRange);
	math::Range endG(m_endGRange);
	math::Range endB(m_endBRange);
	math::Range endA(m_endARange);
	
	beginR.setMinMax(beginR.getMin() * 255.0f, beginR.getMax() * 255.0f);
	beginG.setMinMax(beginG.getMin() * 255.0f, beginG.getMax() * 255.0f);
	beginB.setMinMax(beginB.getMin() * 255.0f, beginB.getMax() * 255.0f);
	beginA.setMinMax(beginA.getMin() * 255.0f, beginA.getMax() * 255.0f);
	endR.setMinMax(endR.getMin() * 255.0f, endR.getMax() * 255.0f);
	endG.setMinMax(endG.getMin() * 255.0f, endG.getMax() * 255.0f);
	endB.setMinMax(endB.getMin() * 255.0f, endB.getMax() * 255.0f);
	endA.setMinMax(endA.getMin() * 255.0f, endA.getMax() * 255.0f);
	
	xml::XmlNode* begin = new xml::XmlNode("begin");
	begin->setAttribute("r", str::toStr(beginR));
	begin->setAttribute("g", str::toStr(beginG));
	begin->setAttribute("b", str::toStr(beginB));
	begin->setAttribute("a", str::toStr(beginA));
	p_node->addChild(begin);
	
	xml::XmlNode* end = new xml::XmlNode("end");
	end->setAttribute("r", str::toStr(endR));
	end->setAttribute("g", str::toStr(endG));
	end->setAttribute("b", str::toStr(endB));
	end->setAttribute("a", str::toStr(endA));
	p_node->addChild(end);
	
	return true;
}


bool ColorAnimation2D::load(const fs::FilePtr& p_file)
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


bool ColorAnimation2D::save(const fs::FilePtr& p_file) const
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


bool ColorAnimation2D::load(const u8*& p_bufferOUT, size_t& p_sizeOUT)
{
	Tags empty;
	return load(p_bufferOUT, p_sizeOUT, empty, empty);
}


bool ColorAnimation2D::load( const u8*& p_bufferOUT, size_t& p_sizeOUT, const Tags& p_applyTags, const Tags& p_acceptedTags )
{
	if (p_sizeOUT < getBufferSize())
	{
		TT_PANIC("Buffer too small, got %d bytes, needs %d\n", p_sizeOUT, getBufferSize());
		return false;
	}
	
	using namespace code::bufferutils;
	
	u16 version = be_get<u16>(p_bufferOUT, p_sizeOUT);
	if (version != s_versionColorAnimation2D)
	{
		TT_PANIC("Invalid version, code %d.%d, data %d.%d",
		         GET_MAJOR_VERSION(s_versionColorAnimation2D), GET_MINOR_VERSION(s_versionColorAnimation2D),
		         GET_MAJOR_VERSION(version), GET_MINOR_VERSION(version));
		return false;
	}
	
	real min = be_get<real>(p_bufferOUT, p_sizeOUT);
	real max = be_get<real>(p_bufferOUT, p_sizeOUT);
	math::Range beginR(min, max);
	
	min = be_get<real>(p_bufferOUT, p_sizeOUT);
	max = be_get<real>(p_bufferOUT, p_sizeOUT);
	math::Range beginG(min, max);
	
	min = be_get<real>(p_bufferOUT, p_sizeOUT);
	max = be_get<real>(p_bufferOUT, p_sizeOUT);
	math::Range beginB(min, max);
	
	min = be_get<real>(p_bufferOUT, p_sizeOUT);
	max = be_get<real>(p_bufferOUT, p_sizeOUT);
	math::Range beginA(min, max);
	
	min = be_get<real>(p_bufferOUT, p_sizeOUT);
	max = be_get<real>(p_bufferOUT, p_sizeOUT);
	math::Range endR(min, max);
	
	min = be_get<real>(p_bufferOUT, p_sizeOUT);
	max = be_get<real>(p_bufferOUT, p_sizeOUT);
	math::Range endG(min, max);
	
	min = be_get<real>(p_bufferOUT, p_sizeOUT);
	max = be_get<real>(p_bufferOUT, p_sizeOUT);
	math::Range endB(min, max);
	
	min = be_get<real>(p_bufferOUT, p_sizeOUT);
	max = be_get<real>(p_bufferOUT, p_sizeOUT);
	math::Range endA(min, max);
	
	setBeginAndEndRange(beginR, beginG, beginB, beginA, endR, endG, endB, endA);
	
	return Animation2D::load(p_bufferOUT, p_sizeOUT, p_applyTags, p_acceptedTags);
}


bool ColorAnimation2D::save(u8*& p_bufferOUT, size_t& p_sizeOUT) const
{
	if (p_sizeOUT < getBufferSize())
	{
		TT_PANIC("Buffer too small, got %d bytes, needs %d\n", p_sizeOUT, getBufferSize());
		return false;
	}
	
	using namespace code::bufferutils;
	be_put(s_versionColorAnimation2D, p_bufferOUT, p_sizeOUT);
	
	be_put(m_beginRRange.getMin(), p_bufferOUT, p_sizeOUT);
	be_put(m_beginRRange.getMax(), p_bufferOUT, p_sizeOUT);
	be_put(m_beginGRange.getMin(), p_bufferOUT, p_sizeOUT);
	be_put(m_beginGRange.getMax(), p_bufferOUT, p_sizeOUT);
	be_put(m_beginBRange.getMin(), p_bufferOUT, p_sizeOUT);
	be_put(m_beginBRange.getMax(), p_bufferOUT, p_sizeOUT);
	be_put(m_beginARange.getMin(), p_bufferOUT, p_sizeOUT);
	be_put(m_beginARange.getMax(), p_bufferOUT, p_sizeOUT);
	
	be_put(m_endRRange.getMin(), p_bufferOUT, p_sizeOUT);
	be_put(m_endRRange.getMax(), p_bufferOUT, p_sizeOUT);
	be_put(m_endGRange.getMin(), p_bufferOUT, p_sizeOUT);
	be_put(m_endGRange.getMax(), p_bufferOUT, p_sizeOUT);
	be_put(m_endBRange.getMin(), p_bufferOUT, p_sizeOUT);
	be_put(m_endBRange.getMax(), p_bufferOUT, p_sizeOUT);
	be_put(m_endARange.getMin(), p_bufferOUT, p_sizeOUT);
	be_put(m_endARange.getMax(), p_bufferOUT, p_sizeOUT);
	
	return Animation2D::save(p_bufferOUT, p_sizeOUT);
}


size_t ColorAnimation2D::getBufferSize() const
{
	return s_sizeColorAnimation2D + Animation2D::getBufferSize();
}


void ColorAnimation2D::setRanges()
{
	Animation2D::setRanges();

	tt::math::Random& rng(tt::math::Random::getEffects());
	m_begin.x = m_beginRRange.getRandom(rng);
	m_begin.y = m_beginGRange.getRandom(rng);
	m_begin.z = m_beginBRange.getRandom(rng);
	m_begin.w = m_beginARange.getRandom(rng);
	m_end.x = m_endRRange.getRandom(rng);
	m_end.y = m_endGRange.getRandom(rng);
	m_end.z = m_endBRange.getRandom(rng);
	m_end.w = m_endARange.getRandom(rng);
	
	m_delta = m_end - m_begin;
}


ColorAnimation2D::ColorAnimation2D(const ColorAnimation2D& p_rhs)
:
Animation2D(p_rhs),
m_begin(p_rhs.m_begin),
m_end(p_rhs.m_end),
m_delta(p_rhs.m_delta),
m_beginRRange(p_rhs.m_beginRRange),
m_beginGRange(p_rhs.m_beginGRange),
m_beginBRange(p_rhs.m_beginBRange),
m_beginARange(p_rhs.m_beginARange),
m_endRRange(p_rhs.m_endRRange),
m_endGRange(p_rhs.m_endGRange),
m_endBRange(p_rhs.m_endBRange),
m_endARange(p_rhs.m_endARange)
{
}


//namespace end
}
}
}
