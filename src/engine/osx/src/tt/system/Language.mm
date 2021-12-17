#import <CoreFoundation/CFLocale.h>
#import <CoreFoundation/CFString.h>

#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>
#include <tt/system/Language.h>
#include <tt/loc/LocStr.h>


namespace tt {
namespace system {


/*
std::string Language::getLanguage(std::vector<std::string> p_supportedLanguages)
{
	//return "en";
	std::string returnStr("en"); // Default to 'en'.
	
	CFArrayRef languages = CFLocaleCopyPreferredLanguages();
	
	if (CFArrayGetCount(languages) > 0)
	{
		CFRange fullRange;
		fullRange.location = 0;
		fullRange.length = CFArrayGetCount(languages);
		
		CFIndex found = CFArrayGetCount(languages);
		for (std::vector<std::string>::iterator it = p_supportedLanguages.begin();
		       it != p_supportedLanguages.end(); ++it)
		{
			CFStringRef supportedLang = CFStringCreateWithCString(NULL, (*it).c_str(), kCFStringEncodingASCII);
			
			CFIndex idx = CFArrayGetFirstIndexOfValue(languages, fullRange, supportedLang);
			if (idx != -1 && idx < found)
			{
				found = idx;
			}
			
			CFRelease(supportedLang);
		}
		
		if (found < CFArrayGetCount(languages))
		{
			TT_ASSERT(found >= 0);
			
			CFStringRef preferredLang = (CFStringRef) CFArrayGetValueAtIndex(languages, found);
			
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
			CFRelease(preferredLang);
		}
	}
	CFRelease(languages);
	TT_Printf("Language::getLanguage: The first preferred lang is: '%s'\n", returnStr.c_str());
	
	return returnStr;
}
 */
	
	
	
std::string Language::setLocStrLanguage(tt::loc::LocStr* p_locStr)
{
	std::string returnStr(getLanguage());
	
	TT_Printf("Language::setLocStrLanguage: Setting language: '%s'\n", returnStr.c_str());
	p_locStr->selectLanguage(returnStr);
	
	return returnStr;	
}
	


std::string Language::getLanguage()
{
	//return "en";
	std::string returnStr("en"); // Default to 'en'.
	
	CFArrayRef languages = CFLocaleCopyPreferredLanguages();
	
	TT_Printf("Language::getLanguage: PreferredLanguages count: %d\n", CFArrayGetCount(languages));
	
	if (CFArrayGetCount(languages) > 0)
	{
		CFStringRef preferredLang = (CFStringRef) CFArrayGetValueAtIndex(languages, 0);
		
		enum { BUFFER_SIZE = 5 };
		char buffer[BUFFER_SIZE];
		bool result = CFStringGetCString(preferredLang, buffer, BUFFER_SIZE, kCFStringEncodingASCII);	
		if (result)
		{
			returnStr = std::string(buffer).substr(0, 2);
		}
		else
		{
			TT_PANIC("CFStringGetString for the preferred language CFString failed.");
		}
	}
	CFRelease(languages);
	TT_Printf("Language::getLanguage: The first preferred lang is: '%s'\n", returnStr.c_str());
	
	return returnStr;
}

// Namespace end
}
}
