#if !defined(INC_TT_WIRELESS_WIRELESSMGR_H)
#define INC_TT_WIRELESS_WIRELESSMGR_H

#include <string>

// Forward declaration
struct WMBssDesc {};


namespace tt {
namespace wireless {

class WirelessMgr
{
public:
	enum ErrorCode
	{
		ErrorCode_NoError,
		
		ErrorCode_DisconnectedFromHost, // Client got disconnected from the host.
		ErrorCode_NoChannelsAvailable,  // WM_GetAllowedChannel() returned 0. 
		                                // Wireless communications is prohibited
		ErrorCode_NoChannelFound,       // While measuring channels no channel without noise was found.
		
		ErrorCode_HostDoesNotAcceptEntry, //Parent doesn't accept entry now
		ErrorCode_HostFull                //Over max entry of BSS group
	};
	
	static bool isOn()
	{ TT_WARN("Not implemented for this platform"); return false; }
	static bool turnOn()
	{ TT_WARN("Not implemented for this platform"); return false; }
	static bool turnOff()
	{ TT_WARN("Not implemented for this platform"); return false; }
	static bool isIdle()
	{ TT_WARN("Not implemented for this platform"); return false; }
	
	static bool hasNewError()
	{ TT_WARN("Not implemented for this platform"); return false; }
	static bool hasError()
	{ TT_WARN("Not implemented for this platform"); return false; }
};


// End namespace
}
}

#endif // !defined(INC_TT_WIRELESS_WIRELESSMGR_H)
