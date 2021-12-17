#if !defined(INC_TT_XML_UTIL_PARSE_H)
#define INC_TT_XML_UTIL_PARSE_H

#include <tt/code/OptionalValue.h>
#include <tt/math/Rect.h> // FIXME: forward declaration of PointRect needed.

// Forward declaration
namespace tt
{
	namespace code
	{
		class ErrorStatus;
	}
	
	namespace engine
	{
		namespace renderer
		{
			struct ColorRGB;
			struct ColorRGBA;
		}
	}
	
	namespace math
	{
		class Range;
		class Vector2;
		class Vector3;
		class Vector4;
		class Point2;
		class Point3;
		class Quaternion;
	}
	
	namespace xml
	{
		class XmlNode;
	}
}


namespace tt {
namespace xml {
namespace util {


// Several helpers for parsing attributes.

bool parseBool(const XmlNode* p_node, const std::string& p_attribute, code::ErrorStatus* p_errStatus);
s32  parseS32( const XmlNode* p_node, const std::string& p_attribute, code::ErrorStatus* p_errStatus);
s16  parseS16( const XmlNode* p_node, const std::string& p_attribute, code::ErrorStatus* p_errStatus);
s8   parseS8(  const XmlNode* p_node, const std::string& p_attribute, code::ErrorStatus* p_errStatus);
u32  parseU32( const XmlNode* p_node, const std::string& p_attribute, code::ErrorStatus* p_errStatus);
u16  parseU16( const XmlNode* p_node, const std::string& p_attribute, code::ErrorStatus* p_errStatus);
u8   parseU8(  const XmlNode* p_node, const std::string& p_attribute, code::ErrorStatus* p_errStatus);
real parseReal(const XmlNode* p_node, const std::string& p_attribute, code::ErrorStatus* p_errStatus);
std::string parseStr(const XmlNode* p_node, const std::string& p_attribute, code::ErrorStatus* p_errStatus);
math::Range parseRange(const XmlNode* p_node, const std::string& p_attribute, code::ErrorStatus* p_errStatus);

// Several Helpers for parsing optional attributes.

tt::code::OptionalValue<bool> parseOptionalBool(const XmlNode* p_node, const std::string& p_attribute, code::ErrorStatus* p_errStatus);
tt::code::OptionalValue<s32>  parseOptionalS32( const XmlNode* p_node, const std::string& p_attribute, code::ErrorStatus* p_errStatus);
tt::code::OptionalValue<s16>  parseOptionalS16( const XmlNode* p_node, const std::string& p_attribute, code::ErrorStatus* p_errStatus);
tt::code::OptionalValue<s8>   parseOptionalS8(  const XmlNode* p_node, const std::string& p_attribute, code::ErrorStatus* p_errStatus);
tt::code::OptionalValue<u32>  parseOptionalU32( const XmlNode* p_node, const std::string& p_attribute, code::ErrorStatus* p_errStatus);
tt::code::OptionalValue<u16>  parseOptionalU16( const XmlNode* p_node, const std::string& p_attribute, code::ErrorStatus* p_errStatus);
tt::code::OptionalValue<u8>   parseOptionalU8(  const XmlNode* p_node, const std::string& p_attribute, code::ErrorStatus* p_errStatus);
tt::code::OptionalValue<real> parseOptionalReal(const XmlNode* p_node, const std::string& p_attribute, code::ErrorStatus* p_errStatus);
tt::code::OptionalValue<math::Range> parseOptionalRange(const XmlNode* p_node, const std::string& p_attribute, code::ErrorStatus* p_errStatus);
tt::code::OptionalValue<std::string> parseOptionalStr(const XmlNode* p_node, const std::string& p_attribute, code::ErrorStatus* p_errStatus);


// Several helpers for parsing tt classes.

engine::renderer::ColorRGB  parseColorRGB (const XmlNode* p_node, code::ErrorStatus* p_errStatus);
engine::renderer::ColorRGBA parseColorRGBA(const XmlNode* p_node, code::ErrorStatus* p_errStatus);
math::Vector2 parseVector2(const XmlNode* p_node, code::ErrorStatus* p_errStatus);
math::Vector3 parseVector3(const XmlNode* p_node, code::ErrorStatus* p_errStatus);
math::Vector4 parseVector4(const XmlNode* p_node, code::ErrorStatus* p_errStatus);
math::Point2  parsePoint2( const XmlNode* p_node, code::ErrorStatus* p_errStatus);
math::Point3  parsePoint3( const XmlNode* p_node, code::ErrorStatus* p_errStatus);
math::Quaternion parseQuaternion(const XmlNode* p_node, code::ErrorStatus* p_errStatus);
math::PointRect parsePointRect(const XmlNode* p_node, code::ErrorStatus* p_errStatus);

// Several helpers for parsing optional tt classes.

tt::code::OptionalValue<engine::renderer::ColorRGB>  parseOptionalColorRGB (const XmlNode* p_node, const std::string& p_nodeName, code::ErrorStatus* p_errStatus);
tt::code::OptionalValue<engine::renderer::ColorRGBA> parseOptionalColorRGBA(const XmlNode* p_node, const std::string& p_nodeName, code::ErrorStatus* p_errStatus);
tt::code::OptionalValue<math::Vector2> parseOptionalVector2(const XmlNode* p_node, const std::string& p_nodeName, code::ErrorStatus* p_errStatus);
tt::code::OptionalValue<math::Vector3> parseOptionalVector3(const XmlNode* p_node, const std::string& p_nodeName, code::ErrorStatus* p_errStatus);
tt::code::OptionalValue<math::Vector4> parseOptionalVector4(const XmlNode* p_node, const std::string& p_nodeName, code::ErrorStatus* p_errStatus);
tt::code::OptionalValue<math::Point2>  parseOptionalPoint2( const XmlNode* p_node, const std::string& p_nodeName, code::ErrorStatus* p_errStatus);
tt::code::OptionalValue<math::Point3>  parseOptionalPoint3( const XmlNode* p_node, const std::string& p_nodeName, code::ErrorStatus* p_errStatus);
tt::code::OptionalValue<math::Quaternion> parseOptionalQuaternion(const XmlNode* p_node, const std::string& p_nodeName, code::ErrorStatus* p_errStatus);



// Namespace end
}
}
}

#endif // !defined(INC_TT_XML_UTIL_PARSE_H)
