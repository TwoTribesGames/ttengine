#ifndef INC_TT_STR_MANIP_H
#define INC_TT_STR_MANIP_H


#include <tt/str/str_types.h>


namespace tt {
namespace str {

/*! \brief Chop a line into tokens.
    
    \param p_line   Input line.
    \param p_delims String containing all delimiters.
    \return Vector of string tokens. */
Strings explode(const std::string& p_line, const std::string& p_delims, bool p_allowEmptyTokens = false);

std::vector<std::wstring> explode(const std::wstring& p_line, const std::wstring& p_delims,
                                  bool p_allowEmptyTokens = false);


/*! \brief Join array elements with a glue string. */
std::string implode(const Strings& p_pieces, const std::string& p_glue);
std::string implode(Strings::const_iterator p_piecesBegin, Strings::const_iterator p_piecesEnd,
                    const std::string& p_glue);


/*! \brief Trims the leading and trailing whitespace from a string.
    \param p_string The string to trim.
    \param p_drop The characters to remove (default = ' \t\n\r').
    \return Trimmed string. */
std::string trim(const std::string& p_string, const std::string& p_drop = " \t\n\r");
std::wstring trim(const std::wstring& p_string, const std::wstring& p_drop = L" \t\n\r");


/*! \brief Splits a string at _first_ occurence of the delimiter.
    \param p_string    The string to be split.
    \param p_delim     The character to use as delimiter.
    \param p_firstOUT  The part of the string before the delimiter.
    \param p_secondOUT The part of the string after the delimiter.
    \return            Whether the string was split. */
bool split(const std::string& p_string, char p_delim, 
           std::string& p_firstOUT, std::string& p_secondOUT);


// Namespace end
}
}


#endif  // !defined(INC_TT_STR_MANIP_H)
