#if !defined(TT_MATH_POINT3_H)
#define TT_MATH_POINT3_H


#include <ostream>

#include <tt/platform/tt_types.h>
#include <tt/platform/tt_error.h>
#include <tt/math/math.h>
#include <tt/math/Vector3.h>
#include <tt/streams/BOStream.h>
#include <tt/streams/BIStream.h>
#include <tt/xml/fwd.h>


namespace tt {
namespace math {

class Point3
{
public:
	typedef s32 ValueType;
	
	explicit Point3(s32 p_x = 0, s32 p_y = 0, s32 p_z = 0)
	:
	x(p_x),
	y(p_y),
	z(p_z)
	{}

	explicit Point3(const Vector3& p_vector)
	:
	x(static_cast<s32>(p_vector.x)),
	y(static_cast<s32>(p_vector.y)),
	z(static_cast<s32>(p_vector.z))
	{}
	
	Point3(const Point3& p_rhs)
	:
	x(p_rhs.x),
	y(p_rhs.y),
	z(p_rhs.z)
	{
	}
	
	inline void setValues(s32 p_x, s32 p_y, s32 p_z)
	{
		x = p_x;
		y = p_y;
		z = p_z;
	}
	
	// Assignment Operators
	inline Point3& operator=(const Point3& p_rhs)
	{
		x = p_rhs.x;
		y = p_rhs.y;
		z = p_rhs.z;
		return *this;
	}
	
	inline Point3 operator-() const
	{
		return Point3(-x, -y, -z);
	}
	
	inline Point3& operator+=(const Point3& p_rhs)
	{
		x += p_rhs.x;
		y += p_rhs.y;
		z += p_rhs.z;
		return *this;
	}
	
	inline Point3& operator-=(const Point3& p_rhs)
	{
		x -= p_rhs.x;
		y -= p_rhs.y;
		z -= p_rhs.z;
		return *this;
	}
	
	template<typename T>
	inline Point3& operator*=(const T p_rhs)
	{
		x *= p_rhs; 
		y *= p_rhs; 
		z *= p_rhs;
		return *this;
	}
	
	template<typename T>
	inline Point3& operator/=(const T p_rhs)
	{
		TT_ASSERTMSG(p_rhs != 0, "Division by zero is undefined.");
		
		x /= p_rhs;
		y /= p_rhs;
		z /= p_rhs;
		
		return *this;
	}
	
	
	s32 x;
	s32 y;
	s32 z;
	
	
	// Constants
	static const Point3 zero;
	static const Point3 unitX;
	static const Point3 unitY;
	static const Point3 unitZ;
	static const Point3 up;
	static const Point3 down;
	static const Point3 left;
	static const Point3 right;
	static const Point3 backward;
	static const Point3 forward;
	static const Point3 allOne;
};


struct Point3Less
{
	// Types expected by standard library functions / containers
	typedef Point3 first_argument_type;
	typedef Point3 second_argument_type;
	typedef bool   result_type;
	
	inline bool operator()(const tt::math::Point3& p_a,
	                       const tt::math::Point3& p_b) const
	{
		if (p_a.x != p_b.x)
		{
			return p_a.x < p_b.x;
		}
		if (p_a.y != p_b.y)
		{
			return p_a.y < p_b.y;
		}
		return p_a.z < p_b.z;
	}
};



inline s32 distanceSquared(const Point3& p_p1, const Point3& p_p2)
{
	return (p_p2.x - p_p1.x) * (p_p2.x - p_p1.x) +
	       (p_p2.y - p_p1.y) * (p_p2.y - p_p1.y) +
	       (p_p2.z - p_p1.z) * (p_p2.z - p_p1.z);
}

inline real distance(const Point3& p_p1, const Point3& p_p2)
{
	return sqrt(real(distanceSquared(p_p1, p_p2)));
}

inline std::ostream& operator<<(std::ostream& s, const Point3& v)
{
	return s << '(' << v.x << ',' << v.y << ',' << v.z << ')';
}


inline tt::streams::BOStream& operator<<(tt::streams::BOStream& s, const Point3& v)
{
	s << v.x;
	s << v.y;
	s << v.z;
	return s;
}


inline tt::streams::BIStream& operator>>(tt::streams::BIStream& s, Point3& v)
{
	s >> v.x;
	s >> v.y;
	s >> v.z;
	return s;
}


inline Point3 operator+(const Point3& p_lhs, const Point3& p_rhs)
{
	return Point3(p_lhs.x + p_rhs.x, p_lhs.y + p_rhs.y, p_lhs.z + p_rhs.z);
}


inline Point3 operator-(const Point3& p_lhs, const Point3& p_rhs)
{
	return Point3(p_lhs.x - p_rhs.x, p_lhs.y - p_rhs.y, p_lhs.z - p_rhs.z);
}


template<typename T>
inline Point3 operator*(const Point3& p_lhs, T p_rhs)
{
	return Point3(p_lhs.x * p_rhs, p_lhs.y * p_rhs, p_lhs.z * p_rhs);
}


template<typename T>
inline Point3 operator*(T p_lhs, const Point3& p_rhs)
{
	return p_rhs * p_lhs;
}


template<typename T>
inline Point3 operator/(const Point3& p_lhs, T p_rhs)
{
	Point3 result(p_lhs);
	return (result /= p_rhs);
}


inline bool operator==(const Point3& p_lhs, const Point3& p_rhs)
{
	return (p_lhs.x == p_rhs.x && p_lhs.y == p_rhs.y && p_lhs.z == p_rhs.z);
}


inline bool operator!=(const Point3& p_lhs, const Point3& p_rhs)
{
	return (p_lhs == p_rhs) == false;
}


bool parsePoint3(const xml::XmlNode* p_node, Point3* p_result);

// Namespace end
}
}


#endif  // !defined(TT_MATH_POINT3_H)
