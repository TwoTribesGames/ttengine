#ifndef INC_TT_STR_TOSTR_H
#define INC_TT_STR_TOSTR_H

#include <iomanip>
#include <sstream>

#include <tt/math/Range.h>
#include <tt/platform/tt_types.h>


namespace tt {
namespace str {

/*! \brief Print a type to a std::string. */
template <typename Type>
inline std::string toStr(Type p_value)
{
	std::ostringstream oss;
	oss << p_value;
	return oss.str();
}

/*! \brief toStr specialization for bool */
template <>
inline std::string toStr<bool>(bool p_value)
{
	std::ostringstream oss;
	oss << std::boolalpha << p_value;
	return oss.str();
}


/*! \brief toStr specialization for s8 */
template <>
inline std::string toStr<s8>(s8 p_value)
{
	std::ostringstream oss;
	oss << s16(p_value);
	return oss.str();
}


/*! \brief toStr specialization for u8 */
template <>
inline std::string toStr<u8>(u8 p_value)
{
	std::ostringstream oss;
	oss << u16(p_value);
	return oss.str();
}

/*! \brief toStr specialization for math::Range */
template <>
inline std::string toStr<math::Range>(math::Range p_value)
{
	std::ostringstream oss;
	
	if (p_value.getRange() > 0)
	{
		oss << "r(";
		if (p_value.getMin() == 0.0f)
		{
			oss << p_value.getMax();
			oss << ")";
		}
		else if (p_value.getMax() == 0.0f)
		{
			oss << p_value.getMin();
			oss << ")";
		}
		else
		{
			oss << p_value.getMin();
			oss << "),(";
			oss << p_value.getMax();
			oss << ")";
		}
	}
	else
	{
		oss << p_value.getMin();
	}
	return oss.str();
}


/*! \brief toStr overload for std::string (simple pass-through; useful for template code) */
inline const std::string& toStr(const std::string& p_value)
{
	return p_value;
}


// Namespace end
}
}


#endif  // !defined(INC_TT_STR_TOSTR_H)
