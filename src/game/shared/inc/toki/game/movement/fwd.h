#if !defined(INC_TOKI_GAME_MOVEMENT_FWD_H)
#define INC_TOKI_GAME_MOVEMENT_FWD_H


#include <string>
#include <vector>

#include <tt/code/BitMask.h>
#include <tt/math/hash/NamedHash.h>
#include <tt/platform/tt_types.h>


namespace toki      /*! */ {
namespace game      /*! */ {
namespace movement  /*! */ {


typedef tt::math::hash::NamedHash<32> NameHash;
typedef std::vector<NameHash>         NameHashes;

class DirectionalMovement;

class          MoveBase;
typedef tt_ptr<MoveBase     >::shared MoveBasePtr;
class          MoveVector;
typedef tt_ptr<MoveVector   >::shared MoveVectorPtr;
class          MoveAnimation;
typedef tt_ptr<MoveAnimation>::shared MoveAnimationPtr;

class MovementSet;
typedef tt_ptr<MovementSet>::shared MovementSetPtr;
typedef tt_ptr<MovementSet>::weak   MovementSetWeakPtr;

class Transition;
typedef tt_ptr<Transition>::shared       TransitionPtr;
typedef tt_ptr<const Transition>::shared ConstTransitionPtr;


enum TransitionSpeed
{
	TransitionSpeed_Zero,
	TransitionSpeed_UseTo,
	TransitionSpeed_UseFrom,

	TransitionSpeed_Count,
	TransitionSpeed_Invalid
};

inline bool isValidTransitionSpeed(TransitionSpeed p_speed) { return p_speed >= 0 && p_speed < TransitionSpeed_Count; }
inline const char* getTransitionSpeedName(TransitionSpeed p_speed)
{
	switch (p_speed)
	{
	case TransitionSpeed_Zero:    return "zero";
	case TransitionSpeed_UseTo:   return "use_to";
	case TransitionSpeed_UseFrom: return "use_from";
	default:
		TT_PANIC("Unknown TransitionSpeed: %d\n", p_speed);
		return "";
	}
}
inline TransitionSpeed getTransitionSpeedFromName(const std::string& p_str)
{
	for (s32 i = 0; i < TransitionSpeed_Count; ++i)
	{
		TransitionSpeed speed = static_cast<TransitionSpeed>(i);
		if (p_str == getTransitionSpeedName(speed))
		{
			return speed;
		}
	}
	return TransitionSpeed_Invalid;
}


/*! \brief Movement Direction. 
    \note These are also used in the movementset xml. (Same name as the Enum without Direction_ and in lower case.) */
enum Direction
{
	Direction_Down,  //!< Down.
	Direction_Right, //!< Right.
	Direction_Up,    //!< Up.
	Direction_Left,  //!< Left.
	
	Direction_None,  //!< No direction.
	
	Direction_Count,
	Direction_Invalid
};

/*! \brief Indicates whether the specified direction is a valid Direction value. */
inline bool        isValidDirection(Direction p_dir) { return p_dir >= 0 && p_dir < Direction_Count; }

/*! \brief Retrieves a human-readable name for the direction value. */
inline const char* getDirectionName(Direction p_dir)
{
	switch (p_dir)
	{
	case Direction_Down:
		return "down";
	case Direction_Right:
		return "right";
	case Direction_Up:
		return "up";
	case Direction_Left:
		return "left";
	case Direction_None:
		return "none";
	default:
		TT_PANIC("Invalid direction enum: %d\n", p_dir);
		return "INVALID";
	}
}

inline std::string getDirectionNameAsString(Direction p_dir) { return getDirectionName(p_dir); }

/*! \brief Returns a Direction value for a given human-readable direction name.
    \note The expect names are the same as in the Enum without Direction_ and as lower case.*/
inline Direction   getDirectionFromName(const std::string& p_name)
{
	for (s32 i = 0; i < Direction_Count; ++i)
	{
		Direction dir = static_cast<Direction>(i);
		if (p_name == getDirectionName(dir))
		{
			return dir;
		}
	}
	// Nothing found
	return Direction_Invalid;
}

/*! \brief Returns the opposite direction of the Direction specified. */
inline Direction getInverseDirection(Direction p_dir)
{
	switch (p_dir)
	{
	case Direction_Down:
		return Direction_Up;
	case Direction_Right:
		return Direction_Left;
	case Direction_Up:
		return Direction_Down;
	case Direction_Left:
		return Direction_Right;
	case Direction_None:
		return Direction_None;
	default:
		TT_PANIC("Invalid direction enum: %d\n", p_dir);
		return Direction_None;
	}
}

typedef tt::code::BitMask<Direction, Direction_Count> Directions; // Some moves support multiple directions.


/*! \brief Survey Results. 
    \note These are also used in the movementset xml. (Same name as the Enum without SurveyResult_ and in lower case.) */
enum SurveyResult
{
	// On a tile edge
	SurveyResult_OnRightEdge,  //!< Touching the right wall tile's edge.
	SurveyResult_OnTopEdge,    //!< Touching the roof       tile's edge.
	SurveyResult_OnLeftEdge,   //!< Touching the left  wall tile's edge.
	SurveyResult_OnBottomEdge, //!< Touching the floor      tile's edge.
	
