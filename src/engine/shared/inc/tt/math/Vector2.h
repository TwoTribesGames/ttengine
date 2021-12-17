#if !defined(INC_TT_MATH_VECTOR2_H)
#define INC_TT_MATH_VECTOR2_H

#include <ostream>

#include <tt/platform/tt_types.h>
#include <tt/platform/tt_error.h>
#include <tt/streams/BOStream.h>
#include <tt/streams/BIStream.h>
#include <tt/math/math.h>


namespace tt
{
	namespace math
	{
		class Point2;
	}
	namespace xml
	{
		class XmlNode;
	}
}


namespace tt {
namespace math {


class Vector2
{
public:
	typedef real ValueType;
	
	// Note JL: explicit is useless here... TODO: create a Vector2 construct without initialization
	explicit Vector2(real p_x = 0.0f, real p_y = 0.0f)
	:
	x(p_x),
	y(p_y)
	{ }
	
	explicit Vector2(const Point2& p_point);
	
	Vector2(const Vector2& p_rhs)
	:
	x(p_rhs.x),
	y(p_rhs.y)
	{
	}
	
	~Vector2() { }
	
	inline void setValues(real p_x, real p_y)
	{
		x = p_x;
		y = p_y;
	}
	
	// Operators
	inline const Vector2& operator=(const Vector2& p_rhs)
	{
		x = p_rhs.x; 
		y = p_rhs.y; 
		return *this;
	}
	
	inline Vector2 operator-() const
	{
		return Vector2(-x, -y);
	}
	
	inline Vector2& operator+=(const Vector2& p_rhs)
	{
		x += p_rhs.x;
		y += p_rhs.y;
		return *this;
	}
	
	inline Vector2& operator-=(const Vector2& p_rhs)
	{
		x -= p_rhs.x;
		y -= p_rhs.y;
		return *this;
	}
	
	template<typename T>
	inline Vector2& operator*=(const T p_rhs)
	{
		x *= p_rhs; 
		y *= p_rhs;
		return *this;
	}
	
	template<typename T>
	inline Vector2& operator/=(const T p_rhs)
	{
		TT_ASSERTMSG(p_rhs != 0, "Division by zero is undefined");
		
		real inv = 1.0f / p_rhs;
		x *= inv; 
		y *= inv;
		
		return *this;
	}
	
	// Normalization
	inline Vector2& normalize()
	{
		const real len = length();
		
		if (len > 0)
		{
			const real inv_len = 1.0f / len;
			
			x *= inv_len;
			y *= inv_len;
		}
		return *this;
	}
	
	inline void normalizeClamp()
	{
		const real len = length();
		if (len > 1.0f && len > 0.0f)
		{
			const real inv_len = 1.0f / len;
			
			x *= inv_len;
			y *= inv_len;
		}
	}
	
	inline Vector2 getNormalized() const
	{
		Vector2 normalized(*this);
		normalized.normalize();
		return normalized;
	}
	
	// Vector length
	inline real lengthSquared() const
	{
		return (x * x) + (y * y);
	}
	
	inline real length() const
	{
		return math::sqrt(lengthSquared());
	}
	
	// Rotation
	inline Vector2 getRotated(real p_angleRad) const
	{
		const real angleSin = math::sin(p_angleRad);
		const real angleCos = math::cos(p_angleRad);
		return Vector2((x * angleCos) + (y * -angleSin),
		               (x * angleSin) + (y *  angleCos));
	}
	
	inline Vector2& rotate(real p_angleRad)
	{
		*this = getRotated(p_angleRad);
		return *this;
	}
	
	static inline Vector2 getRotatedFromUnitX(real p_angleRad)
	{
		return Vector2(tt::math::cos(p_angleRad), tt::math::sin(p_angleRad));
	}
	
	// returns the angle of this vector with the unitX (1.0, 0.0) vector. (Range: 0 to 2Pi).
	inline real getAngleWithUnitX() const
	{
		real angle = tt::math::atan2(y, x);
		if (y < 0.0f)
		{
			angle += tt::math::twoPi;
		}
		return angle;
	}
	
	// returns The angle between this vector and the specified p_otherVector. (Range: -2Pi to 2Pi).
	// Note: If you rotate this vector with the result you (should) get the otherVector.
	//       Alternatively rotate otherVector with -result to get this vector.
	inline real getAngle(const tt::math::Vector2& p_otherVector) const
	{
		return p_otherVector.getAngleWithUnitX() - getAngleWithUnitX();
	}
	
	// Vector constants
	static const Vector2 zero;
	static const Vector2 unitX;
	static const Vector2 unitY;
	static const Vector2 allOne;
	
public:
	real x;
	real y;
};


inline real dotProduct (const Vector2& p_lhs, const Vector2& p_rhs)
{
	return p_lhs.x * p_rhs.x + p_lhs.y * p_rhs.y;
}


inline real distanceSquared(const Vector2& p_v1, const Vector2& p_v2)
{
	return (p_v2.x - p_v1.x) * (p_v2.x - p_v1.x) + 
		   (p_v2.y - p_v1.y) * (p_v2.y - p_v1.y);
}


inline real distance(const Vector2& p_v1, const Vector2& p_v2)
{
	return math::sqrt(distanceSquared(p_v1, p_v2));
}


inline Vector2 floor(const Vector2& p_vec)
{
	return Vector2(floor(p_vec.x), floor(p_vec.y));
}


inline Vector2 ceil(const Vector2& p_vec)
{
	return Vector2(ceil(p_vec.x), ceil(p_vec.y));
}


inline std::ostream& operator<<(std::ostream& s, const Vector2& v)
{
	return s << '(' << v.x << ',' << v.y << ')';
}


inline tt::streams::BOStream& operator<<(tt::streams::BOStream& s, const Vector2& v)
{
	s << v.x;
	s << v.y;
	return s;
}


inline tt::streams::BIStream& operator>>(tt::streams::BIStream& s, Vector2& v)
{
	s >> v.x;
	s >> v.y;
	return s;
}


inline Vector2 operator+(const Vector2& p_lhs, const Vector2& p_rhs)
{
	return Vector2(p_lhs.x + p_rhs.x, p_lhs.y + p_rhs.y);
}


inline Vector2 operator-(const Vector2& p_lhs, const Vector2& p_rhs)
{
	return Vector2(p_lhs.x - p_rhs.x, p_lhs.y - p_rhs.y);
}


inline Vector2 operator*(const Vector2& p_lhs, real p_rhs)
{
	return Vector2(p_lhs.x * p_rhs, p_lhs.y * p_rhs);
}


inline Vector2 operator*(real p_lhs, const Vector2& p_rhs)
{
	return p_rhs * p_lhs;
}


inline Vector2 operator/(const Vector2& p_lhs, real p_rhs)
{
	Vector2 result(p_lhs);
	return (result /= p_rhs);
}


inline bool operator==(const Vector2& p_lhs, const Vector2& p_rhs)
{
	return (p_lhs.x == p_rhs.x && p_lhs.y == p_rhs.y);
}


inline bool operator!=(const Vector2& p_lhs, const Vector2& p_rhs)
{
	return (p_lhs == p_rhs) == false;
}


bool parseVector2(const xml::XmlNode* p_node, Vector2* p_result);


// Namespace end
}
}


#endif  // INC_TT_MATH_VECTOR2_H
