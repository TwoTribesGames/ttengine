#if !defined(INC_TT_APP_WINDOWMESSAGEHELPERS_H)
#define INC_TT_APP_WINDOWMESSAGEHELPERS_H


#if !defined(NOMINMAX)
#define NOMINMAX
#endif
#include <windows.h>


namespace tt {
namespace app {

/*! \brief Debug helper (non-final only) that returns a human-readable name
           for the specified window message (e.g. WM_MOUSEMOVE). */
const char* getMessageName(UINT p_message);

// Namespace end
}
}


#endif  // !defined(INC_TT_APP_WINDOWMESSAGEHELPERS_H)
