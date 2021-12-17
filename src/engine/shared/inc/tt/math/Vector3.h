#if !defined(INC_TT_MATH_VECTOR3_H)
#define INC_TT_MATH_VECTOR3_H


#include <ostream>

#include <tt/platform/tt_types.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>
#include <tt/math/Vector2.h>
#include <tt/math/math.h>
#include <tt/streams/BOStream.h>
#include <tt/streams/BIStream.h>
#include <tt/fs/File.h>


namespace tt
{
	namespace math
	{
		class Point3;
	}
	namespace xml
	{
		class XmlNode;
	}
}


namespace tt {
namespace math {


class Vector3
{
public:
	typedef real ValueType;
	
	// Note JL: explicit is useless here... TODO: create a Vector3 constructor without initialization
	explicit Vector3(real p_x = 0.0f, real p_y = 0.0f, real p_z = 0.0f)
	:
	x(p_x),
	y(p_y),
	z(p_z)
	{ }
	
	explicit Vector3(const Point3& p_point);

	explicit Vector3(const Vector2& p_vector, real p_z = 0.0f)
	:
	x(p_vector.x),
	y(p_vector.y),
	z(p_z)
	{ }
	
	Vector3 (const Vector3& p_rhs)
	:
	x(p_rhs.x),
	y(p_rhs.y),
	z(p_rhs.z)
	{ }
	
	~Vector3() {}
	
	inline Vector2 xy(                   ) const { return Vector2(x,y);    }
	inline void    xy(const Vector2& p_xy)       { x = p_xy.x; y = p_xy.y; }
	
	inline void setValues(real p_x, real p_y, real p_z)
	{
		x = p_x;
		y = p_y;
		z = p_z;
	}
	
	// Assignment Operators
	inline Vector3& operator = (const Vector3& p_rhs)
	{
		x = p_rhs.x; 
		y = p_rhs.y; 
		z = p_rhs.z;
		return *this;
	}
	
	inline Vector3 operator-() const
	{
		return Vector3(-x, -y, -z);
	}
	
	inline Vector3& operator += (const Vector3& p_rhs)
	{
		x += p_rhs.x;
		y += p_rhs.y;
		z += p_rhs.z;
		return *this;
	}
	
	inline Vector3& operator -= (const Vector3& p_rhs)
	{
		x -= p_rhs.x;
		y -= p_rhs.y;
		z -= p_rhs.z;
		return *this;
	}
	
	template<typename T>
	inline Vector3& operator *= (const T p_rhs)
	{
		x *= p_rhs; 
		y *= p_rhs; 
		z *= p_rhs;
		return *this;
	}
	
	template<typename T>
	inline Vector3& operator /= (const T p_rhs)
	{
		TT_ASSERTMSG(p_rhs != 0.0f, "Division by zero is undefined");
		
		real inv = 1.0f / p_rhs;
		x *= inv; 
		y *= inv; 
		z *= inv;
	
		return *this;
	}
	
	// Normalization
	inline Vector3& normalize()
	{
		real len = length();
		
		if (len > 0.0f &&
		    realEqual(len, 1.0f) == false)
		{
			real inv_len = 1.0f / (len + floatTolerance);
			
			x *= inv_len;
			y *= inv_len;
			z *= inv_len;
		}
		return *this;
	}

	inline Vector3& normalize64()
	{
		real64 len = lengthSquared64();

		real64 hx = static_cast<real64>(x*x) / len;
		real64 hy = static_cast<real64>(y*y) / len;
		real64 hz = static_cast<real64>(z*z) / len;

		x = static_cast<real>(math::sqrt(hx));
		y = static_cast<real>(math::sqrt(hy));
		z = static_cast<real>(math::sqrt(hz));

		return *this;
	}
	
	inline real lengthSquared() const
	{
		return x*x + y*y + z*z;
	}
	
	inline real64 lengthSquared64() const
	{
		real64 hx = x;
		real64 hy = y;
		real64 hz = z;
		
		return hx*hx + hy*hy + hz*hz;
	}
	
	// Vector length
	inline real length() const
	{
		return math::sqrt(lengthSquared());
	}

	inline bool load(const fs::FilePtr& p_file)
	{
		// Attempt to load the vector
		if (p_file->read(&x, sizeof(x)) != sizeof(x))
		{
			return false;
		}
		
		if (p_file->read(&y, sizeof(y)) != sizeof(y))
		{
			return false;
		}

		if (p_file->read(&z, sizeof(z)) != sizeof(z))
		{
			return false;
		}
		return true;
	}
	
	inline real& operator[](s32 p_index)
	{
		switch(p_index)
		{
		case 0:
			return x;
		case 1:
			return y;
		case 2:
			return z;
		}
		
		TT_PANIC("Out of bounds");
		return z;
	}
	
