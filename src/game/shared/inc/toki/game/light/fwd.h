#if !defined(INC_TOKI_GAME_LIGHT_FWD_H)
#define INC_TOKI_GAME_LIGHT_FWD_H

#include <vector>

#include <tt/code/Handle.h>
#include <tt/math/Rect.h>
#include <tt/math/Vector2.h>

namespace toki {
namespace game {
namespace light {


enum ShadowQuality
{
	ShadowQuality_Low,
	ShadowQuality_Medium,
	ShadowQuality_High
};

class Glow;
typedef tt_ptr<Glow>::shared GlowPtr;

class JitterEffect;
typedef tt_ptr<JitterEffect>::shared JitterEffectPtr;

class Light;
typedef tt::code::Handle<Light> LightHandle;

class LightMgr;
typedef tt_ptr<LightMgr>::shared LightMgrPtr;

class Darkness;
typedef tt::code::Handle<Darkness> DarknessHandle;

class DarknessMgr;
typedef tt_ptr<DarknessMgr>::shared DarknessMgrPtr;


class Circle;
class LightShape;
class LightTriangle;
class Polygon;
typedef tt_ptr<Polygon>::shared PolygonPtr;

typedef std::vector<Circle*>        Circles;
typedef std::vector<LightShape*>    LightsShapes;
typedef std::vector<LightTriangle*> LightTriangles;
typedef std::vector<Polygon*>       Polygons;

typedef std::vector<bool>           BoolVector;

typedef std::vector<tt::math::Vector2> Vertices;

typedef std::vector<tt::math::VectorRect> Rects;


struct Shadow
{
public:
	tt::math::Vector2 left;
	tt::math::Vector2 right;
	tt::math::Vector2 normal; // Normal of the line described by left and right. (Points away from Light)
	tt::math::Vector2 pointClosestToLightOnLine;
	real              leftDistance;
	real              rightDistance;
	real              leftAngle;
	real              rightAngle;
	real              minimumDistance; // The smallest distance from light to shadowline (used for distance sort.)
	real              projectionDistance; // The length of the projectionline from light projected on the whole shadow line. (Could fall outside of left en right's part!)
	
	inline Shadow(const tt::math::Vector2& p_left,
	              const tt::math::Vector2& p_right,
	              const tt::math::Vector2& p_normal)
	:
	left(         p_left),
	right(        p_right),
	normal(       p_normal),
	pointClosestToLightOnLine(), // Gets calculated in ctor.
	leftDistance( p_left.length()),
	rightDistance(p_right.length()),
	leftAngle(    p_left.getAngleWithUnitX()),
	rightAngle(   p_right.getAngleWithUnitX()),
	minimumDistance(0),
	projectionDistance(0)
	{
		pointClosestToLightOnLine = getIntersectionPoint(normal);
		projectionDistance        = pointClosestToLightOnLine.length();
		calcMinimumDistance();
	}
	
	inline Shadow(real p_rightAngle, real p_leftAngle, real p_distance)
	:
	left(  tt::math::Vector2::getRotatedFromUnitX( p_leftAngle  ) * p_distance),
	right( tt::math::Vector2::getRotatedFromUnitX( p_rightAngle ) * p_distance),
	normal(tt::math::Vector2::getRotatedFromUnitX( p_rightAngle + ((p_leftAngle - p_rightAngle) * 0.5f))),
	pointClosestToLightOnLine(normal * p_distance),
	leftDistance( p_distance),
	rightDistance(p_distance),
	leftAngle(    p_leftAngle),
	rightAngle(   p_rightAngle),
	minimumDistance(p_distance),
	projectionDistance(p_distance)
	{
		TT_ASSERT(p_rightAngle <= p_leftAngle);
		TT_ASSERT(p_rightAngle >= 0.0f && p_rightAngle <= tt::math::twoPi);
		TT_ASSERT(p_leftAngle  >= 0.0f && p_leftAngle  <= tt::math::twoPi);
	}
	
	inline tt::math::Vector2 getIntersectionPoint(real p_angle) const
	{
		return getIntersectionPoint(tt::math::Vector2::getRotatedFromUnitX(p_angle));
	}
	
