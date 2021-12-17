#include <toki/game/event/helpers/SoundChecker.h>
#include <toki/game/Game.h>
#include <toki/level/helpers.h>
#include <toki/level/TileRegistrationMgr.h>
#include <toki/AppGlobal.h>


#include <tt/system/Time.h>

namespace toki {
namespace game {
namespace event {
namespace helpers {

const real SoundChecker::sqrtTwo = tt::math::sqrt(2.0f);

//--------------------------------------------------------------------------------------------------
// Public member functions

SoundChecker::SoundChecker()
:
m_todo(),
m_visited()
{
	m_todo.reserve   (256);
	m_visited.reserve(256);
}


const SoundChecker::Locations& SoundChecker::fill(const tt::math::Vector2& p_startPos, real p_range)
{
	TT_ASSERTMSG(p_range > 0.0f, "Invalid range '%f' for SoundChecker, should be > 0.0f", p_range);
	
	using namespace tt::math;
	
	m_visited.clear();
	m_todo.clear();
	
	const Point2 location(level::worldToTile(p_startPos));
	Game* game = AppGlobal::getGame();
	const level::TileRegistrationMgr& tileMgr = game->getTileRegistrationMgr();
	
	if (tileMgr.isSoundBlocking(location) == false)
	{
		visitLocation(location, location, 0.0f, p_range, true);
	}
	
	// If p_location is exactly at a tileborder, account for adjacent tiles as well or the radius will be wrong.
	// This is because the soundchecker code works with tilepositions and not worldpositions. So the distance of the tiles
	// left, under and left under the tile at p_location should also be 0 or the radius will be off (too far to the right)
	const Point2 bottom     (location + Point2( 0, -1));
	const Point2 bottomLeft (location + Point2(-1, -1));
	const Point2 left       (location + Point2(-1,  0));
	const bool emptyBottom      = tileMgr.isSoundBlocking(bottom)      == false;
	const bool emptyBottomLeft  = tileMgr.isSoundBlocking(bottomLeft)  == false;
	const bool emptyLeft        = tileMgr.isSoundBlocking(left)        == false;
	if (p_startPos.x == static_cast<real>(location.x) && emptyLeft)
	{
		visitLocation(left, location, 0.0f, p_range, true);
		if (p_startPos.y == static_cast<real>(location.y) && emptyBottomLeft)
		{
			visitLocation(bottomLeft, location, 0.0f, p_range, true);
		}
	}
	if (p_startPos.y == static_cast<real>(location.y) && emptyBottom)
	{
		visitLocation(bottom, location, 0.0f, p_range, true);
	}
	
	while (m_todo.empty() == false)
	{
		Locations todo(m_todo);
		m_todo.clear();
		
		for (Locations::iterator it = todo.begin(); it != todo.end(); ++it)
		{
			fillLocation(*it, p_range, tileMgr);
		}
	}
	
	// If no visited positions have been found (e.g., sound was spawned in collision)
	// make sure that at least the spawning location is added to
	// the list of visited positions.
	if (tileMgr.isSoundBlocking(location) && m_visited.empty())
	{
		m_visited.push_back(LocationInfo(location, location, 0.0f));
	}
	
	return m_visited;
}


//--------------------------------------------------------------------------------------------------
// Private member functions

void SoundChecker::visitLocation(const tt::math::Point2& p_location, const tt::math::Point2& p_source,
                                 real p_distance, real p_range, bool p_isEmpty)
{
	// Never visit location if distance is too great
	if (p_distance > p_range)
	{
		return;
	}
	
	LocationInfo info(p_location, p_source, p_distance);
	
	// Check if already visited
	for (Locations::iterator it = m_visited.begin(); it != m_visited.end(); ++it)
	{
		if ((*it).location == p_location)
		{
			(*it).direction += info.direction;
			return;
		}
	}
	
	if (p_isEmpty)
	{
		m_todo.push_back(info);
	}
	else
	{
		info.isInsideCollision = true;
	}
	
	m_visited.push_back(info);
}


void SoundChecker::fillLocation(const LocationInfo& p_locationInfo, real p_range,
                                const level::TileRegistrationMgr& p_tileRegistrationMgr)
{
	using namespace tt::math;
	
	const Point2 top        (p_locationInfo.location + Point2( 0,  1));
	const Point2 topRight   (p_locationInfo.location + Point2( 1,  1));
	const Point2 right      (p_locationInfo.location + Point2( 1,  0));
	const Point2 bottomRight(p_locationInfo.location + Point2( 1, -1));
	const Point2 bottom     (p_locationInfo.location + Point2( 0, -1));
	const Point2 bottomLeft (p_locationInfo.location + Point2(-1, -1));
	const Point2 left       (p_locationInfo.location + Point2(-1,  0));
	const Point2 topLeft    (p_locationInfo.location + Point2(-1,  1));
	
	const bool emptyTop         = p_tileRegistrationMgr.isSoundBlocking(top)         == false;
	const bool emptyTopRight    = p_tileRegistrationMgr.isSoundBlocking(topRight)    == false;
	const bool emptyRight       = p_tileRegistrationMgr.isSoundBlocking(right)       == false;
	const bool emptyBottomRight = p_tileRegistrationMgr.isSoundBlocking(bottomRight) == false;
	const bool emptyBottom      = p_tileRegistrationMgr.isSoundBlocking(bottom)      == false;
	const bool emptyBottomLeft  = p_tileRegistrationMgr.isSoundBlocking(bottomLeft)  == false;
	const bool emptyLeft        = p_tileRegistrationMgr.isSoundBlocking(left)        == false;
	const bool emptyTopLeft     = p_tileRegistrationMgr.isSoundBlocking(topLeft)     == false;
	
	visitLocation(top,    p_locationInfo.location, p_locationInfo.distance + 1.0f, p_range, emptyTop);
	visitLocation(right,  p_locationInfo.location, p_locationInfo.distance + 1.0f, p_range, emptyRight);
	visitLocation(bottom, p_locationInfo.location, p_locationInfo.distance + 1.0f, p_range, emptyBottom);
	visitLocation(left,   p_locationInfo.location, p_locationInfo.distance + 1.0f, p_range, emptyLeft);
	
	if (emptyTop || emptyRight)
	{
		visitLocation(topRight, p_locationInfo.location, p_locationInfo.distance + sqrtTwo, p_range, emptyTopRight);
	}
	
	if (emptyTop || emptyLeft)
	{
		visitLocation(topLeft, p_locationInfo.location, p_locationInfo.distance + sqrtTwo, p_range, emptyTopLeft);
	}
	
	if (emptyBottom || emptyRight)
	{
		visitLocation(bottomRight, p_locationInfo.location, p_locationInfo.distance + sqrtTwo, p_range, emptyBottomRight);
	}
	
	if (emptyBottom || emptyLeft)
	{
		visitLocation(bottomLeft, p_locationInfo.location, p_locationInfo.distance + sqrtTwo, p_range, emptyBottomLeft);
	}
}

// Namespace end
}
}
}
}
