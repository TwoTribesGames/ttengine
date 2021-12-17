#ifndef NO_WIDESTRINGSTREAMS

#ifndef INC_TT_STR_STRINGFORMATTER_H
#define INC_TT_STR_STRINGFORMATTER_H


#include <string>
#include <sstream>

#include <tt/platform/tt_types.h>


namespace tt {
namespace str {

/*! \brief Provides basic stream-based string formatting.
           Replaces $VAR$ in format string with variables passed. */
class StringFormatter
{
public:
	/*! \param p_numberedParams If true, tokens in the form of $VAR1$, $VAR2$,
	                            etc are parsed, instead of $VAR$. */
	explicit StringFormatter(const std::wstring& p_formatString,
	                         bool p_numberedParams = true);
	virtual ~StringFormatter();
	
	/*! \brief Returns the formatted string, optionally verifying that all tokens have been parsed.
	    \param p_validate Whether to verify that all tokens have been replaced. If true, and not all tokens have been replaced, this call will assert.
	    \return The formatted string. */
	std::wstring getResult(bool p_validate = true);
	
	StringFormatter& operator<<(s32 p_var);
	StringFormatter& operator<<(u32 p_var);
	StringFormatter& operator<<(s16 p_var);
	StringFormatter& operator<<(u16 p_var);
	StringFormatter& operator<<(char p_var);
	StringFormatter& operator<<(const char* p_var);
	StringFormatter& operator<<(const wchar_t* p_var);
	StringFormatter& operator<<(const void* p_var);
	StringFormatter& operator<<(const std::string& p_var);
	StringFormatter& operator<<(const std::wstring& p_var);
	
private:
	std::wstring getNextToken();
	void replaceParameter(const std::wstring& p_val);
	
	// No copying or assigning
	StringFormatter(const StringFormatter&);
	const StringFormatter& operator=(const StringFormatter&);
	
	
	//! The string to replace in the format string (for unnumbered parameters).
	static const std::wstring ms_token;
	
	bool                    m_numberedParams;
	std::wstring            m_formatString;
	std::wstring            m_result;
	s32                     m_currentParam;
};

// Namespace end
}
}


#endif  // !defined(INC_TT_STR_STRINGFORMATTER_H)

#endif