	// Basic side collision
	SurveyResult_WallRight,   //!< At least one tile solid to the right.
	SurveyResult_Ceiling,     //!< At least one tile solid above.
	SurveyResult_WallLeft,    //!< At least one tile solid to the left.
	SurveyResult_Floor,       //!< At least one tile solid below.

	// Extended side collision
	SurveyResult_TwoRight,    //!< At least one tile solid two tiles to the right.
	SurveyResult_TwoUp,       //!< At least one tile solid two tiles above.
	SurveyResult_TwoLeft,     //!< At least one tile solid two tiles to the left.
	SurveyResult_TwoDown,     //!< At least one tile solid two tiles below.
	
	// For Walkers - The one, one tile offset.
	SurveyResult_OneUpRight,     //!< Check no collision with rect when moved one tile up and one to the right
	SurveyResult_OneUpLeft,      //!< Check no collision with rect when moved one tile up and one to the right
	SurveyResult_OneDownLeft,    //!< Check no collision with rect when moved one tile down and one to the right
	SurveyResult_OneDownRight,   //!< Check no collision with rect when moved one tile down and one to the right.

	SurveyResult_TopRight,     //!< Check no collision with rect when moved one tile up and one to the right
	SurveyResult_TopLeft,      //!< Check no collision with rect when moved one tile up and one to the right
	SurveyResult_BottomLeft,    //!< Check no collision with rect when moved one tile down and one to the right
	SurveyResult_BottomRight,   //!< Check no collision with rect when moved one tile down and one to the right.
	
	// For Walkers - The full width checked with one tile down.
	SurveyResult_DropRight, //!< On the edge of a drop (no solid tiles) to the right big enough to drop in.
	SurveyResult_DropRight_fromRightAsDown,  //!< DropRight with Right as down orientation
	SurveyResult_DropRight_fromUpAsDown,     //!< DropRight with Up as down orientation
	SurveyResult_DropRight_fromLeftAsDown,   //!< DropRight with Left as down orientation
	
	SurveyResult_DropLeft,  //!< On the edge of a drop (no solid tiles) to the left big enough to drop in.
	SurveyResult_DropLeft_fromRightAsDown,   //!< DropLeft with Right as down orientation
	SurveyResult_DropLeft_fromUpAsDown,	     //!< DropLeft with Up as down orientation
	SurveyResult_DropLeft_fromLeftAsDown,    //!< DropLeft with Left as down orientation
	
	// --- No rotation below this -----------------------------------------------------------------
	
	// NOTE: !SurveyResult_InsideCollision needs to be the first in this no rotation list!
	SurveyResult_InsideCollision, //!< Any of the tiles inside the rect is solid.
	SurveyResult_InsideCollisionRaw, //!< Any of the tiles inside the rect is solid. (without parent ignore code.)
	SurveyResult_StandOnSolid,    //!< Standing on someting solid. (Can be seen as: Floor + OnBottomEdge.)
	SurveyResult_OnParent,        //!< Standing on a collision parent.
	SurveyResult_OnParentVertical, //!< Standing on a collision parent moving vertically relative to the entity.
	
