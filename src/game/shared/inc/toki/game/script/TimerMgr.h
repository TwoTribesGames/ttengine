#if !defined(INC_TOKI_GAME_SCRIPT_TIMERMGR_H)
#define INC_TOKI_GAME_SCRIPT_TIMERMGR_H


#include <set>

#include <toki/game/script/fwd.h>
#include <toki/serialization/fwd.h>


namespace toki {
namespace game {
namespace script {

class TimerMgr
{
public:
	static bool isInitialized();
	
	static void startTimer(const std::string& p_name, real p_timeout, const entity::EntityHandle& p_target);
	static void startCallbackTimer(const std::string& p_callback, real p_timeout,
	                               const entity::EntityHandle& p_target);
	
	static void stopTimer(const std::string& p_name, const entity::EntityHandle& p_target);
	static void stopAllTimers(const entity::EntityHandle& p_target);
	static void suspendTimer(const std::string& p_name, const entity::EntityHandle& p_target);
	static void suspendAllTimers(const entity::EntityHandle& p_target);
	static void resumeTimer(const std::string& p_name, const entity::EntityHandle& p_target);
	static void resumeAllTimers(const entity::EntityHandle& p_target);
	static const TimerPtr& getTimer(const std::string& p_name, const entity::EntityHandle& p_target);
	
	// FIXME: (Un)serialization should probably indicate whether this was successful
	static void serialize  (      toki::serialization::SerializationMgr& p_serializationMgr);
	static void unserialize(const toki::serialization::SerializationMgr& p_serializationMgr);

	static void logTimers();
	
private:
	typedef std::map<TimerHash, TimerPtr> EntityTimers;
	typedef std::map<entity::EntityHandle, EntityTimers> Timers;
	
	TimerMgr();  // Static class. Not implemented
	~TimerMgr(); // Static class. Not implemented
	
	static void init();
	static inline void reset() { ms_timers.clear(); }
	static void deinit();
	static void update(real p_elapsedTime);
	
	static bool   ms_initialized;
	static Timers ms_timers;
	
	friend class EntityScriptMgr; // EntityScriptMgr does the update, init and deinit.
};

// Namespace end
}
}
}


#endif  // !defined(INC_TOKI_GAME_SCRIPT_TIMERMGR_H)
