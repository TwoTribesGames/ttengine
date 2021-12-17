#if !defined(INC_TT_ENGINE_ANIMATION_ANIMATIONCONTROL_H)
#define INC_TT_ENGINE_ANIMATION_ANIMATIONCONTROL_H


#include <tt/code/ErrorStatus.h>
#include <tt/engine/animation/enums.h>
#include <tt/fs/types.h>
#include <tt/platform/tt_types.h>


namespace tt {
namespace engine {
namespace animation {


class AnimationControl
{
public:
	AnimationControl();
	~AnimationControl() {}

	inline void setStartTime(real p_time) {m_startTime = p_time;}
	inline void setEndTime  (real p_time) {m_endTime   = p_time;}

	inline real getStartTime() const {return m_startTime;}
	inline real getEndTime()   const {return m_endTime;}

	inline void setLoop(bool p_loop) {m_loop = p_loop;}

	inline void setSpeed(real p_fps) {m_fps = p_fps;}
	inline real getSpeed() const     {return m_fps;}

	inline void setTime(real p_time) {m_time = p_time;}
	inline real getTime() const      {return m_time;}

	inline void setPaused(bool p_paused) {m_paused = p_paused;}
	inline bool isPaused() const {return m_paused;}

	inline void setMode(Mode p_mode) {m_mode = p_mode;}
	inline Mode getMode() const {return m_mode;}

	inline real getLeftOverTime() const
	{ return (m_time > m_endTime) ? (m_time - m_endTime) : real(0.0f); }

	inline bool isFinished() const { return m_loop ? false : (m_time > m_endTime); }

	void load(const fs::FilePtr& p_file, code::ErrorStatus* p_errStatus);
	
	void update(real p_elapsedTime);

private:
	real m_startTime;
	real m_endTime;
	bool m_loop;
	Mode m_mode;

	real m_time;
	real m_fps;
	bool m_paused;
};


// Namespace end
}
}
}


#endif //#if !defined(INC_TT_ENGINE_ANIMATION_ANIMATION_H)
