#include <tt/settings/mac.h>
#include <tt/platform/tt_error.h>


namespace tt {
namespace settings {

bool getMacAddress(u8* p_addressOUT)
{
	// FIXME: Can get the Mac address by parsing virtual file
	//        /sys/class/net/eth0/address
	(void)p_addressOUT;
	TT_PANIC("Not implemented yet");
	return false;
}

// Namespace end
}
}
