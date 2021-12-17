#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>
#include <tt/system/Language.h>
#include <tt/loc/LocStr.h>


namespace tt {
namespace system {

	
std::string Language::setLocStrLanguage(tt::loc::LocStr* p_locStr)
{
	(void)p_locStr;
	std::string returnStr("en"); // Default to 'en'.
//#warning implement localization
    /*
	CFArrayRef languages = CFLocaleCopyPreferredLanguages();
	TT_NULL_ASSERT(languages);
	
	if (CFArrayGetCount(languages) > 0)
	{
		for (CFIndex idx = 0; idx < CFArrayGetCount(languages); ++idx)
		{
			// Get language code at index and convert to a C-string.
			CFStringRef preferredLang = (CFStringRef) CFArrayGetValueAtIndex(languages, idx);
			TT_NULL_ASSERT(preferredLang);
			
			enum { BUFFER_SIZE = 4 };
			char buffer[BUFFER_SIZE];
			bool result = CFStringGetCString(preferredLang, buffer, BUFFER_SIZE, kCFStringEncodingASCII);
			
			if (result)
			{
				if (p_locStr->supportsLanguage(buffer))
				{
					TT_Printf("Language::setLocStrLanguage: Found supported language "
					          "on preferred language index: %d\n", idx);
					// Found a supported languages.
					returnStr = buffer;
					break;
				};
			}
			else
			{
				TT_PANIC("CFStringGetString for the preferred language CFString failed.");
			}
		}
	}
	
	CFRelease(languages);
	TT_Printf("Language::setLocStrLanguage: Setting language: '%s'\n", returnStr.c_str());
	p_locStr->selectLanguage(returnStr);
	*/
	return returnStr;	
}
	


std::string Language::getLanguage()
{
	//return "en";
	std::string returnStr("en"); // Default to 'en'.
//#warning implenent localization
    /*
	CFArrayRef languages = CFLocaleCopyPreferredLanguages();
	
	TT_Printf("Language::getLanguage: PreferredLanguages count: %d\n", CFArrayGetCount(languages));
	
	if (CFArrayGetCount(languages) > 0)
	{
		CFStringRef preferredLang = (CFStringRef) CFArrayGetValueAtIndex(languages, 0);
		
		enum { BUFFER_SIZE = 4 };
		char buffer[BUFFER_SIZE];
		bool result = CFStringGetCString(preferredLang, buffer, BUFFER_SIZE, kCFStringEncodingASCII);	
		if (result)
		{
			returnStr = buffer;
		}
		else
		{
			TT_PANIC("CFStringGetString for the preferred language CFString failed.");
		}
	}
	CFRelease(languages);
	TT_Printf("Language::getLanguage: The first preferred lang is: '%s'\n", returnStr.c_str());
	*/
	return returnStr;
}

// Namespace end
}
}
