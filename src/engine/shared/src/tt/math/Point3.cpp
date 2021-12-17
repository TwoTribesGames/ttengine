#include <tt/code/ErrorStatus.h>
#include <tt/math/Point3.h>
#include <tt/str/parse.h>
#include <tt/xml/XmlNode.h>


namespace tt {
namespace math {

// Constant point definitions
const Point3 Point3::zero(0, 0, 0);
const Point3 Point3::unitX(1, 0, 0);
const Point3 Point3::unitY(0, 1, 0);
const Point3 Point3::unitZ(0, 0, 1);
const Point3 Point3::up(0, 1, 0);
const Point3 Point3::down(0, -1, 0);
const Point3 Point3::left(-1, 0, 0);
const Point3 Point3::right(1, 0, 0);
const Point3 Point3::backward(0, 0, -1);
const Point3 Point3::forward(0, 0, 1);
const Point3 Point3::allOne(1, 1, 1);


bool parsePoint3(const xml::XmlNode* p_node, Point3* p_result)
{
	TT_NULL_ASSERT(p_node);
	TT_NULL_ASSERT(p_result);
	
	TT_ERR_CREATE("parsePoint3");
	
	std::string attrib;
	
	// Get X component
	attrib = p_node->getAttribute("x");
	if (attrib.empty())
	{
		TT_PANIC("Expected attribute 'x' in node '%s'.",
		         p_node->getName().c_str());
		p_result->setValues(0, 0, 0);
		return false;
	}
	
	p_result->x = str::parseS32(attrib, &errStatus);
	if (errStatus.hasError())
	{
		TT_PANIC("Invalid integer specified for attribute 'x' in node '%s': '%s'",
		         p_node->getName().c_str(), attrib.c_str());
		p_result->setValues(0, 0, 0);
		return false;
	}
	
	// Get Y component
	attrib = p_node->getAttribute("y");
	if (attrib.empty())
	{
		TT_PANIC("Expected attribute 'y' in node '%s'.",
		         p_node->getName().c_str());
		p_result->setValues(0, 0, 0);
		return false;
	}
	
	p_result->y = str::parseS32(attrib, &errStatus);
	if (errStatus.hasError())
	{
		TT_PANIC("Invalid integer specified for attribute 'y' in node '%s': '%s'",
		         p_node->getName().c_str(), attrib.c_str());
		p_result->setValues(0, 0, 0);
		return false;
	}
	
	// Get Z component
	attrib = p_node->getAttribute("z");
	if (attrib.empty())
	{
		TT_PANIC("Expected attribute 'z' in node '%s'.",
		         p_node->getName().c_str());
		p_result->setValues(0, 0, 0);
		return false;
	}
	
	p_result->z = str::parseS32(attrib, &errStatus);
	if (errStatus.hasError())
	{
		TT_PANIC("Invalid integer specified for attribute 'z' in node '%s': '%s'",
		         p_node->getName().c_str(), attrib.c_str());
		p_result->setValues(0, 0, 0);
		return false;
	}
	
	return true;
}

// Namespace end
}
}
