#ifndef INC_TT_SETTINGS_MAC_H
#define INC_TT_SETTINGS_MAC_H


#include <string>

#include <tt/platform/tt_types.h>

namespace tt {
namespace settings {

/*! \brief Gets the MAC address.
    \param p_addressOUT Buffer to hold address (must be at least 6 bytes large).
    \return whether getting the MAC address succeeded.*/
bool getMacAddress(u8* p_addressOUT);

/*! \brief Formats a mac address as a string.
    \param p_address The address to format.
    \return The mac address formatted as xx-xx-xx-xx-xx-xx.*/
std::string formatMacAddress(const u8* p_address);

// Namespace end
}
}


#endif  // !defined(INC_TT_SETTINGS_MAC_H)