	inline const real& operator[](s32 p_index) const
	{
		switch(p_index)
		{
		case 0:
			return x;
		case 1:
			return y;
		case 2:
			return z;
		}
		
		TT_PANIC("Out of bounds");
		return z;
	}

	inline void print() const
	{
		TT_Printf("<%f,%f,%f>", x, y, z);
	}
	
	// Constants
	static const Vector3 zero;
	static const Vector3 unitX;
	static const Vector3 unitY;
	static const Vector3 unitZ;
	static const Vector3 up;
	static const Vector3 down;
	static const Vector3 left;
	static const Vector3 right;
	static const Vector3 backward;
	static const Vector3 forward;
	static const Vector3 allOne;
	
public:
	real x, y, z;
};


struct Vector3Less
{
	// Types expected by standard library functions / containers
	typedef Vector3 first_argument_type;
	typedef Vector3 second_argument_type;
	typedef bool    result_type;
	
	inline bool operator()(const tt::math::Vector3& p_a, const tt::math::Vector3& p_b) const
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


struct Vector3Greater
{
	// Types expected by standard library functions / containers
	typedef Vector3 first_argument_type;
	typedef Vector3 second_argument_type;
	typedef bool    result_type;
	
	inline bool operator()(const tt::math::Vector3& p_a, const tt::math::Vector3& p_b) const
	{
		if (p_a.x != p_b.x)
		{
			return p_a.x > p_b.x;
		}
		if (p_a.y != p_b.y)
		{
			return p_a.y > p_b.y;
		}
		return p_a.z > p_b.z;
	}
};


inline Vector3 crossProduct (const Vector3& p_lhs, const Vector3& p_rhs)
{
	Vector3 cross;
	
	cross.x = p_lhs.y * p_rhs.z - p_lhs.z * p_rhs.y;
	cross.y = p_lhs.z * p_rhs.x - p_lhs.x * p_rhs.z;
	cross.z = p_lhs.x * p_rhs.y - p_lhs.y * p_rhs.x;
	
	return cross;
}


inline real dotProduct (const Vector3& p_lhs, const Vector3& p_rhs)
{
	return p_lhs.x * p_rhs.x + p_lhs.y * p_rhs.y + p_lhs.z * p_rhs.z;
}

inline real distanceSquared(const Vector3& p_v1, const Vector3& p_v2)
{
	return (p_v2.x - p_v1.x) * (p_v2.x - p_v1.x) + 
	       (p_v2.y - p_v1.y) * (p_v2.y - p_v1.y) +
	       (p_v2.z - p_v1.z) * (p_v2.z - p_v1.z);
}

inline real distance(const Vector3& p_v1, const Vector3& p_v2)
{
	return math::sqrt(distanceSquared(p_v1, p_v2));
}

inline std::ostream& operator<<(std::ostream& s, const Vector3& v)
{
	return s << '(' << v.x << ',' << v.y << ',' << v.z << ')';
}


inline tt::streams::BOStream& operator<<(tt::streams::BOStream& s, const Vector3& v)
{
	s <<(v.x);
	s << v.y;
	s << v.z;
	return s;
}


inline tt::streams::BIStream& operator>>(tt::streams::BIStream& s, Vector3& v)
{
	s >> v.x;
	s >> v.y;
	s >> v.z;
	return s;
}


inline Vector3 operator+(const Vector3& p_lhs, const Vector3& p_rhs)
{
	return Vector3(p_lhs.x + p_rhs.x, 
	                         p_lhs.y + p_rhs.y, 
	                         p_lhs.z + p_rhs.z);
}


inline Vector3 operator-(const Vector3& p_lhs, const Vector3& p_rhs)
{
	return Vector3(p_lhs.x - p_rhs.x, 
	                         p_lhs.y - p_rhs.y, 
	                         p_lhs.z - p_rhs.z);
}


inline Vector3 operator*(const Vector3& p_lhs, real p_rhs)
{
	return Vector3(p_lhs.x * p_rhs, 
	                         p_lhs.y * p_rhs, 
	                         p_lhs.z * p_rhs);
}


inline Vector3 operator*(real p_lhs, const Vector3& p_rhs)
{
	return p_rhs * p_lhs;
}


inline Vector3 operator/(const Vector3& p_lhs, real p_rhs)
{
	Vector3 result(p_lhs);
	return (result /= p_rhs);
}

inline bool operator==(const Vector3& p_lhs, const Vector3& p_rhs)
{
	return (p_lhs.x == p_rhs.x && p_lhs.y == p_rhs.y && p_lhs.z == p_rhs.z);
}

inline bool operator!=(const Vector3& p_lhs, const Vector3& p_rhs)
{
	return (p_lhs == p_rhs) == false;
}

bool parseVector3(const xml::XmlNode* p_node, Vector3* p_result);


// Namespace end
}
}


#endif  // INC_TT_MATH_VECTOR3_H


