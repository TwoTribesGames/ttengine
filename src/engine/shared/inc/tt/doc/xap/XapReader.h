#ifndef INC_TT_DOC_XAP_XAPREADER_H
#define INC_TT_DOC_XAP_XAPREADER_H

#include <string>

#include <tt/fs/types.h>
#include <tt/platform/tt_types.h>
#include <tt/xml/fwd.h>


namespace tt {
namespace doc {
namespace xap {

class XapReader
{
public:
	XapReader();
	
	xml::XmlNode* readFile(const fs::FilePtr& p_file);
	xml::XmlNode* readFile(const std::string& p_filename, fs::identifier p_type = 0);
	
private:
	xml::XmlNode* createXmlNode(const std::string& p_name, bool p_first = true);
	std::string getLine();
	
	// No copying
	XapReader(const XapReader&);
	XapReader& operator=(const XapReader&);
	
	
	fs::FilePtr m_file;
	s8*         m_buffer;
	int         m_index;
	int         m_size;
	int         m_line;
};

// namespace end
}
}
}

#endif // !defined(INC_TT_DOC_XAP_XAPREADER_H)
