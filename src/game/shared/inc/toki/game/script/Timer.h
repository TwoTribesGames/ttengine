#if !defined(INC_TOKITORI_GAME_SCRIPT_TIMER_H)
#define INC_TOKITORI_GAME_SCRIPT_TIMER_H


#include <toki/game/entity/fwd.h>
#include <toki/game/script/fwd.h>

namespace toki {
namespace game {
namespace script {


class Timer
{
public:
	Timer(const std::string& p_name, real p_timeout, const entity::EntityHandle& p_target);
	~Timer() {}
	
	bool isAlive() const;
	inline void setTimeout(real p_timeout) { m_timeout = p_timeout; }
	inline real getTimeout() const { return m_timeout; }
	inline const entity::EntityHandle& getTarget() const { return m_target; }
	inline const std::string& getName() const { return m_name; }
	inline const TimerHash getHash() const { return m_hash; }
	inline void setSuspended(bool p_suspended) { m_suspended = p_suspended; }
	inline bool isSuspended() const { return m_suspended; }
	inline void setIsSeparateCallback(bool p_isSeperateCallback) { m_isSeparateCallback = p_isSeperateCallback; }
	inline bool isSeparateCallback() const { return m_isSeparateCallback; }
	
	bool updateTime(real p_elapsedTime);
	void doCallback();
	
private:
	real                 m_timeout;
	bool                 m_suspended;
	bool                 m_isSeparateCallback;
	std::string          m_name;
	TimerHash            m_hash;
	entity::EntityHandle m_target;
};


// Namespace end
}
}
}

#endif // !defined(INC_TOKITORI_GAME_SCRIPT_TIMER_H)
