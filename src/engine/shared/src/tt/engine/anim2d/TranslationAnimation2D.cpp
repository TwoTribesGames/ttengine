#include <tt/code/bufferutils.h>
#include <tt/code/ErrorStatus.h>
#include <tt/engine/anim2d/TranslationAnimation2D.h>
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

static const u16 s_versionTranslationAnimation2D = MAKE_VERSION(1, 6);
static const size_t s_sizeTranslationAnimation2D = 2 + 8 + 8 + 8 + 8 + 8 + 8; // version, begin x, begin y, begin z, end x, end y, end z
static const size_t s_sizeg = 2; // version


TranslationAnimation2D::TranslationAnimation2D(bool p_isGameTranslation)
:
PositionAnimation2D(),
m_begin(math::Vector3::zero),
m_end(math::Vector3::zero),
m_delta(math::Vector3::zero),
m_beginXRange(),
m_beginYRange(),
m_beginZRange(),
m_endXRange(),
m_endYRange(),
m_endZRange(),
m_invertYDuringLoad(false),
m_rangesSet(false),
m_isGameTranslation(p_isGameTranslation)
{
}


void TranslationAnimation2D::applyTransform(Transform* p_transform) const
{
	TT_ASSERTMSG(m_rangesSet,"Ranges not set yet, "
		" Set the ranges for the TranslationAnimation2D before calling getTransform");
	
	p_transform->translation += (m_begin + (m_delta * getTime()));
}


bool TranslationAnimation2D::hasZAnimation() const
{
	return m_beginZRange.getMin() != 0.0f ||
	       m_beginZRange.getMax() != 0.0f ||
	       m_endZRange.getMin() != 0.0f ||
	       m_endZRange.getMax() != 0.0f;
}


int TranslationAnimation2D::getSortWeight() const
{
	return 1;
}


void TranslationAnimation2D::setBeginAndEndRange(const math::Range& p_beginX, const math::Range& p_beginY, const math::Range& p_beginZ,
	                                             const math::Range& p_endX,   const math::Range& p_endY,   const math::Range& p_endZ)
{
	m_beginXRange = p_beginX;
	m_beginYRange = p_beginY;
	m_beginZRange = p_beginZ;
	m_endXRange = p_endX;
	m_endYRange = p_endY;
	m_endZRange = p_endZ;
	setRanges();
}


void TranslationAnimation2D::setBeginAndEnd(const math::Vector3& p_begin, const math::Vector3& p_end)
{
	m_beginXRange.setMinMax(p_begin.x);
	m_beginYRange.setMinMax(p_begin.y);
	m_beginZRange.setMinMax(p_begin.z);
	m_endXRange.setMinMax(p_end.x);
	m_endYRange.setMinMax(p_end.y);
	m_endZRange.setMinMax(p_end.z);
	setRanges();
}


bool TranslationAnimation2D::load(const xml::XmlNode* p_node)
{
	Tags empty;
	return load(p_node, empty, empty);
}


bool TranslationAnimation2D::load(const xml::XmlNode* p_node, 
                                  const Tags& p_applyTags, 
                                  const Tags& p_acceptedTags)
{
	TT_NULL_ASSERT(p_node);
	TT_ERR_CREATE("TranslationAnimation2D load from xml");

	if(p_node->getFirstChild("begin") != 0)
	{
		const xml::XmlNode* begin = p_node->getFirstChild("begin");
		if (begin == 0)
		{
			TT_PANIC("Expected 'begin' node after 'translation' node.");
			return false;
		}

		math::Range beginX = xml::util::parseRange(begin, "x", &errStatus);
		math::Range beginY = xml::util::parseRange(begin, "y", &errStatus);
		math::Range beginZ;
		if (begin->getAttribute("z").empty() == false)
		{
			beginZ = xml::util::parseRange(begin, "z", &errStatus);
		}

		const xml::XmlNode* end = p_node->getFirstChild("end");
		if (end == 0)
		{
			TT_PANIC("Expected 'end' node after 'translation' node.");
			return false;
		}

		math::Range endX = xml::util::parseRange(end, "x", &errStatus);
		math::Range endY = xml::util::parseRange(end, "y", &errStatus);
		math::Range endZ;
		if (end->getAttribute("z").empty() == false)
		{
			endZ = xml::util::parseRange(end, "z", &errStatus);
		}

		if (errStatus.hasError())
		{
			TT_PANIC("%s", errStatus.getErrorMessage().c_str());
			return false;
		}

		if (m_invertYDuringLoad)
		{
			beginY.setMinMax(-beginY.getMax(), -beginY.getMin());
			endY.setMinMax(-endY.getMax(), -endY.getMin());
		}

		if (Animation2D::load(p_node, p_applyTags, p_acceptedTags) == false)
		{
			return false;
		}

		setBeginAndEndRange(beginX, beginY, beginZ, endX, endY, endZ);
		m_rangesSet = true;
		m_isGameTranslation = false;
	}
	else
	{
		if (Animation2D::load(p_node, p_applyTags, p_acceptedTags) == false)
		{
			return false;
		}
		m_rangesSet = false;
		m_isGameTranslation = true;
	}

	return true;
}


