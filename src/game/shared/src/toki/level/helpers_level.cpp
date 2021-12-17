#include <algorithm>

#include <tt/math/math.h>
//#include <tt/code/helpers.h>

//#include <toki/level/AttributeLayer.h>
#include <toki/level/helpers.h>


namespace toki {
namespace level {


tt::math::PointRect worldToTile(const tt::math::VectorRect& p_world)
{
	const tt::math::Point2 minPos = worldToTile(p_world.getMin());
	const tt::math::Point2 maxPos = worldToTile(tt::math::ceil( p_world.getMaxEdge() - tt::math::Vector2(1.0f, 1.0f)));
	return tt::math::PointRect(minPos, tt::math::Point2(std::max(minPos.x, maxPos.x), std::max(minPos.y, maxPos.y)));
}


// Namespace end
}
}
