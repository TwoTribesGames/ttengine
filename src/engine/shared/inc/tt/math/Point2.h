#if !defined(TT_MATH_POINT2_H)
#define TT_MATH_POINT2_H


#include <ostream>
#include <algorithm>

#include <tt/platform/tt_types.h>
#include <tt/platform/tt_error.h>
#include <tt/streams/BOStream.h>
#include <tt/streams/BIStream.h>
#include <tt/math/math.h>
#include <tt/math/Vector2.h>
#include <tt/xml/fwd.h>


namespace tt {
namespace math {

class Point2
{
public:
	typedef s32 ValueType;
	
	inline Point2()
	:
	x(0),
	y(0)
	{}
	
	inline Point2(s32 p_x, s32 p_y)
	:
	x(p_x),
	y(p_y)
	{}
	
	explicit Point2(const Vector2& p_vector)
	:
	x(static_cast<s32>(p_vector.x)),
	y(static_cast<s32>(p_vector.y))
	{}
	
	inline void setValues(s32 p_x, s32 p_y)
	{
		x = p_x;
		y = p_y;
	}
	
	inline Point2 operator-() const
	{
		return Point2(-x, -y);
	}
	
	inline Point2& operator+=(const Point2& p_rhs)
	{
		x += p_rhs.x;
		y += p_rhs.y;
		return *this;
	}
	
	inline Point2& operator-=(const Point2& p_rhs)
	{
		x -= p_rhs.x;
		y -= p_rhs.y;
		return *this;
	}
	
	template<typename T>
	inline Point2& operator*=(const T p_rhs)
	{
		x *= p_rhs;
		y *= p_rhs;
		return *this;
	}
	
	template<typename T>
	inline Point2& operator/=(const T p_rhs)
	{
		TT_ASSERTMSG(p_rhs != 0, "Division by zero is undefined");
		
		real inv = 1.0f / p_rhs;
		x *= inv;
		y *= inv;
		
		return *this;
	}
	
	s32 x;
	s32 y;
	
	
	// Constants
	static const Point2 zero;
	static const Point2 unitX;
	static const Point2 unitY;
	static const Point2 up;
	static const Point2 down;
	static const Point2 left;
	static const Point2 right;
	static const Point2 allOne;
};


struct Point2Less
{
	// Types expected by standard library functions / containers
	typedef Point2 first_argument_type;
	typedef Point2 second_argument_type;
	typedef bool   result_type;
	
	inline bool operator()(const tt::math::Point2& p_a,
	                       const tt::math::Point2& p_b) const
	{
		if (p_a.x != p_b.x)
		{
			return p_a.x < p_b.x;
		}
		return p_a.y < p_b.y;
	}
};


struct Point2LessYThenLessX
{
	// Types expected by standard library functions / containers
	typedef Point2 first_argument_type;
	typedef Point2 second_argument_type;
	typedef bool   result_type;
	
	inline bool operator()(const tt::math::Point2& p_a,
	                       const tt::math::Point2& p_b) const
	{
		if (p_a.y != p_b.y)
		{
			return p_a.y < p_b.y;
		}
		return p_a.x < p_b.x;
	}
};

inline s32 distanceSquared(const Point2& p_p1, const Point2& p_p2)
{
	return (p_p2.x - p_p1.x) * (p_p2.x - p_p1.x) +
		   (p_p2.y - p_p1.y) * (p_p2.y - p_p1.y);
}


inline real distance(const Point2& p_p1, const Point2& p_p2)
{
	return sqrt(real(distanceSquared(p_p1, p_p2)));
}


inline std::ostream& operator<<(std::ostream& s, const Point2& v)
{
	return s << '(' << v.x << ',' << v.y << ')';
}


inline tt::streams::BOStream& operator<<(tt::streams::BOStream& s, const Point2& v)
{
	s << v.x;
	s << v.y;
	return s;
}


inline tt::streams::BIStream& operator>>(tt::streams::BIStream& s, Point2& v)
{
	s >> v.x;
	s >> v.y;
	return s;
}


inline Point2 operator+(const Point2& p_lhs, const Point2& p_rhs)
{
	return Point2(p_lhs.x + p_rhs.x, p_lhs.y + p_rhs.y);
}


inline Point2 operator-(const Point2& p_lhs, const Point2& p_rhs)
{
	return Point2(p_lhs.x - p_rhs.x, p_lhs.y - p_rhs.y);
}


template<typename T>
inline Point2 operator*(const Point2& p_lhs, T p_rhs)
{
	return Point2(p_lhs.x * p_rhs, p_lhs.y * p_rhs);
}

template<typename T>
inline Point2 operator*(T p_lhs, const Point2& p_rhs)
{
	return p_rhs * p_lhs;
}

template<typename T>
inline Point2 operator/(const Point2& p_lhs, T p_rhs)
{
	Point2 result(p_lhs);
	return (result /= p_rhs);
}


inline bool operator==(const Point2& p_lhs, const Point2& p_rhs)
{
	return (p_lhs.x == p_rhs.x && p_lhs.y == p_rhs.y);
}


inline bool operator!=(const Point2& p_lhs, const Point2& p_rhs)
{
	return (p_lhs == p_rhs) == false;
}

inline Point2 pointMin(const tt::math::Point2& p_first, const tt::math::Point2& p_second)
{
	return Point2(std::min(p_first.x, p_second.x), std::min(p_first.y, p_second.y));
}


inline Point2 pointMax(const tt::math::Point2& p_first, const tt::math::Point2& p_second)
{
	return Point2(std::max(p_first.x, p_second.x), std::max(p_first.y, p_second.y));
}


bool parsePoint2(const xml::XmlNode* p_node, Point2* p_result);


// Namespace end
}
}


#endif  // !defined(TT_MATH_POINT2_H)
