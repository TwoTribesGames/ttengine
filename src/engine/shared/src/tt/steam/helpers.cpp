#if !defined(TT_PLATFORM_OSX_IPHONE)  // Steam support works on Mac OS X, but not on iOS

#include <steam/steam_api.h>

#include <tt/http/HttpConnectMgr.h>
#include <tt/platform/tt_error.h>
#include <tt/steam/helpers.h>
#include <tt/str/str.h>


namespace tt {
namespace steam {

std::string getGameLanguageID()
{
	if (SteamApps() == 0)
	{
		TT_WARN("Steam API not available. Cannot determine language.");
		return std::string();
	}
	
	const std::string language(SteamApps()->GetCurrentGameLanguage());
	
	// NOTE: This list contains all languages currently supported by Steam
	if      (language == "english"   ) return "en";
	else if (language == "german"    ) return "de";
	else if (language == "french"    ) return "fr";
	else if (language == "spanish"   ) return "es";
	else if (language == "italian"   ) return "it";
	else if (language == "dutch"     ) return "nl";
	else if (language == "korean"    ) return "ko";
	else if (language == "schinese"  ) return "zh"; // NOTE: ISO 639-1 makes no distinction between simplified and traditional Chinese
	else if (language == "tchinese"  ) return "zh"; // NOTE: ISO 639-1 makes no distinction between simplified and traditional Chinese
	else if (language == "russian"   ) return "ru";
	else if (language == "thai"      ) return "th";
	else if (language == "japanese"  ) return "jp";
	else if (language == "portuguese") return "pt";
	else if (language == "polish"    ) return "pl";
	else if (language == "danish"    ) return "da";
	else if (language == "finnish"   ) return "fi";
	else if (language == "norwegian" ) return "no";
	else if (language == "swedish"   ) return "sv";
	else if (language == "hungarian" ) return "hu";
	else if (language == "czech"     ) return "cs";
	else if (language == "romanian"  ) return "ro";
	else if (language == "turkish"   ) return "tr";
	else if (language == "arabic"    ) return "ar";
	else if (language == "brazilian" ) return "pt"; // NOTE: ISO 639-1 makes no distinction between Portuguese and Brazilian
	else if (language == "bulgarian" ) return "bg";
	else if (language.empty())         return "en"; // Apparently this might happen
	
	TT_PANIC("Unrecognized Steam language name: '%s'.", language.c_str());
	return std::string();
}


void openURL(const std::string& p_url)
{
	ISteamUtils*   steamUtils   = SteamUtils();
	ISteamUser*    steamUser    = SteamUser();
	ISteamFriends* steamFriends = SteamFriends();
	
	const bool overlayAvailable =
		steamUtils   != 0 && steamUtils->IsOverlayEnabled() &&  // Overlay initialized
		steamUser    != 0 && steamUser->BLoggedOn()         &&  // Steam online
		steamFriends != 0;                                      // Steam friends API available
	
	if (overlayAvailable)
	{
		steamFriends->ActivateGameOverlayToWebPage(p_url.c_str());
	}
	else
	{
		if (http::HttpConnectMgr::hasInstance())
		{
			http::HttpConnectMgr::getInstance()->openUrlExternally(p_url);
		}
	}
}


bool openStorePage(u32 p_gameID)
{
	ISteamUtils*   steamUtils   = SteamUtils();
	ISteamUser*    steamUser    = SteamUser();
	ISteamFriends* steamFriends = SteamFriends();
	
	const bool overlayAvailable =
		steamUtils   != 0 && steamUtils->IsOverlayEnabled() &&  // Overlay initialized
		steamUser    != 0 && steamUser->BLoggedOn()         &&  // Steam online
		steamFriends != 0;                                      // Steam friends API available
	
	if (overlayAvailable)
	{
		// Possible parameters for the function call below.
		// k_EOverlayToStoreFlag_None
		// k_EOverlayToStoreFlag_AddToCart
		// k_EOverlayToStoreFlag_AddToCartAndShow
		steamFriends->ActivateGameOverlayToStore(p_gameID, k_EOverlayToStoreFlag_None);
		return true;
	}
	return false;
}


std::string getUsername()
{
	ISteamFriends* steamFriends = SteamFriends();
	if (steamFriends != 0)
	{
		return std::string(steamFriends->GetPersonaName());
	}
	
	TT_PANIC("Unable to retrieve username; Steam friends API is not available");
	
	return "";
}


std::string getSanitizedUsername()
{
	std::string name(tt::steam::getUsername());
	
	// Sanitize the username; only allow a-z0-9 with a max of 32 chars
	name = tt::str::toLower(name.substr(0, 32));
	std::string result;
	for (std::string::const_iterator it = name.begin(); it != name.end(); ++it)
	{
		u8 c = static_cast<u8>(*it);
		if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9'))
		{
			result.push_back(c);
		}
		else
		{
			result.push_back('-');
		}
	}
	
	return result;
}

// Namespace end
}
}


#endif  // !defined(TT_PLATFORM_OSX_IPHONE)
