#include <tt/code/bufferutils.h>
#include <tt/code/ErrorStatus.h>
#include <tt/pres/anim2d/ScaleAnimation2D.h>
#include <tt/fs/File.h>
#include <tt/str/parse.h>
#include <tt/str/toStr.h>
#include <tt/xml/XmlNode.h>


namespace tt {
namespace pres {
namespace anim2d {

#define MAKE_VERSION(major, minor) (((major) << 8) | (minor))
#define GET_MAJOR_VERSION(version) ((version) >> 8)
#define GET_MINOR_VERSION(version) ((version) & 0xFF)

static const u16 s_version = MAKE_VERSION(1, 4);
static const size_t s_size = 2 + 1 + 1; // version + isUniformBegin + isUniformEnd


ScaleAnimation2D::ScaleAnimation2D()
:
PositionAnimation2D(),
m_beginX(1.0f),
m_beginY(1.0f),
m_beginZ(1.0f),
m_endX(1.0f),
m_endY(1.0f),
m_endZ(1.0f),
m_delta(math::Vector3::zero),
m_begin(math::Vector3::allOne),
m_isUniformScaleBegin(false),
m_isUniformScaleEnd(false)
{
}


void ScaleAnimation2D::applyTransform(Transform* p_transform) const
{
	const math::Vector3 currentScale(m_begin + (m_delta * getTime()));
	p_transform->scale.x *= currentScale.x;
	p_transform->scale.y *= currentScale.y;
	p_transform->scale.z *= currentScale.z;
}


bool ScaleAnimation2D::hasZAnimation() const
{
	return false;
}


int ScaleAnimation2D::getSortWeight() const
{
	return 3;
}


void ScaleAnimation2D::setBeginAndEndRange(
		const pres::PresentationValue& p_beginX, const pres::PresentationValue& p_beginY, const pres::PresentationValue& p_beginZ,
		const pres::PresentationValue& p_endX,   const pres::PresentationValue& p_endY,   const pres::PresentationValue& p_endZ)
{
	m_beginX = p_beginX;
	m_beginY = p_beginY;
	m_beginZ = p_beginZ;
	m_endX   = p_endX;
	m_endY   = p_endY;
	m_endZ   = p_endZ;
	setRanges(0);
}


void ScaleAnimation2D::setBeginAndEnd(const math::Vector3& p_begin, const math::Vector3& p_end)
{
	m_beginX.setValue(p_begin.x);
	m_beginY.setValue(p_begin.y);
	m_beginZ.setValue(p_begin.z);
	m_endX.setValue(p_end.x);
	m_endY.setValue(p_end.y);
	m_endZ.setValue(p_end.z);
	setRanges(0);
}


bool ScaleAnimation2D::load( const xml::XmlNode* p_node, const DataTags& p_applyTags, 
                             const Tags& p_acceptedTags,
                             code::ErrorStatus* p_errStatus )
{
	TT_ERR_CHAIN(bool, false, "ScaleAnimation2D load from xml");
	TT_ERR_NULL_ASSERT(p_node);
	
	const xml::XmlNode* begin = p_node->getFirstChild("begin");
	TT_ERR_ASSERTMSG(begin != 0, "Expected child 'begin' node after '" << p_node->getName() << "' node.");
	
	pres::PresentationValue beginX;
	pres::PresentationValue beginY;
	pres::PresentationValue beginZ;
	
	const std::string& beginValue = begin->getAttribute("scale");
	m_isUniformScaleBegin = (beginValue.empty() == false);
	if (m_isUniformScaleBegin == false)
	{
		beginX = pres::parseOptionalPresentationValue(begin, "x", 1.0f, &errStatus);
		beginY = pres::parseOptionalPresentationValue(begin, "y", 1.0f, &errStatus);
	}
	else
	{
		beginX.resetValue(beginValue, &errStatus);
		beginY = beginX;
	}
	beginZ = pres::parseOptionalPresentationValue(begin, "z", 1.0f, &errStatus);
	
	TT_ERR_RETURN_ON_ERROR();
	
	const xml::XmlNode* end = p_node->getFirstChild("end");
	TT_ERR_ASSERTMSG(end != 0, "Expected child 'end' in node '" << p_node->getName() << "'");
	
	pres::PresentationValue endX;
	pres::PresentationValue endY;
	pres::PresentationValue endZ;
	
	const std::string& endValue = end->getAttribute("scale");
	m_isUniformScaleEnd = (endValue.empty() == false);
	if (m_isUniformScaleEnd == false)
	{
		endX = pres::parseOptionalPresentationValue(end, "x", 1.0f, &errStatus);
		endY = pres::parseOptionalPresentationValue(end, "y", 1.0f, &errStatus);
	}
	else
	{
		endX.resetValue(endValue, &errStatus);
		endY = endX;
	}
	endZ = pres::parseOptionalPresentationValue(end, "z", 1.0f, &errStatus);
	
	TT_ERR_RETURN_ON_ERROR();
	
	
	Animation2D::load(p_node, p_applyTags, p_acceptedTags, &errStatus);
	TT_ERR_RETURN_ON_ERROR();
	
	m_beginX = beginX;
	m_beginY = beginY;
	m_beginZ = beginZ;
	m_endX = endX;
	m_endY = endY;
	m_endZ = endZ;

	return true;
}


bool ScaleAnimation2D::loadStatic(const xml::XmlNode* p_node,
                                  const DataTags& p_applyTags,
                                  const Tags& p_acceptedTags,
                                  code::ErrorStatus* p_errStatus)
{
	TT_ERR_CHAIN(bool, false, "Static Scale load from xml");
	TT_ERR_NULL_ASSERT(p_node);
	TT_ERR_ASSERTMSG(p_node->getName() == "scale", "Expected node name 'scale' got '" << 
	                                               p_node->getName() << "'");
	
	pres::PresentationValue x;
	pres::PresentationValue y;
	pres::PresentationValue z;
	
	const std::string& endValue = p_node->getAttribute("scale");
	m_isUniformScaleEnd = (endValue.empty() == false);
	if (m_isUniformScaleEnd == false)
	{
		x = pres::parseOptionalPresentationValue(p_node, "x", 1.0f, &errStatus);
		y = pres::parseOptionalPresentationValue(p_node, "y", 1.0f, &errStatus);
	}
	else
	{
		x.resetValue(endValue, &errStatus);
		y = x;
	}
	z = pres::parseOptionalPresentationValue(p_node, "z", 1.0f, &errStatus);
	
	
	Animation2D::loadStatic(p_node, p_applyTags, p_acceptedTags, &errStatus);
	TT_ERR_RETURN_ON_ERROR();
	
	m_beginX = m_endX = x;
	m_beginY = m_endY = y;
	m_beginZ = m_endZ = z;
	m_isUniformScaleBegin = m_isUniformScaleEnd;
	
	return true;
}


bool ScaleAnimation2D::load( const u8*& p_bufferOUT, size_t& p_sizeOUT, 
                             const DataTags& p_applyTags, const Tags& p_acceptedTags,
                             code::ErrorStatus* p_errStatus )
{
	TT_ERR_CHAIN(bool, false, "Loading ScaleAnimation2D binary");
	TT_ERR_ASSERTMSG(p_sizeOUT >= getBufferSize(), "Buffer too small, got " << p_sizeOUT << 
	                 " bytes, needs " << getBufferSize() << "\n");
	
	using namespace code::bufferutils;
	
	u16 version = be_get<u16>(p_bufferOUT, p_sizeOUT);
	TT_ERR_ASSERTMSG(version == s_version, "Invalid ScaleAnimation2D version, code "
			<< GET_MAJOR_VERSION(s_version) << "." <<GET_MINOR_VERSION(s_version)<<", data "
			<< GET_MAJOR_VERSION(version)   << "." <<GET_MINOR_VERSION(version)  <<
			", Please update your presentation converter");
	
	pres::PresentationValue beginX;
	pres::PresentationValue beginY;
	pres::PresentationValue beginZ;
	pres::PresentationValue endX;
	pres::PresentationValue endY;
	pres::PresentationValue endZ;
	
	beginX.load(p_bufferOUT, p_sizeOUT, &errStatus);
	beginY.load(p_bufferOUT, p_sizeOUT, &errStatus);
	beginZ.load(p_bufferOUT, p_sizeOUT, &errStatus);
	m_isUniformScaleBegin = be_get<bool>(p_bufferOUT, p_sizeOUT);
	
	endX.load(p_bufferOUT, p_sizeOUT, &errStatus);
	endY.load(p_bufferOUT, p_sizeOUT, &errStatus);
	endZ.load(p_bufferOUT, p_sizeOUT, &errStatus);
	m_isUniformScaleEnd = be_get<bool>(p_bufferOUT, p_sizeOUT);
	
	TT_ERR_RETURN_ON_ERROR();
	
	setBeginAndEndRange(beginX, beginY, beginZ, endX, endY, endZ);
	
	return Animation2D::load(p_bufferOUT, p_sizeOUT, p_applyTags, p_acceptedTags, &errStatus);
}


bool ScaleAnimation2D::save(u8*& p_bufferOUT, size_t& p_sizeOUT,
                             code::ErrorStatus* p_errStatus) const
{
	TT_ERR_CHAIN(bool, false, "saving ScaleAnimation2D binary");
	TT_ERR_ASSERTMSG(p_sizeOUT >= getBufferSize(), "Buffer too small, got " << p_sizeOUT << 
	                 " bytes, needs " << getBufferSize() << "\n");
	
	using namespace code::bufferutils;
	be_put(s_version, p_bufferOUT, p_sizeOUT);
	
	
	m_beginX.save(p_bufferOUT, p_sizeOUT, &errStatus);
	m_beginY.save(p_bufferOUT, p_sizeOUT, &errStatus);
	m_beginZ.save(p_bufferOUT, p_sizeOUT, &errStatus);
	be_put(m_isUniformScaleBegin, p_bufferOUT, p_sizeOUT);
	
	m_endX.save(p_bufferOUT, p_sizeOUT, &errStatus);
	m_endY.save(p_bufferOUT, p_sizeOUT, &errStatus);
	m_endZ.save(p_bufferOUT, p_sizeOUT, &errStatus);
	be_put(m_isUniformScaleEnd, p_bufferOUT, p_sizeOUT);
	
	TT_ERR_RETURN_ON_ERROR();
	
	return Animation2D::save(p_bufferOUT, p_sizeOUT, &errStatus);
}


size_t ScaleAnimation2D::getBufferSize() const
{
	return s_size + Animation2D::getBufferSize() +
	       m_beginX.getBufferSize() + m_beginY.getBufferSize() + m_beginZ.getBufferSize() +
	       m_endX.getBufferSize()   + m_endY.getBufferSize()   + m_endZ.getBufferSize();
}


void ScaleAnimation2D::setRanges(PresentationObject* p_presObj)
{
	Animation2D::setRanges(p_presObj);
	
	// Update x values
	m_beginX.updateValue(p_presObj);
	m_endX.updateValue(p_presObj);
	
	if(m_isUniformScaleBegin)
	{
		// Use X value for Y when its uniform
		m_beginY.setValue(m_beginX);
	}
	else
	{
		// Update Y value when not uniform
		m_beginY.updateValue(p_presObj);
	}
	
	if(m_isUniformScaleEnd)
	{
		// Use X value for Y when its uniform
		m_endY.setValue(m_endX);
	}
	else
	{
		// Update Y value when not uniform
		m_endY.updateValue(p_presObj);
	}
	
	// Z values are always separate (usualy value is 1.0f)
	m_beginZ.updateValue(p_presObj);
	m_endZ.updateValue(p_presObj);
	
	m_delta = getEnd() - getBegin();
	m_begin = getBegin();
}


ScaleAnimation2D::ScaleAnimation2D(const ScaleAnimation2D& p_rhs)
:
PositionAnimation2D(p_rhs),
m_beginX(p_rhs.m_beginX),
m_beginY(p_rhs.m_beginY),
m_beginZ(p_rhs.m_beginZ),
m_endX(p_rhs.m_endX),
m_endY(p_rhs.m_endY),
m_endZ(p_rhs.m_endZ),
m_delta(p_rhs.m_delta),
m_begin(p_rhs.m_begin),
m_isUniformScaleBegin(p_rhs.m_isUniformScaleBegin),
m_isUniformScaleEnd(p_rhs.m_isUniformScaleEnd)
{
}

//namespace end
}
}
}
