#if !defined(INC_TT_ENGINE_DEBUG_SCREEN_CAPTURE_H)
#define INC_TT_ENGINE_DEBUG_SCREEN_CAPTURE_H


namespace tt {
namespace engine {
namespace debug {

// These functions must be overriden by platform specific implementations

bool saveScreenCaptureToFile(const std::string& /*p_filename*/, bool /*p_withTransparency*/) { return false; }

bool saveScreenCaptureToClipboard() { return false; }


// Namespace end
}
}
}

#endif // INC_TT_ENGINE_DEBUG_SCREEN_CAPTURE_H
