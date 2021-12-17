#include <tt/code/bufferutils.h>
#include <tt/code/ErrorStatus.h>
#include <tt/engine/anim2d/ScaleAnimation2D.h>
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

static const u16 s_versionScaleAnimation2D = MAKE_VERSION(1, 6);


ScaleAnimation2D::ScaleAnimation2D()
:
PositionAnimation2D(),
m_begin(math::Vector2::zero),
m_end(math::Vector2::zero),
m_delta(math::Vector2::zero),
m_beginXRange(),
m_beginYRange(),
m_endXRange(),
m_endYRange(),
m_isUniform(false)
{
}


void ScaleAnimation2D::applyTransform(Transform* p_transform) const
{
	p_transform->scale.x *= m_begin.x + (m_delta.x * getTime());
	
	if (m_isUniform)
	{
		p_transform->scale.y = p_transform->scale.x;
	}
	else
	{
		p_transform->scale.y *= m_begin.y + (m_delta.y * getTime());
	}
}


bool ScaleAnimation2D::hasZAnimation() const
{
	return false;
}


int ScaleAnimation2D::getSortWeight() const
{
	return 3;
}


void ScaleAnimation2D::setBeginAndEndRange(const math::Range& p_beginXRange, const math::Range& p_beginYRange,
	                                       const math::Range& p_endXRange,   const math::Range& p_endYRange)
{
	m_beginXRange = p_beginXRange;
	m_beginYRange = p_beginYRange;
	m_endXRange   = p_endXRange;
	m_endYRange   = p_endYRange;
	setRanges();
}


void ScaleAnimation2D::setBeginAndEnd(const math::Vector2& p_begin, const math::Vector2& p_end)
{
	m_beginXRange.setMinMax(p_begin.x);
	m_beginYRange.setMinMax(p_begin.y);
	m_endXRange.setMinMax(p_end.x);
	m_endYRange.setMinMax(p_end.y);
	setRanges();
}


bool ScaleAnimation2D::load(const xml::XmlNode* p_node)
{
	Tags empty;
	return load(p_node, empty, empty);
}


