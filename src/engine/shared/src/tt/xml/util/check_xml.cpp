#include <tt/code/ErrorStatus.h>
#include <tt/platform/tt_error.h>
#include <tt/xml/XmlNode.h>

#include <tt/xml/util/check.h>


namespace tt {
namespace xml {
namespace util {

// "private helper for check functions
static bool check(const XmlNode* p_node, code::ErrorStatus& p_errStatus)
{
	if (p_errStatus.hasError())
	{
		return false;
	}
	else if (p_node == 0)
	{
		TT_ERR_CHAIN_NAME(&p_errStatus, errStatus, bool, false, "tt::xml::check*");
		TT_ERR_AND_RETURN_NAME(errStatus, "XmlNode pointer is NULL!");
	}
	return true;
}


bool checkName(const XmlNode* p_node, const std::string& p_name, code::ErrorStatus& p_errStatus)
{
	if (check(p_node, p_errStatus) == false)
	{
		return false;
	}
	else if (p_node->getName() != p_name)
	{
		TT_ERROR_NAME(p_errStatus, "Expected XmlNode with the name '" << p_name << "' "
		                      "but found '" <<p_node->getName() << "'");
		return false;
	}
	return true;
}


bool checkNoChildren(const XmlNode* p_node, code::ErrorStatus& p_errStatus)
{
	if (check(p_node, p_errStatus) == false)
	{
		return false;
	}
	else if (p_node->getChild() != 0)
	{
		TT_ERROR_NAME(p_errStatus, "Expected XmlNode with no more children! "
		              "'" << p_node->getName() << "' has " << p_node->getChildCount() << " children.");
		return false;
	}
	return true;
}


// Namespace end
}
}
}
