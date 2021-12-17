#if !defined(INC_TOKI_GAME_SCRIPT_ENTITYSTATE_H)
#define INC_TOKI_GAME_SCRIPT_ENTITYSTATE_H

#include <string>

#include <squirrel/squirrel.h>

#include <tt/code/BitMask.h>

#include <toki/game/script/fwd.h>


namespace toki {
namespace game {
namespace script {


class EntityState
{
public:
	EntityState();
	EntityState(const std::string& p_name, const HSQOBJECT& p_sqState);
	
	inline const HSQOBJECT&   getSqState() const { return m_sqState; }
	inline const std::string& getName()    const { return m_name;    }
	
	inline bool isValid() const { return sq_isnull(m_sqState) == false; }
	inline void reset() { m_name.clear(); sq_resetobject(&m_sqState); }
	
private:
	std::string m_name;
	HSQOBJECT   m_sqState;
};


// Namespace end
}
}
}


#endif  // !defined(INC_TOKI_GAME_SCRIPT_ENTITYSTATE_H)
