#ifndef INC_TT_SYSTEM_LANGUAGE_H
#define INC_TT_SYSTEM_LANGUAGE_H


#include <string>
#include <vector>

#include <tt/platform/tt_types.h>

// Forward declaration
namespace tt
{
	namespace loc
	{
		class LocStr;
	}
}


namespace tt {
namespace system {


/**
 * Language class provides encapsulation around retrieving language settings.
 */
class Language
{
public:
	static void preInit();
	
	/* \brief Sets the passed LocStr to the preferred language based on the systems languages settings.
	   \return The language code that was selected. */
	static std::string setLocStrLanguage(loc::LocStr* p_locStr);
	
	
	/*! \brief Returns the currently selected language in 2 character lowercase ISO 639 format.
	           see also http://www.wwp.brown.edu/encoding/training/ISO/iso639.html
	    \return The currenty selected language in 2 character lowercase ISO 639 format. */
	static std::string getLanguage();
};

// Namespace end
}
}


#endif  // !defined(INC_TT_SYSTEM_LANGUAGE_H)

