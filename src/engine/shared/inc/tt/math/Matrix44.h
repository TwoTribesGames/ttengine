///////////////////////////////////////////////////////////////////////////////
///  Description   : 4x4 Matrix Class:
///					 Row-Major Order, 
///					 Multiplication with row vector on the left
///					 We are using a right-handed coordinate system

#if !defined(INC_TT_MATH_MATRIX44_H)
#define INC_TT_MATH_MATRIX44_H

#include <ostream>
#include <cstring>

#include <tt/platform/tt_types.h>
#include <tt/platform/tt_error.h>
#include <tt/streams/BOStream.h>
#include <tt/streams/BIStream.h>
#include <tt/math/Vector3.h>
#include <tt/math/Vector4.h>
#include <tt/math/math.h>
#include <tt/math/Matrix33.h>
#include <tt/fs/File.h>


#if !defined(TT_PLATFORM_SDL)
#define USE_WIN_OPTIMIZATIONS
#endif


#if defined(TT_PLATFORM_WIN) && defined(_MSC_VER) && defined (USE_WIN_OPTIMIZATIONS)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <D3dx9math.h>
#endif

// Get rid of minor macro
#ifdef minor
#undef minor
#endif


namespace tt {
namespace math {


class Matrix44;
const Matrix44 operator*(const Matrix44& p_lhs, const Matrix44& p_rhs);
bool operator==(const Matrix44& p_lhs, const Matrix44& p_rhs);


/*! \brief 4x4 Matrix Class: Row-Major Order, Multiplication with column vector on the right.
           We are using a right-handed coordinate system */
class Matrix44
{
public:
	enum InitType { UNINITIALIZED };
	explicit Matrix44(InitType) {}

	Matrix44()
	:
	m_11(1), m_12(0), m_13(0), m_14(0),
	m_21(0), m_22(1), m_23(0), m_24(0),
	m_31(0), m_32(0), m_33(1), m_34(0),
	m_41(0), m_42(0), m_43(0), m_44(1)
	{
	}
	
	Matrix44(real p_11, real p_12, real p_13, real p_14,
	         real p_21, real p_22, real p_23, real p_24,
	         real p_31, real p_32, real p_33, real p_34,
	         real p_41, real p_42, real p_43, real p_44)
	:
	m_11(p_11),	m_12(p_12), m_13(p_13), m_14(p_14),
	m_21(p_21),	m_22(p_22), m_23(p_23), m_24(p_24),
	m_31(p_31), m_32(p_32), m_33(p_33), m_34(p_34),
	m_41(p_41), m_42(p_42), m_43(p_43), m_44(p_44)
	{
	}

	Matrix44(const Matrix44& p_rhs)
	:
	m_11(p_rhs.m_11), m_12(p_rhs.m_12), m_13(p_rhs.m_13), m_14(p_rhs.m_14),
	m_21(p_rhs.m_21), m_22(p_rhs.m_22), m_23(p_rhs.m_23), m_24(p_rhs.m_24),
	m_31(p_rhs.m_31), m_32(p_rhs.m_32), m_33(p_rhs.m_33), m_34(p_rhs.m_34),
	m_41(p_rhs.m_41), m_42(p_rhs.m_42), m_43(p_rhs.m_43), m_44(p_rhs.m_44)
	{
	}
	
	Matrix44(const Matrix33& p_rhs)
	:
	m_11(p_rhs.m_11), m_12(p_rhs.m_12), m_13(p_rhs.m_13), m_14(0),
	m_21(p_rhs.m_21), m_22(p_rhs.m_22), m_23(p_rhs.m_23), m_24(0),
	m_31(p_rhs.m_31), m_32(p_rhs.m_32), m_33(p_rhs.m_33), m_34(0),
	m_41(0),          m_42(0),          m_43(0),          m_44(1)
	{
	}
	
	~Matrix44() {}

	bool load(const fs::FilePtr& p_file)
	{
		if(p_file->read(this, sizeof(real) * 16) != sizeof(real) * 16)
		{
			return false;
		}
		return true;
	}
	
	inline Matrix44& operator = (const Matrix44& p_rhs)
	{
		m_11 = p_rhs.m_11; m_12 = p_rhs.m_12; m_13 = p_rhs.m_13; m_14 = p_rhs.m_14;
		m_21 = p_rhs.m_21; m_22 = p_rhs.m_22; m_23 = p_rhs.m_23; m_24 = p_rhs.m_24;
		m_31 = p_rhs.m_31; m_32 = p_rhs.m_32; m_33 = p_rhs.m_33; m_34 = p_rhs.m_34;
		m_41 = p_rhs.m_41; m_42 = p_rhs.m_42; m_43 = p_rhs.m_43; m_44 = p_rhs.m_44;
		return *this;
	}

