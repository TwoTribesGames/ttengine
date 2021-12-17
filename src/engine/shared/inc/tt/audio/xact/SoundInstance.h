#ifndef INC_TT_AUDIO_XACT_SOUNDINSTANCE_H
#define INC_TT_AUDIO_XACT_SOUNDINSTANCE_H


#include <vector>

#include <tt/audio/xact/fwd.h>
#include <tt/audio/xact/RPCCurve.h>
#include <tt/math/fwd.h>
#include <tt/platform/tt_types.h>
#include <tt/snd/types.h>


namespace tt {
namespace audio {
namespace xact {

class SoundInstance
{
public:
	SoundInstance(Sound* p_sound, CueInstance* p_cue);
	~SoundInstance();
	
	void addTrack(TrackInstance* p_track);
	
	/*! \return Sound volume in dB. */
	real getVolume() const;
	
	/*! \return Reverb mixing volume in dB. */
	inline real getReverbVolume() const { return m_reverbVolumeInDB; }
	
	/*! \return Category volume in dB. */
	real getCategoryVolume() const;
	
	bool belongsToCategory(const Category* p_category) const;
	
	bool play();
	bool stop();
	bool pause();
	bool resume();
	bool stopCue();
	
	void update(real p_time);
	void updateVolume();
	
	bool setPosition(const math::Vector3& p_position);
	bool setEmitterRadius(real p_inner, real p_outer);
	
	/*! \brief Sets new reverb mixing volume (in dB). */
	void setReverbVolume(real p_volumeInDB);
	
	bool setVariable(const std::string& p_name, real  p_value);
	bool getVariable(const std::string& p_name, real* p_value_OUT);
	
	bool isDone() const;
	void setLooping(bool p_enabled);

	snd::size_type getPriority() const;
	
private:
	SoundInstance(const SoundInstance&);
	SoundInstance& operator=(const SoundInstance&);

	void updateTrackVolume();
	
	typedef std::vector<TrackInstance*> Tracks;
	typedef std::pair<RPCCurve*, real> RPCValue;
	typedef std::vector<RPCValue> ParameterValues;
	
	Sound*          m_sound;
	CueInstance*    m_cue;
	Tracks          m_tracks;
	ParameterValues m_paramValues;

	real m_volumeInDB;
	real m_reverbVolumeInDB;
	real m_rpcVolumeInDB;
};

} // namespace end
}
}

#endif // INC_TT_AUDIO_XACT_SOUNDINSTANCE_H
