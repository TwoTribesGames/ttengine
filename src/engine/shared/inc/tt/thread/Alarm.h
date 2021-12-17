#if !defined(INC_TT_THREAD_ALARM_H)
#define INC_TT_THREAD_ALARM_H

#include <vector>

#include <tt/platform/tt_types.h>


namespace tt {
namespace thread {

class Alarm
{
public:
	typedef void (*callback)(void*);
	
	~Alarm();
	
	/*! \brief Creates an alarm.
	    \param p_start Number of microseconds until the alarm is triggered.
	    \param p_callback Function to call when the alarm is triggered.
	    \param p_userData User supplied data for the callback function.
	    \param p_hardware Whether or not the alarm should be implemented in hardware.
	    \return new Alarm object, 0 on failure.*/
	static Alarm* createAlarm(u64 p_start, callback p_callback, void* p_userData = 0, bool p_hardware = false);
	
	/*! \brief Creates a periodic alarm.
	    \param p_start Number of microseconds until the alarm is triggered.
	    \param p_period Interval at which the alarm will be triggered in microseconds.
	    \param p_callback Function to call when the alarm is triggered.
	    \param p_userData User supplied data for the callback function.
	    \param p_hardware Whether or not the alarm should be implemented in hardware.
	    \return new Alarm object, 0 on failure.*/
	static Alarm* createPeriodicAlarm(u64      p_start,
	                                  u64      p_period,
	                                  callback p_callback,
	                                  void*    p_userData = 0,
	                                  bool     p_hardware = false);
	
	/*! \brief Updates software alarms.
	    \param p_delta Time passed since last update in microseconds.*/
	static void update(u64 p_delta);
	
private:
	Alarm(u64 p_start, u64 p_period, callback p_callback, void* p_userData, bool p_hardware);
	Alarm(const Alarm&);                  // no copying allowed
	const Alarm& operator=(const Alarm&); // no assignment allowed
	
	void trigger();
	
	typedef std::vector<Alarm*> Alarms;
	
	static Alarms ms_alarms; //!< Software alarms.
	static u64    ms_time;   //!< Current time.
	
	u64      m_period;   //!< Interval between triggers of the alarm.
	u64      m_time;     //!< Time of the next trigger.
	callback m_callback; //!< Callback function to call at each fire.
	void*    m_userData; //!< User data for the callback.
	bool     m_hardware; //!< Whether the alarm is implemented in hardware.
	void*    m_internal; //!< Internal data for hardware alarm.
};

// namespace end
}
}


#endif // !defined(INC_TT_THREAD_ALARM_H)