	inline Matrix44& operator += (const Matrix44& p_rhs)
	{
		m_11 += p_rhs.m_11; m_12 += p_rhs.m_12; m_13 += p_rhs.m_13; m_14 += p_rhs.m_14;
		m_21 += p_rhs.m_21; m_22 += p_rhs.m_22; m_23 += p_rhs.m_23; m_24 += p_rhs.m_24;
		m_31 += p_rhs.m_31; m_32 += p_rhs.m_32; m_33 += p_rhs.m_33; m_34 += p_rhs.m_34;
		m_41 += p_rhs.m_41; m_42 += p_rhs.m_42; m_43 += p_rhs.m_43; m_44 += p_rhs.m_44;
		return *this;
	}
	
	inline Matrix44& operator -= (const Matrix44& p_rhs)
	{
		m_11 -= p_rhs.m_11; m_12 -= p_rhs.m_12; m_13 -= p_rhs.m_13; m_14 -= p_rhs.m_14;
		m_21 -= p_rhs.m_21; m_22 -= p_rhs.m_22; m_23 -= p_rhs.m_23; m_24 -= p_rhs.m_24;
		m_31 -= p_rhs.m_31; m_32 -= p_rhs.m_32; m_33 -= p_rhs.m_33; m_34 -= p_rhs.m_34;
		m_41 -= p_rhs.m_41; m_42 -= p_rhs.m_42; m_43 -= p_rhs.m_43; m_44 -= p_rhs.m_44;
		return *this;
	}

	inline Matrix44& operator *= (const Matrix44& p_rhs)
	{
		*this = (*this) * p_rhs;
		return *this;
	}

	inline Matrix44& operator *= (const real p_rhs)
	{
		m_11 *= p_rhs; m_12 *= p_rhs; m_13 *= p_rhs; m_14 *= p_rhs;
		m_21 *= p_rhs; m_22 *= p_rhs; m_23 *= p_rhs; m_24 *= p_rhs;
		m_31 *= p_rhs; m_32 *= p_rhs; m_33 *= p_rhs; m_34 *= p_rhs;
		m_41 *= p_rhs; m_42 *= p_rhs; m_43 *= p_rhs; m_44 *= p_rhs;
		return *this;
	}

	inline Matrix44& operator /= (const real p_rhs)
	{
		TT_ASSERTMSG(realEqual(p_rhs, 0) == false, "Division by zero.");

		real inv = 1.0f / p_rhs;

		m_11 *= inv; m_12 *= inv; m_13 *= inv; m_14 *= inv;
		m_21 *= inv; m_22 *= inv; m_23 *= inv; m_24 *= inv;
		m_31 *= inv; m_32 *= inv; m_33 *= inv; m_34 *= inv;
		m_41 *= inv; m_42 *= inv; m_43 *= inv; m_44 *= inv;
		return *this;
	}
	
	inline Matrix44 operator - ()
	{
		return Matrix44(-m_11, -m_12, -m_13, -m_14,
		                -m_21, -m_22, -m_23, -m_24,
		                -m_31, -m_32, -m_33, -m_34,
		                -m_41, -m_42, -m_43, -m_44);
	}
	
	///////////////////////////////////////////////////////////////////////////
	// Modifiers
	///////////////////////////////////////////////////////////////////////////
	inline Matrix44& clear()
	{
		m_11 = m_12 = m_13 = m_14 = 0;
		m_21 = m_22 = m_23 = m_24 = 0;
		m_31 = m_32 = m_33 = m_34 = 0;
		m_41 = m_42 = m_43 = m_44 = 0;
		return *this;
	}

	inline Matrix44& setIdentity()
	{
		m_12 = m_13 = m_14 = 0;
		m_21 = m_23 = m_24 = 0;
		m_31 = m_32 = m_34 = 0;
		m_41 = m_42 = m_43 = 0;
		m_11 = m_22 = m_33 = m_44 = 1;
		return *this;
	}

	inline Matrix44& transpose()
	{
		std::swap(m_12, m_21);
		std::swap(m_13, m_31);
		std::swap(m_14, m_41);
		std::swap(m_23, m_32);
		std::swap(m_24, m_42);
		std::swap(m_34, m_43);
		return *this;
	}
	
	inline Matrix44& inverse()
	{
		// FIXME: Use mtx_inverse44, but make sure there aren't any issues with it
		// Calculate the inverse - Divide the adjoint matrix by the determinant of the matrix
		// We dont check for singular.
		
		real inv_det = 1.0f / getDeterminant();
		adjoint();
		(*this) *=(inv_det);
		
		return *this;
	}
		
	inline Matrix44& cofactor()
	{
		*this = getCofactor();
		return *this;
	}
	
	inline Matrix44& adjoint()
	{
		cofactor().transpose();
		return *this;
	}
	
	inline Matrix44& rotateX(real p_angleRad)
	{
		*this = getRotationX(p_angleRad) * (*this);
		return *this;
	}

