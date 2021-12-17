#ifndef INC_TT_AUDIO_XACT_TRACK_H
#define INC_TT_AUDIO_XACT_TRACK_H


#include <vector>

#include <tt/audio/xact/fwd.h>
#include <tt/fs/types.h>
#include <tt/platform/tt_types.h>
#include <tt/xml/fwd.h>


namespace tt {
namespace audio {
namespace xact {

class Track
{
public:
	Track();
	~Track();
	
	void setVolume(real p_volumeInDB);
	
	/*! \return Volume in dB. */
	inline real getVolume() const { return m_volumeInDB; }
	
	void setPlayEvent(PlayWaveEvent* p_event);
	void setStopEvent(StopEvent* p_event);
	void addVolumeEvent(VolumeEvent* p_event);
	void addPitchEvent(PitchEvent* p_event);
	
	static Track* createTrack(const xml::XmlNode* p_node);
	
	TrackInstance* instantiate(SoundInstance* p_sound);
	
private:
	typedef std::vector<VolumeEvent*> VolumeEvents;
	typedef std::vector<PitchEvent*>  PitchEvents;
	
	
	Track(const Track&);
	Track& operator=(const Track&);
	
	bool load(const fs::FilePtr& p_file);
	bool save(const fs::FilePtr& p_file) const;
	
	
	real m_volumeInDB; // -96.0 - 6.0
	
	PlayWaveEvent* m_playEvent;
	StopEvent*     m_stopEvent;
	VolumeEvents   m_volEvents;
	PitchEvents    m_pitchEvents;
	
	friend class Sound;
};

// Namespace end
}
}
}


#endif // INC_TT_AUDIO_XACT_TRACK_H
