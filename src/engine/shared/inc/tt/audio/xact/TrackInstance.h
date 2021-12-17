#ifndef INC_TT_AUDIO_XACT_TRACKINSTANCE_H
#define INC_TT_AUDIO_XACT_TRACKINSTANCE_H


#include <vector>

#include <tt/audio/xact/fwd.h>
#include <tt/math/fwd.h>
#include <tt/platform/tt_types.h>
#include <tt/snd/types.h>


namespace tt {
namespace audio {
namespace xact {

class TrackInstance
{
public:
	TrackInstance(Track* p_track, SoundInstance* p_sound);
	~TrackInstance();
	
	void setPlayEvent(PlayWaveEventInstance* p_event);
	void setStopEvent(StopEventInstance* p_event);
	void addVolumeEvent(VolumeEventInstance* p_event);
	void addPitchEvent(PitchEventInstance* p_event);
	
	void setPitch(real p_pitch);
	inline real getPitch() const { return m_pitch; }
	
	/*! \brief Sets the track volume (in dB). */
	void setVolume(real p_volumeInDB);
	
	/*! \return Track volume in dB. */
	real getVolume() const;
	
	/*! \return Category volume in dB. */
	real getCategoryVolume() const;
	
	/*! \return Sound volume in dB. */
	real getBaseVolume() const;
	
	/*! \return Reverb mixing volume in dB (retrieved from SoundInstance; this is not a separate setting!). */
	real getReverbVolume() const;
	
	bool play();
	bool stop();
	bool pause();
	bool resume();
	bool stopCue();
	
	void update(real p_time);
	void updateVolume();
	void updateReverbVolume();  // applies the current SoundInstance reverb volume to the playing audio
	
	bool setPosition     (const math::Vector3& p_position);
	bool setEmitterRadius(real p_inner, real p_outer);
	
	bool isDone() const;
	
	void setLooping(bool p_enabled);

	snd::size_type getPriority() const;
	
private:
	TrackInstance(const TrackInstance&);
	TrackInstance& operator=(const TrackInstance&);
	
	typedef std::vector<VolumeEventInstance*> VolumeEvents;
	typedef std::vector<PitchEventInstance*>  PitchEvents;
	
	Track*                 m_track;
	SoundInstance*         m_sound;
	PlayWaveEventInstance* m_playEvent;
	StopEventInstance*     m_stopEvent;
	VolumeEvents           m_volEvents;
	PitchEvents            m_pitchEvents;
	
	real                   m_pitch;
	real                   m_volumeInDB;
};

} // namespace end
}
}

#endif // INC_TT_AUDIO_XACT_TRACKINSTANCE_H
