#if !defined(INC_TOKI_AUDIO_FWD_H)
#define INC_TOKI_AUDIO_FWD_H


#include <tt/code/Handle.h>
#include <tt/platform/tt_types.h>


namespace toki {
namespace audio {

class AudioPlayer;
class MusicTrack;
class MusicTrackMgr;
class PositionalEffect;
class SoundCue;



typedef tt::code::Handle<MusicTrack> MusicTrackHandle;

typedef tt_ptr<SoundCue>::shared SoundCuePtr;
typedef tt_ptr<SoundCue>::weak   SoundCueWeakPtr;
typedef tt_ptr<PositionalEffect>::shared PositionalEffectPtr;


class AudioPositionInterface
{
public:
	virtual const tt::math::Vector3& getAudioPosition() const = 0;
protected:
	inline virtual ~AudioPositionInterface() {};
};
typedef tt_ptr<AudioPositionInterface>::shared AudioPositionInterfacePtr;
typedef tt_ptr<AudioPositionInterface>::weak   AudioPositionInterfaceWeakPtr;


class SoundCueWithPosition;
typedef tt_ptr<SoundCueWithPosition>::shared SoundCueWithPositionPtr;
typedef tt_ptr<SoundCueWithPosition>::weak   SoundCueWithPositionWeakPtr;


// Namespace end
}
}


#endif  // !defined(INC_TOKI_AUDIO_FWD_H)
