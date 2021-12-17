#ifndef INC_TT_STR_COMMON_H
#define INC_TT_STR_COMMON_H

#include <string>


// Forward declaration
namespace tt
{
	namespace code
	{
		class ErrorStatus;
	}
}


namespace tt {
namespace str {

std::wstring widen(const std::string& p_string);
std::string narrow(const std::wstring& p_string, tt::code::ErrorStatus* p_errStatus = 0);

std::wstring utf8ToUtf16(const std::string&  p_utf8String,  tt::code::ErrorStatus* p_errStatus = 0);
std::string  utf16ToUtf8(const std::wstring& p_utf16String, tt::code::ErrorStatus* p_errStatus = 0);

/*! \return The length of the passed UTF-8 string in characters (not the byte length!). */
std::string::size_type getUtf8CharacterLength(const std::string&     p_utf8String,
                                              tt::code::ErrorStatus* p_errStatus = 0);

std::string toLower(const std::string& p_str);
std::string toUpper(const std::string& p_str);

bool startsWith(const std::string& p_string, const std::string& p_search);
bool endsWith(const std::string& p_string, const std::string& p_search);

bool wildcardCompare(const std::string& p_string, const std::string& p_search);

// Namespace end
}
}


#endif  // !defined(INC_TT_STR_COMMON_H)
