#include <algorithm> // needed for transform
#include <cctype>    // needed for tolower
#include <limits>
#include <functional>
#include <utf8/utf8.h>

#include <tt/code/ErrorStatus.h>
#include <tt/platform/tt_error.h>
#include <tt/str/common.h>


namespace tt {
namespace str {


std::wstring widen(const std::string& p_string)
{
	if (p_string.empty())
	{
		return std::wstring();
	}
	
	std::wstring result;
	result.reserve(p_string.length());
	
	for (std::string::const_iterator it = p_string.begin();
	     it != p_string.end(); ++it)
	{
		// Avoid signed/unsigned mismatch.
		result.push_back(static_cast<u8>(*it));
		TT_STATIC_ASSERT(sizeof(u8) == sizeof(std::string::value_type));
	}
	
	return result;
}


std::string narrow(const std::wstring& p_string, tt::code::ErrorStatus* p_errStatus)
{
	TT_ERR_CHAIN(std::string, std::string(), "narrow wide string");
	
	if (p_string.empty())
	{
		return std::string();
	}
	
	typedef std::string::value_type NarrowType;
	std::string narrow;
	narrow.reserve(p_string.length());
	for (std::wstring::const_iterator it = p_string.begin(); it != p_string.end(); ++it)
	{
		TT_STATIC_ASSERT(sizeof(u8) == sizeof(NarrowType)); // Because of the > check we need the unsigned type.
		TT_ERR_ASSERTMSG(*it <= static_cast<std::wstring::value_type>(std::numeric_limits<u8>::max()),
		                 "Can't narrow the character '" << *it << "' (its numeric value is too large)");
		narrow.push_back(static_cast<NarrowType>(*it));
	}
	
	return narrow;
}

// [ER] Really this should be named utf8ToWChar which for Mac+Linux is UTF32
std::wstring utf8ToUtf16(const std::string& p_utf8String, tt::code::ErrorStatus* p_errStatus)
{
	TT_ERR_CHAIN(std::wstring, std::wstring(), "convert UTF-8 to UTF-16");
	
	// Check for invalid UTF-8 data (so that only the valid UTF-8 part is converted)
	std::string::const_iterator endIt = utf8::find_invalid(p_utf8String.begin(), p_utf8String.end());
	if (endIt != p_utf8String.end())
	{
		TT_ERROR("Invalid UTF-8 encoding detected in string (at character position "
		         << utf8::distance(p_utf8String.begin(), endIt) << "). Valid part: '"
		         << std::string(p_utf8String.begin(), endIt) << "'");
		
		// Replace the invalid UTF-8 characters with a Unicode replacement character
		// FIXME: This function can throw exceptions... find alternative or do not use?
		/*
		std::string temp;
		temp.reserve(p_utf8String.length() * 5);
		utf8::replace_invalid(p_utf8String.begin(), p_utf8String.end(), std::back_inserter(temp));
		
		p_utf8String = temp;
		endIt = p_utf8String.end();
		// */
	}

#if __WCHAR_MAX__ > 0x10000
	// Convert the UTF-8 string to a UTF-32 character sequence using the utfcpp lib
	std::vector<u32> utf32Characters;
	utf8::utf8to32(p_utf8String.begin(), endIt, std::back_inserter(utf32Characters));

	// Turn the UTF-16 characters into a std::wstring
	std::wstring utf32String;
	utf32String.reserve(utf32Characters.size());
	for (std::vector<u32>::iterator it = utf32Characters.begin(); it != utf32Characters.end(); ++it)
	{
		utf32String += static_cast<std::wstring::value_type>(*it);
	}
	return utf32String;
#else
	// Convert the UTF-8 string to a UTF-16 character sequence using the utfcpp lib
	std::vector<u16> utf16Characters;
	utf8::utf8to16(p_utf8String.begin(), endIt, std::back_inserter(utf16Characters));
	
	// Turn the UTF-16 characters into a std::wstring
	std::wstring utf16String;
	utf16String.reserve(utf16Characters.size());
	for (std::vector<u16>::iterator it = utf16Characters.begin(); it != utf16Characters.end(); ++it)
	{
		utf16String += static_cast<std::wstring::value_type>(*it);
	}
	return utf16String;
#endif
}

// [ER] Really this should be named wcharToUtf8 which for Mac+Linux is UTF32
std::string utf16ToUtf8(const std::wstring& p_utf16String, tt::code::ErrorStatus* p_errStatus)
{
	TT_ERR_CHAIN(std::string, std::string(), "convert UTF-16 to UTF-8");
	
	// FIXME: Somehow validate the UTF-16 string and report an error in ErrorStatus if invalid
#if __WCHAR_MAX__ > 0x10000
	// utfcpp expects the UTF-32 string to be a sequence of u32, so convert the wstring to one
	std::vector<u32> utf32Characters;
			utf32Characters.reserve(p_utf16String.length());
	for (std::wstring::const_iterator it = p_utf16String.begin(); it != p_utf16String.end(); ++it)
	{
		utf32Characters.push_back(static_cast<u16>(*it));
	}

	// Convert the UTF-16 sequence to a UTF-8 sequence using the utfcpp lib
	std::vector<u8> utf8Characters;
	utf8::utf32to8(utf32Characters.begin(), utf32Characters.end(),
	               std::back_inserter(utf8Characters));
#else
	// utfcpp expects the UTF-16 string to be a sequence of u16, so convert the wstring to one
	std::vector<u16> utf16Characters;
	utf16Characters.reserve(p_utf16String.length());
	for (std::wstring::const_iterator it = p_utf16String.begin(); it != p_utf16String.end(); ++it)
	{
		utf16Characters.push_back(static_cast<u16>(*it));
	}
	
	// Convert the UTF-16 sequence to a UTF-8 sequence using the utfcpp lib
	std::vector<u8> utf8Characters;
	utf8::utf16to8(utf16Characters.begin(), utf16Characters.end(),
	               std::back_inserter(utf8Characters));
#endif
	// Turn the UTF-8 sequence into a std::string (std::string::value_type may not be u8-compatible)
	std::string utf8String;
	utf8String.reserve(utf8Characters.size());
	for (std::vector<u8>::const_iterator it = utf8Characters.begin();
	     it != utf8Characters.end(); ++it)
	{
		utf8String += static_cast<std::string::value_type>(*it);
	}
	
	return utf8String;
}


std::string::size_type getUtf8CharacterLength(const std::string&     p_utf8String,
                                              tt::code::ErrorStatus* p_errStatus)
{
	TT_ERR_CHAIN(std::string::size_type, 0, "get UTF-8 string character length");
	
	// Ensure we're not trying to process invalid UTF-8 characters
	std::string::const_iterator endIt = utf8::find_invalid(p_utf8String.begin(), p_utf8String.end());
	if (endIt != p_utf8String.end())
	{
		TT_ERROR("Invalid UTF-8 encoding detected in string (at character position "
		         << utf8::distance(p_utf8String.begin(), endIt) << "). Valid part: '"
		         << std::string(p_utf8String.begin(), endIt) << "'");
	}
	
	return static_cast<std::string::size_type>(utf8::distance(p_utf8String.begin(), endIt));
}


std::string toLower(const std::string& p_str)
{
	if (p_str.empty())
	{
		return std::string();
	}
	
	std::string lowerCase;
	std::transform(p_str.begin(), p_str.end(), std::back_inserter(lowerCase), [](unsigned char c) { return static_cast<unsigned char>(std::tolower(c)); });
	
	// FIXME: Should use locale (see: http://gcc.gnu.org/onlinedocs/libstdc++/22_locale/howto.html )
	return lowerCase;
}


std::string toUpper(const std::string& p_str)
{
	if (p_str.empty())
	{
		return std::string();
	}
	
	std::string upperCase;
	std::transform(p_str.begin(), p_str.end(), std::back_inserter(upperCase), [](unsigned char c) { return static_cast<unsigned char>(std::toupper(c)); });
	
	// FIXME: Should use locale (see: http://gcc.gnu.org/onlinedocs/libstdc++/22_locale/howto.html )
	return upperCase;
}


bool startsWith(const std::string& p_string, const std::string& p_search)
{
	return p_string.find(p_search) == 0;
}


bool endsWith(const std::string& p_string, const std::string& p_search)
{
	std::string::size_type foundIdx = p_string.rfind(p_search);
	return foundIdx != std::string::npos &&
	       foundIdx == (p_string.length() - p_search.length());
}


bool wildcardCompare(const std::string& p_string, const std::string& p_search)
{
	if (p_search == "*")
	{
		return true;
	}
	if (p_string.empty())
	{
		return false;
	}
	
	// Taken from http://www.codeproject.com/Articles/1088/Wildcard-string-compare-globbing
	const char* string = p_string.c_str();
	const char* wild   = p_search.c_str();
	
	const char *cp = 0, *mp = 0;
	
	while ((*string) && (*wild != '*')) 
	{
		if ((*wild != *string) && (*wild != '?')) 
		{
			return false;
		}
		++wild;
		++string;
	}
	
	while (*string)
	{
		if (*wild == '*')
		{
			if (!*++wild)
			{
				return true;
			}
			mp = wild;
			cp = string + 1;
		}
		else if ((*wild == *string) || (*wild == '?'))
		{
			++wild;
			++string;
		}
		else
		{
			wild = mp;
			string = cp++;
		}
	}
	
	while (*wild == '*')
	{
		++wild;
	}
	return !*wild;
}

// Namespace end
}
}
