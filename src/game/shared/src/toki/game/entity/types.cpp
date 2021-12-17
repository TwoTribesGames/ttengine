
#include <toki/game/entity/types.h>
#include <toki/game/entity/Entity.h>

namespace toki {
namespace game {
namespace entity {


movement::Direction getDirectionFromLocalDir(LocalDir            p_localDir,
                                             movement::Direction p_orientationDown,
                                             bool                p_orientationForwardIsLeft)
{
	movement::Direction result = movement::Direction_Right;
	
	switch (p_localDir)
	{
	case LocalDir_Forward: result = (p_orientationForwardIsLeft) ? movement::Direction_Left  : movement::Direction_Right; break;
	case LocalDir_Up:      result = movement::Direction_Up;                break;
	case LocalDir_Back:    result = (p_orientationForwardIsLeft) ? movement::Direction_Right : movement::Direction_Left;  break;
	case LocalDir_Down:    result = movement::Direction_Down;              break;
	case LocalDir_None: return movement::Direction_None; // Nothing else to be done, return.
	default:
		TT_PANIC("Unknown LocalDir: %d!", p_localDir);
		return movement::Direction_Invalid;
	}
	
	// The following algorithm depends on this specific order in the enum.
	// (Each step is a counter-clockwise rotation and +1 in value.)
	TT_STATIC_ASSERT(movement::Direction_Down  == 0); // 'Normal' orientation.
	TT_STATIC_ASSERT(movement::Direction_Right == 1); // 1 rotation.
	TT_STATIC_ASSERT(movement::Direction_Up    == 2); // 2 rotations.
	TT_STATIC_ASSERT(movement::Direction_Left  == 3); // 3 rotations.
	TT_STATIC_ASSERT(movement::Direction_None  == 4); // This value is used to wrap our rotation around again.
	TT_STATIC_ASSERT(movement::Direction_Count == 5); // This assert is to make sure there are no more values.
	
	// Rotate our result based on the current orientation. (and wrap based on the max rotation (none)).
	result = static_cast<movement::Direction>((result + p_orientationDown) % movement::Direction_None);
	
	TT_ASSERT(movement::isValidDirection(result));
	return result;
}


tt::math::Vector2 applyOrientationToVector2(const tt::math::Vector2& p_vec,
                                            movement::Direction      p_orientationDown,
                                            bool                     p_orientationForwardIsLeft)
{
	// Flip
	const real x = (p_orientationForwardIsLeft) ? -p_vec.x : p_vec.x;
	
	// Rotate
	switch (p_orientationDown)
	{
	case movement::Direction_Down:  return tt::math::Vector2(       x,  p_vec.y);
	case movement::Direction_Right: return tt::math::Vector2(-p_vec.y,        x);
	case movement::Direction_Up:    return tt::math::Vector2(      -x, -p_vec.y);
	case movement::Direction_Left:  return tt::math::Vector2( p_vec.y,       -x);
	case movement::Direction_None:  return tt::math::Vector2(       x,  p_vec.y); // Same as down.
	default:
		TT_PANIC("Unknown m_orientationDown: %d!", p_orientationDown);
		return p_vec;
	}
}


tt::math::Point2 applyOrientationToPoint2(const tt::math::Point2& p_point,
                                          movement::Direction     p_orientationDown,
                                          bool                    p_orientationForwardIsLeft)
{
	// Flip
	const s32 x = (p_orientationForwardIsLeft) ? -p_point.x : p_point.x;
	
	// Rotate
	switch (p_orientationDown)
	{
	case movement::Direction_Down:  return tt::math::Point2(         x,  p_point.y);
	case movement::Direction_Right: return tt::math::Point2(-p_point.y,        x);
	case movement::Direction_Up:    return tt::math::Point2(        -x, -p_point.y);
	case movement::Direction_Left:  return tt::math::Point2( p_point.y,       -x);
	case movement::Direction_None:  return tt::math::Point2(         x,  p_point.y); // Same as down.
	default:
		TT_PANIC("Unknown m_orientationDown: %d!", p_orientationDown);
		return p_point;
	}
}


tt::math::VectorRect applyOrientationToVectorRect(const tt::math::VectorRect& p_rect,
                                                  movement::Direction p_orientationDown,
                                                  bool                p_orientationForwardIsLeft)
{
	const tt::math::Vector2 cornerOne(applyOrientationToVector2(p_rect.getMin(),
	                                                            p_orientationDown, 
	                                                            p_orientationForwardIsLeft));
	const tt::math::Vector2 cornerTwo(applyOrientationToVector2(p_rect.getMaxInside(),
	                                                            p_orientationDown,
	                                                            p_orientationForwardIsLeft));
	
	const tt::math::Vector2 min(std::min(cornerOne.x, cornerTwo.x), 
	                            std::min(cornerOne.y, cornerTwo.y));
	const tt::math::Vector2 max(std::max(cornerOne.x, cornerTwo.x), 
	                            std::max(cornerOne.y, cornerTwo.y));
	
	return tt::math::VectorRect(min, max);
}


real applyOrientationToAngle(real p_angle,
                             movement::Direction p_orientationDown,
                             bool                p_orientationForwardIsLeft)
{
	TT_ASSERT(p_angle >= 0.0f && p_angle < tt::math::twoPi);
	
	if (p_orientationForwardIsLeft)
	{
		// 'mirror' horizontaly.
		
		if (p_angle <= tt::math::pi)
		{
			real diff = tt::math::pi - p_angle;
			p_angle = tt::math::pi + diff;
			
			if (p_angle >= tt::math::twoPi)
			{
				p_angle -= tt::math::twoPi;
			}
		}
		else
		{
			p_angle = tt::math::twoPi - p_angle;
		}
		
		TT_ASSERT(p_angle >= 0.0f && p_angle < tt::math::twoPi);
	}
	
	switch (p_orientationDown)
	{
	case movement::Direction_Down:
	case movement::Direction_None:
		break;
		
	case movement::Direction_Right: p_angle += tt::math::halfPi; break;
	case movement::Direction_Up:    p_angle += tt::math::pi; break;
	case movement::Direction_Left:  p_angle += tt::math::oneAndAHalfPi; break;
	default:
		TT_PANIC("Unknown m_orientationDown: %d!", p_orientationDown);
		break;
	}
	
	if (p_angle >= tt::math::twoPi)
	{
		p_angle -= tt::math::twoPi;
	}
	TT_ASSERT(p_angle >= 0.0f && p_angle < tt::math::twoPi);
	
	return p_angle;
}


LocalDir getLocalDirFromDirection(movement::Direction p_worldDir,
                                  movement::Direction p_orientationDown,
                                  bool                p_orientationForwardIsLeft)
{
	TT_ASSERT(movement::isValidDirection(p_worldDir));
	if (p_worldDir == movement::Direction_None)
	{
		return LocalDir_None;
	}
	
	LocalDir result = LocalDir_Forward;
	// Do the same as getDirectionFromLocalDir but in reverse.
	const movement::Direction dir = static_cast<movement::Direction>(
			(p_worldDir - p_orientationDown + movement::Direction_None) % movement::Direction_None);
	switch (dir)
	{
	case movement::Direction_Right: result = (p_orientationForwardIsLeft) ? LocalDir_Back    : LocalDir_Forward; break;
	case movement::Direction_Up:    result = LocalDir_Up;      break;
	case movement::Direction_Left:  result = (p_orientationForwardIsLeft) ? LocalDir_Forward : LocalDir_Back;    break;
	case movement::Direction_Down:  result = LocalDir_Down;    break;
	default:
		TT_PANIC("Incorrect direction (%d) calculated from worldDir: %d!", dir, p_worldDir);
		return LocalDir_Invalid;
	}
	
	// Make sure the reverse function has the same result.
	TT_ASSERT(p_worldDir == getDirectionFromLocalDir(result, p_orientationDown, p_orientationForwardIsLeft));
	return result;
}


tt::math::Vector2 removeOrientationFromVector2(const tt::math::Vector2& p_worldVec,
                                               movement::Direction p_orientationDown,
                                               bool                p_orientationForwardIsLeft)
{
	tt::math::Vector2 result(p_worldVec);
	
	// Rotate
	switch (p_orientationDown)
	{
	case movement::Direction_Down:                                                            break;
	case movement::Direction_Right: result = tt::math::Vector2( p_worldVec.y, -p_worldVec.x); break;
	case movement::Direction_Up:    result = tt::math::Vector2(-p_worldVec.x, -p_worldVec.y); break;
	case movement::Direction_Left:  result = tt::math::Vector2(-p_worldVec.y,  p_worldVec.x); break;
	case movement::Direction_None:                                                            break;
	default:
		TT_PANIC("Unknown m_orientationDown: %d!", p_orientationDown);
		break;
	}
	
	if (p_orientationForwardIsLeft)
	{
		result.x = -result.x;
	}
	
	// Make sure the reverse function has the same result.
	TT_ASSERT(applyOrientationToVector2(result, p_orientationDown, p_orientationForwardIsLeft) == p_worldVec);
	return result;
}


tt::math::VectorRect removeOrientationFromVectorRect(const tt::math::VectorRect& p_rect,
                                                     movement::Direction p_orientationDown,
                                                     bool                p_orientationForwardIsLeft)
{
	const tt::math::Vector2 cornerOne(removeOrientationFromVector2(p_rect.getMin(),
	                                                               p_orientationDown, 
	                                                               p_orientationForwardIsLeft));
	const tt::math::Vector2 cornerTwo(removeOrientationFromVector2(p_rect.getMaxInside(),
	                                                               p_orientationDown,
	                                                               p_orientationForwardIsLeft));
	
	const tt::math::Vector2 min(std::min(cornerOne.x, cornerTwo.x), 
	                            std::min(cornerOne.y, cornerTwo.y));
	const tt::math::Vector2 max(std::max(cornerOne.x, cornerTwo.x), 
	                            std::max(cornerOne.y, cornerTwo.y));
	
	return tt::math::VectorRect(min, max);
}


tt::math::Quaternion getOrientationQuaternion(movement::Direction p_orientationDown)
{
	using tt::math::Quaternion;
	Quaternion rotation;
	
	switch (p_orientationDown)
	{
	case movement::Direction_Down: break;
	case movement::Direction_Right: rotation = Quaternion::getRotationZ(tt::math::halfPi);        break;
	case movement::Direction_Up:    rotation = Quaternion::getRotationZ(tt::math::pi);            break;
	case movement::Direction_Left:  rotation = Quaternion::getRotationZ(tt::math::oneAndAHalfPi); break;
	case movement::Direction_None: break;
	default:
		TT_PANIC("Unknown direction: %d!", p_orientationDown);
		break;
	}
	
	return rotation;
}

// Namespace end
}
}
}
