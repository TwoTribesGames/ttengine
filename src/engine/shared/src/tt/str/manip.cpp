#include <tt/str/manip.h>


namespace tt {
namespace str {

Strings explode(const std::string& p_line, const std::string& p_delims, bool p_allowEmptyTokens)
{
	// Iterating index
	std::string::size_type idx = 0;
	
	// Placeholder for the current token
	std::string tkn;
	
	// List of tokens - for return
	Strings tokens;
	
	// Trap empty input string
	if (p_line.empty()) return tokens;
	
	// Run until break
	bool firstChar = true;
	for (;;)
	{
		// Read character
		std::string::value_type chr = p_line[idx];
		
		// Hit a valid character?
		if (p_delims.find(chr) == std::string::npos)
		{
			tkn += chr;
		}
		// Delimiter - push token and zap current
		else
		{
			if (tkn.empty() == false || (p_allowEmptyTokens && firstChar == false))
			{
				tokens.push_back(tkn);
				tkn.clear();
			}
		}
		
		// Move along
		++idx;
		
		// Reached end?
		if (idx >= p_line.length())
		{
			if (tkn.empty() == false || (p_allowEmptyTokens && firstChar == false))
			{
				tokens.push_back(tkn);
			}
			break;
		}
		
		firstChar = false;
	}
	
	// List of tokens
	return tokens;
}


std::vector<std::wstring> explode(const std::wstring& p_line,
                                  const std::wstring& p_delims,
                                  bool p_allowEmptyTokens)
{
	// Iterating index
	std::wstring::size_type idx = 0;
	
	// Placeholder for the current token
	std::wstring tkn;
	
	// List of tokens - for return
	std::vector<std::wstring> tokens;
	
	// Trap empty input string
	if (p_line.empty()) return tokens;
	
	// Run until break
	bool firstChar = true;
	for (;;)
	{
		// Read character
		std::wstring::value_type chr = p_line[idx];
		
		// Hit a valid character?
		if (p_delims.find(chr) == std::wstring::npos)
		{
			tkn += chr;
		}
		// Delimiter - push token and zap current
		else
		{
			if (tkn.empty() == false || (p_allowEmptyTokens && firstChar == false))
			{
				tokens.push_back(tkn);
				tkn.clear();
			}
		}
		
		// Move along
		++idx;
		
		// Reached end?
		if (idx >= p_line.length())
		{
			if (tkn.empty() == false || (p_allowEmptyTokens && firstChar == false))
			{
				tokens.push_back(tkn);
			}
			break;
		}
		
		firstChar = false;
	}
	
	// List of tokens
	return tokens;
}


std::string implode(const Strings& p_pieces, const std::string& p_glue)
{
	return implode(p_pieces.begin(), p_pieces.end(), p_glue);
}


std::string implode(Strings::const_iterator p_piecesBegin, Strings::const_iterator p_piecesEnd,
                    const std::string& p_glue)
{
	std::string ret;
	
	for ( ; p_piecesBegin != p_piecesEnd; ++p_piecesBegin)
	{
		if (ret.empty() == false)
		{
			ret += p_glue;
		}
		ret += *p_piecesBegin;
	}
	
	return ret;
}


std::string trim(const std::string& p_string, const std::string& p_drop)
{
	std::string out(p_string);
	
	std::string::size_type lastWhite = out.find_last_not_of(p_drop);
	if (lastWhite != std::string::npos)
	{
		// Found trailing whitespace
		out.erase(lastWhite + 1);
	}
	else
	{
		// Could not find anything that is not whitespace
		if (out.find_first_of(p_drop) != std::string::npos)
		{
			// Entire string is whitespace
			out.clear();
		}
	}
	
	std::string::size_type firstWhite = out.find_first_not_of(p_drop);
	if (firstWhite != std::string::npos)
	{
		// Found leading whitespace
		out.erase(0, firstWhite);
	}
	
	return out;
}


std::wstring trim(const std::wstring& p_string, const std::wstring& p_drop)
{
	// FIXME: This code is identical to the narrow string version, except that it uses wide strings...
	//        A templated solution would be much better.
	std::wstring out(p_string);
	
	std::wstring::size_type lastWhite = out.find_last_not_of(p_drop);
	if (lastWhite != std::wstring::npos)
	{
		// Found trailing whitespace
		out.erase(lastWhite + 1);
	}
	else
	{
		// Could not find anything that is not whitespace
		if (out.find_first_of(p_drop) != std::wstring::npos)
		{
			// Entire string is whitespace
			out.clear();
		}
	}
	
	std::wstring::size_type firstWhite = out.find_first_not_of(p_drop);
	if (firstWhite != std::wstring::npos)
	{
		// Found leading whitespace
		out.erase(0, firstWhite);
	}
	
	return out;
}


bool split(const std::string& p_string, char p_delim,
           std::string& p_firstOUT, std::string& p_secondOUT)
{
	std::string::size_type pos = p_string.find(p_delim);
	if (pos == std::string::npos)
	{
		return false;
	}
	
	p_firstOUT  = p_string.substr(0, pos);
	p_secondOUT = p_string.substr(pos + 1);
	return true;
}


// Namespace end
}
}
