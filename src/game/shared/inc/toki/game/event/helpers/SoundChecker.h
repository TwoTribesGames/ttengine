#if !defined(INC_TOKI_GAME_EVENT_HELPERS_SOUNDCHECKER_H)
#define INC_TOKI_GAME_EVENT_HELPERS_SOUNDCHECKER_H


#include <algorithm>
#include <map>
#include <set>

#include <tt/math/Point2.h>

#include <toki/game/event/helpers/fwd.h>
#include <toki/level/fwd.h>


namespace toki {
namespace game {
namespace event {
namespace helpers {


class SoundChecker
{
public:
	struct LocationInfo
	{
		LocationInfo(const tt::math::Point2& p_location, const tt::math::Point2& p_source, real p_distance)
		:
		location(p_location),
		direction(p_location - p_source),
		distance(p_distance),
		isInsideCollision(false)
		{
		}
		
		tt::math::Point2 location;
		tt::math::Point2 direction;
		real distance;
		bool isInsideCollision;
	};
	
	typedef std::vector<LocationInfo> Locations;
	
	SoundChecker();
	
	const Locations& fill(const tt::math::Vector2& p_startPos, real p_range);
	
private:
	static const real sqrtTwo;
	
	void visitLocation(const tt::math::Point2& p_location, const tt::math::Point2& p_source,
	                   real p_distance, real p_range, bool p_isEmpty);
	
	void fillLocation(const LocationInfo& p_locationInfo, real p_range,
	                  const level::TileRegistrationMgr& p_tileRegistrationMgr);
	
	Locations m_todo;
	Locations m_visited;
};

// Namespace end
}
}
}
}


#endif  // !defined(INC_TOKI_GAME_EVENT_HELPERS_SOUNDCHECKER_H)
