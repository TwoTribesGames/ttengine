#if !defined(INC_TOKI_GAME_MOVEMENT_MOVEMENTSETCACHE_H)
#define INC_TOKI_GAME_MOVEMENT_MOVEMENTSETCACHE_H

#include <map>
#include <string>

#include <toki/game/movement/fwd.h>


namespace toki {
namespace game {
namespace movement {

class MovementSetCache
{
public:
	static MovementSetPtr get (const std::string& p_filename);
	static MovementSetPtr find(const std::string& p_filename);
	
private:
	typedef std::map<std::string, MovementSetWeakPtr> Cache;

	//not implemented
	MovementSetCache();
	~MovementSetCache();
	
	static MovementSetPtr load(const std::string& p_filename);
	static void remove(MovementSet* p_movementSet);
	
	static Cache ms_cache;
};


// Namespace end
}
}
}

#endif // !defined(INC_TOKI_GAME_MOVEMENT_MOVEMENTSETCACHE_H)