	inline tt::math::Vector2 getIntersectionPoint(const tt::math::Vector2& p_dirFromLightPoint) const
	{
		// Find the line line intersection.
		// http://paulbourke.net/geometry/lineline2d/
		// 
		// P1 = left      (x1 = left.x, y1 = left.y);
		// P2 = right
		// P3 = 0
		// P4 = p_dirFromLightPoint
		// 
		// Normal ((x4 - x3)*(y1 - y3) - (y4 - y3)*(x1 - x3)) / ((y4 - y3)*(x2 - x1) - (x4 - x3)*(y2 - y1));
		// Remove all 3s because they are zero. (replace 2 - 1 with precalc diff.)
		const tt::math::Vector2 diff(right - left);
		
		const real denominator = (p_dirFromLightPoint.y * diff.x) - (p_dirFromLightPoint.x * diff.y);
		
		if (denominator != 0)
		{
			const real t = ((p_dirFromLightPoint.x * left.y) - (p_dirFromLightPoint.y * left.x))
			               /
			               denominator;
			
			return tt::math::Vector2(left.x + (t * diff.x),
			                         left.y + (t * diff.y));
		}
		
		return left;
	}
	
	inline void setRightAngle(real p_angle)
	{
		right = getIntersectionPoint(p_angle);
		
		rightDistance = right.length();
		TT_WARNING(tt::math::realEqual(right.getAngleWithUnitX(),                   p_angle) ||
		           tt::math::realEqual(right.getAngleWithUnitX() + tt::math::twoPi, p_angle) ||
		           tt::math::realEqual(right.getAngleWithUnitX() - tt::math::twoPi, p_angle) ||
		           rightDistance < 0.0001f,
		           "right.getAngleWithUnitX() (%f) didn't equal p_angle (%f) (Vector (%f, %f) length: %f.)",
		           right.getAngleWithUnitX(), p_angle, right.x, right.y, rightDistance);
		rightAngle = p_angle;
		
		calcMinimumDistance();
	}
	
	inline void setLeftAngle(real p_angle)
	{
		left = getIntersectionPoint(p_angle);
		
		leftDistance = left.length();
		TT_WARNING(tt::math::realEqual(left.getAngleWithUnitX(),                   p_angle) ||
		           tt::math::realEqual(left.getAngleWithUnitX() + tt::math::twoPi, p_angle) ||
		           tt::math::realEqual(left.getAngleWithUnitX() - tt::math::twoPi, p_angle) ||
		           leftDistance < 0.0001f,
		           "left.getAngleWithUnitX() (%f) didn't equal p_angle (%f) (Vector (%f, %f) length: %f.)",
		           left.getAngleWithUnitX(), p_angle, left.x, left.y, leftDistance);
		leftAngle = p_angle;
		
		calcMinimumDistance();
	}
	
	inline bool angleIsInbetweenRightAndLeft(real p_angle) const
	{
		if (rightAngle <= leftAngle) // We don't go over 0/2PI
		{
			// Is it in between right and left?
			if (p_angle >= rightAngle &&
			    p_angle <= leftAngle)
			{
				return true;
			}
		}
		else // We have 0/2PI in between left and right.
		{
			if ( (p_angle >= 0 &&
			      p_angle <= rightAngle)
			    ||
			     (p_angle >= leftAngle &&
			      p_angle <= tt::math::twoPi))
			{
				return true;
			}
		}
		return false;
	}
	
	inline void calcMinimumDistance()
	{
		const real angleForShortestDistanceToLine = normal.getAngleWithUnitX();
		
		// Does the shortest distance fall on our part of the line.
		if (angleIsInbetweenRightAndLeft(angleForShortestDistanceToLine))
		{
			minimumDistance = projectionDistance;
			return;
		}
		
		// Fallback to left and right distance.
		minimumDistance = std::min(leftDistance, rightDistance);
	}
	
	static inline bool sortDistance(const  Shadow& p_left, const Shadow& p_right)
	{
		return p_left.minimumDistance < p_right.minimumDistance;
	}
	
	static inline bool sortRightAngle(const Shadow& p_left, const Shadow& p_right)
	{
		if (p_left.rightAngle == p_right.rightAngle)
		{
			return p_left.leftAngle < p_right.leftAngle;
		}
		return p_left.rightAngle < p_right.rightAngle;
	}
};

typedef std::vector<Shadow> Shadows;


// Namespace end
}
}
}


#endif  // !defined(INC_TOKI_GAME_LIGHT_FWD_H)
