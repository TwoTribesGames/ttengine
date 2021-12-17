#if !defined(INC_TT_WIRELESS_CONNECTION_H)
#define INC_TT_WIRELESS_CONNECTION_H

#include <tt/platform/tt_types.h>

#include <tt/wireless/WirelessMgr.h>


namespace tt {
namespace wireless {


class Connection : protected WirelessMgr
{
public:
	static bool readyToSendOrRecieve()
	{ TT_WARN("Not implemented for this platform"); return false; }
	static bool send(const u8* /*p_data*/, u32 /*p_size*/)
	{ TT_PANIC("Not implemented for this platform"); return false; }
	
	static bool recievedData()
	{ /*TT_PANIC("Not implemented for this platform");*/ return false; }
	static u32 recieve(u8* /*p_data*/, u32 /*p_size*/)
	{ TT_PANIC("Not implemented for this platform"); return 0; }
};


// End namespace
}
}

#endif // !defined(INC_TT_WIRELESS_CONNECTION_H)