	inline Matrix44& rotateY(real p_angleRad)
	{
		*this = getRotationY(p_angleRad) * (*this);
		return *this;
	}

	inline Matrix44& rotateZ(real p_angleRad)
	{
		*this = getRotationZ(p_angleRad) * (*this);
		return *this;
	}

	inline Matrix44& rotateXYZ(real p_x, real p_y, real p_z)
	{
		*this = getRotationXYZ(p_x, p_y, p_z) * (*this);
		return *this;
	}

	inline Matrix44& rotateXYZ(const Vector3& p_rotation)
	{
		*this = getRotationXYZ(p_rotation.x, 
							   p_rotation.y, 
							   p_rotation.z) * (*this);
		return *this;
	}

	inline Matrix44& scale(real p_x, real p_y)
	{
		*this = getScale(p_x, p_y) * (*this);
		return *this;
	}

	inline Matrix44& scale(real p_x, real p_y, real p_z)
	{
		*this = getScale(p_x, p_y, p_z) * (*this);
		return *this;
	}

	inline Matrix44& scale(const Vector3& p_scale)
	{
		return scale(p_scale.x, p_scale.y, p_scale.z);
	}

	inline Matrix44& uniformScale(real p_scale)
	{
		return scale(p_scale, p_scale);
	}

	/*inline void shearXY(real p_shearS, real p_shearT)
	{
		*this = getShearXY(p_shearS, p_shearT) * (*this);
	}

	inline void shearXZ(real p_shearS, real p_shearT)
	{
		*this = getShearXZ(p_shearS, p_shearT) * (*this);
	}

	inline void shearYZ(real p_shearS, real p_shearT)
	{
		*this = getShearYZ(p_shearS, p_shearT) * (*this);
	}*/

	inline Matrix44& translate(real p_x, real p_y)
	{
		m_41 += m_11 * p_x + m_21 * p_y;
		m_42 += m_12 * p_x + m_22 * p_y;
		m_43 += m_13 * p_x + m_23 * p_y;
		return *this;
	}

	inline Matrix44& translate(real p_x, real p_y, real p_z)
	{
		m_41 += m_11 * p_x + m_21 * p_y + m_31 * p_z;
		m_42 += m_12 * p_x + m_22 * p_y + m_32 * p_z;
		m_43 += m_13 * p_x + m_23 * p_y + m_33 * p_z;
		return *this;
	}

	inline Matrix44& translate(const Vector2& p_translate)
	{
		return translate(p_translate.x, p_translate.y);
	}

	inline Matrix44& translate(const Vector3& p_translate)
	{
		return translate(p_translate.x, p_translate.y, p_translate.z);
	}


	///////////////////////////////////////////////////////////////////////////
	// Getters
	///////////////////////////////////////////////////////////////////////////
	inline Matrix44 getInverse() const
	{
		const real s0 = m_11 * m_22 - m_21 * m_12;
		const real s1 = m_11 * m_23 - m_21 * m_13;
		const real s2 = m_11 * m_24 - m_21 * m_14;
		const real s3 = m_12 * m_23 - m_22 * m_13;
		const real s4 = m_12 * m_24 - m_22 * m_14;
		const real s5 = m_13 * m_24 - m_23 * m_14;
 
		const real c5 = m_33 * m_44 - m_43 * m_34;
		const real c4 = m_32 * m_44 - m_42 * m_34;
		const real c3 = m_32 * m_43 - m_42 * m_33;
		const real c2 = m_31 * m_44 - m_41 * m_34;
		const real c1 = m_31 * m_43 - m_41 * m_33;
		const real c0 = m_31 * m_42 - m_41 * m_32;

		real determinant = (s0 * c5 - s1 * c4 + s2 * c3 + s3 * c2 - s4 * c1 + s5 * c0);
		TT_ASSERT(realEqual(determinant, 0) == false);

		// Should check for 0 determinant

		real invdet = 1.0f / determinant;
		Matrix44 m(UNINITIALIZED);

		m.m_11 = ( m_22 * c5 - m_23 * c4 + m_24 * c3) * invdet;
		m.m_12 = (-m_12 * c5 + m_13 * c4 - m_14 * c3) * invdet;
		m.m_13 = ( m_42 * s5 - m_43 * s4 + m_44 * s3) * invdet;
		m.m_14 = (-m_32 * s5 + m_33 * s4 - m_34 * s3) * invdet;

		m.m_21 = (-m_21 * c5 + m_23 * c2 - m_24 * c1) * invdet;
		m.m_22 = ( m_11 * c5 - m_13 * c2 + m_14 * c1) * invdet;
		m.m_23 = (-m_41 * s5 + m_43 * s2 - m_44 * s1) * invdet;
		m.m_24 = ( m_31 * s5 - m_33 * s2 + m_34 * s1) * invdet;

		m.m_31 = ( m_21 * c4 - m_22 * c2 + m_24 * c0) * invdet;
		m.m_32 = (-m_11 * c4 + m_12 * c2 - m_14 * c0) * invdet;
		m.m_33 = ( m_41 * s4 - m_42 * s2 + m_44 * s0) * invdet;
		m.m_34 = (-m_31 * s4 + m_32 * s2 - m_34 * s0) * invdet;

		m.m_41 = (-m_21 * c3 + m_22 * c1 - m_23 * c0) * invdet;
		m.m_42 = ( m_11 * c3 - m_12 * c1 + m_13 * c0) * invdet;
		m.m_43 = (-m_41 * s3 + m_42 * s1 - m_43 * s0) * invdet;
		m.m_44 = ( m_31 * s3 - m_32 * s1 + m_33 * s0) * invdet;

		return m;
	}

	
	inline Matrix44 getCofactor() const
	{
		// COFACTOR
		// 
		// | + - + - |  : The cofactor matrix is created by replacing each element by its cofactor, that is the signed minor 
		// | - + - + |  : value obtained by calculating a minor determinant.
		// | + - + - |  :
		// | - + - + |  :
		//
		
		return Matrix44( minor(1, 1), -minor(1, 2),  minor(1, 3), -minor(1, 4),
		                -minor(2, 1),  minor(2, 2), -minor(2, 3),  minor(2, 4),
		                 minor(3, 1), -minor(3, 2),  minor(3, 3), -minor(3, 4),
		                -minor(4, 1),  minor(4, 2), -minor(4, 3),  minor(4, 4));
	}

