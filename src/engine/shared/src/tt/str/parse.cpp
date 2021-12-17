#include <limits>
#include <sstream>

#include <tt/code/ErrorStatus.h>
#include <tt/code/Uncopyable.h>
#include <tt/math/Range.h>
#include <tt/platform/tt_compile_time_error.h>
#include <tt/platform/tt_error.h>
#include <tt/str/parse.h>
#include <tt/str/toStr.h>
#include <tt/platform/tt_printf.h>


namespace tt {
namespace str {


bool parseBool(const std::string& p_string, tt::code::ErrorStatus* p_errStatus)
{
	TT_ERR_CHAIN(bool, false, "parsing bool");
	
	if (p_string == "false")
	{
		return false;
	}
	else if (p_string == "true")
	{
		return true;
	}
	
	TT_ERR_AND_RETURN("Can't parse the string '" << p_string << "' to bool");
}


// "private" helper function.
template<typename Type>
static Type parse(const std::string&     p_string,
                  tt::code::ErrorStatus* p_errStatus,
                  const char*            p_typeName)
{
	TT_ERR_CHAIN(Type, Type(0), "Parsing string to " << p_typeName << ".");
	
	std::istringstream iss(p_string);
	Type result;
	
	iss >> result;
	
	if (!iss || !iss.eof())
	{
		TT_ERR_AND_RETURN("Can't parse the string '" << p_string << "' to " << p_typeName);
	}
	
	return result;
}


template<typename Type,
         template<typename> class LimitCheckPolicy>
static Type parseInt(const std::string&     p_string,
              tt::code::ErrorStatus* p_errStatus,
              const char*            p_typeName)
{
	TT_ERR_CHAIN(Type, Type(0), "ParseInt");
	
	// Check for '.' which indicates a float (real) value.
	std::string::size_type pos = p_string.find('.');
	
	TT_ERR_ASSERTMSG(pos == std::string::npos,
	                 "Can't parse the string '" << p_string << "' to " << p_typeName << ". "
	                 "(It has a '.' so it might be a real (float).)");
	
	typedef typename LimitCheckPolicy<Type>::LargeType LargeType;
	LargeType largeInt = parse<LargeType>(p_string, p_errStatus, p_typeName);
	
	bool result = LimitCheckPolicy<Type>::checkLimits(largeInt, errStatus, p_string, p_typeName);
	TT_ERR_RETURN_ON_ERROR();
	TT_ASSERT(result);
	return static_cast<Type>(largeInt);
}


template<typename Type>
struct SignedIntLimit : private tt::code::Uncopyable
{
	typedef s32 LargeType;
	typedef Type ValueType;
	
	TT_STATIC_ASSERT(std::numeric_limits<Type>::is_signed);
	TT_STATIC_ASSERT(std::numeric_limits<Type>::is_integer);
	//TT_STATIC_ASSERT(std::numeric_limits<Type>::max() <= std::numeric_limits<LargeType>::max());
	//TT_STATIC_ASSERT(std::numeric_limits<Type>::min() >= std::numeric_limits<LargeType>::min());
	
	static bool checkLimits(LargeType p_value,
	                        tt::code::ErrorStatus& p_errStatus,
	                        const std::string& p_string,
	                        const char* p_typeName)
	{
		TT_ASSERT(std::numeric_limits<Type>::max() <= std::numeric_limits<LargeType>::max());
		TT_ASSERT(std::numeric_limits<Type>::min() >= std::numeric_limits<LargeType>::min());
		
		if (p_value > std::numeric_limits<Type>::max())
		{
			TT_ERROR_NAME(p_errStatus,
			         "Error parsing the string '" << p_string << "' to " << p_typeName << ". "
			         "It's higher than the limit of " << p_typeName << "."
			         "(" << toStr(std::numeric_limits<Type>::max()) << ")");
			return false;
		}
		else if (p_value < std::numeric_limits<Type>::min())
		{
			TT_ERROR_NAME(p_errStatus,
			         "Error parsing the string '" << p_string << "' to " << p_typeName << ". "
			         "It's lower than the limit of " << p_typeName << "."
			         "(" << toStr(std::numeric_limits<Type>::min()) << ")");
			return false;
		}
		return true;
	}
};


template<typename Type>
struct UnsignedIntLimit : private tt::code::Uncopyable
{
	typedef u32 LargeType;
	
	TT_STATIC_ASSERT(std::numeric_limits<Type>::is_signed == false);
	TT_STATIC_ASSERT(std::numeric_limits<Type>::is_integer);
	//TT_STATIC_ASSERT(std::numeric_limits<Type>::max() <= std::numeric_limits<LargeType>::max());
	//TT_STATIC_ASSERT(std::numeric_limits<Type>::min() >= std::numeric_limits<LargeType>::min());
	
