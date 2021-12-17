#if !defined(INC_TT_MATH_MATRIX33_H)
#define INC_TT_MATH_MATRIX33_H

#include <ostream>
#include <cstring>

#include <tt/platform/tt_types.h>
#include <tt/platform/tt_error.h>
#include <tt/math/Vector3.h>
#include <tt/math/Vector2.h>
#include <tt/math/math.h>
#include <tt/streams/BOStream.h>
#include <tt/streams/BIStream.h>



namespace tt {
namespace math {

class Matrix33;
const Matrix33 operator*(const Matrix33& p_lhs, const Matrix33& p_rhs);
bool operator==(const Matrix33& p_lhs, const Matrix33& p_rhs);

/*! \brief 3x3 Matrix Class: Row-Major Order, Multiplication with column vector on the right */
class Matrix33
{
public:
	Matrix33()
	:
	m_11(1), m_12(0), m_13(0),
	m_21(0), m_22(1), m_23(0),
	m_31(0), m_32(0), m_33(1)
	{
	}

	Matrix33(real p_11, real p_12, real p_13,
			 real p_21, real p_22, real p_23,
			 real p_31, real p_32, real p_33)
	:
	m_11(p_11),	m_12(p_12), m_13(p_13),
	m_21(p_21),	m_22(p_22), m_23(p_23),
	m_31(p_31), m_32(p_32), m_33(p_33)
	{
	}

	Matrix33(const Matrix33& p_rhs)
	:
	m_11(p_rhs.m_11), m_12(p_rhs.m_12), m_13(p_rhs.m_13),
	m_21(p_rhs.m_21), m_22(p_rhs.m_22), m_23(p_rhs.m_23),
	m_31(p_rhs.m_31), m_32(p_rhs.m_32), m_33(p_rhs.m_33)
	{
	}

	~Matrix33() {}

	inline Matrix33& operator = (const Matrix33& p_rhs)
	{
		m_11 = p_rhs.m_11; m_12 = p_rhs.m_12; m_13 = p_rhs.m_13;
		m_21 = p_rhs.m_21; m_22 = p_rhs.m_22; m_23 = p_rhs.m_23;
		m_31 = p_rhs.m_31; m_32 = p_rhs.m_32; m_33 = p_rhs.m_33;
		return *this;
	}

	inline Matrix33& operator += (const Matrix33& p_rhs)
	{
		m_11 += p_rhs.m_11; m_12 += p_rhs.m_12; m_13 += p_rhs.m_13;
		m_21 += p_rhs.m_21; m_22 += p_rhs.m_22; m_23 += p_rhs.m_23;
		m_31 += p_rhs.m_31; m_32 += p_rhs.m_32; m_33 += p_rhs.m_33;
		return *this;
	}

	inline Matrix33& operator -= (const Matrix33& p_rhs)
	{
		m_11 -= p_rhs.m_11; m_12 -= p_rhs.m_12; m_13 -= p_rhs.m_13;
		m_21 -= p_rhs.m_21; m_22 -= p_rhs.m_22; m_23 -= p_rhs.m_23;
		m_31 -= p_rhs.m_31; m_32 -= p_rhs.m_32; m_33 -= p_rhs.m_33;
		return *this;
	}

	inline Matrix33& operator *= (const Matrix33& p_rhs)
	{
		*this = (*this) * p_rhs;
		
		return *this;
	}

	inline Matrix33& operator *= (const real p_rhs)
	{
		m_11 *= p_rhs; m_12 *= p_rhs; m_13 *= p_rhs;
		m_21 *= p_rhs; m_22 *= p_rhs; m_23 *= p_rhs;
		m_31 *= p_rhs; m_32 *= p_rhs; m_33 *= p_rhs;
		return *this;
	}

	inline Matrix33& operator /= (const real p_rhs)
	{
		TT_ASSERTMSG(p_rhs != 0, "Division by zero.");

		real inv = 1.0f / p_rhs;

		m_11 *= inv; m_12 *= inv; m_13 *= inv;
		m_21 *= inv; m_22 *= inv; m_23 *= inv;
		m_31 *= inv; m_32 *= inv; m_33 *= inv;
		return *this;
	}

