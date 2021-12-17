#if !defined(INC_TT_AUDIO_PLAYER_XACT3HELPERS_H)
#define INC_TT_AUDIO_PLAYER_XACT3HELPERS_H


#define NOMINMAX
#if (_WIN32_WINNT >= 0x0602 /*_WIN32_WINNT_WIN8*/)
#include <XAudio2.h>
#else
#include <C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Include\XAudio2.h>
#endif
#include <xact3.h>


namespace tt {
namespace audio {
namespace player {

#if !defined(TT_BUILD_FINAL)

const char* getXactErrorName(HRESULT p_errorCode);
const char* getXactErrorDesc(HRESULT p_errorCode);

#else

inline const char* getXactErrorName(HRESULT) { return ""; }
inline const char* getXactErrorDesc(HRESULT) { return ""; }

#endif

// Namespace end
}
}
}


#endif  // !defined(INC_TT_AUDIO_PLAYER_XACT3HELPERS_H)
