#ifndef INC_TT_STR_FORMAT_H
#define INC_TT_STR_FORMAT_H

#include <string>

#include <tt/platform/tt_types.h>


namespace tt {
namespace str {


/*! \brief Indicates whether a string is empty or composed entirely of whitespace.
    \note Think visible pixels. */
bool isEmptyOrWhitespace(const std::wstring& p_str);

/*! \brief Replaces a substring with something else within a string.
    \param p_src The string to search & replace in.
    \param p_search The string to search for.
    \param p_replacement The string to replace p_search with
    \return Number of replacements that have occured. */
s32 replace(std::string& p_src, const std::string& p_search,  const std::string& p_replacement);

/*! \brief Replaces a substring with something else within a string.
    \param p_src The string to search & replace in.
    \param p_search The string to search for.
    \param p_replacement The string to replace p_search with
    \return Number of replacements that have occured. */
s32 replace(std::wstring& p_src, const std::wstring& p_search, const std::wstring& p_replacement);

/*! \brief Converts milliseconds to a string with the format MM:SS.ss
           (where "ss" is hundreths of seconds).
    \param p_milliseconds The number of milliseconds.
    \return String with the format MM:SS.ss based on the number of
            milliseconds specified by p_milliseconds. */
std::string formatMillisecondsToMMSSss(s32                p_milliseconds,
                                       const std::string& p_emptySymbol = "-",
                                       const std::string& p_sep1 = ":",
                                       const std::string& p_sep2 = ".");

/*! \brief Converts seconds to a string with the format MM:SS
    \param p_seconds The number of seconds.
    \return String with the format MM:SS based on the number of seconds
            specified by p_seconds. */
std::string formatSecondsToMMSS(s32 p_seconds,
                                const std::string& p_emptySymbol = "-",
                                const std::string& p_sep1 = ":");

/*! \brief Converts seconds to a string with the format HH:MM:SS
    \param p_seconds The number of seconds.
    \return String with the format HH:MM:SS based on the number of seconds
            specified by p_seconds. */
std::string formatSecondsToHHMMSS(s32 p_seconds, 
                                  const std::string& p_emptySymbol = "-",
                                  const std::string& p_sep1 = ":", 
                                  const std::string& p_sep2 = ":");


// Namespace end
}
}


#endif  // !defined(INC_TT_STR_FORMAT_H)
