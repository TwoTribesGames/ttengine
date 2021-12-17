#ifndef NO_WIDESTRINGSTREAMS

#include <tt/str/common.h>
#include <tt/str/format.h>
#include <tt/str/StringFormatter.h>
#include <tt/platform/tt_error.h>


namespace tt {
namespace str {

const std::wstring StringFormatter::ms_token(L"$VAR$");


//--------------------------------------------------------------------------------------------------
// Public member functions

StringFormatter::StringFormatter(const std::wstring& p_formatString,
                                 bool                p_numberedParams)
:
m_numberedParams(p_numberedParams),
m_formatString(p_formatString),
m_result(p_formatString),
m_currentParam(0)
{
}


StringFormatter::~StringFormatter()
{
}


std::wstring StringFormatter::getResult(bool p_validate)
{
#if !defined(TT_BUILD_FINAL) // Result validation only makes sense if asserts are available
	if (p_validate)
	{
		// Check for any occurrences of parameter tokens in result string
		if (m_numberedParams == false)
		{
			if (m_result.find(ms_token) != std::wstring::npos)
			{
				TT_PANIC("Not all tokens in string '%s' have been parsed!\n"
				         "Either too many tokens in format string, "
				         "or not enough variables passed.\n"
				         "Number of variables passed: %d",
				         utf16ToUtf8(m_formatString).c_str(), m_currentParam);
			}
		}
		else
		{
			// Search for $VAR([0-9]*)$ in result string
			const std::wstring tokenStart(L"$VAR");
			std::wstring::size_type varIdx = m_result.find(tokenStart);
			if (varIdx != std::wstring::npos)
			{
				// The start of a token remains; check if any numbers are present
				bool foundNumber = false;
				std::wstring::size_type numLen = 0;
				for (std::wstring::size_type numIdx = varIdx + tokenStart.length();
				     numIdx < m_result.length(); ++numIdx)
				{
					// FIXME: This check can probably be more elegant
					std::wstring::value_type c = m_result.at(numIdx);
					if (c == L'0' || c == L'1' || c == L'2' || c == L'3' || c == L'4' ||
					    c == L'5' || c == L'6' || c == L'7' || c == L'8' || c == L'9')
					{
						foundNumber = true;
						++numLen;
					}
					else
					{
						break;
					}
				}
				
				if (foundNumber)
				{
					// A number is present after the token start; check for token end marker
					const std::wstring::size_type tokenEndIdx = varIdx + tokenStart.length() + numLen;
					if (tokenEndIdx < m_result.length() && m_result.at(tokenEndIdx) == L'$')
					{
						// A complete numbered token remains: this is a problem
						TT_PANIC("Not all tokens in string '%s' have been parsed!\n"
						         "Either too many tokens in format string, "
						         "or not enough variables passed.\n"
						         "Number of variables passed: %d\nFormatted string: '%s'",
						         utf16ToUtf8(m_formatString).c_str(), m_currentParam,
						         utf16ToUtf8(m_result).c_str());
					}
				}
			}
		}
	}
#else
	(void)p_validate;
#endif  // !defined(TT_BUILD_FINAL)
	
	return m_result;
}


StringFormatter& StringFormatter::operator<<(s32 p_var)
{
	std::wostringstream var;
	var << p_var;
	replaceParameter(var.str());
	return *this;
}


StringFormatter& StringFormatter::operator<<(u32 p_var)
{
	std::wostringstream var;
	var << p_var;
	replaceParameter(var.str());
	return *this;
}


StringFormatter& StringFormatter::operator<<(s16 p_var)
{
	std::wostringstream var;
	var << p_var;
	replaceParameter(var.str());
	return *this;
}


StringFormatter& StringFormatter::operator<<(u16 p_var)
{
	std::wostringstream var;
	var << p_var;
	replaceParameter(var.str());
	return *this;
}


StringFormatter& StringFormatter::operator<<(char p_var)
{
	std::wostringstream var;
	var << p_var;
	replaceParameter(var.str());
	return *this;
}


StringFormatter& StringFormatter::operator<<(const char* p_var)
{
	replaceParameter(utf8ToUtf16(p_var));
	return *this;
}


StringFormatter& StringFormatter::operator<<(const wchar_t* p_var)
{
	replaceParameter(p_var);
	return *this;
}


StringFormatter& StringFormatter::operator<<(const void* p_var)
{
	std::wostringstream var;
	var << p_var;
	replaceParameter(var.str());
	return *this;
}


StringFormatter& StringFormatter::operator<<(const std::string& p_var)
{
	replaceParameter(utf8ToUtf16(p_var));
	return *this;
}


StringFormatter& StringFormatter::operator<<(const std::wstring& p_var)
{
	replaceParameter(p_var);
	return *this;
}


//--------------------------------------------------------------------------------------------------
// Private member functions

std::wstring StringFormatter::getNextToken()
{
	if (m_numberedParams == false)
	{
		++m_currentParam;
		return ms_token;
	}
	
	// FIXME: Remove hard-coded part of token string somehow.
	std::wostringstream token;
	token << L"$VAR" << (m_currentParam + 1) << L"$";
	++m_currentParam;
	return token.str();
}


void StringFormatter::replaceParameter(const std::wstring& p_val)
{
	s32 replaced = 0;
	if (m_numberedParams)
	{
		replaced = replace(
			m_result,
			getNextToken(),
			p_val);
	}
	else
	{
		std::wstring token(getNextToken());
		std::wstring::size_type idx = m_result.find(token);
		if (idx != std::wstring::npos)
		{
			m_result.replace(idx, token.length(), p_val);
			replaced = 1;
		}
	}
	
	if (replaced == 0)
	{
		TT_PANIC("No more tokens remaining in format string '%s', "
		         "but another variable was passed (variable number %d).",
		         utf16ToUtf8(m_formatString).c_str(), m_currentParam);
	}
}

// Namespace end
}
}


#endif  // !defined(NO_WIDESTRINGSTREAMS)
