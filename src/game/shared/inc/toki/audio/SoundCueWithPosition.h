#if !defined(INC_TOKI_AUDIO_SOUNDCUEWITHPOSITION_H)
#define INC_TOKI_AUDIO_SOUNDCUEWITHPOSITION_H

#include <string>

#include <tt/platform/tt_types.h>
#include <tt/math/Vector3.h>

#include <toki/audio/fwd.h>
#include <toki/audio/AudioPlayer.h>


namespace toki {
namespace audio {


class SoundCueWithPosition : public AudioPositionInterface
{
public:
	virtual inline const tt::math::Vector3& getAudioPosition() const { return m_audioPostion; }
	inline void setAudioPosition(const tt::math::Vector3& p_pos) { m_audioPostion = p_pos; }
	
	inline const tt::audio::player::SoundCuePtr& getCue() const { return m_cue; }
	
	static inline SoundCueWithPositionPtr create(const std::string& p_soundbank, const std::string& p_name,
	                                             const tt::math::Vector3& p_audioPostion)
	{
		SoundCueWithPositionPtr ptr(new SoundCueWithPosition(p_audioPostion));
		TT_NULL_ASSERT(ptr);
		ptr->initCue(ptr, p_soundbank, p_name);
		return ptr;
	}
	
private:
	SoundCueWithPosition(const SoundCueWithPosition&);                  // Not implemented. No copy
	const SoundCueWithPosition& operator=(const SoundCueWithPosition&); // Not implemented. No assignment
	
	inline SoundCueWithPosition(const tt::math::Vector3& p_audioPostion)
	:
	m_audioPostion(p_audioPostion)
	{
	}
	
	inline void initCue(const AudioPositionInterfacePtr& p_this, const std::string& p_soundbank, const std::string& p_name)
	{
		m_cue = AudioPlayer::getInstance()->playPositionalEffectCue(p_soundbank, p_name, p_this);
	}
	
	tt::math::Vector3              m_audioPostion;
	tt::audio::player::SoundCuePtr m_cue;
};


// Namespace end
}
}


#endif  // !defined(INC_TOKI_AUDIO_SOUNDCUEWITHPOSITION_H)
