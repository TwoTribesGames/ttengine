#include <tt/code/bufferutils.h>
#include <tt/code/ErrorStatus.h>
#include <tt/pres/anim2d/ColorAnimation2D.h>
#include <tt/pres/PresentationObject.h>
#include <tt/fs/File.h>
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
static const size_t s_size = 2; // version


ColorAnimation2D::ColorAnimation2D()
:
Animation2D(),
m_delta(0.0f, 0.0f, 0.0f, 0.0f),
m_begin(0.0f, 0.0f, 0.0f, 0.0f),
m_beginR(0.0f),
m_beginG(0.0f),
m_beginB(0.0f),
m_beginA(0.0f),
m_endR(0.0f),
m_endG(0.0f),
m_endB(0.0f),
m_endA(0.0f)
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


void ColorAnimation2D::setBeginAndEndRange(const pres::PresentationValue& p_beginR, const pres::PresentationValue& p_beginG, const pres::PresentationValue& p_beginB, const pres::PresentationValue& p_beginA,
	                                       const pres::PresentationValue& p_endR,   const pres::PresentationValue& p_endG,   const pres::PresentationValue& p_endB,   const pres::PresentationValue& p_endA)
{
	m_beginR = p_beginR;
	m_beginG = p_beginG;
	m_beginB = p_beginB;
	m_beginA = p_beginA;
	m_endR   = p_endR;
	m_endG   = p_endG;
	m_endB   = p_endB;
	m_endA   = p_endA;
	setRanges(0);
}


bool ColorAnimation2D::load(const xml::XmlNode* p_node, const DataTags& p_applyTags, 
                            const Tags& p_acceptedTags, code::ErrorStatus* p_errStatus )
{
	TT_ERR_CHAIN(bool, false, "ColorAnimation2D load from xml");
	TT_ERR_NULL_ASSERT(p_node);
	
	const xml::XmlNode* begin = p_node->getFirstChild("begin");
	TT_ERR_ASSERTMSG(begin != 0, "Expected child 'begin' in node '" << p_node->getName() << "'");
	
	pres::PresentationValue beginR(0);
	pres::PresentationValue beginG(0);
	pres::PresentationValue beginB(0);
	pres::PresentationValue beginA(0);
	
	std::string beginRStr(begin->getAttribute("r"));
	std::string beginGStr(begin->getAttribute("g"));
	std::string beginBStr(begin->getAttribute("b"));
	std::string beginAStr(begin->getAttribute("a"));
	
	if (beginRStr.empty() == false) beginR.resetValue(beginRStr, &errStatus);
	if (beginGStr.empty() == false) beginG.resetValue(beginGStr, &errStatus);
	if (beginBStr.empty() == false) beginB.resetValue(beginBStr, &errStatus);
	if (beginAStr.empty() == false) beginA.resetValue(beginAStr, &errStatus);
	
	TT_ERR_RETURN_ON_ERROR();
	
	const xml::XmlNode* end = p_node->getFirstChild("end");
	TT_ERR_ASSERTMSG(end != 0, "Expected child 'end' in node '" << p_node->getName() << "'");
	
	pres::PresentationValue endR(0);
	pres::PresentationValue endG(0);
	pres::PresentationValue endB(0);
	pres::PresentationValue endA(0);
	
	const std::string& endRStr(end->getAttribute("r"));
	const std::string& endGStr(end->getAttribute("g"));
	const std::string& endBStr(end->getAttribute("b"));
	const std::string& endAStr(end->getAttribute("a"));
	
	if (endRStr.empty() == false) endR.resetValue(endRStr, &errStatus);
	if (endGStr.empty() == false) endG.resetValue(endGStr, &errStatus);
	if (endBStr.empty() == false) endB.resetValue(endBStr, &errStatus);
	if (endAStr.empty() == false) endA.resetValue(endAStr, &errStatus);
	
	TT_ERR_RETURN_ON_ERROR();
	
	Animation2D::load(p_node, p_applyTags, p_acceptedTags, &errStatus);
	
	TT_ERR_RETURN_ON_ERROR();
	
	setBeginAndEndRange(beginR, beginG, beginB, beginA, endR, endG, endB, endA);
	
	return true;
}


