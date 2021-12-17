#if !defined(INC_TT_MATH_MATRIX22_H)
#define INC_TT_MATH_MATRIX22_H


#include <algorithm>
#include <ostream>
#include <cstring>

#include <tt/platform/tt_types.h>
#include <tt/platform/tt_error.h>
#include <tt/math/Vector2.h>
#include <tt/math/math.h>
#include <tt/streams/BOStream.h>
#include <tt/streams/BIStream.h>


namespace tt {
namespace math {

class Matrix22;
const Matrix22 operator*(const Matrix22& p_lhs, const Matrix22& p_rhs);
bool operator==(const Matrix22& p_lhs, const Matrix22& p_rhs);

/*! \brief 2x2 Matrix Class: Row-Major Order, Multiplication with column vector on the right */
class Matrix22
{
public:
	Matrix22()
	:
	m_11(1), m_12(0),
	m_21(0), m_22(1)
	{
	}

	Matrix22(real p_11, real p_12,
			 real p_21, real p_22)
	:
	m_11(p_11),	m_12(p_12),
	m_21(p_21),	m_22(p_22)
	{
	}

	Matrix22(const Matrix22& p_rhs)
	:
	m_11(p_rhs.m_11), m_12(p_rhs.m_12),
	m_21(p_rhs.m_21), m_22(p_rhs.m_22)
	{
	}

	~Matrix22() {}

	inline Matrix22& operator = (const Matrix22& p_rhs)
	{
		m_11 = p_rhs.m_11;
		m_12 = p_rhs.m_12;
		m_21 = p_rhs.m_21;
		m_22 = p_rhs.m_22;
		return *this;
	}

	inline Matrix22& operator += (const Matrix22& p_rhs)
	{
		m_11 += p_rhs.m_11;
		m_12 += p_rhs.m_12;
		m_21 += p_rhs.m_21;
		m_22 += p_rhs.m_22;
		return *this;
	}

	inline Matrix22& operator -= (const Matrix22& p_rhs)
	{
		m_11 -= p_rhs.m_11;
		m_12 -= p_rhs.m_12;
		m_21 -= p_rhs.m_21;
		m_22 -= p_rhs.m_22;
		return *this;
	}

	inline Matrix22& operator *= (const Matrix22& p_rhs)
	{
		*this = (*this) * p_rhs;
		
		return *this;
	}

	inline Matrix22& operator *= (const real p_rhs)
	{
		m_11 *= p_rhs;
		m_12 *= p_rhs;
		m_21 *= p_rhs;
		m_22 *= p_rhs;
		return *this;
	}

	inline Matrix22& operator /= (const real p_rhs)
	{
		TT_ASSERTMSG(p_rhs != 0, "Division by zero.");

		real inv = 1.0f / p_rhs;
		m_11 *= inv;
		m_12 *= inv;
		m_21 *= inv;
		m_22 *= inv;
		return *this;
	}
	
	inline Matrix22 operator-() const
	{
		return Matrix22(-m_11, -m_12, -m_21, -m_22);
	}
	
	///////////////////////////////////////////////////////////////////////////
	// Modifiers
	///////////////////////////////////////////////////////////////////////////
	inline Matrix22& clear()
	{
		m_11 = m_12 = m_21 = m_22 = 0;
		return *this;
	}

	inline Matrix22& setIdentity()
	{
		m_11 = m_22 = 1;
		m_12 = m_21 = 0;
		return *this;
	}

	inline Matrix22& transpose()
	{
		using std::swap;
		swap(m_12, m_21);
		return *this;
	}

	inline Matrix22& inverse()
	{
		real inv_determinant = 1.0f / getDeterminant();
		transpose();
		
		m_11 *=  inv_determinant;
		m_12 *= -inv_determinant;
		m_21 *= -inv_determinant;
		m_22 *=  inv_determinant;

		return (*this);
	}

	inline Matrix22& rotate(real p_angle_rad)
	{
		*this = getRotation(p_angle_rad) * (*this);
		return *this;
	}

	inline Matrix22& scale(real p_x, real p_y)
	{
		*this = getScale(p_x, p_y) * (*this);
		return *this;
	}

	inline Matrix22& shearX(real p_shear)
	{
		*this = getShearX(p_shear) * (*this);
		return *this;
	}

	inline Matrix22& shearY(real p_shear)
	{
		*this = getShearY(p_shear) * (*this);
		return *this;
	}

	///////////////////////////////////////////////////////////////////////////
	// Getters
	///////////////////////////////////////////////////////////////////////////
	inline real getDeterminant() const
	{
		return m_11 * m_22 - m_12 * m_21;
	}

