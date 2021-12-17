#include <tt/code/bufferutils.h>
#include <tt/code/ErrorStatus.h>
#include <tt/pres/anim2d/TranslationAnimation2D.h>
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

static const u16 s_version = MAKE_VERSION(1, 5);
static const size_t s_size = 2; // version
static const size_t s_sizeg = 2; // version

bool TranslationAnimation2D::ms_invertY = false;

TranslationAnimation2D::TranslationAnimation2D(bool p_isGameTranslation)
:
PositionAnimation2D(),
m_delta(math::Vector3::zero),
m_begin(math::Vector3::zero),
m_beginX(0.0f),
m_beginY(0.0f),
m_beginZ(0.0f),
m_endX(0.0f),
m_endY(0.0f),
m_endZ(0.0f),
m_rangesSet(false),
m_isGameTranslation(p_isGameTranslation)
{
}


void TranslationAnimation2D::setInvertYAxis(bool p_invert)
{
	ms_invertY = p_invert;
}


bool TranslationAnimation2D::getInvertYAxis()
{
	return ms_invertY;
}


void TranslationAnimation2D::applyTransform(Transform* p_transform) const
{

	TT_ASSERTMSG(m_rangesSet,"Ranges not set yet, "
		" Set the ranges for the TranslationAnimation2D before calling getTransform");
	
	p_transform->translation += (m_begin + (m_delta * getTime()));
}


bool TranslationAnimation2D::hasZAnimation() const
{
	return m_beginZ.getMin() != 0.0f ||
	       m_beginZ.getMax() != 0.0f ||
	       m_endZ.getMin() != 0.0f ||
	       m_endZ.getMax() != 0.0f;
}


int TranslationAnimation2D::getSortWeight() const
{
	return 1;
}


void TranslationAnimation2D::setBeginAndEndRange(const pres::PresentationValue& p_beginX, const pres::PresentationValue& p_beginY, const pres::PresentationValue& p_beginZ,
	                                             const pres::PresentationValue& p_endX,   const pres::PresentationValue& p_endY,   const pres::PresentationValue& p_endZ)
{
	m_beginX = p_beginX;
	m_beginY = p_beginY;
	m_beginZ = p_beginZ;
	m_endX = p_endX;
	m_endY = p_endY;
	m_endZ = p_endZ;
	setRanges(0);
}


void TranslationAnimation2D::setBeginAndEnd(const math::Vector3& p_begin, const math::Vector3& p_end)
{
	m_beginX.setValue(p_begin.x);
	m_beginY.setValue(p_begin.y);
	m_beginZ.setValue(p_begin.z);
	m_endX.setValue(p_end.x);
	m_endY.setValue(p_end.y);
	m_endZ.setValue(p_end.z);
	setRanges(0);
}


bool TranslationAnimation2D::load(const xml::XmlNode* p_node, 
                                  const DataTags& p_applyTags, 
                                  const Tags& p_acceptedTags, code::ErrorStatus* p_errStatus)
{
	TT_ERR_CHAIN(bool, false, "TranslationAnimation2D load from xml");
	TT_ERR_NULL_ASSERT(p_node);
	
	if(p_node->getFirstChild("begin") != 0)
	{
		const xml::XmlNode* begin = p_node->getFirstChild("begin");
		
		TT_ERR_ASSERTMSG(begin != 0, "Expected 'begin' node after 'translation' node.");
		
		pres::PresentationValue beginX(pres::parseOptionalPresentationValue(begin, "x", 0.0f, &errStatus));
		pres::PresentationValue beginY(pres::parseOptionalPresentationValue(begin, "y", 0.0f, &errStatus));
		pres::PresentationValue beginZ(pres::parseOptionalPresentationValue(begin, "z", 0.0f, &errStatus));
		
		const xml::XmlNode* end = p_node->getFirstChild("end");
		TT_ERR_ASSERTMSG(end != 0, "Expected 'end' node after 'translation' node.");
		
		pres::PresentationValue endX(pres::parseOptionalPresentationValue(end, "x", 0.0f, &errStatus));
		pres::PresentationValue endY(pres::parseOptionalPresentationValue(end, "y", 0.0f, &errStatus));
		pres::PresentationValue endZ(pres::parseOptionalPresentationValue(end, "z", 0.0f, &errStatus));
		
		TT_ERR_RETURN_ON_ERROR();
		
		if (ms_invertY)
		{
			beginY.invertValue();
			endY.invertValue();
		}
		
		Animation2D::load(p_node, p_applyTags, p_acceptedTags, &errStatus);
		TT_ERR_RETURN_ON_ERROR();
		
		setBeginAndEndRange(beginX, beginY, beginZ, endX, endY, endZ);
		m_rangesSet = true;
		m_isGameTranslation = false;
	}
	else
	{
		Animation2D::load(p_node, p_applyTags, p_acceptedTags, &errStatus);
		TT_ERR_RETURN_ON_ERROR();
		
		m_rangesSet = false;
		m_isGameTranslation = true;
	}
	
	return true;
}


bool TranslationAnimation2D::loadStatic(const xml::XmlNode* p_node,
                                        const DataTags& p_applyTags,
                                        const Tags& p_acceptedTags,
                                        code::ErrorStatus* p_errStatus)
{
	TT_ERR_CHAIN(bool, false, "Static Translation load from xml");
	TT_ERR_NULL_ASSERT(p_node);
	TT_ERR_ASSERTMSG(p_node->getName() == "translation" || p_node->getName() == "origin" , 
	                 "Expected node name 'translation' ort 'origin' got '" << 
	                 p_node->getName() << "'");
	
	pres::PresentationValue x(pres::parseOptionalPresentationValue(p_node, "x", 0.0f, &errStatus));
	pres::PresentationValue y(pres::parseOptionalPresentationValue(p_node, "y", 0.0f, &errStatus));
	pres::PresentationValue z(pres::parseOptionalPresentationValue(p_node, "z", 0.0f, &errStatus));
	
	if (ms_invertY)
	{
		y.invertValue();
	}
	
	Animation2D::loadStatic(p_node, p_applyTags, p_acceptedTags, &errStatus);
	TT_ERR_RETURN_ON_ERROR();
	
	setBeginAndEndRange(x, y, z, x, y, z);
	m_rangesSet = true;
	m_isGameTranslation = false;
	
	return true;
}