bool TranslationAnimation2D::save(xml::XmlNode* p_node) const
{
	TT_NULL_ASSERT(p_node);
	
	if (Animation2D::save(p_node) == false)
	{
		return false;
	}
	if(m_isGameTranslation==false)
	{
		xml::XmlNode* begin = new xml::XmlNode("begin");
		begin->setAttribute("x", str::toStr(getBeginXRange()));
		begin->setAttribute("y", str::toStr(getBeginYRange()));
		begin->setAttribute("z", str::toStr(getBeginZRange()));
		p_node->addChild(begin);

		xml::XmlNode* end = new xml::XmlNode("end");
		end->setAttribute("x", str::toStr(getEndXRange()));
		end->setAttribute("y", str::toStr(getEndYRange()));
		end->setAttribute("z", str::toStr(getEndZRange()));
		p_node->addChild(end);
	}
	
	return true;
}


bool TranslationAnimation2D::load(const fs::FilePtr& p_file)
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


bool TranslationAnimation2D::save(const fs::FilePtr& p_file) const
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


bool TranslationAnimation2D::load(const u8*& p_bufferOUT, size_t& p_sizeOUT)
{
	Tags empty;
	return load(p_bufferOUT, p_sizeOUT, empty, empty);
}


bool TranslationAnimation2D::load( const u8*& p_bufferOUT, size_t& p_sizeOUT, const Tags& p_applyTags, const Tags& p_acceptedTags )
{
	if (p_sizeOUT < getBufferSize())
	{
		TT_PANIC("Buffer too small, got %d bytes, needs %d\n", p_sizeOUT, getBufferSize());
		return false;
	}
	
	using namespace code::bufferutils;
	
	u16 version = be_get<u16>(p_bufferOUT, p_sizeOUT);
	if (version != s_versionTranslationAnimation2D)
	{
		TT_PANIC("Invalid version, code %d.%d, data %d.%d, Please update your converter",
		         GET_MAJOR_VERSION(s_versionTranslationAnimation2D), GET_MINOR_VERSION(s_versionTranslationAnimation2D),
		         GET_MAJOR_VERSION(version), GET_MINOR_VERSION(version));
		return false;
	}
	if(m_isGameTranslation == false)
	{
		real min = be_get<real>(p_bufferOUT, p_sizeOUT);
		real max = be_get<real>(p_bufferOUT, p_sizeOUT);
		math::Range beginX(min, max);

		min = be_get<real>(p_bufferOUT, p_sizeOUT);
		max = be_get<real>(p_bufferOUT, p_sizeOUT);
		math::Range beginY(min, max);

		min = be_get<real>(p_bufferOUT, p_sizeOUT);
		max = be_get<real>(p_bufferOUT, p_sizeOUT);
		math::Range beginZ(min, max);

		min = be_get<real>(p_bufferOUT, p_sizeOUT);
		max = be_get<real>(p_bufferOUT, p_sizeOUT);
		math::Range endX(min, max);

		min = be_get<real>(p_bufferOUT, p_sizeOUT);
		max = be_get<real>(p_bufferOUT, p_sizeOUT);
		math::Range endY(min, max);

		min = be_get<real>(p_bufferOUT, p_sizeOUT);
		max = be_get<real>(p_bufferOUT, p_sizeOUT);
		math::Range endZ(min, max);

		if (m_invertYDuringLoad)
		{
			beginY.setMinMax(-beginY.getMax(), -beginY.getMin());
			endY.setMinMax(-endY.getMax(), -endY.getMin());
		}

		setBeginAndEndRange(beginX, beginY, beginZ, endX, endY, endZ);

		m_isGameTranslation = false;
		m_rangesSet = true;
	}
	else
	{
		m_isGameTranslation = true;
		m_rangesSet = false;
	}
	
	return Animation2D::load(p_bufferOUT, p_sizeOUT, p_applyTags, p_acceptedTags);
}


