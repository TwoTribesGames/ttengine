#if !defined(INC_TT_WIRELESS_SCANNER_H)
#define INC_TT_WIRELESS_SCANNER_H

#include <tt/platform/tt_types.h>

#include <tt/wireless/WirelessMgr.h>


namespace tt {
namespace wireless {


class Scanner : private WirelessMgr
{
public:
	
	static bool isBusyScanning()
	{ TT_PANIC("Not implemented for this platform"); return false; }
	
	static bool startLowNoiseChannelScan()
	{ TT_PANIC("Not implemented for this platform"); return false; }
	static u16 getLowNoiseChannel()
	{ TT_PANIC("Not implemented for this platform"); return 0; }
	
	static bool startParentSearch()
	{ TT_PANIC("Not implemented for this platform"); return false; }
	static bool stopParentSearch()
	{ TT_PANIC("Not implemented for this platform"); return false; }
	
	static const WMBssDesc* getParentSearchResult()
	{ TT_PANIC("Not implemented for this platform"); return 0; }
};


// End namespace
}
}

#endif // !defined(INC_TT_WIRELESS_SCANNER_H)