bool TranslationAnimation2D::load( const u8*& p_bufferOUT, size_t& p_sizeOUT, 
                                   const DataTags& p_applyTags, const Tags& p_acceptedTags, 
                                   code::ErrorStatus* p_errStatus )
{
	TT_ERR_CHAIN(bool, false, "Loading TranslationAnimation2D binary");
	
	using namespace code::bufferutils;
	
	const u16 version = be_get<u16>(p_bufferOUT, p_sizeOUT);
	TT_ERR_ASSERTMSG(version == s_version, "Invalid TranslationAnimation2D version: code "
			<< GET_MAJOR_VERSION(s_version) << "." << GET_MINOR_VERSION(s_version) << ", data "
			<< GET_MAJOR_VERSION(version)   << "." << GET_MINOR_VERSION(version)   <<
			" -- please update your presentation converter.");
	
	if (m_isGameTranslation == false)
	{
		pres::PresentationValue beginX;
		pres::PresentationValue beginY;
		pres::PresentationValue beginZ;
		pres::PresentationValue endX;
		pres::PresentationValue endY;
		pres::PresentationValue endZ;
		
		beginX.load(p_bufferOUT, p_sizeOUT, &errStatus);
		beginY.load(p_bufferOUT, p_sizeOUT, &errStatus);
		beginZ.load(p_bufferOUT, p_sizeOUT, &errStatus);
		endX.load(p_bufferOUT, p_sizeOUT, &errStatus);
		endY.load(p_bufferOUT, p_sizeOUT, &errStatus);
		endZ.load(p_bufferOUT, p_sizeOUT, &errStatus);
		
		TT_ERR_RETURN_ON_ERROR();
		
		if (ms_invertY)
		{
			beginY.invertValue();
			endY.invertValue();
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
	
	TT_ERR_RETURN_ON_ERROR();
	return Animation2D::load(p_bufferOUT, p_sizeOUT, p_applyTags, p_acceptedTags, &errStatus);
}


bool TranslationAnimation2D::save(u8*& p_bufferOUT, size_t& p_sizeOUT,
                                  code::ErrorStatus* p_errStatus) const
{
	TT_ERR_CHAIN(bool, false, "saving TranslationAnimation2D binary");
	TT_ERR_ASSERTMSG(p_sizeOUT >= getBufferSize(), "Buffer too small, got " << p_sizeOUT << 
	                 " bytes, needs " << getBufferSize() << "\n");
	
	using namespace code::bufferutils;
	be_put(s_version, p_bufferOUT, p_sizeOUT);

	if(m_isGameTranslation == false)
	{
		
		m_beginX.save(p_bufferOUT, p_sizeOUT, &errStatus);
		m_beginY.save(p_bufferOUT, p_sizeOUT, &errStatus);
		m_beginZ.save(p_bufferOUT, p_sizeOUT, &errStatus);
		m_endX.save(p_bufferOUT, p_sizeOUT, &errStatus);
		m_endY.save(p_bufferOUT, p_sizeOUT, &errStatus);
		m_endZ.save(p_bufferOUT, p_sizeOUT, &errStatus);
		
		TT_ERR_RETURN_ON_ERROR();
	}
	
	size_t beforeSize = p_sizeOUT;
	
	Animation2D::save(p_bufferOUT, p_sizeOUT, &errStatus);
	TT_ERR_RETURN_ON_ERROR();
	
	size_t neededbuffersize = Animation2D::getBufferSize();
	size_t usedbuffersize = beforeSize - p_sizeOUT;
	TT_ASSERT(neededbuffersize == usedbuffersize);
	
	return true;
}


size_t TranslationAnimation2D::getBufferSize() const
{
	if(m_isGameTranslation)
	{
		return s_sizeg + Animation2D::getBufferSize();
	}
	else
	{
		return s_size + Animation2D::getBufferSize() + 
		       m_beginX.getBufferSize() + m_beginY.getBufferSize() + m_beginZ.getBufferSize() +
		       m_endX.getBufferSize()   + m_endY.getBufferSize()   + m_endZ.getBufferSize();
	}
}


void TranslationAnimation2D::setRanges(PresentationObject* p_presObj)
{
	Animation2D::setRanges(p_presObj);
	
	m_beginX.updateValue(p_presObj);
	m_beginY.updateValue(p_presObj);
	m_beginZ.updateValue(p_presObj);
	m_endX.updateValue(p_presObj);
	m_endY.updateValue(p_presObj);
	m_endZ.updateValue(p_presObj);
	
	m_delta = getEnd() - getBegin();
	m_begin = getBegin();
	
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
m_delta(p_rhs.m_delta),
m_begin(p_rhs.m_begin),
m_beginX(p_rhs.m_beginX),
m_beginY(p_rhs.m_beginY),
m_beginZ(p_rhs.m_beginZ),
m_endX(p_rhs.m_endX),
m_endY(p_rhs.m_endY),
m_endZ(p_rhs.m_endZ),
m_rangesSet(p_rhs.m_rangesSet),
m_isGameTranslation(p_rhs.m_isGameTranslation)
{
}

//namespace end
}
}
}
