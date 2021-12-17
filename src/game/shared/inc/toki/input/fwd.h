#if !defined(INC_TOKI_INPUT_FWD_H)
#define INC_TOKI_INPUT_FWD_H


#include <tt/platform/tt_types.h>


namespace toki {
namespace input {

class Controller;

class Recorder;
typedef tt_ptr<Recorder>::shared RecorderPtr;

class Recording;
typedef tt_ptr<Recording>::shared RecordingPtr;

class RecorderGui;
typedef tt_ptr<RecorderGui>::shared RecorderGuiPtr;


#if defined(TT_BUILD_FINAL)
#	define ENABLE_RECORDER 0
#else
#	define ENABLE_RECORDER 1
#endif


// Namespace end
}
}


#endif  // !defined(INC_TOKI_INPUT_FWD_H)
