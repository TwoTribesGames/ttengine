#if !defined(INC_TT_FILEFORMAT_CURSOR_FWD_H)
#define INC_TT_FILEFORMAT_CURSOR_FWD_H


#include <tt/platform/tt_types.h>


namespace tt {
namespace fileformat {
namespace cursor {

class CursorData;

class CursorDirectory;
typedef tt_ptr<CursorDirectory>::shared CursorDirectoryPtr;

// Namespace end
}
}
}


#endif  // !defined(INC_TT_FILEFORMAT_CURSOR_FWD_H)