bool ScaleAnimation2D::load(const xml::XmlNode* p_node,
                            const Tags& p_applyTags,
                            const Tags& p_acceptedTags)
{
	TT_NULL_ASSERT(p_node);
	
	TT_ERR_CREATE("load");
	
	if (p_node->getFirstChild("begin") == 0)
	{
		TT_PANIC("Expected child 'begin' in node '%s'",
		         p_node->getName().c_str());
		return false;
	}
	
	m_isUniform = true;
	
	// Parse begin ranges
	math::Range beginXRange(1.0f);
	math::Range beginYRange(1.0f);
	{
		const std::string& valueScale = p_node->getFirstChild("begin")->getAttribute("scale");
		const bool isUniformScaleBegin = (valueScale.empty() == false);
		if (isUniformScaleBegin)
		{
			beginXRange = str::parseRange(valueScale, &errStatus);
			if (errStatus.hasError())
			{
				TT_PANIC("Value '%s' of attribute 'scale' from node 'begin' from node '%s' "
						 "can't be converted to a range.",
						 valueScale.c_str(), 
						 p_node->getName().c_str());
				return false;
			}
			beginYRange = beginXRange;
		}
		else
		{
			// Not uniform anymore
			m_isUniform = false;
			
			// Parse X scale
			const std::string& valueX = p_node->getFirstChild("begin")->getAttribute("x");
			if (valueX.empty() == false)
			{
				beginXRange = str::parseRange(valueX, &errStatus);
				if (errStatus.hasError())
				{
					TT_PANIC("Value '%s' of attribute 'x' from node 'begin' from node '%s' "
							 "can't be converted to a range.",
							 valueX.c_str(), 
							 p_node->getName().c_str());
					return false;
				}
			}
			
			// Parse Y scale
			const std::string& valueY = p_node->getFirstChild("begin")->getAttribute("y");
			if (valueY.empty() == false)
			{
				beginYRange = str::parseRange(valueY, &errStatus);
				if (errStatus.hasError())
				{
					TT_PANIC("Value '%s' of attribute 'y' from node 'begin' from node '%s' "
							 "can't be converted to a range.",
							 valueY.c_str(), 
							 p_node->getName().c_str());
					return false;
				}
			}
			
			if (valueX.empty() && valueY.empty())
			{
				TT_PANIC("Expected at least one of the following attributes 'scale', 'x' or 'y' "
						 "in node 'begin' in node '%s'",
						 p_node->getName().c_str());
				return false;
			}
		}
	}
	
	// Parse end ranges
	math::Range endXRange(1.0f);
	math::Range endYRange(1.0f);
	{
		const std::string& valueScale = p_node->getFirstChild("end")->getAttribute("scale");
		const bool isUniformScaleEnd = (valueScale.empty() == false);
		if (isUniformScaleEnd)
		{
			endXRange = str::parseRange(valueScale, &errStatus);
			if (errStatus.hasError())
			{
				TT_PANIC("Value '%s' of attribute 'scale' from node 'end' from node '%s' "
						 "can't be converted to a range.",
						 valueScale.c_str(), 
						 p_node->getName().c_str());
				return false;
			}
			endYRange = endXRange;
		}
		else
		{
			// Not uniform anymore
			m_isUniform = false;
			
			// Parse X scale
			const std::string& valueX = p_node->getFirstChild("end")->getAttribute("x");
			if (valueX.empty() == false)
			{
				endXRange = str::parseRange(valueX, &errStatus);
				if (errStatus.hasError())
				{
					TT_PANIC("Value '%s' of attribute 'x' from node 'end' from node '%s' "
							 "can't be converted to a range.",
							 valueX.c_str(), 
							 p_node->getName().c_str());
					return false;
				}
			}
			
			// Parse Y scale
			const std::string& valueY = p_node->getFirstChild("end")->getAttribute("y");
			if (valueY.empty() == false)
			{
				endYRange = str::parseRange(valueY, &errStatus);
				if (errStatus.hasError())
				{
					TT_PANIC("Value '%s' of attribute 'y' from node 'end' from node '%s' "
							 "can't be converted to a range.",
							 valueY.c_str(), 
							 p_node->getName().c_str());
					return false;
				}
			}
			
			if (valueX.empty() && valueY.empty())
			{
				TT_PANIC("Expected at least one of the following attributes 'scale', 'x' or 'y' "
						 "in node 'end' in node '%s'",
						 p_node->getName().c_str());
				return false;
			}
		}
	}
	
	if (Animation2D::load(p_node, p_applyTags, p_acceptedTags) == false)
	{
		return false;
	}
	
	setBeginAndEndRange(beginXRange, beginYRange, endXRange, endYRange);
	
	return true;
}


bool ScaleAnimation2D::save(xml::XmlNode* p_node) const
{
	TT_NULL_ASSERT(p_node);
	
	if (Animation2D::save(p_node) == false)
	{
		return false;
	}
	
	xml::XmlNode* begin = new xml::XmlNode("begin");
	if (m_isUniform)
	{
		begin->setAttribute("scale", str::toStr(getBeginXRange()));
	}
	else
	{
		begin->setAttribute("x", str::toStr(getBeginXRange()));
		begin->setAttribute("y", str::toStr(getBeginYRange()));
	}
	p_node->addChild(begin);
	
	xml::XmlNode* end = new xml::XmlNode("end");
	if (m_isUniform)
	{
		end->setAttribute("scale", str::toStr(getEndXRange()));
	}
	else
	{
		end->setAttribute("x", str::toStr(getEndXRange()));
		end->setAttribute("y", str::toStr(getEndYRange()));
	}
	p_node->addChild(end);
	
	return true;
}


bool ScaleAnimation2D::load(const fs::FilePtr& p_file)
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


bool ScaleAnimation2D::save(const fs::FilePtr& p_file) const
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


bool ScaleAnimation2D::load(const u8*& p_bufferOUT, size_t& p_sizeOUT)
{
	Tags empty;
	return load(p_bufferOUT, p_sizeOUT, empty, empty);
}