bool TranslationAnimation2D::save(u8*& p_bufferOUT, size_t& p_sizeOUT) const
{
	if (p_sizeOUT < getBufferSize())
	{
		TT_PANIC("Buffer too small, got %d bytes, needs %d\n", p_sizeOUT, getBufferSize());
		return false;
	}
	using namespace code::bufferutils;
	be_put(s_versionTranslationAnimation2D, p_bufferOUT, p_sizeOUT);

	if(m_isGameTranslation == false)
	{
		be_put(m_beginXRange.getMin(), p_bufferOUT, p_sizeOUT);
		be_put(m_beginXRange.getMax(), p_bufferOUT, p_sizeOUT);
		be_put(m_beginYRange.getMin(), p_bufferOUT, p_sizeOUT);
		be_put(m_beginYRange.getMax(), p_bufferOUT, p_sizeOUT);
		be_put(m_beginZRange.getMin(), p_bufferOUT, p_sizeOUT);
		be_put(m_beginZRange.getMax(), p_bufferOUT, p_sizeOUT);

		be_put(m_endXRange.getMin(), p_bufferOUT, p_sizeOUT);
		be_put(m_endXRange.getMax(), p_bufferOUT, p_sizeOUT);
		be_put(m_endYRange.getMin(), p_bufferOUT, p_sizeOUT);
		be_put(m_endYRange.getMax(), p_bufferOUT, p_sizeOUT);
		be_put(m_endZRange.getMin(), p_bufferOUT, p_sizeOUT);
		be_put(m_endZRange.getMax(), p_bufferOUT, p_sizeOUT);
	}
	
	return Animation2D::save(p_bufferOUT, p_sizeOUT);
}


size_t TranslationAnimation2D::getBufferSize() const
{
	if(m_isGameTranslation)
	{
		return s_sizeg + Animation2D::getBufferSize();
	}
	else
	{
		return s_sizeTranslationAnimation2D + Animation2D::getBufferSize();
	}
}


void TranslationAnimation2D::setRanges()
{
	Animation2D::setRanges();
	tt::math::Random& rng(tt::math::Random::getEffects());

	m_begin.x = m_beginXRange.getRandom(rng);
	m_begin.y = m_beginYRange.getRandom(rng);
	m_begin.z = m_beginZRange.getRandom(rng);
	m_end.x = m_endXRange.getRandom(rng);
	m_end.y = m_endYRange.getRandom(rng);
	m_end.z = m_endZRange.getRandom(rng);
	
	m_delta = m_end - m_begin;

	m_rangesSet = true;
}

PositionAnimation2D::AnimationType TranslationAnimation2D::getAnimationType() const
{
	if (m_isGameTranslation)
	{
		return PositionAnimation2D::AnimationType_GameTranslation;
	}
	else
	{
		return PositionAnimation2D::AnimationType_Translation;
	}
}


TranslationAnimation2D::TranslationAnimation2D(const TranslationAnimation2D& p_rhs)
:
PositionAnimation2D(p_rhs),
m_begin(p_rhs.m_begin),
m_end(p_rhs.m_end),
m_delta(p_rhs.m_delta),
m_beginXRange(p_rhs.m_beginXRange),
m_beginYRange(p_rhs.m_beginYRange),
m_beginZRange(p_rhs.m_beginZRange),
m_endXRange(p_rhs.m_endXRange),
m_endYRange(p_rhs.m_endYRange),
m_endZRange(p_rhs.m_endZRange),
m_rangesSet(p_rhs.m_rangesSet),
m_isGameTranslation(p_rhs.m_isGameTranslation)
{
}

//namespace end
}
}
}
