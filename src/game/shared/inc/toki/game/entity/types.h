#if !defined(INC_TOKI_GAME_ENTITY_TYPES_H)
#define INC_TOKI_GAME_ENTITY_TYPES_H


#include <string>
#include <vector>

#include <tt/math/Quaternion.h>
#include <tt/math/Rect.h>
#include <tt/math/Vector2.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_types.h>

#include <toki/game/entity/sensor/fwd.h>
#include <toki/game/movement/fwd.h>


namespace toki   /*! */ {
namespace game   /*! */ {
namespace entity /*! */ {


typedef std::vector<sensor::SensorHandle> SensorHandles;


/*! \brief LocalDir is used to get the (world) Direction based on an entity's orientation.
    \note These are also used in the movementset xml. (Same name as the Enum without LocalDir_ and in lower case.) */
enum LocalDir
{
	LocalDir_Forward, //!< Forward.
	LocalDir_Up,      //!< Up.
	LocalDir_Back,    //!< Back.
	LocalDir_Down,    //!< Down.
	
	LocalDir_None,  //!< No direction.
	
	LocalDir_Count,
	LocalDir_Invalid
};


/*! \brief Indicates whether the specified direction is a valid LocalDir value. */
inline bool        isValidLocalDir(LocalDir p_dir) { return p_dir >= 0 && p_dir < LocalDir_Count; }


/*! \brief Retrieves a human-readable name for the direction value.
    \returns the same name as the Enum without LocalDir_ in lower case.*/
inline const char* getLocalDirName(LocalDir p_dir)
{
	switch (p_dir)
	{
	case LocalDir_Forward:
		return "forward";
	case LocalDir_Up:
		return "up";
	case LocalDir_Back:
		return "back";
	case LocalDir_Down:
		return "down";
	case LocalDir_None:
		return "none";
	default:
		TT_PANIC("Invalid direction enum: %d\n", p_dir);
		return "INVALID";
	}
}


inline std::string getLocalDirNameAsString(LocalDir p_dir) { return getLocalDirName(p_dir); }


/*! \brief Returns a LocalDir value for a given human-readable direction name.
    \note The expect names are the same as in the Enum without LocalDir_ and as lower case.*/
inline LocalDir   getLocalDirFromName(const std::string& p_name)
{
	for (s32 i = 0; i < LocalDir_Count; ++i)
	{
		LocalDir dir = static_cast<LocalDir>(i);
		if (p_name == getLocalDirName(dir))
		{
			return dir;
		}
	}
	// Nothing found
	return LocalDir_Invalid;
}


/*! \brief Returns the opposite direction of the LocalDir specified. */
inline LocalDir getInverseLocalDir(LocalDir p_dir)
{
	switch (p_dir)
	{
	case LocalDir_Forward:
		return LocalDir_Back;
	case LocalDir_Up:
		return LocalDir_Down;
	case LocalDir_Back:
		return LocalDir_Forward;
	case LocalDir_Down:
		return LocalDir_Up;
	case LocalDir_None:
		return LocalDir_None;
	default:
		TT_PANIC("Invalid direction enum: %d\n", p_dir);
		return LocalDir_None;
	}
}

movement::Direction getDirectionFromLocalDir(LocalDir            p_localDir,
                                             movement::Direction p_orientationDown,
                                             bool                p_orientationForwardIsLeft = false);

tt::math::Vector2 applyOrientationToVector2(const tt::math::Vector2& p_vec,
                                            movement::Direction      p_orientationDown,
                                            bool                     p_orientationForwardIsLeft = false);

tt::math::Point2 applyOrientationToPoint2(const tt::math::Point2& p_point,
                                          movement::Direction     p_orientationDown,
                                          bool                    p_orientationForwardIsLeft = false);

tt::math::VectorRect applyOrientationToVectorRect(const tt::math::VectorRect& p_rect,
                                                  movement::Direction p_orientationDown,
                                                  bool                p_orientationForwardIsLeft = false);

real applyOrientationToAngle(real p_angle,
                             movement::Direction p_orientationDown,
                             bool                p_orientationForwardIsLeft = false);

LocalDir getLocalDirFromDirection(movement::Direction p_worldDir,
                                  movement::Direction p_orientationDown,
                                  bool                p_orientationForwardIsLeft = false);

tt::math::Vector2 removeOrientationFromVector2(const tt::math::Vector2& p_worldVec,
                                               movement::Direction p_orientationDown,
                                               bool                p_orientationForwardIsLeft = false);

tt::math::VectorRect removeOrientationFromVectorRect(const tt::math::VectorRect& p_rect,
                                                     movement::Direction p_orientationDown,
                                                     bool                p_orientationForwardIsLeft);

tt::math::Quaternion getOrientationQuaternion(movement::Direction p_orientationDown);

// Namespace end
}
}
}


#endif  // !defined(INC_TOKI_GAME_ENTITY_TYPES_H)