	inline Matrix33 operator-() const
	{
		return Matrix33(-m_11, -m_12, -m_13,
		                -m_21, -m_22, -m_23,
		                -m_31, -m_32, -m_33);
	}

	///////////////////////////////////////////////////////////////////////////
	// Modifiers
	///////////////////////////////////////////////////////////////////////////
	inline Matrix33& clear()
	{
		m_11 = m_12 = m_13 = 0;
		m_21 = m_22 = m_23 = 0;
		m_31 = m_32 = m_33 = 0;
		return *this;
	}

	inline Matrix33& setIdentity()
	{
		m_12 = m_13 = 0;
		m_21 = m_23 = 0;
		m_31 = m_32 = 0;
		m_11 = m_22 = m_33 = 1;
		return *this;
	}

	inline Matrix33& transpose()
	{
		std::swap(m_12, m_21);
		std::swap(m_13, m_31);
		std::swap(m_23, m_32);
		return *this;
	}

	/*inline void inverse()
	{
		real inv_determinant = 1.0f / determinant();
		transpose();
		
		m_11 *=  inv_determinant;
		m_12 *= -inv_determinant;
		m_21 *= -inv_determinant;
		m_22 *=  inv_determinant;
		return *this;
	}*/

	inline Matrix33& rotateX(real p_angleRad)
	{
		*this = getRotationX(p_angleRad) * (*this);
		return *this;
	}

	inline Matrix33& rotateY(real p_angleRad)
	{
		*this = getRotationY(p_angleRad) * (*this);
		return *this;
	}

	inline Matrix33& rotateZ(real p_angleRad)
	{
		*this = getRotationZ(p_angleRad) * (*this);
		return *this;
	}

	inline Matrix33& scale(real p_x, real p_y, real p_z)
	{
		*this = getScale(p_x, p_y, p_z) * (*this);
		return *this;
	}

	/*inline void shearXY(real p_shearS, real p_shearT)
	{
		*this = getShearXY(p_shearS, p_shearT) * (*this);
		return *this;
	}

	inline void shearXZ(real p_shearS, real p_shearT)
	{
		*this = getShearXZ(p_shearS, p_shearT) * (*this);
		return *this;
	}

	inline void shearYZ(real p_shearS, real p_shearT)
	{
		*this = getShearYZ(p_shearS, p_shearT) * (*this);
		return *this;
	}*/


	inline Matrix33& translate(real p_x, real p_y)
	{
		m_31 = m_11 * p_x + m_21 * p_y + m_31;
		m_32 = m_12 * p_x + m_22 * p_y + m_32;
		return *this;
	}

	inline Matrix33& translate(const Vector2& p_translate)
	{
		return translate(p_translate.x, p_translate.y);
	}


	///////////////////////////////////////////////////////////////////////////
	// Getters
	///////////////////////////////////////////////////////////////////////////
	inline real getDeterminant() const
	{
		return ((m_11 * m_22 * m_33) + (m_12 * m_23 * m_31) + (m_13 * m_21 * m_32)) -
			   ((m_31 * m_22 * m_13) + (m_32 * m_23 * m_11) + (m_33 * m_21 * m_12));
	}

	static inline Matrix33 getRotationX(real p_angleRad)
	{
		return Matrix33(1, 0, 0, 
						0,  math::cos(p_angleRad), math::sin(p_angleRad),
						0, -math::sin(p_angleRad), math::cos(p_angleRad));
	}

	static inline Matrix33 getRotationY(real p_angleRad)
	{
		return Matrix33(math::cos(p_angleRad), 0, -math::sin(p_angleRad),
						0, 1, 0,
						math::sin(p_angleRad), 0,  math::cos(p_angleRad));
	}