	static inline Matrix22 getRotation(real p_angle_rad)
	{
		return Matrix22( math::cos(p_angle_rad), math::sin(p_angle_rad),
						-math::sin(p_angle_rad), math::cos(p_angle_rad));
	}

	static inline Matrix22 getScale(real p_x, real p_y)
	{
		return Matrix22(p_x, 0, 0, p_y);
	}

	static inline Matrix22 getShearX(real p_shear)
	{
		return Matrix22(1, 0, p_shear, 1);
	}

	static inline Matrix22 getShearY(real p_shear)
	{
		return Matrix22(1, p_shear, 0, 1);
	}

	inline bool isIdentity() const
	{
		return (*this) == Matrix22::identity;
	}

	///////////////////////////////////////////////////////////////////////////
	// Constants
	///////////////////////////////////////////////////////////////////////////
	static const Matrix22 zero;
	static const Matrix22 identity;

	// Make these public for optimization purposes
	real m_11, m_12; 
	real m_21, m_22;
};


inline std::ostream& operator<<(std::ostream& s, const Matrix22& m)
{
	return s << std::endl << '(' << m.m_11 << ' ' << m.m_12 << ')' << 
	            std::endl << '(' << m.m_21 << ' ' << m.m_22 << ')';
}


inline tt::streams::BOStream& operator<<(tt::streams::BOStream& s, const Matrix22& m)
{
	s << m.m_11;
	s << m.m_12;
	s << m.m_21;
	s << m.m_22;
	return s;
}


inline tt::streams::BIStream& operator>>(tt::streams::BIStream& s, Matrix22& m)
{
	s >> m.m_11;
	s >> m.m_12;
	s >> m.m_21;
	s >> m.m_22;
	return s;
}


// Multiplication with a scalar
inline const Matrix22 operator*(const real p_lhs, const Matrix22& p_rhs)
{
	return Matrix22(p_lhs * p_rhs.m_11, p_lhs * p_rhs.m_12, 
	                p_lhs * p_rhs.m_21, p_lhs * p_rhs.m_22);
}

inline const Matrix22 operator*(const Matrix22& p_lhs, const real p_rhs)
{
	return p_rhs * p_lhs;
}

inline const Matrix22 operator/(const Matrix22& p_lhs, const real p_rhs)
{
	TT_ASSERTMSG(p_rhs != 0, "Division by zero.");
	return (1.0f / p_rhs) * p_lhs;
}

// Matrix-Matrix Multiplication
inline const Matrix22 operator*(const Matrix22& p_lhs, const Matrix22& p_rhs)
{
	return Matrix22(p_lhs.m_11 * p_rhs.m_11 + p_lhs.m_12 * p_rhs.m_21, 
	                          p_lhs.m_11 * p_rhs.m_12 + p_lhs.m_12 * p_rhs.m_22,
	                          p_lhs.m_21 * p_rhs.m_11 + p_lhs.m_22 * p_rhs.m_21,
	                          p_lhs.m_21 * p_rhs.m_12 + p_lhs.m_22 * p_rhs.m_22);
}

// Matrix-Vector Multiplication
inline const Vector2 operator*(const Vector2& p_lhs, const Matrix22& p_rhs)
{
	return Vector2(p_lhs.x * p_rhs.m_11 + p_lhs.y * p_rhs.m_21,
							 p_lhs.x * p_rhs.m_12 + p_lhs.y * p_rhs.m_22);
}

// Matrix Addition
inline const Matrix22 operator+(const Matrix22& p_lhs, const Matrix22& p_rhs)
{
	return Matrix22(p_lhs.m_11 + p_rhs.m_11, p_lhs.m_12 + p_rhs.m_12,
	                          p_lhs.m_21 + p_rhs.m_21, p_lhs.m_22 + p_rhs.m_22);
}

// Matrix Subtraction
inline const Matrix22 operator-(const Matrix22& p_lhs, const Matrix22& p_rhs)
{
	return Matrix22(p_lhs.m_11 - p_rhs.m_11, p_lhs.m_12 - p_rhs.m_12,
	                          p_lhs.m_21 - p_rhs.m_21, p_lhs.m_22 - p_rhs.m_22);
}

inline bool operator==(const Matrix22& p_lhs, const Matrix22& p_rhs)
{
	// TODO: verify that this works correctly
	return std::memcmp(&p_lhs, &p_rhs, sizeof(Matrix22)) == 0;
}

inline bool operator!=(const Matrix22& p_lhs, const Matrix22& p_rhs)
{
	return (p_lhs == p_rhs) == false;
}

// Namespace end
}
}


#endif  // INC_TT_MATH_MATRIX22_H
