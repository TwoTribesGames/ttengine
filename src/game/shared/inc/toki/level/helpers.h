#if !defined(INC_TOKI_LEVEL_HELPERS_H)
#define INC_TOKI_LEVEL_HELPERS_H


#include <tt/math/Point2.h>
#include <tt/math/Rect.h>
#include <tt/math/Vector2.h>


namespace toki {
namespace level {

// Helpers to translate from tile space to world space

inline real tileToWorld(s32 p_tile)
{
	return static_cast<real>(p_tile);
}

inline tt::math::Vector2 tileToWorld(const tt::math::Point2& p_tile)
{
	return tt::math::Vector2(tileToWorld(p_tile.x), tileToWorld(p_tile.y));
}

inline tt::math::VectorRect tileToWorld(const tt::math::PointRect& p_tile)
{
	return tt::math::VectorRect(
		tileToWorld(p_tile.getPosition()),
		tileToWorld(p_tile.getWidth()),
		tileToWorld(p_tile.getHeight()));
}


// Hacky helper to make a world rect that can be used to detect which entities are selected
inline tt::math::VectorRect makeEntitySelectionWorldRect(const tt::math::PointRect& p_tile)
{
	tt::math::VectorRect worldRect(tileToWorld(p_tile));
	// HACK: Tweak the height slightly, so that entities just above the selection rect aren't selected
	//       (entity positions are for the entity bottom-center)
	worldRect.setHeight(worldRect.getHeight() - 0.01f);
	return worldRect;
}


// Helpers to translate from world space to tile space

inline s32 worldToTile(real p_world)
{
	return static_cast<s32>(tt::math::floor(p_world));
}

inline tt::math::Point2 worldToTile(const tt::math::Vector2& p_world)
{
	return tt::math::Point2(worldToTile(p_world.x), worldToTile(p_world.y));
}

tt::math::PointRect worldToTile(const tt::math::VectorRect& p_world);


inline tt::math::Vector2 snapToTilePos(const tt::math::Vector2& p_pos,
                                       const tt::math::Vector2& p_offset)
{
	// Move pos with offset (in most cases a rect edge) and half a tile.
	const tt::math::Vector2 pos(p_pos + p_offset + tt::math::Vector2(0.5f, 0.5f));
	
	// Snap to tiles and remove offset.
	return tt::math::Vector2(level::worldToTile(pos)) - p_offset;
}

// Collision helpers

//


// Path helpers

inline std::string getUserLevelShoeboxPath()
{
	return "levels/userlevel_shoeboxes/";
}

// Namespace end
}
}


#endif  // !defined(INC_TOKI_LEVEL_HELPERS_H)