	static inline Matrix33 getRotationZ(real p_angleRad)
	{
		return Matrix33( math::cos(p_angleRad), math::sin(p_angleRad), 0,
						-math::sin(p_angleRad), math::cos(p_angleRad), 0,
						0, 0, 1);
	}

	static inline Matrix33 getScale(real p_x, real p_y, real p_z)
	{
		return Matrix33(p_x, 0, 0,
						0, p_y, 0, 
						0, 0, p_z);
	}

	static inline Matrix33 getShearXY(real p_shearS, real p_shearT)
	{
		return Matrix33(1, 0, 0,
						0, 1, 0,
						p_shearS, p_shearT, 1);
	}

	inline Matrix33 getShearXZ(real p_shearS, real p_shearT)
	{
		return Matrix33(1, 0, 0,
						p_shearS, 1, p_shearT,
						0, 0, 1);
	}

	inline Matrix33 getShearYZ(real p_shearS, real p_shearT)
	{
		return Matrix33(1, p_shearS, p_shearT,
						0, 1, 0,
						0, 0, 1);
	}
	
	static inline Matrix33 getTranslation(real p_x, real p_y)
	{
		return Matrix33(1,   0,   0,
						0,   1,   0,
						p_x, p_y, 1);
	}
	
	static inline Matrix33 getTranslation(Vector2 p_translation)
	{
		return getTranslation(p_translation.x, p_translation.y);
	}
	
	
	inline bool isIdentity() const
	{
		return (*this) == Matrix33::identity;
	}

	///////////////////////////////////////////////////////////////////////////
	// Constants
	///////////////////////////////////////////////////////////////////////////
	static const Matrix33 zero;
	static const Matrix33 identity;