bool ScaleAnimation2D::load( const u8*& p_bufferOUT, size_t& p_sizeOUT, 
                             const Tags& p_applyTags, const Tags& p_acceptedTags )
{
	if (p_sizeOUT < getBufferSize())
	{
		TT_PANIC("Buffer too small, got %d bytes, needs %d\n", p_sizeOUT, getBufferSize());
		return false;
	}
	
	using namespace code::bufferutils;
	
	u16 version = be_get<u16>(p_bufferOUT, p_sizeOUT);
	if (version != s_versionScaleAnimation2D)
	{
		TT_PANIC("Invalid version, code %d.%d, data %d.%d",
		         GET_MAJOR_VERSION(s_versionScaleAnimation2D), GET_MINOR_VERSION(s_versionScaleAnimation2D),
		         GET_MAJOR_VERSION(version), GET_MINOR_VERSION(version));
		return false;
	}
	
	m_isUniform = be_get<bool>(p_bufferOUT, p_sizeOUT);
	
	// Fetch begin
	real min = be_get<real>(p_bufferOUT, p_sizeOUT);
	real max = be_get<real>(p_bufferOUT, p_sizeOUT);
	math::Range beginX(min, max);
	
	min = be_get<real>(p_bufferOUT, p_sizeOUT);
	max = be_get<real>(p_bufferOUT, p_sizeOUT);
	math::Range beginY(min, max);
	
	// Fetch end
	min = be_get<real>(p_bufferOUT, p_sizeOUT);
	max = be_get<real>(p_bufferOUT, p_sizeOUT);
	math::Range endX(min, max);
	
	min = be_get<real>(p_bufferOUT, p_sizeOUT);
	max = be_get<real>(p_bufferOUT, p_sizeOUT);
	math::Range endY(min, max);
	
	setBeginAndEndRange(beginX, beginY, endX, endY);
	
	return Animation2D::load(p_bufferOUT, p_sizeOUT, p_applyTags, p_acceptedTags);
}


bool ScaleAnimation2D::save(u8*& p_bufferOUT, size_t& p_sizeOUT) const
{
	if (p_sizeOUT < getBufferSize())
	{
		TT_PANIC("Buffer too small, got %d bytes, needs %d\n", p_sizeOUT, getBufferSize());
		return false;
	}
	
	using namespace code::bufferutils;
	be_put(s_versionScaleAnimation2D, p_bufferOUT, p_sizeOUT);
	
	// Store begin
	be_put(m_isUniform, p_bufferOUT, p_sizeOUT);
	be_put(m_beginXRange.getMin(), p_bufferOUT, p_sizeOUT);
	be_put(m_beginXRange.getMax(), p_bufferOUT, p_sizeOUT);
	be_put(m_beginYRange.getMin(), p_bufferOUT, p_sizeOUT);
	be_put(m_beginYRange.getMax(), p_bufferOUT, p_sizeOUT);
	
	// Store end
	be_put(m_endXRange.getMin(), p_bufferOUT, p_sizeOUT);
	be_put(m_endXRange.getMax(), p_bufferOUT, p_sizeOUT);
	be_put(m_endYRange.getMin(), p_bufferOUT, p_sizeOUT);
	be_put(m_endYRange.getMax(), p_bufferOUT, p_sizeOUT);
	
	return Animation2D::save(p_bufferOUT, p_sizeOUT);
}


size_t ScaleAnimation2D::getBufferSize() const
{
	const size_t size = sizeof(s_versionScaleAnimation2D) + sizeof(m_isUniform) +
	                    sizeof(m_beginXRange) + sizeof(m_beginYRange) +
	                    sizeof(m_endXRange) + sizeof(m_endYRange);
	
	return size + Animation2D::getBufferSize();
}


void ScaleAnimation2D::setRanges()
{
	Animation2D::setRanges();
	
	const real xBegin = m_beginXRange.getRandom(tt::math::Random::getEffects());
	const real xEnd   = m_endXRange.getRandom(tt::math::Random::getEffects());
	
	const real yBegin = m_isUniform ? xBegin :
	                                  m_beginYRange.getRandom(tt::math::Random::getEffects());
	const real yEnd   = m_isUniform ? yBegin :
	                                  m_endYRange.getRandom(tt::math::Random::getEffects());
	
	m_begin.setValues(xBegin, yBegin);
	m_end.setValues(xEnd, yEnd);
	m_delta = m_end - m_begin;
}


ScaleAnimation2D::ScaleAnimation2D(const ScaleAnimation2D& p_rhs)
:
PositionAnimation2D(p_rhs),
m_begin(p_rhs.m_begin),
m_end(p_rhs.m_end),
m_delta(p_rhs.m_delta),
m_beginXRange(p_rhs.m_beginXRange),
m_beginYRange(p_rhs.m_beginYRange),
m_endXRange(p_rhs.m_endXRange),
m_endYRange(p_rhs.m_endYRange),
m_isUniform(p_rhs.m_isUniform)
{
}

//namespace end
}
}
}