	inline Matrix44 getAdjoint() const
	{
		return Matrix44(*this).adjoint();
	}

	
	inline Matrix44 getTranspose() const
	{
		return Matrix44(*this).transpose();
	}
	
	inline real getDeterminant() const
	{
		return m_11 * minor(1, 1) - 
		       m_12 * minor(1, 2) + 
		       m_13 * minor(1, 3) - 
		       m_14 * minor(1, 4);
	}

	static inline Matrix44 getRotationX(real p_angleRad)
	{
		return Matrix44(1, 0, 0, 0,
						0,  math::cos(p_angleRad), math::sin(p_angleRad), 0,
						0, -math::sin(p_angleRad), math::cos(p_angleRad),0,
						0, 0, 0, 1);
	}

	static inline Matrix44 getRotationY(real p_angleRad)
	{
		return Matrix44( math::cos(p_angleRad), 0, -math::sin(p_angleRad), 0,
						                     0, 1,                     0, 0,
						 math::sin(p_angleRad), 0, math::cos(p_angleRad), 0,
						                     0, 0,                     0, 1);
	}

	static inline Matrix44 getRotationZ(real p_angleRad)
	{
#if defined(TT_PLATFORM_WIN) && defined(_MSC_VER) && defined(USE_WIN_OPTIMIZATIONS)
		Matrix44 result;
		D3DXMatrixRotationZ(reinterpret_cast<D3DXMATRIX*>(&result), p_angleRad);
		return result;
#else
		const real s = math::sin(p_angleRad);
		const real c = math::cos(p_angleRad);

		return Matrix44( c, s, 0, 0,
						-s, c, 0, 0,
						 0, 0, 1, 0,
						 0, 0, 0, 1);
#endif
	}

	// FIXME: Try to optimize this one as well
	static inline Matrix44 getRotationXYZ(real p_x, real p_y, real p_z)
	{
		// Work out the repeated values
		real cx = math::cos(p_x);
		real sx = math::sin(p_x);
		real cy = math::cos(p_y);
		real sy = math::sin(p_y);
		real cz = math::cos(p_z);
		real sz = math::sin(p_z);

		real sxcz = (sx * cz);
		real cxcz = (cx * cz);
		real sxsz = (sx * sz);
		real cxsz = (cx * sz);

		return Matrix44(          cy * cz,            cy * sz,       -sy, 0,
					   (sxcz * sy) - cxsz, (sxsz * sy) + cxcz, (sx * cy), 0, 
					   (cxcz * sy) + sxsz, (cxsz * sy) - sxcz, (cx * cy), 0,
					                    0,                  0,        0,  1);

	}

	static inline Matrix44 getScale(real p_x, real p_y, real p_z)
	{
		// Scaling by zero is not really a problem, however it can indicate bad data
		//TT_WARNING(realEqual(p_x, 0) == false, "X-axis is scaled by 0");
		//TT_WARNING(realEqual(p_y, 0) == false, "Y-axis is scaled by 0");
		//TT_WARNING(realEqual(p_z, 0) == false, "Z-axis is scaled by 0");

		return Matrix44(p_x, 0, 0, 0,
						0, p_y, 0, 0,
						0, 0, p_z, 0,
						0, 0, 0, 1);
	}
	