	// Water info (Only one can be true at a time.)
	SurveyResult_OnWaterLeft,      //!< On water surface which is moving to left.
	SurveyResult_OnWaterRight,     //!< On water surface which is moving to right.
	SurveyResult_OnWaterStatic,    //!< On water surface which isn't moving.
	SurveyResult_NotInWater,       //!< Not in water. (Not submerged or on surface.)
	SurveyResult_SubmergedInWater, //!< Submerged in water. (Below the surface.)
	
	// Water Flow direction (Only one can be true at a time.) (No flag set when not in water.)
	SurveyResult_WaterFlowLeft,   //!< The direction of the water flow is left.
	SurveyResult_WaterFlowRight,  //!< The direction of the water flow is right.
	SurveyResult_WaterFlowStatic, //!< The direction of the water flow is static.
	
	// Lava info (Only one can be true at a time.)
	SurveyResult_OnLavaLeft,      //!< On lava surface which is moving to left.
	SurveyResult_OnLavaRight,     //!< On lava surface which is moving to right.
	SurveyResult_OnLavaStatic,    //!< On lava surface which isn't moving.
	SurveyResult_NotInLava,       //!< Not in lava. (Not submerged or on surface.)
	SurveyResult_SubmergedInLava, //!< Submerged in lava. (Below the surface.)
	
	SurveyResult_Count,
	SurveyResult_Invalid,
	