	// Make these public for optimization purposes
	real m_11, m_12, m_13; 
	real m_21, m_22, m_23;
	real m_31, m_32, m_33;
};

inline std::ostream& operator<<(std::ostream& s, const Matrix33& m)
{
	return s << std::endl << '(' << m.m_11 << ' ' << m.m_12 << ' ' << m.m_13 << ')' <<
	            std::endl << '(' << m.m_21 << ' ' << m.m_22 << ' ' << m.m_23 << ')' <<
	            std::endl << '(' << m.m_31 << ' ' << m.m_32 << ' ' << m.m_33 << ')';
}


inline tt::streams::BOStream& operator<<(tt::streams::BOStream& s, const Matrix33& m)
{
	s << m.m_11;
	s << m.m_12;
	s << m.m_13;
	s << m.m_21;
	s << m.m_22;
	s << m.m_23;
	s << m.m_31;
	s << m.m_32;
	s << m.m_33;
	return s;
}


inline tt::streams::BIStream& operator>>(tt::streams::BIStream& s, Matrix33& m)
{
	s >> m.m_11;
	s >> m.m_12;
	s >> m.m_13;
	s >> m.m_21;
	s >> m.m_22;
	s >> m.m_23;
	s >> m.m_31;
	s >> m.m_32;
	s >> m.m_33;
	return s;
}


// Multiplication with a scalar
inline const Matrix33 operator*(const real p_lhs, const Matrix33& p_rhs)
{
	return Matrix33(
			p_lhs * p_rhs.m_11, p_lhs * p_rhs.m_12, p_lhs * p_rhs.m_13,
			p_lhs * p_rhs.m_21, p_lhs * p_rhs.m_22, p_lhs * p_rhs.m_23,
			p_lhs * p_rhs.m_31, p_lhs * p_rhs.m_32, p_lhs * p_rhs.m_33);
}

inline const Matrix33 operator*(const Matrix33& p_lhs, const real p_rhs)
{
	return p_rhs * p_lhs;
}

inline const Matrix33 operator/(const Matrix33& p_lhs, const real p_rhs)
{
	TT_ASSERTMSG(p_rhs != 0, "Division by zero.");
	return (1.0f / p_rhs) * p_lhs;
}

// Matrix-Matrix Multiplication
inline const Matrix33 operator*(const Matrix33& p_lhs, const Matrix33& p_rhs)
{
	return Matrix33(
			p_lhs.m_11 * p_rhs.m_11 + p_lhs.m_12 * p_rhs.m_21 + p_lhs.m_13 * p_rhs.m_31, 
			p_lhs.m_11 * p_rhs.m_12 + p_lhs.m_12 * p_rhs.m_22 + p_lhs.m_13 * p_rhs.m_32,
			p_lhs.m_11 * p_rhs.m_13 + p_lhs.m_12 * p_rhs.m_23 + p_lhs.m_13 * p_rhs.m_33,
			
			p_lhs.m_21 * p_rhs.m_11 + p_lhs.m_22 * p_rhs.m_21 + p_lhs.m_23 * p_rhs.m_31,
			p_lhs.m_21 * p_rhs.m_12 + p_lhs.m_22 * p_rhs.m_22 + p_lhs.m_23 * p_rhs.m_32,
			p_lhs.m_21 * p_rhs.m_13 + p_lhs.m_22 * p_rhs.m_23 + p_lhs.m_23 * p_rhs.m_33,
			
			p_lhs.m_31 * p_rhs.m_11 + p_lhs.m_32 * p_rhs.m_21 + p_lhs.m_33 * p_rhs.m_31,
			p_lhs.m_31 * p_rhs.m_12 + p_lhs.m_32 * p_rhs.m_22 + p_lhs.m_33 * p_rhs.m_32,
			p_lhs.m_31 * p_rhs.m_13 + p_lhs.m_32 * p_rhs.m_23 + p_lhs.m_33 * p_rhs.m_33);
}

// Matrix-Vector Multiplication
inline const Vector3 operator*(const Vector3& p_lhs, const Matrix33& p_rhs)
{
	return Vector3(
			p_lhs.x * p_rhs.m_11 + p_lhs.y * p_rhs.m_21 + p_lhs.z * p_rhs.m_31,
			p_lhs.x * p_rhs.m_12 + p_lhs.y * p_rhs.m_22 + p_lhs.z * p_rhs.m_32,
			p_lhs.x * p_rhs.m_13 + p_lhs.y * p_rhs.m_23 + p_lhs.z * p_rhs.m_33);
}

// Matrix Addition
inline const Matrix33 operator+(const Matrix33& p_lhs, const Matrix33& p_rhs)
{
	return Matrix33(
			p_lhs.m_11 + p_rhs.m_11, p_lhs.m_12 + p_rhs.m_12, p_lhs.m_13 + p_rhs.m_13,
			p_lhs.m_21 + p_rhs.m_21, p_lhs.m_22 + p_rhs.m_22, p_lhs.m_23 + p_rhs.m_23,
			p_lhs.m_31 + p_rhs.m_31, p_lhs.m_32 + p_rhs.m_32, p_lhs.m_33 + p_rhs.m_33);
}

// Matrix Subtraction
inline const Matrix33 operator-(const Matrix33& p_lhs, const Matrix33& p_rhs)
{
	return Matrix33(
			p_lhs.m_11 - p_rhs.m_11, p_lhs.m_12 - p_rhs.m_12, p_lhs.m_13 - p_rhs.m_13,
			p_lhs.m_21 - p_rhs.m_21, p_lhs.m_22 - p_rhs.m_22, p_lhs.m_23 - p_rhs.m_23,
			p_lhs.m_31 - p_rhs.m_31, p_lhs.m_32 - p_rhs.m_32, p_lhs.m_33 - p_rhs.m_33);
}

inline bool operator==(const Matrix33& p_lhs, const Matrix33& p_rhs)
{
	// TODO: verify that this works correctly
	return std::memcmp(&p_lhs, &p_rhs, sizeof(Matrix33)) == 0;
}

inline bool operator!=(const Matrix33& p_lhs, const Matrix33& p_rhs)
{
	return (p_lhs == p_rhs) == false;
}


// Namespace end
}
}


#endif  // INC_TT_MATH_MATRIX33_H
