#if !defined(INC_TT_CODE_ENUM_H)
#define INC_TT_CODE_ENUM_H


#include <tt/platform/tt_error.h>
#include <tt/platform/tt_types.h>

/* TT_RegisterEnum helper macro
   
   Make sure the enum and a const char * const getEnumName(ENUM_TYPE) exist.
   
   Creates:
    - a template specialization for EnumCountTrait.
    - bool isValid*(ENUM_TYPE p_enumValue)
    - get*FromName(const std::string& p_name)
    - get*Name(ENUM_TYPE p_enumValue)
   Where * is the enum name.
   
   These are created in the scope where the macro is called.
   
   Example:
   
   enum EnumTest
   {
      EnumTest_Zero,
      EnumTest_One,
      EnumTest_Two,
      
      EnumTest_Count,
      EnumTest_Invalid
   };
   
   inline const char* const getEnumName(EnumTest p_value)
   {
      switch (p_value)
      {
      case EnumTest_Zero:  return "zero";
      case EnumTest_One:   return "one";
      case EnumTest_Two:   return "two";
      default:
         TT_PANIC("Unknown EnumTest value: %d\n", p_value);
         return "";
      }
   }
   
   TT_RegisterEnum(EnumTest, EnumTest_Count);
   */


#define TT_RegisterEnum( ENUM_TYPE, ENUM_COUNT ) \
	template < > \
	class tt::code::EnumCountTrait< ENUM_TYPE > \
	{ \
		public: \
		static const s32 Count = ENUM_COUNT; \
	}; \
	\
	inline bool isValid ## ENUM_TYPE( ENUM_TYPE p_enumValue ) \
	{ \
		return tt::code::isValidEnum(p_enumValue); \
	} \
	\
	inline ENUM_TYPE get ## ENUM_TYPE ## FromName( const std::string& p_name ) \
	{ \
		return tt::code::getEnumFromName<ENUM_TYPE>(p_name); \
	} \
	\
	inline const char * const get ## ENUM_TYPE ## Name( ENUM_TYPE p_enumValue ) \
	{ \
		return getEnumName(p_enumValue); \
	}
	


namespace tt {
namespace code {

template < class EnumType >
class EnumCountTrait
{
public:
	static const s32 Count = -1;
};


template < class EnumType >
class EnumTrait
{
public:
	typedef EnumType Type;
	static const s32      Count        = EnumCountTrait<EnumType>::Count;
	static const EnumType InvalidValue = static_cast<   EnumType>(Count + 1);
	
private:
	EnumTrait(); // Disable construction.
	
	TT_STATIC_ASSERT(Count >= 0); // If this assert is triggered make sure you have a template specialization for the enum.
	                              // That means: using the TT_RegisterEnum define with enum and count as parameters.
};


template < typename EnumType >
inline bool isValidEnum(EnumType p_enum)
{
	return p_enum >= 0 && p_enum < EnumTrait<EnumType>::Count;
}


template < typename EnumType >
inline EnumType getEnumFromName(const std::string& p_name)
{
	for (s32 i = 0; i < EnumTrait<EnumType>::Count; ++i)
	{
		EnumType enumValue = static_cast<EnumType>(i);
		if (getEnumName(enumValue) == p_name)
		{
			return enumValue;
		}
	}
	return EnumTrait<EnumType>::InvalidValue;
}


// Namespace end
}
}


#endif  // !defined(INC_TT_CODE_ENUM_H)
