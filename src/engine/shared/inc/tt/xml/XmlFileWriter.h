#if !defined(INC_TT_XML_XMLFILEWRITER_H)
#define INC_TT_XML_XMLFILEWRITER_H


#include <string>

#include <tt/fs/types.h>


namespace tt {
namespace xml {

// Forward declarations
class XmlNode;

class XmlFileWriter
{
public:
	explicit XmlFileWriter(const XmlNode* p_tree);
	
	//! Saves an XML tree to file
	//! \return false if an error occured during writing
	bool save(bool p_stripWhitespace, const std::string& p_filename, fs::identifier p_type = 0);
	bool save(bool p_stripWhitespace, const fs::FilePtr& p_file);
	
private:
	bool write(const fs::FilePtr& p_file, const std::string& p_str);
	bool writeNode(const fs::FilePtr& p_file, const XmlNode* p_node, int p_depth, bool p_strip);
	
	std::string replaceSpecialChars(const std::string& p_str);
	
	// No copying
	XmlFileWriter(const XmlFileWriter&);
	XmlFileWriter& operator=(const XmlFileWriter&);
	
	
	const XmlNode* m_tree;
};

// Namespace end
}
}


#endif  // !defined(INC_TT_XML_XMLFILEWRITER_H)
