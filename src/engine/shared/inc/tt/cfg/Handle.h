#if !defined(INC_TT_CFG_HANDLE_H)
#define INC_TT_CFG_HANDLE_H


#include <string>

#include <tt/platform/tt_types.h>


namespace tt {
namespace cfg {

class ConfigHive;

/*! \brief Type-safe handle for referring to configuration options. */
template<typename T>
struct Handle
{
	typedef T value_type;
	
	Handle()
	:
	index(-1)
#if !defined(TT_BUILD_FINAL)
	,
	optionName(),
	source(0)
#endif
	{ }
	
	inline bool isValid() const { return index >= 0; }
	
private:
	Handle(const std::string& p_optionName)
	:
	index(-1)
#if !defined(TT_BUILD_FINAL)
	,
	optionName(p_optionName),
	source(0)
#endif
	{ (void)p_optionName; }
	
	
	s32 index;
	
#if !defined(TT_BUILD_FINAL)
	std::string       optionName; //!< So that sensible panic messages can be generated.
	const ConfigHive* source;     //!< Used for checking whether handle is valid for ConfigHive.
#endif
	
	friend class ConfigHive;
};


// Type definitions for the specific handle types supported by the config system
typedef Handle<const char*> HandleString;
typedef Handle<real>        HandleReal;
typedef Handle<s32>         HandleInteger;
typedef Handle<bool>        HandleBool;

// Namespace end
}
}


#endif  // !defined(INC_TT_CFG_HANDLE_H)
