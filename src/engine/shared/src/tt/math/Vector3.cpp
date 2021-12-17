#include <tt/code/ErrorStatus.h>
#include <tt/math/Point3.h>
#include <tt/math/Vector3.h>
#include <tt/str/parse.h>
#include <tt/xml/XmlNode.h>


namespace tt {
namespace math {

// Constant vector definitions
const Vector3 Vector3::zero(0, 0, 0);
const Vector3 Vector3::unitX(1, 0, 0);
const Vector3 Vector3::unitY(0, 1, 0);
const Vector3 Vector3::unitZ(0, 0, 1);
const Vector3 Vector3::up(0, 1, 0);
const Vector3 Vector3::down(0, -1, 0);
const Vector3 Vector3::left(-1, 0, 0);
const Vector3 Vector3::right(1, 0, 0);
const Vector3 Vector3::backward(0, 0, -1);
const Vector3 Vector3::forward(0, 0, 1);
const Vector3 Vector3::allOne(1, 1, 1);


Vector3::Vector3(const Point3& p_point)
:
x(static_cast<real>(p_point.x)),
y(static_cast<real>(p_point.y)),
z(static_cast<real>(p_point.z))
{
}


bool parseVector3(const xml::XmlNode* p_node, Vector3* p_result)
{
	TT_NULL_ASSERT(p_node);
	TT_NULL_ASSERT(p_result);
	
	TT_ERR_CREATE("parseVector3");
	
	if (p_node->getAttribute("x").empty())
	{
		TT_PANIC("Expected attribute 'x' in node '%s'", 
		         p_node->getName().c_str());
		*p_result = Vector3::zero;
		return false;
	}
	
	p_result->x = str::parseReal(p_node->getAttribute("x"), &errStatus);
	if (errStatus.hasError())
	{
		TT_PANIC("Value '%s' of attribute 'x' from node '%s' can't be converted to a float.",
		         p_node->getAttribute("x").c_str(), 
		         p_node->getName().c_str());
		*p_result = Vector3::zero;
		return false;
	}
	
	
	if (p_node->getAttribute("y").empty())
	{
		TT_PANIC("Expected attribute 'y' in node '%s'", 
		         p_node->getName().c_str());
		*p_result = Vector3::zero;
		return false;
	}
	
	p_result->y = str::parseReal(p_node->getAttribute("y"), &errStatus);
	if (errStatus.hasError())
	{
		TT_PANIC("Value '%s' of attribute 'y' from node '%s' can't be converted to a float.",
		         p_node->getAttribute("y").c_str(), p_node->getName().c_str());
		*p_result = Vector3::zero;
		return false;
	}
	
	
	if (p_node->getAttribute("z").empty())
	{
		TT_PANIC("Expected attribute 'z' in node '%s'", 
		         p_node->getName().c_str());
		*p_result = Vector3::zero;
		return false;
	}
	
	p_result->z = str::parseReal(p_node->getAttribute("z"), &errStatus);
	if (errStatus.hasError())
	{
		TT_PANIC("Value '%s' of attribute 'z' from node '%s' can't be converted to a float.",
		         p_node->getAttribute("z").c_str(), p_node->getName().c_str());
		*p_result = Vector3::zero;
		return false;
	}
	
	return true;
}

// Namespace end
}
}
