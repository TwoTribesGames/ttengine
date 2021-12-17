#if !defined(INC_TT_XML_XMLDOCUMENT_H)
#define INC_TT_XML_XMLDOCUMENT_H


#include <string>

#include <tt/fs/types.h>
#include <tt/platform/tt_types.h>
#include <tt/xml/fwd.h>


namespace tt {
namespace xml {

class XmlDocument
{
public:
	XmlDocument(const std::string& p_filename, fs::identifier p_type = 0);
	XmlDocument(const fs::FilePtr& p_file);
	~XmlDocument();
	
	inline XmlNode* getRootNode() const { return m_rootNode; }
	
private:
	// No copying
	XmlDocument(const XmlDocument& p_rhs);
	XmlDocument& operator=(const XmlDocument& p_rhs);
	
	XmlNode* m_rootNode;
};

// Namespace end
}
}


#endif  // !defined(INC_TT_XML_XMLDOCUMENT_H)
