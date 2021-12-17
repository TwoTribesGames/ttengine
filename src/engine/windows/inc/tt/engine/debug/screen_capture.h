#if !defined(INC_TT_ENGINE_DEBUG_SCREEN_CAPTURE_H)
#define INC_TT_ENGINE_DEBUG_SCREEN_CAPTURE_H

#include <string>


namespace tt {
namespace engine {
namespace debug {

bool saveScreenCaptureToFile(const std::string& p_filename, bool p_withTransparency);

bool saveScreenCaptureToClipboard();


// Namespace end
}
}
}

#endif // INC_TT_ENGINE_DEBUG_SCREEN_CAPTURE_H
