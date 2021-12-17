#include <tt/code/ErrorStatus.h>
#include <tt/engine/renderer/ColorRGB.h>
#include <tt/engine/renderer/ColorRGBA.h>
#include <tt/math/Vector2.h>
#include <tt/math/Vector3.h>
#include <tt/math/Vector4.h>
#include <tt/math/Point2.h>
#include <tt/math/Point3.h>
#include <tt/math/Range.h>
#include <tt/math/Rect.h>
#include <tt/math/Quaternion.h>
#include <tt/platform/tt_error.h>
#include <tt/str/parse.h>
#include <tt/xml/XmlNode.h>

#include <tt/xml/util/parse.h>


namespace tt {
namespace xml {
namespace util {


/* \brief "private" helper function for xml attribute parsing. */
template<class ReturnType>
static ReturnType parse(const XmlNode* p_node, const std::string& p_attribute, 
                        code::ErrorStatus* p_errStatus,
                        ReturnType p_parseFun(const std::string&, code::ErrorStatus*))
{
	TT_ERR_CHAIN(ReturnType, ReturnType(0), "parsing attribute '" << p_attribute << "'");
	TT_ERR_ASSERTMSG(p_node != 0, "Null XML node pointer passed.");
	
	TT_ERR_ADD_LOC(" from node '" << p_node->getName() << "'");
	
	TT_ERR_ASSERTMSG(p_node->hasAttribute(p_attribute),
	                 "Expected attribute '" << p_attribute << "' in node '" << p_node->getName() << "'");
	
	return p_parseFun(p_node->getAttribute(p_attribute), &errStatus);
}


bool parseBool(const XmlNode* p_node, const std::string& p_attribute, code::ErrorStatus* p_errStatus)
{
	return parse(p_node, p_attribute, p_errStatus, str::parseBool);
}


s32  parseS32( const XmlNode* p_node, const std::string& p_attribute, code::ErrorStatus* p_errStatus)
{
	return parse(p_node, p_attribute, p_errStatus, str::parseS32);
}


s16  parseS16( const XmlNode* p_node, const std::string& p_attribute, code::ErrorStatus* p_errStatus)
{
	return parse(p_node, p_attribute, p_errStatus, str::parseS16);
}


s8   parseS8(  const XmlNode* p_node, const std::string& p_attribute, code::ErrorStatus* p_errStatus)
{
	return parse(p_node, p_attribute, p_errStatus, str::parseS8);
}


u32  parseU32( const XmlNode* p_node, const std::string& p_attribute, code::ErrorStatus* p_errStatus)
{
	return parse(p_node, p_attribute, p_errStatus, str::parseU32);
}


u16  parseU16( const XmlNode* p_node, const std::string& p_attribute, code::ErrorStatus* p_errStatus)
{
	return parse(p_node, p_attribute, p_errStatus, str::parseU16);
}


u8   parseU8(  const XmlNode* p_node, const std::string& p_attribute, code::ErrorStatus* p_errStatus)
{
	return parse(p_node, p_attribute, p_errStatus, str::parseU8);
}


real parseReal(const XmlNode* p_node, const std::string& p_attribute, code::ErrorStatus* p_errStatus)
{
	return parse(p_node, p_attribute, p_errStatus, str::parseReal);
}


math::Range parseRange(const XmlNode* p_node, const std::string& p_attribute, code::ErrorStatus* p_errStatus)
{
	return parse(p_node, p_attribute, p_errStatus, str::parseRange);
}


std::string parseStr(const XmlNode* p_node, const std::string& p_attribute, code::ErrorStatus* p_errStatus)
{
	TT_ERR_CHAIN(std::string, std::string(), "parsing attribute '" << p_attribute << "'");
	TT_ERR_ASSERTMSG(p_node != 0, "Null XML node pointer passed.");
	
	TT_ERR_ADD_LOC(" from node '" << p_node->getName() << "'");
	
	TT_ERR_ASSERTMSG(p_node->hasAttribute(p_attribute),
	                 "Expected attribute '" << p_attribute << "' in node '" << p_node->getName() << "'");
	
	return p_node->getAttribute(p_attribute);
}


/* \brief "private" helper function for parsing of nodes.
   \note Type needs a default constructor! */
template<class Type>
static Type parseNode(const XmlNode* p_node, code::ErrorStatus& p_errStatus,
                      Type p_parseFun(const XmlNode* p_node, code::ErrorStatus& p_errStatus))
{
	if (p_errStatus.hasError())
	{
		return Type();
	}
	
	if (p_node == 0)
	{
		TT_ERROR_NAME(p_errStatus, "p_node is null!");
		return Type();
	}
	
	Type ret(p_parseFun(p_node, p_errStatus));
	
	if (p_errStatus.hasError())
	{
		return Type();
	}
	return ret;
}


tt::code::OptionalValue<bool> parseOptionalBool(const XmlNode* p_node,
                                           const std::string& p_attribute, 
                                           code::ErrorStatus* p_errStatus)
{
	if (p_node == 0 || p_node->hasAttribute(p_attribute) == false)
	{
		return tt::code::OptionalValue<bool>();
	}
	return parseBool(p_node, p_attribute, p_errStatus);
}


tt::code::OptionalValue<s32>  parseOptionalS32(const XmlNode* p_node,
                                          const std::string& p_attribute,
                                          code::ErrorStatus* p_errStatus)
{
	if (p_node == 0 || p_node->hasAttribute(p_attribute) == false)
	{
		return tt::code::OptionalValue<s32>();
	}
	return parseS32(p_node, p_attribute, p_errStatus);
}


tt::code::OptionalValue<s16>  parseOptionalS16( const XmlNode* p_node,
                                           const std::string& p_attribute,
                                           code::ErrorStatus* p_errStatus)
{
	if (p_node == 0 || p_node->hasAttribute(p_attribute) == false)
	{
		return tt::code::OptionalValue<s16>();
	}
	return parseS16(p_node, p_attribute, p_errStatus);
}


tt::code::OptionalValue<s8>   parseOptionalS8(const XmlNode* p_node,
                                         const std::string& p_attribute,
                                         code::ErrorStatus* p_errStatus)
{
	if (p_node == 0 || p_node->hasAttribute(p_attribute) == false)
	{
		return tt::code::OptionalValue<s8>();
	}
	return parseS8(p_node, p_attribute, p_errStatus);
}


tt::code::OptionalValue<u32>  parseOptionalU32( const XmlNode* p_node,
                                           const std::string& p_attribute,
                                           code::ErrorStatus* p_errStatus)
{
	if (p_node == 0 || p_node->hasAttribute(p_attribute) == false)
	{
		return tt::code::OptionalValue<u32>();
	}
	return parseU32(p_node, p_attribute, p_errStatus);
}


tt::code::OptionalValue<u16>  parseOptionalU16( const XmlNode* p_node,
                                           const std::string& p_attribute,
                                           code::ErrorStatus* p_errStatus)
{
	if (p_node == 0 || p_node->hasAttribute(p_attribute) == false)
	{
		return tt::code::OptionalValue<u16>();
	}
	return parseU16(p_node, p_attribute, p_errStatus);
}


tt::code::OptionalValue<u8>   parseOptionalU8(  const XmlNode* p_node,
                                           const std::string& p_attribute,
                                           code::ErrorStatus* p_errStatus)
{
	if (p_node == 0 || p_node->hasAttribute(p_attribute) == false)
	{
		return tt::code::OptionalValue<u8>();
	}
	return parseU8(p_node, p_attribute, p_errStatus);
}


tt::code::OptionalValue<real> parseOptionalReal(const XmlNode* p_node,
                                           const std::string& p_attribute,
                                           code::ErrorStatus* p_errStatus)
{
	if (p_node == 0 || p_node->hasAttribute(p_attribute) == false)
	{
		return tt::code::OptionalValue<real>();
	}
	return parseReal(p_node, p_attribute, p_errStatus);
}


tt::code::OptionalValue<math::Range> parseOptionalRange(const XmlNode* p_node,
                                           const std::string& p_attribute,
                                           code::ErrorStatus* p_errStatus)
{
	if (p_node == 0 || p_node->hasAttribute(p_attribute) == false)
	{
		return tt::code::OptionalValue<math::Range>();
	}
	return parseRange(p_node, p_attribute, p_errStatus);
}


tt::code::OptionalValue<std::string> parseOptionalStr(const XmlNode* p_node, 
                                                 const std::string& p_attribute,
                                                 code::ErrorStatus* p_errStatus)
{
	if (p_node == 0 || p_node->hasAttribute(p_attribute) == false)
	{
		return tt::code::OptionalValue<std::string>();
	}
	return parseStr(p_node, p_attribute, p_errStatus);
}


static engine::renderer::ColorRGB parseColorRGBImpl(const XmlNode* p_node, code::ErrorStatus& p_errStatus)
{
	engine::renderer::ColorRGB ret;
	ret.r = parseU8(p_node, "r", &p_errStatus);
	ret.g = parseU8(p_node, "g", &p_errStatus);
	ret.b = parseU8(p_node, "b", &p_errStatus);
	return ret;
}


engine::renderer::ColorRGB parseColorRGB(const XmlNode* p_node, code::ErrorStatus* p_errStatus)
{
	TT_ERR_CHAIN(engine::renderer::ColorRGB, engine::renderer::ColorRGB(255, 255, 255), "parse ColorRGB");
	return parseNode(p_node, errStatus, parseColorRGBImpl);
}


static engine::renderer::ColorRGBA parseColorRGBAImpl(const XmlNode* p_node, code::ErrorStatus& p_errStatus)
{
	engine::renderer::ColorRGBA ret;
	ret.r = parseU8(p_node, "r", &p_errStatus);
	ret.g = parseU8(p_node, "g", &p_errStatus);
	ret.b = parseU8(p_node, "b", &p_errStatus);
	ret.a = parseU8(p_node, "a", &p_errStatus);
	return ret;
}


engine::renderer::ColorRGBA parseColorRGBA(const XmlNode* p_node, code::ErrorStatus* p_errStatus)
{
	TT_ERR_CHAIN(engine::renderer::ColorRGBA, engine::renderer::ColorRGBA(255, 255, 255, 255), "parse ColorRGBA");
	return parseNode(p_node, errStatus, parseColorRGBAImpl);
}


static math::Vector2 parseVector2Impl(const XmlNode* p_node, code::ErrorStatus& p_errStatus)
{
	math::Vector2 ret;
	ret.x = parseReal(p_node, "x", &p_errStatus);
	ret.y = parseReal(p_node, "y", &p_errStatus);
	return ret;
}


math::Vector2 parseVector2(const XmlNode* p_node, code::ErrorStatus* p_errStatus)
{
	TT_ERR_CHAIN(math::Vector2, math::Vector2(), "parse Vector2");
	return parseNode(p_node, errStatus, parseVector2Impl);
}



static math::Vector3 parseVector3Impl(const XmlNode* p_node, code::ErrorStatus& p_errStatus)
{
	math::Vector3 ret;
	ret.x = parseReal(p_node, "x", &p_errStatus);
	ret.y = parseReal(p_node, "y", &p_errStatus);
	ret.z = parseReal(p_node, "z", &p_errStatus);
	return ret;
}


math::Vector3 parseVector3(const XmlNode* p_node, code::ErrorStatus* p_errStatus)
{
	TT_ERR_CHAIN(math::Vector3, math::Vector3(), "parse Vector3");
	return parseNode(p_node, errStatus, parseVector3Impl);
}


static math::Vector4 parseVector4Impl(const XmlNode* p_node, code::ErrorStatus& p_errStatus)
{
	math::Vector4 ret;
	ret.x = parseReal(p_node, "x", &p_errStatus);
	ret.y = parseReal(p_node, "y", &p_errStatus);
	ret.z = parseReal(p_node, "z", &p_errStatus);
	ret.w = parseReal(p_node, "w", &p_errStatus);
	return ret;
}


math::Vector4 parseVector4(const XmlNode* p_node, code::ErrorStatus* p_errStatus)
{
	TT_ERR_CHAIN(math::Vector4, math::Vector4(), "parse Vector4");
	return parseNode(p_node, errStatus, parseVector4Impl);
}


static math::Point2 parsePoint2Impl(const XmlNode* p_node, code::ErrorStatus& p_errStatus)
{
	math::Point2 ret;
	ret.x = parseS32(p_node, "x", &p_errStatus);
	ret.y = parseS32(p_node, "y", &p_errStatus);
	return ret;
}


math::Point2 parsePoint2(const XmlNode* p_node, code::ErrorStatus* p_errStatus)
{
	TT_ERR_CHAIN(math::Point2, math::Point2(), "parse Point2");
	return parseNode(p_node, errStatus, parsePoint2Impl);
}


static math::Point3 parsePoint3Impl(const XmlNode* p_node, code::ErrorStatus& p_errStatus)
{
	math::Point3 ret;
	ret.x = parseS32(p_node, "x", &p_errStatus);
	ret.y = parseS32(p_node, "y", &p_errStatus);
	ret.z = parseS32(p_node, "z", &p_errStatus);
	return ret;
}


math::Point3 parsePoint3(const XmlNode* p_node, code::ErrorStatus* p_errStatus)
{
	TT_ERR_CHAIN(math::Point3, math::Point3(), "parse Point3");
	return parseNode(p_node, errStatus, parsePoint3Impl);
}


static math::Quaternion parseQuaternionImpl(const XmlNode* p_node, code::ErrorStatus& p_errStatus)
{
	math::Quaternion ret;
	ret.m_x = parseReal(p_node, "x", &p_errStatus);
	ret.m_y = parseReal(p_node, "y", &p_errStatus);
	ret.m_z = parseReal(p_node, "z", &p_errStatus);
	ret.m_w = parseReal(p_node, "w", &p_errStatus);
	return ret;
}


math::Quaternion parseQuaternion(const XmlNode* p_node, code::ErrorStatus* p_errStatus)
{
	TT_ERR_CHAIN(math::Quaternion, math::Quaternion(), "parse Quaternion");
	return parseNode(p_node, errStatus, parseQuaternionImpl);
}


math::PointRect parsePointRect(const XmlNode* p_node, code::ErrorStatus* p_errStatus)
{
	TT_ERR_CHAIN(math::PointRect, math::PointRect(), "Parse PointRect");
	
	math::Point2 position = parsePoint2(p_node, &errStatus);
	s32 width       = parseS32(p_node, "width", &errStatus);
	s32 height      = parseS32(p_node, "height", &errStatus);
	TT_ERR_RETURN_ON_ERROR();
	
	return math::PointRect(position, width, height);
}


tt::code::OptionalValue<engine::renderer::ColorRGBA> parseOptionalColorRGBA(const XmlNode* p_node,
                                                                       const std::string& p_nodeName,
                                                                       code::ErrorStatus* p_errStatus)
{
	if (p_node == 0 || p_node->getFirstChild(p_nodeName) == 0)
	{
		return tt::code::OptionalValue<engine::renderer::ColorRGBA>();
	}
	return parseColorRGBA(p_node->getFirstChild(p_nodeName), p_errStatus);
}


tt::code::OptionalValue<math::Vector2> parseOptionalVector2(const XmlNode* p_node,
                                                       const std::string& p_nodeName,
                                                       code::ErrorStatus* p_errStatus)
{
	if (p_node == 0 || p_node->getFirstChild(p_nodeName) == 0)
	{
		return tt::code::OptionalValue<math::Vector2>();
	}
	return parseVector2(p_node->getFirstChild(p_nodeName), p_errStatus);
}


tt::code::OptionalValue<math::Vector3> parseOptionalVector3(const XmlNode* p_node,
                                                       const std::string& p_nodeName,
                                                       code::ErrorStatus* p_errStatus)
{
	if (p_node == 0 || p_node->getFirstChild(p_nodeName) == 0)
	{
		return tt::code::OptionalValue<math::Vector3>();
	}
	return parseVector3(p_node->getFirstChild(p_nodeName), p_errStatus);
}


tt::code::OptionalValue<math::Vector4> parseOptionalVector4(const XmlNode* p_node,
                                                       const std::string& p_nodeName,
                                                       code::ErrorStatus* p_errStatus)
{
	if (p_node == 0 || p_node->getFirstChild(p_nodeName) == 0)
	{
		return tt::code::OptionalValue<math::Vector4>();
	}
	return parseVector4(p_node->getFirstChild(p_nodeName), p_errStatus);
}


tt::code::OptionalValue<math::Point2>  parseOptionalPoint2(const XmlNode* p_node,
                                                      const std::string& p_nodeName,
                                                      code::ErrorStatus* p_errStatus)
{
	if (p_node == 0 || p_node->getFirstChild(p_nodeName) == 0)
	{
		return tt::code::OptionalValue<math::Point2>();
	}
	return parsePoint2(p_node->getFirstChild(p_nodeName), p_errStatus);
}


tt::code::OptionalValue<math::Point3>  parseOptionalPoint3( const XmlNode* p_node,
                                                       const std::string& p_nodeName,
                                                       code::ErrorStatus* p_errStatus)
{
	if (p_node == 0 || p_node->getFirstChild(p_nodeName) == 0)
	{
		return tt::code::OptionalValue<math::Point3>();
	}
	return parsePoint3(p_node->getFirstChild(p_nodeName), p_errStatus);
}


tt::code::OptionalValue<math::Quaternion> parseOptionalQuaternion(const XmlNode* p_node,
                                                             const std::string& p_nodeName,
                                                             code::ErrorStatus* p_errStatus)
{
	if (p_node == 0 || p_node->getFirstChild(p_nodeName) == 0)
	{
		return tt::code::OptionalValue<math::Quaternion>();
	}
	return parseQuaternion(p_node->getFirstChild(p_nodeName), p_errStatus);
}

// Namespace end
}
}
}
