#if !defined(INC_TT_WIRELESS_PARENT_H)
#define INC_TT_WIRELESS_PARENT_H

#include <tt/platform/tt_types.h>

#include <tt/wireless/Connection.h>


namespace tt {
namespace wireless {


class Parent : private Connection
{
public:
	static bool startParent(u16 /*p_channel*/)
	{ TT_PANIC("Not implemented for this platform"); return false; }
	
	static s32 getChildrenCount()
	{ return 0; }
};


// End namespace
}
}

#endif // !defined(INC_TT_WIRELESS_PARENT_H)
