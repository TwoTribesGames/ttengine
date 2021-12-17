#ifndef INC_TT_AUDIO_XACT_WAVEINSTANCE_H
#define INC_TT_AUDIO_XACT_WAVEINSTANCE_H


#include <string>

#include <tt/audio/xact/fwd.h>
#include <tt/math/Vector3.h>
#include <tt/platform/tt_types.h>
#include <tt/snd/types.h>


namespace tt {
namespace audio {
namespace xact {

class WaveInstance
{
public:
	explicit WaveInstance(Wave* p_wave);
	~WaveInstance();
	
	void setVolume(real p_volumeInDB, real p_normalizedCategoryVolume);
	inline real getVolume() const { return m_volumeInDB; }
	
	void setReverbVolume(real p_volumeInDB);
	inline real getReverbVolume() const { return m_reverbVolumeInDB; }
	
	void setPitch(real p_pitch);
	inline real getPitch() const { return m_pitch; }
	
	void setPan(int p_pan);
	inline int getPan() const { return m_pan; }
	
	bool setPosition     (const math::Vector3& p_position);
	bool setEmitterRadius(real p_inner, real p_outer);
	
	bool play(bool p_loop, snd::size_type p_priority);
	bool stop();
	bool pause();
	bool resume();
	
	void update(real p_time);
	inline void updateVolume(real p_normalizedCategoryVolume) { setVolume(m_volumeInDB, p_normalizedCategoryVolume); }
	
	bool isPlaying();
	
	const std::string& getWaveFilename() const;
	inline const Wave* getWave() const { return m_wave; }
	
private:
	WaveInstance(const WaveInstance&);
	WaveInstance& operator=(const WaveInstance&);
	
	
	Wave* m_wave;
	
	real m_volumeInDB;
	real m_pitch;
	int  m_pan;
	math::Vector3 m_position;
	real          m_emitterRadiusInner; // negative value if not set/overridden
	real          m_emitterRadiusOuter; // negative value if not set/overridden
	
	real m_reverbVolumeInDB;
	
	real m_hwVolume;  // Normalized volume
	
	snd::VoicePtr m_voice;
	
	bool m_paused;
	bool m_isPositional;
	real m_silenceTime;
	real m_currentTime;
};

// Namespace end
}
}
}

#endif // INC_TT_AUDIO_XACT_WAVEINSTANCE_H
