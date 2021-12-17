#include <iomanip>
#include <sstream>

#include <tt/platform/tt_error.h>
#include <tt/str/format.h>


namespace tt {
namespace str {


bool isEmptyOrWhitespace(const std::wstring& p_str)
{
	// First check if string is empty (simplest check)
	if (p_str.empty())
	{
		return true;
	}
	
	// Check if there is anything else than whitespace in the string
	for (std::wstring::const_iterator it = p_str.begin(); it != p_str.end(); ++it)
	{
		if (*it != ' ' && *it != '\t' && *it != '\n')
		{
			// Current character is not a space or tab;
			// now make sure it isn't part of the newline
			// marker either
			std::wstring::const_iterator next_char = it + 1;
			if (*it == '\\' && next_char != p_str.end() && *next_char == 'n')
			{
				// Found a newline marker; advance the iterator one further
				// (because marker is two characters)
				++it;
			}
			else
			{
				// Found a character that is not a space, tab
				// or newline marker, so the string is not all whitespace
				return false;
			}
		}
	}
	
	// Getting here means the entire string is composed
	// only of spaces, tabs and newline markers
	return true;
}


s32 replace(std::string& p_src, const std::string& p_search, const std::string& p_replacement)
{
	if (p_search.empty())
	{
		TT_PANIC("Search string cannot be empty.");
		return 0;
	}
	
	if (p_search == p_replacement)
	{
		// String will remain the same; just abort now
		return 0;
	}
	
	// Replace all occurences
	s32 count = 0;
	
	std::string::size_type searchStartIndex = 0;
	std::string::size_type occurenceStartPos = p_src.find(p_search, searchStartIndex);
	while (occurenceStartPos != std::string::npos)
	{
		p_src.replace(occurenceStartPos, p_search.size(), p_replacement);
		++count;
		
		searchStartIndex  = occurenceStartPos + p_replacement.size();
		occurenceStartPos = p_src.find(p_search, searchStartIndex);
	}
	
	return count;
}


s32 replace(std::wstring& p_src, const std::wstring& p_search, const std::wstring& p_replacement)
{
	if (p_search.empty())
	{
		TT_PANIC("Search string cannot be empty.");
		return 0;
	}
	
	if (p_search == p_replacement)
	{
		// String will remain the same; just abort now
		return 0;
	}
	
	// Replace all occurences
	s32 count = 0;
	
	std::wstring::size_type searchStartIndex = 0;
	std::wstring::size_type occurenceStartPos = p_src.find(p_search, searchStartIndex);
	while (occurenceStartPos != std::wstring::npos)
	{
		p_src.replace(occurenceStartPos, p_search.size(), p_replacement);
		++count;
		
		searchStartIndex  = occurenceStartPos + p_search.size();
		occurenceStartPos = p_src.find(p_search, searchStartIndex);
	}
	
	return count;
}


std::string formatMillisecondsToMMSSss(s32                p_milliseconds,
                                       const std::string& p_emptySymbol,
                                       const std::string& p_sep1,
                                       const std::string& p_sep2)
{
	if (p_milliseconds < 0)
	{
		std::string twoEmptySymbols(p_emptySymbol + p_emptySymbol);
		return twoEmptySymbols + p_sep1 + twoEmptySymbols + p_sep2 + twoEmptySymbols;
	}
	
	std::ostringstream oss;
	s32 seconds = p_milliseconds / 1000;
	oss << std::setw(2) << std::setfill('0') << (seconds / 60) << p_sep1
	    << std::setw(2) << std::setfill('0') << (seconds % 60) << p_sep2
	    << std::setw(2) << std::setfill('0') << ((p_milliseconds % 1000) / 10);
	
	return oss.str();
}


std::string formatSecondsToMMSS(s32 p_seconds,
                                const std::string& p_emptySymbol,
                                const std::string& p_sep1)
{
	if (p_seconds < 0)
	{
		std::string twoEmptySymbols(p_emptySymbol + p_emptySymbol);
		return twoEmptySymbols + p_sep1 + twoEmptySymbols;
	}
	
	std::ostringstream oss;
	oss << std::setw(2) << std::setfill('0') << (p_seconds / 60) << p_sep1
	    << std::setw(2) << std::setfill('0') << (p_seconds % 60);
	
	return oss.str();
}


std::string formatSecondsToHHMMSS(s32 p_seconds,
                                  const std::string& p_emptySymbol,
                                  const std::string& p_sep1,
                                  const std::string& p_sep2)
{
	if (p_seconds < 0)
	{
		std::string twoEmptySymbols(p_emptySymbol + p_emptySymbol);
		return twoEmptySymbols + p_sep1 + twoEmptySymbols + p_sep2 + twoEmptySymbols;
	}
	
	s32 p_minutes = p_seconds / 60;
	
	std::ostringstream oss;
	oss << std::setw(2) << std::setfill('0') << (p_minutes / 60) << p_sep1
	    << std::setw(2) << std::setfill('0') << (p_minutes % 60) << p_sep2
	    << std::setw(2) << std::setfill('0') << (p_seconds % 60);
	
	return oss.str();
}


// Namespace end
}
}
