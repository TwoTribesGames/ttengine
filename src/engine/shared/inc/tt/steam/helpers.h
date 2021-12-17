#if !defined(INC_STEAM_HELPERS_H)
#define INC_STEAM_HELPERS_H

#if !defined(TT_PLATFORM_OSX_IPHONE)  // Steam support works on Mac OS X, but not on iOS


#include <string>

#include <tt/platform/tt_types.h>


namespace tt {
namespace steam {

/*! \return The ISO 639-1 language code deduced from the Steam language setting for this game. */
std::string getGameLanguageID();


/*! \brief Opens a URL in the Steam overlay if the overlay is available,
           or in the user's default browser if no overlay is available. */
void openURL(const std::string& p_url);


/*! \brief Opens the store page for the specified game. 
    \returns False if it couldn't open the overlay/storpage. */
bool openStorePage(u32 p_gameID);


/*! \return The username of the person running Steam */
std::string getUsername();


/*! \return The sanitized username of the person running Steam (only a-z and 0-9 allowed) */
std::string getSanitizedUsername();

// Namespace end
}
}


#endif  // !defined(TT_PLATFORM_OSX_IPHONE)

#endif // !defined(INC_STEAM_HELPERS_H)
