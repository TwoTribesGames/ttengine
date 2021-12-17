#if !defined(INC_TT_WIRELESS_CHILD_H)
#define INC_TT_WIRELESS_CHILD_H


#include <tt/platform/tt_types.h>

#include <tt/wireless/Connection.h>

namespace tt {
namespace wireless {


class Child : private Connection
{
public:
	static bool startChild(const WMBssDesc* /*p_bssDesc*/)
	{ TT_PANIC("Not implemented for this platform"); return false; }
};


// End namespace
}
}

#endif // !defined(INC_TT_WIRELESS_CHILD_H)
