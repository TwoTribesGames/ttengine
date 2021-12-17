#if !defined(INC_TT_WIRELESS_WIRELESSHELPER_H)
#define INC_TT_WIRELESS_WIRELESSHELPER_H

#include <tt/platform/tt_printf.h>
#include <tt/platform/tt_error.h>


namespace tt {
namespace wireless {


class WirelessHelper
{
public:
	static bool isError(WMErrCode /*p_code*/)
	{ TT_PANIC("Not implemented for this platform"); return false; }
	static const char* getErrorCodeName(WMErrCode /*p_code*/)
	{ TT_PANIC("Not implemented for this platform"); return "ERR"; }
};


// End namespace
}
}


#endif //!defined(INC_TT_WIRELESS_WIRELESSHELPER_H)
