#ifndef INC_TT_CODE_TILERAYTRACER_H
#define INC_TT_CODE_TILERAYTRACER_H

#include <algorithm>

#include <tt/math/Point2.h>
#include <tt/math/Vector2.h>
#include <tt/platform/tt_types.h>


namespace tt {
namespace code {


//typedef bool (*hitTest)(const tt::math::Point2& p_location);


//-------------------------------------------------------------------------------------------------
// 'private' implementation details
namespace impl
{


inline void calcHitLocation(const tt::math::Vector2& p_start, const tt::math::Vector2& p_end,
                            const tt::math::Point2& p_hitTile, tt::math::Vector2* p_hitLocation)
{
	TT_NULL_ASSERT(p_hitLocation);
	
	real left   = static_cast<real>(p_hitTile.x);
	real right  = static_cast<real>(p_hitTile.x + 1);
	real bottom = static_cast<real>(p_hitTile.y);
	real top    = static_cast<real>(p_hitTile.y + 1);
	
	// Startpoint is in hit tile
	if (p_start.x >= left   && p_start.x <= right && 
		p_start.y >= bottom && p_start.y <= top)
	{
		*p_hitLocation = p_start;
		return;
	}
	
	real min = 0.0f;
	real max = 1.0f;
	real t0;
	real t1;
	
	// Check for div by zero
	real dx = p_end.x - p_start.x;
	if (tt::math::fabs(dx) > 1.0E-8f)
	{
		real invDx = 1.0f / dx;
		if (dx > 0)
		{
			t0 = (left - p_start.x) * invDx;
			t1 = (right - p_start.x) * invDx;
		}
		else
		{
			t1 = (left - p_start.x) * invDx;
			t0 = (right - p_start.x) * invDx;
		}
		
		if (t0 > min) min = t0;
		if (t1 < max) max = t1;
		
		TT_ASSERT(min <= max && max > 0.0f);
	}
	
	real dy = p_end.y - p_start.y;
	if (tt::math::fabs(dy) > 1.0E-8f)
	{
		real invDy = 1.0f / dy;
		if (dy > 0)
		{
			t0 = (bottom - p_start.y) * invDy;
			t1 = (top - p_start.y) * invDy;
		}
		else
		{
			t1 = (bottom - p_start.y) * invDy;
			t0 = (top - p_start.y) * invDy;
		}
		
		if (t0 > min) min = t0;
		if (t1 < max) max = t1;
		
		TT_ASSERT(tt::math::realLessEqual(min, max) && max > 0.0f);
	}
	
	p_hitLocation->x = p_start.x + dx * min;
	p_hitLocation->y = p_start.y + dy * min;
}


// impl namespace end.
}


//-------------------------------------------------------------------------------------------------
// 'public' functions

template <typename HitTester>
inline bool tileRayTrace(const tt::math::Vector2& p_start, const tt::math::Vector2& p_end,
                         HitTester& p_hitTest, tt::math::Vector2* p_traceEndLocation = 0)
{
	real x0 = p_start.x;
	real x1 = p_end.x;
	real y0 = p_start.y;
	real y1 = p_end.y;
	
	real dx = tt::math::fabs(x1 - x0);
	real dy = tt::math::fabs(y1 - y0);
	
	tt::math::Point2 location(
		static_cast<s32>(tt::math::floor(x0)),
		static_cast<s32>(tt::math::floor(y0)));
	
	s32 n = 1;
	s32 x_inc, y_inc;
	real error;
	
	if (dx == 0)
	{
		x_inc = 0;
		error = std::numeric_limits<real>::infinity();
	}
	else if (x1 > x0)
	{
		x_inc = 1;
		n += static_cast<s32>(tt::math::floor(x1)) - location.x;
		error = (tt::math::floor(x0) + 1 - x0) * dy;
	}
	else
	{
		x_inc = -1;
		n += location.x - static_cast<s32>(tt::math::floor(x1));
		error = (x0 - tt::math::floor(x0)) * dy;
	}
	
	if (dy == 0)
	{
		y_inc = 0;
		error -= std::numeric_limits<real>::infinity();
	}
	else if (y1 > y0)
	{
		y_inc = 1;
		n += static_cast<s32>(tt::math::floor(y1)) - location.y;
		error -= (tt::math::floor(y0) + 1 - y0) * dx;
	}
	else
	{
		y_inc = -1;
		n += location.y - static_cast<s32>(tt::math::floor(y1));
		error -= (y0 - tt::math::floor(y0)) * dx;
	}
	
	for (; n > 0; --n)
	{
		if (p_hitTest(location))
		{
			if (p_traceEndLocation != 0)
			{
				impl::calcHitLocation(p_start, p_end, location, p_traceEndLocation);
			}
			return false;
		}
		
		if (error > 0)
		{
			location.y += y_inc;
			error -= dx;
		}
		else
		{
			location.x += x_inc;
			error += dy;
		}
	}
	
	if (p_traceEndLocation != 0)
	{
		*p_traceEndLocation = p_end;
	}
	
	return true;
}


// Namespace end
}
}

#endif  // !defined(INC_TT_CODE_TILERAYTRACER_H)
