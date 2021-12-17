#include <tt/code/ErrorStatus.h>
#include <tt/math/Point2.h>
#include <tt/str/parse.h>
#include <tt/xml/XmlNode.h>


namespace tt {
namespace math {

// Constant point definitions
const Point2 Point2::zero(0, 0);
const Point2 Point2::unitX(1, 0);
const Point2 Point2::unitY(0, 1);
const Point2 Point2::up(0, -1);
const Point2 Point2::down(0, 1);
const Point2 Point2::left(-1, 0);
const Point2 Point2::right(1, 0);
const Point2 Point2::allOne(1, 1);


bool parsePoint2(const xml::XmlNode* p_node, Point2* p_result)
{
	TT_NULL_ASSERT(p_node);
	TT_NULL_ASSERT(p_result);
	
	TT_ERR_CREATE("parsePoint2");
	
	std::string attrib;
	
	// Get X component
	attrib = p_node->getAttribute("x");
	if (attrib.empty())
	{
		TT_PANIC("Expected attribute 'x' in node '%s'.",
		         p_node->getName().c_str());
		p_result->setValues(0, 0);
		return false;
	}
	
	p_result->x = str::parseS32(attrib, &errStatus);
	if (errStatus.hasError())
	{
		TT_PANIC("Invalid integer specified for attribute 'x' in node '%s': '%s'",
		         p_node->getName().c_str(), attrib.c_str());
		p_result->setValues(0, 0);
		return false;
	}
	
	// Get Y component
	attrib = p_node->getAttribute("y");
	if (attrib.empty())
	{
		TT_PANIC("Expected attribute 'y' in node '%s'.",
		         p_node->getName().c_str());
		p_result->setValues(0, 0);
		return false;
	}
	
	p_result->y = str::parseS32(attrib, &errStatus);
	if (errStatus.hasError())
	{
		TT_PANIC("Invalid integer specified for attribute 'y' in node '%s': '%s'",
		         p_node->getName().c_str(), attrib.c_str());
		p_result->setValues(0, 0);
		return false;
	}
	
	return true;
}

// Namespace end
}
}
