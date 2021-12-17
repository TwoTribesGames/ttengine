#include <tt/code/ErrorStatus.h>
#include <tt/math/Quaternion.h>
#include <tt/str/parse.h>
#include <tt/xml/XmlNode.h>


namespace tt {
namespace math {

// Constant vector definitions
const Quaternion Quaternion::identity(1.0f, 0.0f, 0.0f, 0.0f);


bool parseQuaternion(const xml::XmlNode* p_node, Quaternion* p_result)
{
	TT_NULL_ASSERT(p_node);
	TT_NULL_ASSERT(p_result);
	
	TT_ERR_CREATE("parseQuaternion");
	
	if (p_node->getAttribute("x").empty())
	{
		TT_PANIC("Expected attribute 'x' in node '%s'",
		         p_node->getName().c_str());
		p_result->setIdentity();
		return false;
	}
	
	p_result->m_x = str::parseReal(p_node->getAttribute("x"), &errStatus);
	if (errStatus.hasError())
	{
		TT_PANIC("Value '%s' of attribute 'x' from node '%s' can't be converted to a float.",
		         p_node->getAttribute("x").c_str(),
		         p_node->getName().c_str());
		p_result->setIdentity();
		return false;
	}
	
	if (p_node->getAttribute("y").empty())
	{
		TT_PANIC("Expected attribute 'y' in node '%s'",
		         p_node->getName().c_str());
		p_result->setIdentity();
		return false;
	}
	
	p_result->m_y = str::parseReal(p_node->getAttribute("y"), &errStatus);
	if (errStatus.hasError())
	{
		TT_PANIC("Value '%s' of attribute 'y' from node '%s' can't be converted to a float.",
		         p_node->getAttribute("y").c_str(), p_node->getName().c_str());
		p_result->setIdentity();
		return false;
	}
	
	if (p_node->getAttribute("z").empty())
	{
		TT_PANIC("Expected attribute 'z' in node '%s'",
		         p_node->getName().c_str());
		p_result->setIdentity();
		return false;
	}
	
	p_result->m_z = str::parseReal(p_node->getAttribute("z"), &errStatus);
	if (errStatus.hasError())
	{
		TT_PANIC("Value '%s' of attribute 'z' from node '%s' can't be converted to a float.",
		         p_node->getAttribute("z").c_str(), p_node->getName().c_str());
		p_result->setIdentity();
		return false;
	}
	
	if (p_node->getAttribute("w").empty())
	{
		TT_PANIC("Expected attribute 'w' in node '%s'",
		         p_node->getName().c_str());
		p_result->setIdentity();
		return false;
	}
	
	p_result->m_w = str::parseReal(p_node->getAttribute("w"), &errStatus);
	if (errStatus.hasError())
	{
		TT_PANIC("Value '%s' of attribute 'w' from node '%s' can't be converted to a float.",
		         p_node->getAttribute("w").c_str(), p_node->getName().c_str());
		p_result->setIdentity();
		return false;
	}
	
	return true;
}

// Namespace end
}
}
