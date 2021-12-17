#ifndef INC_TT_DOC_STR_STRINGDOCUMENT_H
#define INC_TT_DOC_STR_STRINGDOCUMENT_H

#include <string>
#include <vector>

#include <tt/fs/types.h>


namespace tt {
namespace doc {
namespace str {

class StringDocument
{
public:
	explicit StringDocument(const std::string& p_filename,
	               bool               p_cachedContent = true);
	
	StringDocument(const StringDocument& p_rhs);
	
	inline s32 getStringCount() const
	{
		return static_cast<s32>(m_names.size());
	}
	
	std::string  getString(s32 p_index);
	std::wstring getWString(s32 p_index);
	
	s32 getIndex(const std::string&  p_string);
	s32 getIndex(const std::wstring& p_string);
	
private:
	void construct();
	
	// No assignment
	StringDocument& operator=(const StringDocument&);
	
	struct NameEntry
	{
		s32 start;
		s32 length;
	};
	
	typedef std::vector<NameEntry> Names;
	
	Names       m_names;
	fs::FilePtr m_file;
	std::string m_filename;
	std::string m_content;
	bool        m_cachedContent;
};

// Namespace end
}
}
}


#endif  // !defined(INC_TT_DOC_STR_STRINGDOCUMENT_H)