bool ColorAnimation2D::load(const u8*& p_bufferOUT, size_t& p_sizeOUT, const DataTags& p_applyTags,
                            const Tags& p_acceptedTags, code::ErrorStatus* p_errStatus )
{
	TT_ERR_CHAIN(bool, false, "Loading ColorAnimation2D binary");
	TT_ERR_ASSERTMSG(p_sizeOUT >= getBufferSize(), "Buffer too small, got " << p_sizeOUT << 
	                 " bytes, needs " << getBufferSize() << "\n");
	
	using namespace code::bufferutils;
	
	u16 version = be_get<u16>(p_bufferOUT, p_sizeOUT);
	TT_ERR_ASSERTMSG(version == s_version, "Invalid ColorAnimation2D version, code "
			<< GET_MAJOR_VERSION(s_version) << "." <<GET_MINOR_VERSION(s_version)<<", data "
			<< GET_MAJOR_VERSION(version)   << "." <<GET_MINOR_VERSION(version)  <<
			", Please update your presentation converter");
	
	pres::PresentationValue beginR;
	pres::PresentationValue beginG;
	pres::PresentationValue beginB;
	pres::PresentationValue beginA;
	pres::PresentationValue endR;
	pres::PresentationValue endG;
	pres::PresentationValue endB;
	pres::PresentationValue endA;
	
	
	beginR.load(p_bufferOUT, p_sizeOUT, &errStatus);
	beginG.load(p_bufferOUT, p_sizeOUT, &errStatus);
	beginB.load(p_bufferOUT, p_sizeOUT, &errStatus);
	beginA.load(p_bufferOUT, p_sizeOUT, &errStatus);
	endR.load(p_bufferOUT, p_sizeOUT, &errStatus);
	endG.load(p_bufferOUT, p_sizeOUT, &errStatus);
	endB.load(p_bufferOUT, p_sizeOUT, &errStatus);
	endA.load(p_bufferOUT, p_sizeOUT, &errStatus);
	
	TT_ERR_RETURN_ON_ERROR();
	
	setBeginAndEndRange(beginR, beginG, beginB, beginA, endR, endG, endB, endA);
	
	return Animation2D::load(p_bufferOUT, p_sizeOUT, p_applyTags, p_acceptedTags, &errStatus);
}


bool ColorAnimation2D::save(u8*& p_bufferOUT, size_t& p_sizeOUT, code::ErrorStatus* p_errStatus) const
{
	TT_ERR_CHAIN(bool, false, "saving ColorAnimation2D binary");
	TT_ERR_ASSERTMSG(p_sizeOUT >= getBufferSize(), "Buffer too small, got " << p_sizeOUT << 
	                 " bytes, needs " << getBufferSize() << "\n");
	
	using namespace code::bufferutils;
	be_put(s_version, p_bufferOUT, p_sizeOUT);
	
	m_beginR.save(p_bufferOUT, p_sizeOUT, &errStatus);
	m_beginG.save(p_bufferOUT, p_sizeOUT, &errStatus);
	m_beginB.save(p_bufferOUT, p_sizeOUT, &errStatus);
	m_beginA.save(p_bufferOUT, p_sizeOUT, &errStatus);
	m_endR.save(p_bufferOUT, p_sizeOUT, &errStatus);
	m_endG.save(p_bufferOUT, p_sizeOUT, &errStatus);
	m_endB.save(p_bufferOUT, p_sizeOUT, &errStatus);
	m_endA.save(p_bufferOUT, p_sizeOUT, &errStatus);
	
	TT_ERR_RETURN_ON_ERROR();
	
	return Animation2D::save(p_bufferOUT, p_sizeOUT, &errStatus);
}


size_t ColorAnimation2D::getBufferSize() const
{
	return s_size + Animation2D::getBufferSize() + 
	       m_beginR.getBufferSize() +
	       m_beginG.getBufferSize() +
	       m_beginB.getBufferSize() +
	       m_beginA.getBufferSize() +
	       m_endR.getBufferSize() +
	       m_endG.getBufferSize() +
	       m_endB.getBufferSize() +
	       m_endA.getBufferSize();
}


void ColorAnimation2D::setRanges(PresentationObject* p_presObj)
{
	Animation2D::setRanges(p_presObj);
	
	m_beginR.updateValue(p_presObj);
	m_beginG.updateValue(p_presObj);
	m_beginB.updateValue(p_presObj);
	m_beginA.updateValue(p_presObj);
	m_endR.updateValue(p_presObj);
	m_endG.updateValue(p_presObj);
	m_endB.updateValue(p_presObj);
	m_endA.updateValue(p_presObj);
	
	m_begin = math::Vector4(m_beginR, m_beginG, m_beginB, m_beginA) / 255.0f;
	m_delta = math::Vector4(m_endR, m_endG, m_endB, m_endA) / 255.0f - m_begin;
	
}


ColorAnimation2D::ColorAnimation2D(const ColorAnimation2D& p_rhs)
:
Animation2D(p_rhs),
m_delta(p_rhs.m_delta),
m_begin(p_rhs.m_begin),
m_beginR(p_rhs.m_beginR),
m_beginG(p_rhs.m_beginG),
m_beginB(p_rhs.m_beginB),
m_beginA(p_rhs.m_beginA),
m_endR(p_rhs.m_endR),
m_endG(p_rhs.m_endG),
m_endB(p_rhs.m_endB),
m_endA(p_rhs.m_endA)
{
}


//namespace end
}
}
}
