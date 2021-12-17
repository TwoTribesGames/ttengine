#include <tt/code/ErrorStatus.h>
#include <tt/math/Point2.h>
#include <tt/math/Vector2.h>
#include <tt/str/parse.h>
#include <tt/xml/XmlNode.h>


namespace tt {
namespace math {

// Constant vector definitions
const Vector2 Vector2::zero(  0.0f, 0.0f);
const Vector2 Vector2::unitX( 1.0f, 0.0f);
const Vector2 Vector2::unitY( 0.0f, 1.0f);
const Vector2 Vector2::allOne(1.0f, 1.0f);


Vector2::Vector2(const Point2& p_point)
:
x(static_cast<real>(p_point.x)),
y(static_cast<real>(p_point.y))
{
}


bool parseVector2(const xml::XmlNode* p_node, Vector2* p_result)
{
	TT_NULL_ASSERT(p_node);
	TT_NULL_ASSERT(p_result);
	
	TT_ERR_CREATE("parseVector2");
	
	if (p_node->getAttribute("x").empty())
	{
		TT_PANIC("Expected attribute 'x' in node '%s'", 
		         p_node->getName().c_str());
		p_result->setValues(0.0f, 0.0f);
		return false;
	}
	
	p_result->x = str::parseReal(p_node->getAttribute("x"), &errStatus);
	if (errStatus.hasError())
	{
		TT_PANIC("Value '%s' of attribute 'x' from node '%s' can't be converted to a real.",
		         p_node->getAttribute("x").c_str(), 
		         p_node->getName().c_str());
		p_result->setValues(0.0f, 0.0f);
		return false;
	}
	
	
	if (p_node->getAttribute("y").empty())
	{
		TT_PANIC("Expected attribute 'y' in node '%s'", 
		         p_node->getName().c_str());
		p_result->setValues(0.0f, 0.0f);
		return false;
	}
	
	p_result->y = str::parseReal(p_node->getAttribute("y"), &errStatus);
	if (errStatus.hasError())
	{
		TT_PANIC("Value '%s' of attribute 'y' from node '%s' can't be converted to a real.",
		         p_node->getAttribute("y").c_str(), p_node->getName().c_str());
		p_result->setValues(0.0f, 0.0f);
		return false;
	}
	
	return true;
}

// Namespace end
}
}
