#if !defined(INC_TT_XML_UTIL_CHECK_H)
#define INC_TT_XML_UTIL_CHECK_H


// Forward declaration
namespace tt
{
	namespace code
	{
		class ErrorStatus;
	}
	
	namespace xml
	{
		class XmlNode;
	}
}


namespace tt {
namespace xml {
namespace util {

/* \brief Helper for checking the name of a node.
          Will trigger an error if the name isn't correct.
   \param p_node The XmlNode who's name should be checked.
   \param p_name The name to check the XmlNode's name against.
   \param p_errStatus A _reference_ to an errorStatus object.
          If the object already has has error false will be returned.
          If the name check failes an error will triggered.
   \return true if the name of the XmlNode equals the specified p_name. */
bool checkName(const XmlNode* p_node, const std::string& p_name, code::ErrorStatus& p_errStatus);

/* \brief Helper for checking that an XmlNode has no more children. 
   \return true if no children are found. */
bool checkNoChildren(const XmlNode* p_node, code::ErrorStatus& p_errStatus);

// Namespace end
}
}
}

#endif // !defined(INC_TT_XML_UTIL_CHECK_H)