	static bool checkLimits(LargeType p_value,
	                        tt::code::ErrorStatus& p_errStatus,
	                        const std::string& p_string,
	                        const char* p_typeName)
	{
		TT_ASSERT(std::numeric_limits<Type>::max() <= std::numeric_limits<LargeType>::max());
		TT_ASSERT(std::numeric_limits<Type>::min() >= std::numeric_limits<LargeType>::min());
		
		// The following assert is here because there is no std::numeric_limits<Type>::min() check.
		TT_ASSERT(std::numeric_limits<Type>::min() == 0);
		
		if (p_string.empty() == false && p_string[0] == '-')
		{
			TT_ERROR_NAME(p_errStatus,
			         "Error parsing the string '" << p_string << "' to " << p_typeName << ". "
			         "(Found '-' but it must be a unsigned integer!)");
			return false;
		}
		else if (p_value > std::numeric_limits<Type>::max())
		{
			TT_ERROR_NAME(p_errStatus,
			         "Error parsing the string '" << p_string << "' to " << p_typeName << ". "
			         "It's higher than the limit of " << p_typeName << "."
			         "(" << toStr(std::numeric_limits<Type>::max()) << ")");
			return false;
		}
		return true;
	}
};


s64 parseS64(const std::string& p_string, tt::code::ErrorStatus* p_errStatus)
{
	// NOTE: This is a custom implementation, so that LargeType of the checking
	//       helper structs can remain 32-bit (for performance reasons).
	
	TT_ERR_CHAIN(s64, 0, "parseS64");
	
	// Check for '.' which indicates a float (real) value.
	std::string::size_type pos = p_string.find('.');
	
	TT_ERR_ASSERTMSG(pos == std::string::npos,
	                 "Can't parse the string '" << p_string << "' to s64. "
	                 "(It has a '.' so it might be a real (float).)");
	
	std::istringstream iss(p_string);
	s64 result;
	
	iss >> result;
	
	if (!iss || !iss.eof())
	{
		TT_ERR_AND_RETURN("Can't parse the string '" << p_string << "' to s64.");
	}
	
	TT_ERR_RETURN_ON_ERROR();
	return result;
}


s32 parseS32(const std::string& p_string, tt::code::ErrorStatus* p_errStatus)
{
	return parseInt<s32, SignedIntLimit>(p_string, p_errStatus, "s32");
}


s16 parseS16(const std::string& p_string, tt::code::ErrorStatus* p_errStatus)
{
	return parseInt<s16, SignedIntLimit>(p_string, p_errStatus, "s16");
}


s8 parseS8(const std::string& p_string, tt::code::ErrorStatus* p_errStatus)
{
	return parseInt<s8, SignedIntLimit>(p_string, p_errStatus, "s8");
}


u64 parseU64(const std::string& p_string, tt::code::ErrorStatus* p_errStatus)
{
	// NOTE: This is a custom implementation, so that LargeType of the checking
	//       helper structs can remain 32-bit (for performance reasons).
	
	TT_ERR_CHAIN(u64, 0, "parseU64");
	
	// Check for '.' which indicates a float (real) value.
	std::string::size_type pos = p_string.find('.');
	
	TT_ERR_ASSERTMSG(pos == std::string::npos,
	                 "Can't parse the string '" << p_string << "' to u64. "
	                 "(It has a '.' so it might be a real (float).)");
	TT_ERR_ASSERTMSG(p_string.at(0) != '-',
	                 "Can't parse the string '" << p_string << "' to u64. "
	                 "(It has a '-' so it might be a signed integer.)");
	
	std::istringstream iss(p_string);
	u64 result;
	
	iss >> result;
	
	if (!iss || !iss.eof())
	{
		TT_ERR_AND_RETURN("Can't parse the string '" << p_string << "' to u64.");
	}
	
	TT_ERR_RETURN_ON_ERROR();
	return result;
}


u32 parseU32(const std::string& p_string, tt::code::ErrorStatus* p_errStatus)
{
	return parseInt<u32, UnsignedIntLimit>(p_string, p_errStatus, "u32");
}


u16 parseU16(const std::string& p_string, tt::code::ErrorStatus* p_errStatus)
{
	return parseInt<u16, UnsignedIntLimit>(p_string, p_errStatus, "u16");
}


u8  parseU8( const std::string& p_string, tt::code::ErrorStatus* p_errStatus)
{
	return parseInt<u8, UnsignedIntLimit>(p_string, p_errStatus, "u8");
}


u64 parseU64Hex(const std::string& p_string, tt::code::ErrorStatus* p_errStatus)
{
	// NOTE: This is almost a duplicate of parseU64.
	TT_ERR_CHAIN(u64, 0, "parseU64Hex");
	
	// Check for '.' which indicates a float (real) value.
	std::string::size_type pos = p_string.find('.');
	
	TT_ERR_ASSERTMSG(pos == std::string::npos,
	                 "Can't parse the string '" << p_string << "' to u64. "
	                 "(It has a '.' so it might be a real (float).)");
	TT_ERR_ASSERTMSG(p_string.at(0) != '-',
	                 "Can't parse the string '" << p_string << "' to u64. "
	                 "(It has a '-' so it might be a signed integer.)");
	
	std::istringstream iss(p_string);
	iss.setf(std::ios::hex, std::ios::basefield);
	u64 result;
	
	iss >> result;
	
	if (!iss || !iss.eof())
	{
		TT_ERR_AND_RETURN("Can't parse the string '" << p_string << "' to u64.");
	}
	
	TT_ERR_RETURN_ON_ERROR();
	return result;
}


u32 parseU32Hex(const std::string& p_string, tt::code::ErrorStatus* p_errStatus)
{
	TT_ERR_CHAIN(u32, u32(0), "parseU32Hex");
	
	// Check for '.' which indicates a float (real) value.
	std::string::size_type pos = p_string.find('.');
	
	TT_ERR_ASSERTMSG(pos == std::string::npos,
	                 "Can't parse the string '" << p_string << "' to u32. "
	                 "(It has a '.' so it might be a real (float).)");
	TT_ERR_ASSERTMSG(p_string.at(0) != '-',
	                 "Can't parse the string '" << p_string << "' to u32. "
	                 "(It has a '-' so it might be a signed integer.)");
	
	std::istringstream iss(p_string);
	iss.setf(std::ios::hex, std::ios::basefield);
	u32 result;
	
	iss >> result;
	
	if (!iss || !iss.eof())
	{
		TT_ERR_AND_RETURN("Can't parse the string '" << p_string << "' to u32.");
	}
	
	bool policyCheck = UnsignedIntLimit<u32>::checkLimits(result, errStatus, p_string, "u32");
	TT_ERR_RETURN_ON_ERROR();
	TT_ASSERT(policyCheck);
	return static_cast<u32>(result);
}


real parseReal(const std::string& p_string, tt::code::ErrorStatus* p_errStatus)
{
	return parse<real>(p_string, p_errStatus, "real");
}


math::Range parseRange(const std::string& p_string, tt::code::ErrorStatus* p_errStatus)
{
	// Syntax:    Results in:
	// minmax     Range(minmax)
	// r(min < 0) Range(min, 0)
	// r(max > 0) Range(0, max)
	// r(min,max) Range(min,max)
	// r(max,min) Range(min,max)
	
	TT_ERR_CHAIN(math::Range, math::Range(), "Parsing string to math::Range.");
	if (p_string.empty())
	{
		TT_ERR_AND_RETURN("Can't parse empty string to math::Range.");
	}
	
	if (p_string.find("r(") != 0)
	{
		// handle as an ordinary real
		real minmax = parseReal(p_string, &errStatus);
		TT_ERR_RETURN_ON_ERROR();
		return math::Range(minmax);
	}
	std::string::size_type pos = p_string.find(')', 2);
	if (pos == std::string::npos)
	{
		TT_ERR_AND_RETURN("Expected ')' after 'r(' in '" << p_string << "'.");
	}
	std::string minStr(p_string.substr(2, pos - 2));
	real min = parseReal(minStr, &errStatus);
	TT_ERR_RETURN_ON_ERROR();
	
	if (pos + 1 == p_string.length())
	{
		if (min >= 0.0f)
		{
			return math::Range(0.0f, min);
		}
		else
		{
			return math::Range(min, 0.0f);
		}
	}
	
	if (p_string.find("),(", pos) != pos)
	{
		TT_ERR_AND_RETURN("Expected ',(' after ')' in '" << p_string << "'.");
	}
	std::string::size_type end = p_string.find(")", pos + 3);
	if (end == std::string::npos)
	{
		TT_ERR_AND_RETURN("Expected ')' after ',(' in '" << p_string << "'.");
	}
	
	if (end + 1 != p_string.length())
	{
		TT_ERR_AND_RETURN("Expected no more data after second ')' in '" << p_string << "'.");
	}
	
	std::string maxStr(p_string.substr(pos + 3, end - (pos + 3)));
	real max = parseReal(maxStr, &errStatus);
	TT_ERR_RETURN_ON_ERROR();
	
	if (min > max)
	{
		std::swap(min, max);
	}
	return math::Range(min, max);
}


// No parse of float, int, fixed, etc types. This is to encourage tt type usage.


// Namespace end
}
}
