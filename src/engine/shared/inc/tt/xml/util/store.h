#if !defined(INC_TT_XML_UTIL_STORE_H)
#define INC_TT_XML_UTIL_STORE_H

#include <tt/code/DefaultValue.h>
#include <tt/code/OptionalValue.h>
#include <tt/math/Rect.h> // FIXME: forward declaration of PointRect needed.
#include <tt/str/toStr.h>
#include <tt/xml/XmlNode.h>

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
}


namespace tt {
namespace xml {
namespace util {


// Several helpers for storing tt classes.

void store(XmlNode* p_node, const engine::renderer::ColorRGB& p_value, code::ErrorStatus* p_errStatus);
void store(XmlNode* p_node, const engine::renderer::ColorRGBA& p_value, code::ErrorStatus* p_errStatus);
void store(XmlNode* p_node, const math::Vector2& p_value, code::ErrorStatus* p_errStatus);
void store(XmlNode* p_node, const math::Vector3& p_value, code::ErrorStatus* p_errStatus);
void store(XmlNode* p_node, const math::Vector4& p_value, code::ErrorStatus* p_errStatus);
void store(XmlNode* p_node, const math::Point2& p_value, code::ErrorStatus* p_errStatus);
void store(XmlNode* p_node, const math::Point3& p_value, code::ErrorStatus* p_errStatus);
void store(XmlNode* p_node, const math::Quaternion& p_value, code::ErrorStatus* p_errStatus);
void store(XmlNode* p_node, const math::PointRect& p_value, code::ErrorStatus* p_errStatus);
	
	
// Several helpers for storing attributes.

template<typename T>
inline void store(XmlNode* p_node, const std::string& p_attribute, T p_value, code::ErrorStatus* p_errStatus)
{
	TT_ERR_CHAIN_VOID("storing attribute '" << p_attribute << "'");
	TT_ERR_ASSERTMSG(p_node != 0, "Null XML node pointer passed.");
	
	TT_ERR_ADD_LOC(" to node '" << p_node->getName() << "'");
	
	p_node->setAttribute(p_attribute, str::toStr(p_value));
}


// specialisation for std::string
template<>
inline void store<const std::string&>(XmlNode* p_node, const std::string& p_attribute, const std::string& p_value, code::ErrorStatus* p_errStatus)
{
	TT_ERR_CHAIN_VOID("storing attribute '" << p_attribute << "'");
	TT_ERR_ASSERTMSG(p_node != 0, "Null XML node pointer passed.");
	
	TT_ERR_ADD_LOC(" to node '" << p_node->getName() << "'");
	
	p_node->setAttribute(p_attribute, p_value);
}


void store(XmlNode* p_node, const std::string& p_childName, const engine::renderer::ColorRGB& p_value, code::ErrorStatus* p_errStatus);
void store(XmlNode* p_node, const std::string& p_childName, const engine::renderer::ColorRGBA& p_value, code::ErrorStatus* p_errStatus);
void store(XmlNode* p_node, const std::string& p_childName, const math::Vector2& p_value, code::ErrorStatus* p_errStatus);
void store(XmlNode* p_node, const std::string& p_childName, const math::Vector3& p_value, code::ErrorStatus* p_errStatus);
void store(XmlNode* p_node, const std::string& p_childName, const math::Vector4& p_value, code::ErrorStatus* p_errStatus);
void store(XmlNode* p_node, const std::string& p_childName, const math::Point2& p_value, code::ErrorStatus* p_errStatus);
void store(XmlNode* p_node, const std::string& p_childName, const math::Point3& p_value, code::ErrorStatus* p_errStatus);
void store(XmlNode* p_node, const std::string& p_childName, const math::Quaternion& p_value, code::ErrorStatus* p_errStatus);
void store(XmlNode* p_node, const std::string& p_childName, const math::PointRect& p_value, code::ErrorStatus* p_errStatus);


// specialisation for OptionalValue
template<typename T>
inline void store(XmlNode* p_node, const std::string& p_attribute, const tt::code::OptionalValue<T>& p_value, code::ErrorStatus* p_errStatus)
{
	if (p_value.isValid() == false)
	{
		// nothing to do
		return;
	}
	store(p_node, p_attribute, p_value.get(), p_errStatus);
}


// specialisation for DefaultValue
template<typename T>
inline void store(XmlNode* p_node, const std::string& p_attribute, const tt::code::DefaultValue<T>& p_value, code::ErrorStatus* p_errStatus)
{
	if (p_value.isValid() == false)
	{
		// nothing to do
		return;
	}
	store(p_node, p_attribute, p_value.get(), p_errStatus);
}


template<typename T>
inline void store(XmlNode* p_node, const tt::code::OptionalValue<T>& p_value, code::ErrorStatus* p_errStatus)
{
	if (p_value.isValid())
	{
		store(p_node, p_value.get(), p_errStatus);
	}
}

template<typename T>
inline void store(XmlNode* p_node, const tt::code::DefaultValue<T>& p_value, code::ErrorStatus* p_errStatus)
{
	if (p_value.isValid())
	{
		store(p_node, p_value.get(), p_errStatus);
	}
}


// Namespace end
}
}
}

#endif // !defined(INC_TT_XML_UTIL_STORE_H)
