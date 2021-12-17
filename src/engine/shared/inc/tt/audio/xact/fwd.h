#if !defined(INC_TT_AUDIO_XACT_FWD_H)
#define INC_TT_AUDIO_XACT_FWD_H


#include <tt/platform/tt_types.h>


namespace tt {
namespace audio {
namespace xact {

class AudioTT;
class Category;
class Cue;
class CueInstance;
class PitchEvent;
class PitchEventInstance;
class PlayWaveEvent;
class PlayWaveEventInstance;
class RPCCurve;
class RuntimeParameterControl;
class Sound;
class SoundBank;
class SoundInstance;
class StopEvent;
class StopEventInstance;
class Track;
class TrackInstance;
class VolumeEvent;
class VolumeEventInstance;
class Wave;
class WaveBank;
class WaveInstance;

typedef tt_ptr<CueInstance>::shared CueInstancePtr;
typedef tt_ptr<CueInstance>::weak   CueInstanceWeakPtr;

// Namespace end
}
}
}


#endif  // !defined(INC_TT_AUDIO_XACT_FWD_H)
