#if !defined(INC_TT_MATH_VECTOR4_H)
#define INC_TT_MATH_VECTOR4_H


#include <ostream>

#include <tt/platform/tt_types.h>
#include <tt/platform/tt_error.h>
#include <tt/streams/BOStream.h>
#include <tt/streams/BIStream.h>
#include <tt/math/math.h>
#include <tt/math/Vector3.h>


namespace tt {
namespace math {

class Vector4
{
public:
	// Default constructor
	// Note JL: explicit is useless here... TODO: create a Vector4 constructor without initialization
	inline explicit Vector4(real p_x = 0, real p_y = 0, real p_z = 0, real p_w = 1)
	:
	x(p_x),
	y(p_y),
	z(p_z),
	w(p_w)
	{}
	
	inline explicit Vector4(const Vector3& p_vector3, real p_w = 1.0f)
	:
	x(p_vector3.x),
	y(p_vector3.y),
	z(p_vector3.z),
	w(p_w)
	{}
	
	inline void setValues(real p_x, real p_y, real p_z, real p_w)
	{
		x = p_x;
		y = p_y;
		z = p_z;
		w = p_w;
	}
	
	inline Vector4 operator-() const
	{
		return Vector4(-x, -y, -z, -w);
	}
	
	inline Vector4& operator += (const Vector4& p_rhs)
	{
		x += p_rhs.x;
		y += p_rhs.y;
		z += p_rhs.z;
		w += p_rhs.w;
		return *this;
	}
	
	inline Vector4& operator -= (const Vector4& p_rhs)
	{
		x -= p_rhs.x;
		y -= p_rhs.y;
		z -= p_rhs.z;
		w -= p_rhs.w;
		return *this;
	}
	
	template<typename T>
	inline Vector4& operator *= (const T p_rhs)
	{
		x *= p_rhs; 
		y *= p_rhs; 
		z *= p_rhs;
		w *= p_rhs;
		return *this;
	}
	
	template<typename T>
	inline Vector4& operator /= (const T p_rhs)
	{
		TT_ASSERTMSG(p_rhs != 0, "Division by zero is undefined");
		
		real inv = 1.0f / p_rhs;
		x *= inv; 
		y *= inv; 
		z *= inv;
		w *= inv;
		
		return *this;
	}
	
	// Normalization
	inline Vector4& normalize ()
	{
		real len = length();

		if(len > 0)
		{
			real inv_len = 1.0f / len;
		
			x *= inv_len;
			y *= inv_len;
			z *= inv_len;
			w *= inv_len;
		}
		return *this;
	}
	
	// Vector length
	inline real length() const
	{
		return math::sqrt(x*x + y*y + z*z + w*w);
	}

	// Constants
	static const Vector4 zero;
	static const Vector4 unitX;
	static const Vector4 unitY;
	static const Vector4 unitZ;
	static const Vector4 up;
	static const Vector4 down;
	static const Vector4 left;
	static const Vector4 right;
	static const Vector4 backward;
	static const Vector4 forward;

public:
	real x, y, z, w;
};

// NOTE: fourth component is ignored
inline Vector4 crossProduct (const Vector4& p_lhs, const Vector4& p_rhs)
{
	Vector4 cross;

	cross.x = p_lhs.y * p_rhs.z - p_lhs.z * p_rhs.y;
	cross.y = p_lhs.z * p_rhs.x - p_lhs.x * p_rhs.z;
	cross.z = p_lhs.x * p_rhs.y - p_lhs.y * p_rhs.x;
	cross.w = 1;

	return cross;
}


inline real dotProduct (const Vector4& p_lhs, const Vector4& p_rhs)
{
	return p_lhs.x * p_rhs.x + p_lhs.y * p_rhs.y + 
		   p_lhs.z * p_rhs.z + p_lhs.w * p_rhs.w;
}


inline real distanceSquared(const Vector4& p_v1, const Vector4& p_v2)
{
	return (p_v2.x - p_v1.x) * (p_v2.x - p_v1.x) + 
		   (p_v2.y - p_v1.y) * (p_v2.y - p_v1.y) +
		   (p_v2.z - p_v1.z) * (p_v2.z - p_v1.z) +
		   (p_v2.w - p_v1.w) * (p_v2.w - p_v1.w);
}

inline real distance(const Vector4& p_v1, const Vector4& p_v2)
{
	return math::sqrt(distanceSquared(p_v1, p_v2));
}


inline std::ostream& operator<<(std::ostream& s, const Vector4& v)
{
	return s << '(' << v.x << ',' << v.y << ',' << v.z << ',' << v.w << ')';
}


inline tt::streams::BOStream& operator<<(tt::streams::BOStream& s, const Vector4& v)
{
	s << v.x;
	s << v.y;
	s << v.z;
	s << v.w;
	return s;
}


inline tt::streams::BIStream& operator>>(tt::streams::BIStream& s, Vector4& v)
{
	s >> v.x;
	s >> v.y;
	s >> v.z;
	s >> v.w;
	return s;
}


inline Vector4 operator+(const Vector4& p_lhs, const Vector4& p_rhs)
{
	return Vector4(p_lhs.x + p_rhs.x, p_lhs.y + p_rhs.y, 
	               p_lhs.z + p_rhs.z, p_lhs.w + p_rhs.w);
}


inline Vector4 operator-(const Vector4& p_lhs, const Vector4& p_rhs)
{
	return Vector4(p_lhs.x - p_rhs.x, p_lhs.y - p_rhs.y, 
	               p_lhs.z - p_rhs.z, p_lhs.w - p_rhs.w);
}


inline Vector4 operator*(const Vector4& p_lhs, real p_rhs)
{
	return Vector4(p_lhs.x * p_rhs, p_lhs.y * p_rhs, 
	               p_lhs.z * p_rhs, p_lhs.w * p_rhs);
}


inline Vector4 operator*(real p_lhs, const Vector4& p_rhs)
{
	return p_rhs * p_lhs;
}


inline Vector4 operator/(const Vector4& p_lhs, real p_rhs)
{
	Vector4 result(p_lhs);
	return (result /= p_rhs);
}

inline bool operator==(const Vector4& p_lhs, const Vector4& p_rhs)
{
	return (p_lhs.x == p_rhs.x && p_lhs.y == p_rhs.y && 
			p_lhs.z == p_rhs.z && p_lhs.w == p_rhs.w);
}

inline bool operator!=(const Vector4& p_lhs, const Vector4& p_rhs)
{
	return (p_lhs == p_rhs) == false;
}


// Namespace end
}
}


#endif  // TT_MATH_VECTOR4_H

