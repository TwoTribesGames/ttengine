#if !defined(INC_TT_INPUT_STICK_H)
#define INC_TT_INPUT_STICK_H


#include <tt/math/math.h>
#include <tt/math/Vector2.h>
#include <tt/platform/tt_types.h>


namespace tt {
namespace input {

struct Stick : public tt::math::Vector2
{
public:
	static constexpr real directionThresholdSquared = 0.36f;
	
	enum Direction4
	{
		Direction4_Up,
		Direction4_Right,
		Direction4_Down,
		Direction4_Left,
		Direction4_None
	};

	enum Direction8
	{
		Direction8_Up,
		Direction8_UpRight,
		Direction8_Right,
		Direction8_DownRight,
		Direction8_Down,
		Direction8_DownLeft,
		Direction8_Left,
		Direction8_UpLeft,
		Direction8_None
	};
	
	Stick();
	explicit Stick(const tt::math::Vector2& p_vector);
	
	/*! \brief Returns the angle of the stick in radians.
	    \return The angle of the stick in radians. */
	inline real getAngle() const
	{
		return math::atan2(x, y);
	}
	
	/*! \brief Returns the angle of the stick in degrees.
	    \return The angle of the stick in degrees [-180..180]. */
	inline real getAngleDeg() const
	{
		return math::radToDeg(getAngle());
	}
	
	inline void reset() { x = 0.0f; y = 0.0f; }
	
	Stick getNormalizedStick() const;
	Direction4 getDirection4() const;
	Direction8 getDirection8() const;
	int getWedgeSector(int p_numberOfDirections) const;
};

// Namespace end
}
}


#endif  // !defined(INC_TT_INPUT_STICK_H)
