#ifndef INC_TT_STR_PARSE_H
#define INC_TT_STR_PARSE_H

#include <string>

#include <tt/platform/tt_types.h>

// Forward declaration
namespace tt
{
	namespace code
	{
		class ErrorStatus;
	}
	namespace math
	{
		class Range;
	}
}


namespace tt {
namespace str {

/*! \brief Parses the specified string as a boolean.
    \param p_string The string to parse as a boolean.
    \param p_errStatus Pointer to a ErrorStatus object used for error handling.
    \return boolean value parsed from string. (false if parsing failed.) */
bool parseBool(const std::string& p_string, tt::code::ErrorStatus* p_errStatus);

/*! \brief Parses the specified string as a integer.
    \param p_string The string to parse as a integer.
    \param p_errStatus Pointer to a ErrorStatus object used for error handling.
    \return Integer value parsed from string. (0 if parsing failed.) */
s64 parseS64(const std::string& p_string, tt::code::ErrorStatus* p_errStatus);
s32 parseS32(const std::string& p_string, tt::code::ErrorStatus* p_errStatus);
s16 parseS16(const std::string& p_string, tt::code::ErrorStatus* p_errStatus);
s8  parseS8( const std::string& p_string, tt::code::ErrorStatus* p_errStatus);
u64 parseU64(const std::string& p_string, tt::code::ErrorStatus* p_errStatus);
u32 parseU32(const std::string& p_string, tt::code::ErrorStatus* p_errStatus);
u16 parseU16(const std::string& p_string, tt::code::ErrorStatus* p_errStatus);
u8  parseU8( const std::string& p_string, tt::code::ErrorStatus* p_errStatus);

u64 parseU64Hex(const std::string& p_string, tt::code::ErrorStatus* p_errStatus);
u32 parseU32Hex(const std::string& p_string, tt::code::ErrorStatus* p_errStatus);

/*! \brief Parses the specified string as a real.
    \param p_string The string to parse as a real.
    \param p_errStatus Pointer to a ErrorStatus object used for error handling.
    \return real value parsed from string. (0 if parsing failed.) */
real parseReal(const std::string& p_string, tt::code::ErrorStatus* p_errStatus);

/*! \brief Parses the specified string as a range.
    \param p_string The string to parse as a range.
    \param p_errStatus Pointer to a ErrorStatus object used for error handling.
    \return range value parsed from string. (0 if parsing failed.) */
math::Range parseRange(const std::string& p_string, tt::code::ErrorStatus* p_errStatus);

// Namespace end
}
}


#endif  // !defined(INC_TT_STR_PARSE_H)
