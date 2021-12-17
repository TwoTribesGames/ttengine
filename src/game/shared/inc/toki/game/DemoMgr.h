#if !defined(INC_TOKI_GAME_DEMOMGR_H)
#define INC_TOKI_GAME_DEMOMGR_H

#include <string>

#include <tt/code/fwd.h>
#include <tt/fs/types.h>

#include <toki/serialization/fwd.h>


namespace toki {
namespace game {

class DemoMgr
{
public:
	DemoMgr();
	
	void init();
	void update(real p_deltaTime);
	void serialize  (serialization::SerializationMgr& p_serializationMgr) const;
	bool unserialize(const serialization::SerializationMgr& p_serializationMgr);
	void resetCountdown();
	inline void setCountdownEnabled(bool p_enabled)
	{
		TT_Printf("Set countdown enabled: %s\n", p_enabled ? "true" : "false");
		m_countdownEnabled = p_enabled;
	}
	
private:
	u32  hash(real p_value) const;
	
	real m_countdown;  // in seconds
	s32  m_countdownMinutesLastCallback;
	bool m_countdownEnabled;
};

// Namespace end
}
}


#endif  // !defined(INC_TOKI_GAME_DEMOMGR_H)