	SurveyResult_NoRotationAfterThis = SurveyResult_InsideCollision
};

inline bool isValidSurveyResult(SurveyResult p_result) { return p_result >= 0 && p_result < SurveyResult_Count; }

inline const char* getSurveyResultName(SurveyResult p_result)
{
	switch (p_result)
	{
	case SurveyResult_OnBottomEdge:    return "on_bottom_edge";
	case SurveyResult_OnTopEdge:       return "on_top_edge";
	case SurveyResult_OnRightEdge:     return "on_right_edge";
	case SurveyResult_OnLeftEdge:      return "on_left_edge";
	case SurveyResult_Floor:           return "floor";
	case SurveyResult_Ceiling:         return "ceiling";
	case SurveyResult_WallLeft:        return "wall_left";
	case SurveyResult_WallRight:       return "wall_right";
	case SurveyResult_TwoRight:        return "two_right";
	case SurveyResult_TwoUp:           return "two_up";
	case SurveyResult_TwoLeft:         return "two_left";
	case SurveyResult_TwoDown:         return "two_down";
	case SurveyResult_OneUpRight:      return "one_up_right";
	case SurveyResult_OneUpLeft:       return "one_up_left";
	case SurveyResult_OneDownRight:    return "one_down_right";
	case SurveyResult_OneDownLeft:     return "one_down_left";
	case SurveyResult_TopRight:        return "top_right";
	case SurveyResult_TopLeft:         return "top_left";
	case SurveyResult_BottomRight:     return "bottom_right";
	case SurveyResult_BottomLeft:      return "bottom_left";
	case SurveyResult_DropRight:       return "drop_right";
	case SurveyResult_DropRight_fromRightAsDown: return "drop_right_from_right_as_down";
	case SurveyResult_DropRight_fromUpAsDown:    return "drop_right_from_up_as_down";
	case SurveyResult_DropRight_fromLeftAsDown:  return "drop_right_from_left_as_down";
	case SurveyResult_DropLeft:        return "drop_left";
	case SurveyResult_DropLeft_fromRightAsDown: return "drop_left_from_right_as_down";
	case SurveyResult_DropLeft_fromUpAsDown:    return "drop_left_from_up_as_down";
	case SurveyResult_DropLeft_fromLeftAsDown:  return "drop_left_from_left_as_down";
	case SurveyResult_InsideCollision:  return "inside_collision";
	case SurveyResult_InsideCollisionRaw: return "inside_collision_raw";
	case SurveyResult_StandOnSolid:     return "stand_on_solid";
	case SurveyResult_OnParent:         return "on_parent";
	case SurveyResult_OnParentVertical: return "on_parent_vertical";
	case SurveyResult_OnWaterLeft:      return "on_water_left";
	case SurveyResult_OnWaterRight:     return "on_water_right";
	case SurveyResult_OnWaterStatic:    return "on_water_static";
	case SurveyResult_NotInWater:       return "not_in_water";
	case SurveyResult_SubmergedInWater: return "submerged_in_water";
	case SurveyResult_WaterFlowLeft:    return "water_flow_left";
	case SurveyResult_WaterFlowRight:   return "water_flow_right";
	case SurveyResult_WaterFlowStatic:  return "water_flow_static";
	case SurveyResult_OnLavaLeft:       return "on_lava_left";
	case SurveyResult_OnLavaRight:      return "on_lava_right";
	case SurveyResult_OnLavaStatic:     return "on_lava_static";
	case SurveyResult_NotInLava:        return "not_in_lava";
	case SurveyResult_SubmergedInLava:  return "submerged_in_lava";
	default:
		TT_PANIC("Unknown SurveyResult: %d", p_result);
		return "";
	}
}


inline SurveyResult getSurveyResultFromName(const std::string& p_name)
{
	for (s32 i = 0; i < SurveyResult_Count; ++i)
	{
		SurveyResult result = static_cast<SurveyResult>(i);
		if (p_name == getSurveyResultName(result))
		{
			return result;
		}
	}
	return SurveyResult_Invalid;
}
typedef tt::code::BitMask<SurveyResult, SurveyResult_Count> SurveyResults;


inline SurveyResult rotateForDown(SurveyResult p_result, Direction p_down, bool p_forwardIsLeft)
{
	if (p_result >= SurveyResult_NoRotationAfterThis)
	{
		return p_result;
	}
	
	const s32 indexWithInType = p_result        / 4; // Every 4 values are a check in all (4) rotations.
	s32       firstOfType     = indexWithInType * 4; // Get the index for the first of this type.
	s32 value = p_result - firstOfType;    // Get value in the range 0 to 3;
	TT_ASSERT(value >= 0 && value <= 3);
	
	if (p_forwardIsLeft) // If left or right check and we need to flip it.
	{
		// Some types need different kind of flip.
		switch (firstOfType)
		{
			// "Left" becomes "Right", "Right" becomes "Left"
		case SurveyResult_OneUpRight: // Fall-through, no break.
		case SurveyResult_TopRight:   value ^= 1;                           break;
		case SurveyResult_DropRight:  firstOfType = SurveyResult_DropLeft;  break;
		case SurveyResult_DropLeft:   firstOfType = SurveyResult_DropRight; break;
		default:
			if ((value % 2) == 0) // 'Normal' left-right flip.
			{
				value += 2; // Flip
			}
		}
	}
	
	value += p_down; // Rotate
	value %= 4;      // Clamp
	
	value += firstOfType; // Return to proper check
	TT_ASSERT(p_down != Direction_Down || p_forwardIsLeft || value == p_result); // Check that a rotation with down as down hasn't changed anything.
	return static_cast<SurveyResult>(value);
}


inline SurveyResults rotateForDown(const SurveyResults& p_results, Direction p_down, bool p_forwardIsLeft)
{
	SurveyResults rotated;
	for (s32 i = 0; i < SurveyResult_Count; ++i)
	{
		SurveyResult result = static_cast<SurveyResult>(i);
		
		if (p_results.checkFlag(result))
		{
			rotated.setFlag(rotateForDown(result, p_down, p_forwardIsLeft));
		}
	}
	TT_ASSERT(p_down != Direction_Down || p_forwardIsLeft || rotated == p_results); // Check that a rotation with down as down hasn't changed anything.
	return rotated;
}


class Validator;
class SurroundingsSurvey;

// Namespace end
}
}
}


#endif  // !defined(INC_TOKI_GAME_MOVEMENT_FWD_H)