	static inline Matrix44 getScale(real p_x, real p_y)
	{
		// Scaling by zero is not really a problem, however it can indicate bad data
		//TT_WARNING(realEqual(p_x, 0) == false, "X-axis is scaled by 0");
		//TT_WARNING(realEqual(p_y, 0) == false, "Y-axis is scaled by 0");

		return Matrix44(p_x, 0, 0, 0,
						0, p_y, 0, 0,
						0, 0, 1, 0,
						0, 0, 0, 1);
	}

	// TODO: Verify Shear Matrices (Not working correctly)
	static inline Matrix44 getShearXY(real p_shearX, real p_shearY)
	{
		return Matrix44(1, p_shearX, 0, 0,
						p_shearY, 1, 0, 0,
						0, 0, 1, 0,
						0, 0, 0, 1);
	}

	static inline Matrix44 getShearXZ(real p_shearS, real p_shearT)
	{
		return Matrix44(1, 0, 0, 0,
						p_shearS, 1, p_shearT, 0,
						0, 0, 1, 0,
						0, 0, 0, 1);
	}

	static inline Matrix44 getShearYZ(real p_shearS, real p_shearT)
	{
		return Matrix44(1, p_shearS, p_shearT, 0,
						0, 1, 0, 0,
						0, 0, 1, 0,
						0, 0, 0, 1);
	}

	static inline Matrix44 getTranslation(real p_x, real p_y, real p_z)
	{
		return Matrix44(1, 0, 0, 0,
						0, 1, 0, 0,
						0, 0, 1, 0,
						p_x, p_y, p_z, 1);
	}
	
	static inline Matrix44 getTranslation(Vector3 p_translation)
	{
		return getTranslation(p_translation.x, p_translation.y, p_translation.z);
	}
	
	static inline Matrix44 getSRT(const Vector3& p_scale, 
								  const Vector3& p_rotation, 
								  const Vector3& p_translation)
	{
		// Can be better optimized
		Matrix44 temp;

		temp.translate(p_translation);
		temp.rotateXYZ(p_rotation);
		temp.scale(p_scale);

		return temp;
	}

	// NOTE: Rotation is in DEGREES, internally converted to radians
	static Matrix44 getMayaTextureMatrix(real p_scaleS, real p_scaleT,
		                                 real p_rotation,
		                                 real p_translateS, real p_translateT);
	
	inline bool isIdentity() const
	{
		return (*this) == Matrix44::identity;
	}
	
	// Decomposition of scale from Matrix44
	inline real getScaleVectorX() const
	{
		return reinterpret_cast<const Vector3*>(&m_11)->length();
	}
	
	inline real getScaleVectorY() const
	{
		return reinterpret_cast<const Vector3*>(&m_21)->length();
	}
	
	inline real getScaleVectorZ() const
	{
		return reinterpret_cast<const Vector3*>(&m_31)->length();
	}
	
	inline Vector3 getScaleVector() const
	{
		return Vector3(getScaleVectorX(), getScaleVectorY(), getScaleVectorZ());
	}
	
	///////////////////////////////////////////////////////////////////////////
	// Constants
	///////////////////////////////////////////////////////////////////////////
	static const Matrix44 zero;
	static const Matrix44 identity;
	
	// Make these public for optimization purposes
	real m_11, m_12, m_13, m_14; 
	real m_21, m_22, m_23, m_24;
	real m_31, m_32, m_33, m_34;
	real m_41, m_42, m_43, m_44;

	
	/* \brief get value of matrix cell p_row x p_column.
	   \param p_row    1 based index of row
	   \param p_column 1 based index of column
	   \return Value of the matrix cell p_row, p_column.
	   \note Both indexes are 1 based (Start counting at 1 not 0)!*/
	inline real get(s32 p_row, s32 p_column) const
	{
		switch(p_row)
		{
		case 1:
			switch(p_column)
			{
			case 1: return m_11;
			case 2: return m_12;
			case 3: return m_13;
			case 4: return m_14;
			}
		case 2:
			switch(p_column)
			{
			case 1: return m_21;
			case 2: return m_22;
			case 3: return m_23;
			case 4: return m_24;
			}
		case 3:
			switch(p_column)
			{
			case 1: return m_31;
			case 2: return m_32;
			case 3: return m_33;
			case 4: return m_34;
			}
		case 4:
			switch(p_column)
			{
			case 1: return m_41;
			case 2: return m_42;
			case 3: return m_43;
			case 4: return m_44;
			}
		}
		
		TT_PANIC("Not a valid row (%d) and/or column (%d)\n", 
		         p_row, p_column);
		return real(0.0f);
	}

private:

	/* \brief Calculate the determinant of a submatrix.
	   \param p_row    1 based index of row
	   \param p_column 1 based index of column
	   \return Determinant of submatrix p_row, p_column.
	   \note Both indexes are 1 based (Start counting at 1 not 0)!*/
	inline real minor( s32 p_row, s32 p_column) const
	{
		static const s32 submatrixSize = 3; // The size of a submatrix.
		static const s32 matrixSize    = 4; // The size of this matrix.
		
		s32 row[submatrixSize];
		s32 column[submatrixSize];
		s32 row_entry = 0;
		s32 column_entry = 0;
		
		// Work out which rows and colums to do
		// (Remember row and column indexes are 1 based!)
		for (s32 i = 1; i < matrixSize + 1; ++i)
		{
			// As long as this isnt the row we want - we can use it
			if (i != p_row)
			{
				row[row_entry++] = i;
			}
			
			// And the columns
			if (i != p_column)
			{
				column[column_entry++] = i;
			}
		}
		
		// Now calculate the determinant of this submatrix
		return (get(row[0], column[0]) * get(row[1], column[1]) * get(row[2], column[2])) -
		       (get(row[0], column[0]) * get(row[1], column[2]) * get(row[2], column[1])) -
		       (get(row[0], column[1]) * get(row[1], column[0]) * get(row[2], column[2])) +
		       (get(row[0], column[1]) * get(row[1], column[2]) * get(row[2], column[0])) +
		       (get(row[0], column[2]) * get(row[1], column[0]) * get(row[2], column[1])) -
		       (get(row[0], column[2]) * get(row[1], column[1]) * get(row[2], column[0]));
	}
	

};

inline std::ostream& operator<<(std::ostream& s, const Matrix44& m)
{
	return s << std::endl << '(' << m.m_11 << ' ' << m.m_12 << ' ' << 
	                                m.m_13 << ' ' << m.m_14 << ')' <<
				std::endl << '(' << m.m_21 << ' ' << m.m_22 << ' ' << 
	                                m.m_23 << ' ' << m.m_24 << ')' <<
				std::endl << '(' << m.m_31 << ' ' << m.m_32 << ' ' << 
	                                m.m_33 << ' ' << m.m_34 << ')' <<
				std::endl << '(' << m.m_41 << ' ' << m.m_42 << ' ' << 
	                                m.m_43 << ' ' << m.m_44 << ')';
}


inline tt::streams::BOStream& operator<<(tt::streams::BOStream& s, const Matrix44& m)
{
	s << m.m_11;
	s << m.m_12;
	s << m.m_13;
	s << m.m_14;
	s << m.m_21;
	s << m.m_22;
	s << m.m_23;
	s << m.m_24;
	s << m.m_31;
	s << m.m_32;
	s << m.m_33;
	s << m.m_34;
	s << m.m_41;
	s << m.m_42;
	s << m.m_43;
	s << m.m_44;
	return s;
}


inline tt::streams::BIStream& operator>>(tt::streams::BIStream& s, Matrix44& m)
{
	s >> m.m_11;
	s >> m.m_12;
	s >> m.m_13;
	s >> m.m_14;
	s >> m.m_21;
	s >> m.m_22;
	s >> m.m_23;
	s >> m.m_24;
	s >> m.m_31;
	s >> m.m_32;
	s >> m.m_33;
	s >> m.m_34;
	s >> m.m_41;
	s >> m.m_42;
	s >> m.m_43;
	s >> m.m_44;
	return s;
}


// Multiplication with a scalar
inline const Matrix44 operator*(const real p_lhs, const Matrix44& p_rhs)
{
	return Matrix44(p_lhs * p_rhs.m_11, p_lhs * p_rhs.m_12, 
                    p_lhs * p_rhs.m_13, p_lhs * p_rhs.m_14,
                    p_lhs * p_rhs.m_21, p_lhs * p_rhs.m_22, 
                    p_lhs * p_rhs.m_23, p_lhs * p_rhs.m_24,
                    p_lhs * p_rhs.m_31, p_lhs * p_rhs.m_32, 
                    p_lhs * p_rhs.m_33, p_lhs * p_rhs.m_34,
                    p_lhs * p_rhs.m_41, p_lhs * p_rhs.m_42, 
                    p_lhs * p_rhs.m_43, p_lhs * p_rhs.m_44);
}

inline const Matrix44 operator*(const Matrix44& p_lhs, const real p_rhs)
{
	return p_rhs * p_lhs;
}

inline const Matrix44 operator/(const Matrix44& p_lhs, const real p_rhs)
{
	TT_ASSERTMSG(realEqual(p_rhs, 0) == false, "Division by zero.");
	
	return (1.0f / p_rhs) * p_lhs;
}

// Matrix-Matrix Multiplication
inline const Matrix44 operator*(const Matrix44& p_lhs, const Matrix44& p_rhs)
{
// Platform specific optimizations
#if defined(TT_PLATFORM_WIN) && defined(_MSC_VER) && defined(USE_WIN_OPTIMIZATIONS)
	Matrix44 result(Matrix44::UNINITIALIZED);
	D3DXMatrixMultiply(reinterpret_cast<D3DXMATRIX*>(&result),
		               reinterpret_cast<const D3DXMATRIX*>(&p_lhs),
					   reinterpret_cast<const D3DXMATRIX*>(&p_rhs));
	return result;
#else
	return Matrix44(
            p_lhs.m_11 * p_rhs.m_11 + p_lhs.m_12 * p_rhs.m_21 + 
            p_lhs.m_13 * p_rhs.m_31 + p_lhs.m_14 * p_rhs.m_41,
            p_lhs.m_11 * p_rhs.m_12 + p_lhs.m_12 * p_rhs.m_22 + 
            p_lhs.m_13 * p_rhs.m_32 + p_lhs.m_14 * p_rhs.m_42,
            p_lhs.m_11 * p_rhs.m_13 + p_lhs.m_12 * p_rhs.m_23 + 
            p_lhs.m_13 * p_rhs.m_33 + p_lhs.m_14 * p_rhs.m_43,
            p_lhs.m_11 * p_rhs.m_14 + p_lhs.m_12 * p_rhs.m_24 + 
            p_lhs.m_13 * p_rhs.m_34 + p_lhs.m_14 * p_rhs.m_44,
            
            p_lhs.m_21 * p_rhs.m_11 + p_lhs.m_22 * p_rhs.m_21 + 
            p_lhs.m_23 * p_rhs.m_31 + p_lhs.m_24 * p_rhs.m_41,
            p_lhs.m_21 * p_rhs.m_12 + p_lhs.m_22 * p_rhs.m_22 + 
            p_lhs.m_23 * p_rhs.m_32 + p_lhs.m_24 * p_rhs.m_42,
            p_lhs.m_21 * p_rhs.m_13 + p_lhs.m_22 * p_rhs.m_23 + 
            p_lhs.m_23 * p_rhs.m_33 + p_lhs.m_24 * p_rhs.m_43,
            p_lhs.m_21 * p_rhs.m_14 + p_lhs.m_22 * p_rhs.m_24 + 
            p_lhs.m_23 * p_rhs.m_34 + p_lhs.m_24 * p_rhs.m_44,
            
            p_lhs.m_31 * p_rhs.m_11 + p_lhs.m_32 * p_rhs.m_21 + 
            p_lhs.m_33 * p_rhs.m_31 + p_lhs.m_34 * p_rhs.m_41,
            p_lhs.m_31 * p_rhs.m_12 + p_lhs.m_32 * p_rhs.m_22 + 
            p_lhs.m_33 * p_rhs.m_32 + p_lhs.m_34 * p_rhs.m_42,
            p_lhs.m_31 * p_rhs.m_13 + p_lhs.m_32 * p_rhs.m_23 + 
            p_lhs.m_33 * p_rhs.m_33 + p_lhs.m_34 * p_rhs.m_43,
            p_lhs.m_31 * p_rhs.m_14 + p_lhs.m_32 * p_rhs.m_24 + 
            p_lhs.m_33 * p_rhs.m_34 + p_lhs.m_34 * p_rhs.m_44,
            
            p_lhs.m_41 * p_rhs.m_11 + p_lhs.m_42 * p_rhs.m_21 + 
            p_lhs.m_43 * p_rhs.m_31 + p_lhs.m_44 * p_rhs.m_41,
            p_lhs.m_41 * p_rhs.m_12 + p_lhs.m_42 * p_rhs.m_22 + 
            p_lhs.m_43 * p_rhs.m_32 + p_lhs.m_44 * p_rhs.m_42,
            p_lhs.m_41 * p_rhs.m_13 + p_lhs.m_42 * p_rhs.m_23 + 
            p_lhs.m_43 * p_rhs.m_33 + p_lhs.m_44 * p_rhs.m_43,
            p_lhs.m_41 * p_rhs.m_14 + p_lhs.m_42 * p_rhs.m_24 + 
            p_lhs.m_43 * p_rhs.m_34 + p_lhs.m_44 * p_rhs.m_44);
#endif
}


// Matrix-Vector Multiplication
inline const Vector4 operator*(const Vector4& p_lhs, const Matrix44& p_rhs)
{
	return Vector4(p_lhs.x * p_rhs.m_11 + p_lhs.y * p_rhs.m_21 + 
                   p_lhs.z * p_rhs.m_31 + p_lhs.w * p_rhs.m_41,
                   p_lhs.x * p_rhs.m_12 + p_lhs.y * p_rhs.m_22 + 
                   p_lhs.z * p_rhs.m_32 + p_lhs.w * p_rhs.m_42,
                   p_lhs.x * p_rhs.m_13 + p_lhs.y * p_rhs.m_23 + 
                   p_lhs.z * p_rhs.m_33 + p_lhs.w * p_rhs.m_43,
                   p_lhs.x * p_rhs.m_14 + p_lhs.y * p_rhs.m_24 + 
                   p_lhs.z * p_rhs.m_34 + p_lhs.w * p_rhs.m_44);
}

// Matrix-Vector3 Multiplication (Vector3 has implicit 4th element of 1)
inline const Vector3 operator*(const Vector3& p_lhs, const Matrix44& p_rhs)
{
#if defined(TT_PLATFORM_WIN) && defined(_MSC_VER) && defined(USE_WIN_OPTIMIZATIONS)
	D3DXVECTOR4 result;
	D3DXVec3Transform(&result,
					  reinterpret_cast<const D3DXVECTOR3*>(&p_lhs),
					  reinterpret_cast<const D3DXMATRIX*>(&p_rhs));
	return Vector3(result.x, result.y, result.z);
#else
	return Vector3(
		p_lhs.x * p_rhs.m_11 + p_lhs.y * p_rhs.m_21 + p_lhs.z * p_rhs.m_31 + p_rhs.m_41,
		p_lhs.x * p_rhs.m_12 + p_lhs.y * p_rhs.m_22 + p_lhs.z * p_rhs.m_32 + p_rhs.m_42,
		p_lhs.x * p_rhs.m_13 + p_lhs.y * p_rhs.m_23 + p_lhs.z * p_rhs.m_33 + p_rhs.m_43);
#endif
}

// Matrix Addition
inline const Matrix44 operator+(const Matrix44& p_lhs, const Matrix44& p_rhs)
{
	return Matrix44(p_lhs.m_11 + p_rhs.m_11, p_lhs.m_12 + p_rhs.m_12, 
                    p_lhs.m_13 + p_rhs.m_13, p_lhs.m_14 + p_rhs.m_14,
                    p_lhs.m_21 + p_rhs.m_21, p_lhs.m_22 + p_rhs.m_22, 
                    p_lhs.m_23 + p_rhs.m_23, p_lhs.m_24 + p_rhs.m_24,
                    p_lhs.m_31 + p_rhs.m_31, p_lhs.m_32 + p_rhs.m_32, 
                    p_lhs.m_33 + p_rhs.m_33, p_lhs.m_34 + p_rhs.m_34,
                    p_lhs.m_41 + p_rhs.m_41, p_lhs.m_42 + p_rhs.m_42, 
                    p_lhs.m_43 + p_rhs.m_43, p_lhs.m_44 + p_rhs.m_44);
}

// Matrix Subtraction
inline const Matrix44 operator-(const Matrix44& p_lhs, const Matrix44& p_rhs)
{
	return Matrix44(p_lhs.m_11 - p_rhs.m_11, p_lhs.m_12 - p_rhs.m_12, 
                    p_lhs.m_13 - p_rhs.m_13, p_lhs.m_14 - p_rhs.m_14,
                    p_lhs.m_21 - p_rhs.m_21, p_lhs.m_22 - p_rhs.m_22, 
                    p_lhs.m_23 - p_rhs.m_23, p_lhs.m_24 - p_rhs.m_24,
                    p_lhs.m_31 - p_rhs.m_31, p_lhs.m_32 - p_rhs.m_32, 
                    p_lhs.m_33 - p_rhs.m_33, p_lhs.m_34 - p_rhs.m_34,
                    p_lhs.m_41 - p_rhs.m_41, p_lhs.m_42 - p_rhs.m_42, 
                    p_lhs.m_43 - p_rhs.m_43, p_lhs.m_44 - p_rhs.m_44);
}

inline bool operator==(const Matrix44& p_lhs, const Matrix44& p_rhs)
{
	// TODO: verify that this works correctly
	return std::memcmp(&p_lhs, &p_rhs, sizeof(Matrix44)) == 0;

	//// First checks SRT area
	//return p_lhs.m_11 == p_rhs.m_11 && p_lhs.m_12 == p_rhs.m_12 && p_lhs.m_13 == p_rhs.m_13 &&
	//	   p_lhs.m_21 == p_rhs.m_21 && p_lhs.m_22 == p_rhs.m_22 && p_lhs.m_23 == p_rhs.m_23 &&
	//	   p_lhs.m_31 == p_rhs.m_31 && p_lhs.m_32 == p_rhs.m_32 && p_lhs.m_33 == p_rhs.m_33 &&
	//	   p_lhs.m_41 == p_rhs.m_41 && p_lhs.m_42 == p_rhs.m_42 && p_lhs.m_43 == p_rhs.m_43 &&

	//	   p_lhs.m_14 == p_rhs.m_14 && p_lhs.m_24 == p_rhs.m_24 && 
	//	   p_lhs.m_34 == p_rhs.m_34 && p_lhs.m_44 == p_rhs.m_44;
}

inline bool operator!=(const Matrix44& p_lhs, const Matrix44& p_rhs)
{
	return (p_lhs == p_rhs) == false;
}


// Namespace end
}
}

#endif  // INC_TT_MATH_MATRIX44_H
