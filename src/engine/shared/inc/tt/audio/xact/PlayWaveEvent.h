#ifndef INC_TT_AUDIO_XACT_PLAYWAVEEVENT_H
#define INC_TT_AUDIO_XACT_PLAYWAVEEVENT_H


#include <utility>
#include <vector>

#include <tt/audio/xact/fwd.h>
#include <tt/fs/types.h>
#include <tt/platform/tt_types.h>
#include <tt/xml/fwd.h>


namespace tt {
namespace audio {
namespace xact {

class PlayWaveEvent
{
public:
	enum VariationType
	{
		Variation_Ordered,           // play sequentially
		Variation_OrderedFromRandom, // play sequentially from random start
		Variation_Random,            // play randomly
		Variation_RandomNoRepeat,    // play randomly without repeating the previous
		Variation_Shuffle            // play randomly without repeating any, reset when all have been played
	};
	
	enum Operator
	{
		Operator_Replace,
		Operator_Add
	};
	
	
	PlayWaveEvent();
	
	void setTimeStamp(real p_time);
	inline real getTimeStamp() const { return m_timeStamp; }
	
	void setRandomOffset(real p_offset);
	inline real getRandomOffset() const { return m_randomOffset; }
	
	void addWave(Wave* p_wave, int p_weight);
	
	inline void          setVariation(VariationType p_type) { m_playOrder = p_type; }
	inline VariationType getVariation() const               { return m_playOrder;   }
	
	void setLoopCount(int p_loopCount);
	inline int getLoopCount() const { return m_loopCount; }
	
	inline void setLoopInfinite(bool p_infinite) { m_infinite = p_infinite; }
	inline bool getLoopInfinite() const          { return m_infinite;       }
	
	inline void setBreakLoop(bool p_breakLoop) { m_breakLoop = p_breakLoop; }
	inline bool getBreakLoop() const           { return m_breakLoop;        }
	
	inline void setVolumeVariationEnabled(bool p_enabled) { m_volVariationEnabled = p_enabled; }
	inline bool getVolumeVariationEnabled() const         { return m_volVariationEnabled;      }
	
	inline void     setVolumeOperator(Operator p_operator) { m_volOperator = p_operator; }
	inline Operator getVolumeOperator() const              { return m_volOperator;       }
	
	inline void setVolumeNewOnLoop(bool p_new) { m_volNewOnLoop = p_new; }
	inline bool getVolumeNewOnLoop() const     { return m_volNewOnLoop;  }
	
	void setVolumeRangeMin(real p_minInDB);
	/*! \return Minimum volume in dB. */
	inline real getVolumeRangeMin() const { return m_volRangeMinInDB; }
	
	void setVolumeRangeMax(real p_maxInDB);
	/*! \return Maximum volume in dB. */
	inline real getVolumeRangeMax() const { return m_volRangeMaxInDB; }
	
	inline void setPitchVariationEnabled(bool p_enabled) { m_pitchVariationEnabled = p_enabled; }
	inline bool getPitchVariationEnabled() const         { return m_pitchVariationEnabled;      }
	
	inline void     setPitchOperator(Operator p_operator) { m_pitchOperator = p_operator; }
	inline Operator getPitchOperator() const              { return m_pitchOperator;       }
	
	inline void setPitchNewOnLoop(bool p_new) { m_pitchNewOnLoop = p_new; }
	inline bool getPitchNewOnLoop() const { return m_pitchNewOnLoop; }
	
	void setPitchRangeMin(real p_min);
	inline real getPitchRangeMin() const { return m_pitchRangeMin; }
	
	void setPitchRangeMax(real p_max);
	inline real getPitchRangeMax() const { return m_pitchRangeMax; }
	
	inline void setPanVariationEnabled(bool p_enabled) { m_panVariationEnabled = p_enabled; }
	inline bool getPanVariationEnabled() const         { return m_panVariationEnabled;      }
	
	inline void setPanNewOnLoop(bool p_new) { m_panNewOnLoop = p_new; }
	inline bool getPanNewOnLoop() const     { return m_panNewOnLoop;  }
	
	void setPanAngle(int p_angle);
	inline int getPanAngle() const { return m_panAngle; }
	
	void setPanArc(int p_arc);
	inline int getPanArc() const { return m_panArc; }
	
	inline void setUseCenterSpeaker(bool p_use) { m_useCenterSpeaker = p_use; }
	inline bool getUseCenterSpeaker() const     { return m_useCenterSpeaker;  }
	
	static PlayWaveEvent* createPlayWaveEvent(const xml::XmlNode* p_node);
	
	PlayWaveEventInstance* instantiate(TrackInstance* p_track);
	
	real getStartDelay() const;
	Wave* getNextWave();
	s32 getPlayListSize() const { return static_cast<s32>(m_playList.size()); }
	
private:
	typedef std::pair<Wave*, int>      PlayListEntry;
	typedef std::vector<PlayListEntry> PlayList;
	typedef std::vector<Wave*>         OrderedList;
	
	PlayWaveEvent(const PlayWaveEvent&);
	PlayWaveEvent& operator=(const PlayWaveEvent&);
	
	void orderPlayList();
	void shufflePlayList();
	
	bool load(const fs::FilePtr& p_file);
	bool save(const fs::FilePtr& p_file) const;
	void initPlayList();
	
	real m_timeStamp;    // The earliest time at which this event can occur, in seconds.
	real m_randomOffset; // Randomly delay playback of the event by up to this much time, in seconds.
	
	// looping variables
	int  m_loopCount; // 0 - 254
	bool m_infinite;
	bool m_breakLoop;
	bool m_newWaveOnLoop;
	
	// volume variation variables
	bool     m_volVariationEnabled;
	Operator m_volOperator;
	bool     m_volNewOnLoop; // calculate new volume on every loop
	real     m_volRangeMinInDB;  // -96.0 - 6.0
	real     m_volRangeMaxInDB;  // -96.0 - 6.0
	bool     m_volReplace;   // whether to replace the volume or add it to the existing
	
	// pitch variation variables
	bool     m_pitchVariationEnabled;
	Operator m_pitchOperator;
	bool     m_pitchNewOnLoop;
	real     m_pitchRangeMin; // -12.0 - 12.0
	real     m_pitchRangeMax; // -12.0 - 12.0
	bool     m_pitchReplace;
	
	// panning variation variables
	bool m_panVariationEnabled;
	bool m_panNewOnLoop;
	int  m_panAngle;         // 0 - 359
	int  m_panArc;           // 0 - 360
	bool m_useCenterSpeaker; // unused
	
	// playlist variables
	VariationType  m_playOrder;
	PlayList       m_playList;
	OrderedList    m_playOrderedList;
	int            m_totalWeight;
	Wave*          m_prevWave;
	
	friend class Track;
};

// Namespace end
}
}
}


#endif // INC_TT_AUDIO_XACT_PLAYWAVEEVENT_H
