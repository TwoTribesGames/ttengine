#ifndef INC_TT_AUDIO_XACT_PLAYWAVEEVENTINSTANCE_H
#define INC_TT_AUDIO_XACT_PLAYWAVEEVENTINSTANCE_H


#include <utility>
#include <vector>

#include <tt/audio/xact/fwd.h>
#include <tt/audio/xact/PlayWaveEvent.h>
#include <tt/math/Vector3.h>
#include <tt/platform/tt_types.h>


namespace tt {
namespace audio {
namespace xact {

class PlayWaveEventInstance
{
public:
	PlayWaveEventInstance(PlayWaveEvent* p_playEvent, TrackInstance* p_track);
	
	void setVolume(real p_volumeInDB);
	
	/*! \return Volume in dB. */
	inline real getVolume() const { return m_volumeInDB; }
	
	void setPitch(real p_pitch);
	inline real getPitch() const { return m_pitch; }
	
	void setPan(int p_pan);
	inline int getPan() const { return m_pan; }

	bool setPosition     (const math::Vector3& p_position);
	bool setEmitterRadius(real p_inner, real p_outer);
	
	bool play();
	bool stop();
	bool pause();
	bool resume();
	
	void update(real p_time);
	void updateVolume(real p_normalizedCategoryVolume);
	void updateReverbVolume();  // applies the current SoundInstance reverb volume to the playing audio
	
	inline bool isDone() const { return m_isDone; }
	
private:
	PlayWaveEventInstance(const PlayWaveEventInstance&);
	PlayWaveEventInstance& operator=(const PlayWaveEventInstance&);
	
	real getRandomVolume() const;  //!< \return Random volume in dB.
	real getRandomPitch()  const;
	int  getRandomPan()    const;
	//real getTimeStamp()    const;
	inline bool shouldLoopInfinitely() const
	{ return m_playEvent->getLoopInfinite(); }
	
	PlayWaveEvent* m_playEvent;
	TrackInstance* m_track;
	
	real           m_delay;
	int            m_loopCount;
	bool           m_breakLoop;
	
	WaveInstance*  m_activeWave;
	real           m_volumeInDB;
	real           m_volumeVariation;
	real           m_pitch;
	int            m_pan;
	bool           m_isDone;
	bool           m_paused;
	bool           m_isPositional;
	math::Vector3  m_position;
	real           m_emitterRadiusInner; // negative value if not set/overridden
	real           m_emitterRadiusOuter; // negative value if not set/overridden
};

} // namespace end
}
}

#endif // INC_TT_AUDIO_XACT_PLAYWAVEEVENTINSTANCE_H
