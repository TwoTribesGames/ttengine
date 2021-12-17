#ifndef INC_TT_DOC_XAP_XAPWRITER_H
#define INC_TT_DOC_XAP_XAPWRITER_H

#include <string>

#include <tt/fs/types.h>
#include <tt/platform/tt_types.h>
#include <tt/xml/fwd.h>


namespace tt {
namespace doc {
namespace xap {

class XapWriter
{
public:
	XapWriter();
	
	bool writeFile(xml::XmlNode*, const fs::FilePtr& p_file);
	bool writeFile(xml::XmlNode*, const std::string& p_filename, fs::identifier p_type = 0);
	
private:
	void writeNode(xml::XmlNode* p_node, int p_indent);
	void writeIndent(int p_indent);
	void writeString(const std::string& p_string);
	void writeNewline();
	
	fs::FilePtr m_file;
};

// Namespace end
}
}
}

#endif // !defined(INC_TT_DOC_XAP_XAPWRITER_H)
