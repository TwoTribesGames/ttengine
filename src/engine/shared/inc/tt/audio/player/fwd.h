#if !defined(INC_TT_AUDIO_PLAYER_FWD_H)
#define INC_TT_AUDIO_PLAYER_FWD_H


#include <tt/audio/player/platform_fwd.h>
#include <tt/platform/tt_types.h>


namespace tt {
namespace audio {
namespace player {

class MusicJinglePlayer;
class MusicPlayer;
class MusicPlayerCallbackInterface;
class SoundCue;
class SoundCueSettings;
class SoundPlayer;
class TTIMPlayer;
class TTXactCue;
class TTXactPlayer;
class XMMusicPlayer;


typedef tt_ptr<MusicJinglePlayer>::shared MusicJinglePlayerPtr;
typedef tt_ptr<MusicPlayer      >::shared MusicPlayerPtr;
typedef tt_ptr<SoundPlayer      >::shared SoundPlayerPtr;

typedef tt_ptr<MusicPlayerCallbackInterface>::shared MusicPlayerCallbackInterfacePtr;
typedef tt_ptr<MusicPlayerCallbackInterface>::weak   MusicPlayerCallbackInterfaceWeakPtr;

typedef tt_ptr<SoundCue>::shared SoundCuePtr;
typedef tt_ptr<SoundCue>::weak   SoundCueWeakPtr;
typedef tt_ptr<SoundCueSettings>::shared SoundCueSettingsPtr;

// Namespace end
}
}
}


#endif  // !defined(INC_TT_AUDIO_PLAYER_FWD_H)
