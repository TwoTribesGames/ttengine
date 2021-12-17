#ifndef INC_TT_AUDIO_XACT_PITCHEVENT_H
#define INC_TT_AUDIO_XACT_PITCHEVENT_H


#include <tt/audio/xact/fwd.h>
#include <tt/fs/types.h>
#include <tt/platform/tt_types.h>
#include <tt/xml/fwd.h>


namespace tt {
namespace audio {
namespace xact {

class PitchEvent
{
public:
	// pitch properties
	enum SettingType
	{
		Setting_Equation,
		Setting_Ramp      // not supported
	};
	
	enum EquationType
	{
		Equation_Value,
		Equation_Random
	};
	
	enum OperationType
	{
		Operation_Replace,
		Operation_Add
	};
	
	PitchEvent();
	~PitchEvent();
	
	void setTimeStamp(real p_time);
	inline real getTimeStamp() const { return m_timeStamp; }
	
	void setRandomOffset(real p_offset);
	inline real getRandomOffset() const { return m_randomOffset; }
	
	inline void        setSettingType(SettingType p_type) { m_settingType = p_type; }
	inline SettingType getSettingType() const             { return m_settingType;   }
	
	inline void         setEquationType(EquationType p_type) { m_equationType = p_type; }
	inline EquationType getEquationType() const              { return m_equationType;   }
	
	inline void          setOperationType(OperationType p_type) { m_operationType = p_type; }
	inline OperationType getOperationType() const               { return m_operationType;   }
	
	void setValue(real p_value);
	inline real getValue() const { return m_value; }
	
	void setRangeMin(real p_min);
	inline real getRangeMin() const { return m_rangeMin; }
	
	void setRangeMax(real p_max);
	inline real getRangeMax() const { return m_rangeMax; }
	
	void setInitialValue(real p_init);
	inline real getInitialValue() const { return m_init; }
	
	inline void setSlope(real p_slope) { m_slope = p_slope; }
	inline real getSlope() const { return m_slope; }
	
	inline void  setSlopeDelta(real p_delta) { m_slopeDelta = p_delta; }
	inline real getSlopeDelta() const { return m_slopeDelta; }
	
	void setDuration(real p_duration);
	inline real getDuration() const { return m_duration; }
	
	void setRepeats(int p_repeats);
	inline int getRepeats() const { return m_repeats; }
	
	inline void setInfinite(bool p_infinite) { m_infinite = p_infinite; }
	inline bool getInfinite() const { return m_infinite; }
	
	void  setFrequency(real p_frequency);
	inline real getFrequency() const { return m_frequency; }
	
	static PitchEvent* createPitchEvent(const xml::XmlNode* p_node);
	
	PitchEventInstance* instantiate(TrackInstance* p_track);
	
private:
	PitchEvent(const PitchEvent&);
	PitchEvent& operator=(const PitchEvent&);
	
	bool load(const fs::FilePtr& p_file);
	bool save(const fs::FilePtr& p_file) const;
	
	
	real m_timeStamp;    // The earliest time at which this event can occur, in seconds.
	real m_randomOffset; // Randomly delay playback of the event by up to this much time, in seconds.
	
	SettingType   m_settingType;
	EquationType  m_equationType;
	OperationType m_operationType;
	
	// equation variables
	real m_value;    // -12.0 - 12.0 (only when equationtype is value)
	real m_rangeMin; // -12.0 - 12.0 (only when equationtype is random)
	real m_rangeMax; // -12.0 - 12.0 (only when equationtype is random)
	
	// ramp variables
	real m_init;       // -12.0 - 12.0  The starting pitch of this pitch ramp.
	real m_slope;      // The initial slope of this ramp.
	real m_slopeDelta; // The amount with which the slope is incremented each millisecond.
	real m_duration;   // The duration of the ramp in seconds.
	
	// repeat variables (only when settingtype is equation)
	int  m_repeats;   // 0 - 254
	bool m_infinite;
	real m_frequency; // The amount of time between repeats of this event in seconds.
	
	friend class Track;
};

} // namespace end
}
}

#endif // INC_TT_AUDIO_XACT_PITCHEVENT_H
