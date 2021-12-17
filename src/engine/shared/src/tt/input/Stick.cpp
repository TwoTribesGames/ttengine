#include <tt/input/Stick.h>


namespace tt {
namespace input {

Stick::Stick()
:
Vector2(Vector2::zero)
{
}


Stick::Stick(const tt::math::Vector2& p_vector)
:
Vector2(p_vector)
{
}


Stick Stick::getNormalizedStick() const
{
	Stick result;
	
	// Deadzone
	const real deadzoneSize = 0.25f; // FIXME: Add this value to the cfg.
	const real len = lengthSquared();
	if (len > (deadzoneSize * deadzoneSize) + 0.0001f)
	{
		result = *this;
		if (math::realGreaterThan(len, 1.0f))
		{
			result.normalize();
		}
		
		const tt::math::Vector2 normalized = getNormalized();
		result -= (normalized * deadzoneSize); // Remove deadzone
		result *= 1.0f / (1.0f - deadzoneSize); // Scale back to full range. (-1.0 - 1.0f)
		
		// Make 100% sure result is normalized (rounding errors of the calculations above can lead to length() > 1
		if (math::realGreaterThan(len, 1.0f))
		{
			result.normalize();
		}
	}
	
	return result;
}


Stick::Direction4 Stick::getDirection4() const
{
	// Should normally operate on normalized sticks
	const real len(lengthSquared());
	TT_ASSERTMSG(math::realLessEqual(len, 1.0f), "getDirection4 should use a normalized stick. Squared length is %.2f", len);
	
	if (len > directionThresholdSquared)
	{
		// Translate analog movement to left/right/up/down presses
		const int sector(getWedgeSector(4));
		TT_ASSERT(sector >= 0 && sector < Direction4_None);
		return static_cast<Stick::Direction4>(sector);
	}
	return Direction4_None;
}


Stick::Direction8 Stick::getDirection8() const
{
	// Should normally operate on normalized sticks
	const real len(lengthSquared());
	TT_ASSERTMSG(math::realLessEqual(len, 1.0f), "getDirection8 should use a normalized stick. Squared length is %.2f", len);
	
	if (len > directionThresholdSquared)
	{
		const int sector(getWedgeSector(8));
		TT_ASSERT(sector >= 0 && sector < Direction8_None);
		return static_cast<Stick::Direction8>(sector);
	}
	return Direction8_None;
}


int Stick::getWedgeSector(int p_numberOfDirections) const
{
	// convert to -1..1
	real angle = getAngle() / math::pi;
	// rotate so that the sector 0 wedge faces up
	angle += (1.0f / p_numberOfDirections);
	
	// ensure range is 0..2
	if (angle < 0.0f)
	{
		angle += 2.0f;
	}
	
	// finally convert to sector
	return static_cast<int>(angle * p_numberOfDirections * 0.5f);
}


// Namespace end
}
}
