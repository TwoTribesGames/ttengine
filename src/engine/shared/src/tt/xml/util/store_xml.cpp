#include <tt/code/ErrorStatus.h>
#include <tt/engine/renderer/ColorRGB.h>
#include <tt/engine/renderer/ColorRGBA.h>
#include <tt/math/Vector2.h>
#include <tt/math/Vector3.h>
#include <tt/math/Vector4.h>
#include <tt/math/Point2.h>
#include <tt/math/Point3.h>
#include <tt/math/Rect.h>
#include <tt/math/Quaternion.h>
#include <tt/platform/tt_error.h>
#include <tt/str/toStr.h>
#include <tt/xml/XmlNode.h>

#include <tt/xml/util/store.h>


namespace tt {
namespace xml {
namespace util {


void store(XmlNode* p_node, const engine::renderer::ColorRGB& p_value, code::ErrorStatus* p_errStatus)
{
	store(p_node, "r", p_value.r, p_errStatus);
	store(p_node, "g", p_value.g, p_errStatus);
	store(p_node, "b", p_value.b, p_errStatus);
}


void store(XmlNode* p_node, const engine::renderer::ColorRGBA& p_value, code::ErrorStatus* p_errStatus)
{
	store(p_node, "r", p_value.r, p_errStatus);
	store(p_node, "g", p_value.g, p_errStatus);
	store(p_node, "b", p_value.b, p_errStatus);
	store(p_node, "a", p_value.a, p_errStatus);
}

void store(XmlNode* p_node, const math::Vector2& p_value, code::ErrorStatus* p_errStatus)
{
	store(p_node, "x", p_value.x, p_errStatus);
	store(p_node, "y", p_value.y, p_errStatus);
}

void store(XmlNode* p_node, const math::Vector3& p_value, code::ErrorStatus* p_errStatus)
{
	store(p_node, "x", p_value.x, p_errStatus);
	store(p_node, "y", p_value.y, p_errStatus);
	store(p_node, "z", p_value.z, p_errStatus);
}

void store(XmlNode* p_node, const math::Vector4& p_value, code::ErrorStatus* p_errStatus)
{
	store(p_node, "x", p_value.x, p_errStatus);
	store(p_node, "y", p_value.y, p_errStatus);
	store(p_node, "z", p_value.z, p_errStatus);
	store(p_node, "w", p_value.w, p_errStatus);
}

void store(XmlNode* p_node, const math::Point2& p_value, code::ErrorStatus* p_errStatus)
{
	store(p_node, "x", p_value.x, p_errStatus);
	store(p_node, "y", p_value.y, p_errStatus);
}

void store(XmlNode* p_node, const math::Point3& p_value, code::ErrorStatus* p_errStatus)
{
	store(p_node, "x", p_value.x, p_errStatus);
	store(p_node, "y", p_value.y, p_errStatus);
	store(p_node, "z", p_value.z, p_errStatus);
}

void store(XmlNode* p_node, const math::Quaternion& p_value, code::ErrorStatus* p_errStatus)
{
	store(p_node, "x", p_value.m_x, p_errStatus);
	store(p_node, "y", p_value.m_y, p_errStatus);
	store(p_node, "z", p_value.m_z, p_errStatus);
	store(p_node, "w", p_value.m_w, p_errStatus);
}

void store(XmlNode* p_node, const math::PointRect& p_value, code::ErrorStatus* p_errStatus)
{
	store(p_node, p_value.getPosition(), p_errStatus);
	store(p_node, "width", p_value.getWidth(), p_errStatus);
	store(p_node, "height", p_value.getHeight(), p_errStatus);
}

template<typename T>
void createChild(XmlNode* p_node, const std::string& p_childName, T p_value, code::ErrorStatus* p_errStatus)
{
	TT_ERR_CHAIN_VOID("Create child node.");
	if (errStatus.hasError())
	{
		return;
	}
	
	xml::XmlNode* child = new xml::XmlNode(p_childName);
	TT_ERR_NULL_ASSERT(child);
	store(child, p_value, &errStatus);
	if (errStatus.hasError())
	{
		delete child;
	}
	else
	{
		p_node->addChild(child);
	}
}

void store(XmlNode* p_node, const std::string& p_childName, const engine::renderer::ColorRGB& p_value, code::ErrorStatus* p_errStatus)
{
	createChild(p_node, p_childName, p_value, p_errStatus);
}

void store(XmlNode* p_node, const std::string& p_childName, const engine::renderer::ColorRGBA& p_value, code::ErrorStatus* p_errStatus)
{
	createChild(p_node, p_childName, p_value, p_errStatus);
}

void store(XmlNode* p_node, const std::string& p_childName, const math::Vector2& p_value, code::ErrorStatus* p_errStatus)
{
	createChild(p_node, p_childName, p_value, p_errStatus);
}

void store(XmlNode* p_node, const std::string& p_childName, const math::Vector3& p_value, code::ErrorStatus* p_errStatus)
{
	createChild(p_node, p_childName, p_value, p_errStatus);
}

void store(XmlNode* p_node, const std::string& p_childName, const math::Vector4& p_value, code::ErrorStatus* p_errStatus)
{
	createChild(p_node, p_childName, p_value, p_errStatus);
}

void store(XmlNode* p_node, const std::string& p_childName, const math::Point2& p_value, code::ErrorStatus* p_errStatus)
{
	createChild(p_node, p_childName, p_value, p_errStatus);
}

void store(XmlNode* p_node, const std::string& p_childName, const math::Point3& p_value, code::ErrorStatus* p_errStatus)
{
	createChild(p_node, p_childName, p_value, p_errStatus);
}

void store(XmlNode* p_node, const std::string& p_childName, const math::Quaternion& p_value, code::ErrorStatus* p_errStatus)
{
	createChild(p_node, p_childName, p_value, p_errStatus);
}

void store(XmlNode* p_node, const std::string& p_childName, const math::PointRect& p_value, code::ErrorStatus* p_errStatus)
{
	createChild(p_node, p_childName, p_value, p_errStatus);
}


// Namespace end
}
}
}
