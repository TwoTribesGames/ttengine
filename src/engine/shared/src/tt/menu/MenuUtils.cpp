#include <sstream>
#include <iomanip>

#include <tt/platform/tt_error.h>
#include <tt/menu/MenuUtils.h>
#include <tt/menu/MenuSystem.h>


namespace tt {
namespace menu {

//------------------------------------------------------------------------------
// Public member functions

std::string MenuUtils::wideStringToHex(const std::wstring& p_string)
{
	std::ostringstream ret;
	
	for (std::wstring::const_iterator it = p_string.begin();
	     it != p_string.end(); ++it)
	{
		ret << std::hex << std::setw(4) << std::setfill('0')
		    << static_cast<s32>(*it);
	}
	
	return ret.str();
	
	/*
	char        character[5];
	std::string ret;
	ret.reserve(p_string.length() * 4);
	
	for (std::wstring::const_iterator it = p_string.begin();
	     it != p_string.end(); ++it)
	{
		OS_SPrintf(character, "%04X", (*it));
		ret += character;
	}
	
	return ret;
	*/
}


std::wstring MenuUtils::hexToWideString(const std::string& p_string)
{
	TT_ASSERTMSG(p_string.length() % 4 == 0, "Invalid hex widestring: '%s'",
	             p_string.c_str());
	
	std::wstring ret;
	ret.reserve(p_string.length() / 4);
	
	for (std::string::size_type i = 0; i < p_string.length();)
	{
		wchar_t w = 0;
		char    c = p_string.at(i);
		
		w += asciiToHex(c);
		w <<= 4;
		++i;
		
		c  = p_string.at(i);
		w += asciiToHex(c);
		w <<= 4;
		++i;
		
		c  = p_string.at(i);
		w += asciiToHex(c);
		w <<= 4;
		++i;
		
		c  = p_string.at(i);
		w += asciiToHex(c);
		
		ret += w;
		++i;
	}
	
	return ret;
}


//------------------------------------------------------------------------------
// Private member functions

wchar_t MenuUtils::asciiToHex(char p_char)
{
	if (p_char >= 'a' && p_char <= 'f')
	{
		return static_cast<wchar_t>((p_char - 'a') + 0x0A);
	}
	else if (p_char >= 'A' && p_char <= 'F')
	{
		return static_cast<wchar_t>((p_char - 'A') + 0x0A);
	}
	else if (p_char >= '0' && p_char <= '9')
	{
		return static_cast<wchar_t>(p_char - '0');
	}
	
	TT_PANIC("Invalid hex character: '%c'", p_char);
	return 0;
}

// Namespace end
}
}
