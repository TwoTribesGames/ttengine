#include <tt/code/bufferutils.h>
#include <tt/code/ErrorStatus.h>
#include <tt/pres/anim2d/RotationAnimation2D.h>
#include <tt/fs/File.h>
#include <tt/str/parse.h>
#include <tt/str/toStr.h>
#include <tt/xml/util/parse.h>
#include <tt/xml/XmlNode.h>


namespace tt {
namespace pres {
namespace anim2d {

#define MAKE_VERSION(major, minor) (((major) << 8) | (minor))
#define GET_MAJOR_VERSION(version) ((version) >> 8)
#define GET_MINOR_VERSION(version) ((version) & 0xFF)

static const u16 s_version = MAKE_VERSION(1, 2);
static const size_t s_size = 2; // version


RotationAnimation2D::RotationAnimation2D()
:
PositionAnimation2D(),
m_begin(0.0f),
m_end(0.0f),
m_delta(0.0f),
m_beginAngle(0.0f)
{
}


void RotationAnimation2D::applyTransform(Transform* p_transform) const
{
	p_transform->rotation += m_beginAngle + (m_delta * getTime());
}


bool RotationAnimation2D::hasZAnimation() const
{
	return false;
}


int RotationAnimation2D::getSortWeight() const
{
	return 2;
}


void RotationAnimation2D::setBeginAndEndRange(const pres::PresentationValue& p_begin, const pres::PresentationValue& p_end)
{
	m_begin = p_begin;
	m_end = p_end;
	setRanges(0);
}


void RotationAnimation2D::setBeginAndEnd(real p_begin, real p_end)
{
	m_begin.setValue(p_begin);
	m_end.setValue(p_end);
	setRanges(0);
}


bool RotationAnimation2D::load(const xml::XmlNode* p_node, const DataTags& p_applyTags,
                               const Tags& p_acceptedTags, code::ErrorStatus* p_errStatus)
{
	TT_ERR_CHAIN(bool, false, "RotationAnimation2D load from xml");
	TT_ERR_NULL_ASSERT(p_node);
	
	
	TT_ERR_ASSERTMSG(p_node->getFirstChild("begin") != 0, "Expected child 'begin' in node '" << p_node->getName() << "'");
	pres::PresentationValue begin(pres::parsePresentationValue(p_node->getFirstChild("begin"), "rotation", &errStatus));
	TT_ERR_RETURN_ON_ERROR();
	
	TT_ERR_ASSERTMSG(p_node->getFirstChild("end") != 0, "Expected child 'end' in node '" << p_node->getName() << "'");
	pres::PresentationValue end(pres::parsePresentationValue(p_node->getFirstChild("end"), "rotation", &errStatus));
	
	Animation2D::load(p_node, p_applyTags, p_acceptedTags, &errStatus);
	TT_ERR_RETURN_ON_ERROR();
	
	
	setBeginAndEndRange(begin, end);
	return true;
}


bool RotationAnimation2D::loadStatic(const xml::XmlNode* p_node,
                                     const DataTags& p_applyTags,
                                     const Tags& p_acceptedTags,
                                     code::ErrorStatus* p_errStatus)
{
	TT_ERR_CHAIN(bool, false, "Static Rotation load from xml");
	TT_ERR_NULL_ASSERT(p_node);
	TT_ERR_ASSERTMSG(p_node->getName() == "rotation", "Expected node name 'rotation' got '" << 
	                                                     p_node->getName() << "'");
	
	pres::PresentationValue rotation(pres::parseOptionalPresentationValue(p_node, "value", 0.0f, &errStatus));
	
	Animation2D::loadStatic(p_node, p_applyTags, p_acceptedTags, &errStatus);
	TT_ERR_RETURN_ON_ERROR();
	
	setBeginAndEndRange(rotation, rotation);
	return true;
}


bool RotationAnimation2D::load(const u8*& p_bufferOUT, size_t& p_sizeOUT, 
                               const DataTags& p_applyTags, const Tags& p_acceptedTags, 
                               code::ErrorStatus* p_errStatus)
{
	TT_ERR_CHAIN(bool, false, "Loading RotationAnimation2D binary");
	TT_ERR_ASSERTMSG(p_sizeOUT >= getBufferSize(), "Buffer too small, got " << p_sizeOUT << 
	                 " bytes, needs " << getBufferSize() << "\n");
	
	using namespace code::bufferutils;
	
	u16 version = be_get<u16>(p_bufferOUT, p_sizeOUT);
	TT_ERR_ASSERTMSG(version == s_version, "Invalid RotationAnimation2D version, code "
			<< GET_MAJOR_VERSION(s_version) << "." <<GET_MINOR_VERSION(s_version)<<", data "
			<< GET_MAJOR_VERSION(version)   << "." <<GET_MINOR_VERSION(version)  <<
			", Please update your presentation converter");
	
	pres::PresentationValue begin;
	pres::PresentationValue end;
	
	begin.load(p_bufferOUT, p_sizeOUT, &errStatus);
	end.load(p_bufferOUT, p_sizeOUT, &errStatus);
	
	TT_ERR_RETURN_ON_ERROR();
	
	setBeginAndEndRange(begin, end);
	
	return Animation2D::load(p_bufferOUT, p_sizeOUT, p_applyTags, p_acceptedTags, &errStatus);
}


bool RotationAnimation2D::save(u8*& p_bufferOUT, size_t& p_sizeOUT, code::ErrorStatus* p_errStatus) const
{
	TT_ERR_CHAIN(bool, false, "saving RotationAnimation2D binary");
	TT_ERR_ASSERTMSG(p_sizeOUT >= getBufferSize(), "Buffer too small, got " << p_sizeOUT << 
	                 " bytes, needs " << getBufferSize() << "\n");
	
	using namespace code::bufferutils;
	be_put(s_version, p_bufferOUT, p_sizeOUT);
	
	m_begin.save(p_bufferOUT, p_sizeOUT, &errStatus);
	m_end.save(p_bufferOUT, p_sizeOUT, &errStatus);
	
	TT_ERR_RETURN_ON_ERROR();
	
	return Animation2D::save(p_bufferOUT, p_sizeOUT, &errStatus);
}


size_t RotationAnimation2D::getBufferSize() const
{
	return s_size + Animation2D::getBufferSize() + m_begin.getBufferSize() + m_end.getBufferSize();
}


void RotationAnimation2D::setRanges(PresentationObject* p_presObj)
{
	Animation2D::setRanges(p_presObj);
	m_begin.updateValue(p_presObj);
	m_end.updateValue(p_presObj);
	
	m_delta = math::degToRad(m_end - m_begin);
	m_beginAngle = math::degToRad(m_begin);
}


RotationAnimation2D::RotationAnimation2D(const RotationAnimation2D& p_rhs)
:
PositionAnimation2D(p_rhs),
m_begin(p_rhs.m_begin),
m_end(p_rhs.m_end),
m_delta(p_rhs.m_delta),
m_beginAngle(p_rhs.m_beginAngle)
{
}

//namespace end
}
}
}
