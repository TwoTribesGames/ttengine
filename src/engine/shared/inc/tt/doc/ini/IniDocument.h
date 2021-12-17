#ifndef INC_TT_DOC_INI_INIDOCUMENT_H
#define INC_TT_DOC_INI_INIDOCUMENT_H

#include <map>
#include <string>

#include <tt/fs/types.h>
#include <tt/platform/tt_types.h>


namespace tt {
namespace doc {
namespace ini {

class IniDocument
{
public:
	IniDocument();
	explicit IniDocument(const std::string& p_filename, fs::identifier p_type = 0);
	explicit IniDocument(const fs::FilePtr& p_file);
	
	bool hasSection(const std::string& p_section) const;
	bool hasKey(const std::string& p_section, const std::string& p_key) const;
	
	std::string getString(const std::string& p_section,
	                      const std::string& p_key) const;
	
private:
	typedef std::map<std::string, std::string> Section;
	typedef std::map<std::string, Section>     Config;
	
	
	void parseDocument(const fs::FilePtr& p_file);
	
	
	Config m_config;
};

} // namespace end
}
}

#endif // INC_TT_DOC_INI_INIDOCUMENT_H
