#if !defined(INC_TT_INPUT_PLATFORMFWD_H)
#define INC_TT_INPUT_PLATFORMFWD_H


#include <tt/platform/tt_types.h>


namespace tt {
namespace input {

#if defined(TT_PLATFORM_OSX_IPHONE)

// iPhone input classes

struct IPhoneController;

#else

// Desktop OS X input classes

struct KeyboardController;
struct MouseController;

class MouseCursor; // custom mouse cursor for MouseController
typedef tt_ptr<MouseCursor>::shared MouseCursorPtr;

#endif

// Namespace end
}
}


#endif  // !defined(INC_TT_INPUT_PLATFORMFWD_H)
