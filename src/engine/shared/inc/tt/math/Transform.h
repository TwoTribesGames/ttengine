#if !defined(INC_TT_MATH_TRANSFORMATION_H)
#define INC_TT_MATH_TRANSFORMATION_H

#include <tt/platform/tt_types.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>
#include <tt/math/Quaternion.h>
#include <tt/math/Vector3.h>
#include <tt/math/Matrix44.h>


namespace tt {
namespace math {

class Transform
{
public:
	static const Transform identity;
	
	inline Transform()
	:
	m_rotation(Quaternion::identity),
	m_translation(Vector3::zero),
	m_scale(Vector3::allOne),
	m_matrix(),
	m_matrixValid(true),
	m_rotMatrix(),
	m_rotMatrixValid(true),
	m_isIdentity(true),
	m_isUniformScale(true)
	{ }
	
	inline void loadIdentity()
	{
		*this = identity;
	}

	/*! \brief Apply this transform on a input "child" Transform 
	           and return the resulting Transform. */
	inline Transform applyTransform(const Transform& p_child) const
	{
		if (m_isIdentity)
		{
			return Transform(p_child);
		}
		
		if (p_child.m_isIdentity)
		{
			return Transform(*this);
		}
		
		Transform ret(*this);
		
		// Rotation
		ret.modifyRotation() = p_child.getRotation() * getRotation();
		
		// Translation
		ret.m_translation = applyForward(p_child.getTranslation());
		
		// Scale
		if (m_isUniformScale && p_child.m_isUniformScale)
		{
			ret.setUniformScale(getUniformScale() * 
			                    p_child.getUniformScale());
		}
		else
		{
			ret.m_scale.x = m_scale.x * p_child.m_scale.x;
			ret.m_scale.y = m_scale.y * p_child.m_scale.y;
			ret.m_scale.z = m_scale.z * p_child.m_scale.z;
			ret.m_isUniformScale = false;
		}
		return ret;
	}
	
	/*! \brief Calculate the local Transform of p_child.
	           This is parent and p_child is child. */
	inline Transform calcChildLocal(const Transform& p_child) const
	{
		if (m_isIdentity)
		{
			return Transform(p_child);
		}
		
		if (p_child.m_isIdentity)
		{
			Transform temp(*this);
			temp.inverse();
			return temp;
		}
		
		Transform invThis(*this);
		invThis.inverse();
		
		
		Transform result(p_child);
		
		// Rotation
		result.modifyRotation() = p_child.getRotation() * invThis.getRotation();
		
		// Translation
		result.modifyTranslation() = applyInverse(p_child.getTranslation());
		
		// Scale
		if (p_child.m_isUniformScale && invThis.m_isUniformScale)
		{
			result.setUniformScale(p_child.getUniformScale() * 
			                       invThis.getUniformScale());
		}
		else
		{
			result.modifyScale().x *= invThis.getScale().x;
			result.modifyScale().y *= invThis.getScale().y;
			result.modifyScale().z *= invThis.getScale().z;
		}
		
		return result;
	}
	
	void inverse()
	{
		if(m_isIdentity)
		{
			return;
		}

		m_rotation.inverse();
		
		Vector3 scaledTranslation(m_translation);
		if (m_isUniformScale)
		{
			real scale = getUniformScale();
			setUniformScale(1.0f / scale);
			
			scaledTranslation *= getUniformScale();
		}
		else
		{
			m_scale.x = 1.0f / m_scale.x;
			m_scale.y = 1.0f / m_scale.y;
			m_scale.z = 1.0f / m_scale.z;
			
			scaledTranslation.x *= m_scale.x;
			scaledTranslation.y *= m_scale.y;
			scaledTranslation.z *= m_scale.z;
		}
		
		m_translation = -m_rotation.applyRotation(scaledTranslation);
		
		m_isIdentity     = false;
		m_matrixValid    = false;
		m_rotMatrixValid = false;
	}

	
	inline void setRotation(const Quaternion& p_rotation)
	{
		if (m_rotation != p_rotation)
		{
			m_rotation       = p_rotation;
			m_isIdentity     = false;
			m_matrixValid    = false;
			m_rotMatrixValid = false;
		}
	}
	
	inline const Quaternion& getRotation() const
	{
		return m_rotation;
	}
	
	inline Quaternion& modifyRotation()
	{
		m_isIdentity     = false;
		m_matrixValid    = false;
		m_rotMatrixValid = false;

		return m_rotation;
	}
	
	inline void setTranslation(const Vector3& p_translation)
	{
		if (m_translation != p_translation)
		{
			m_translation = p_translation;
			m_matrixValid = false;
			
			m_isIdentity = false;
		}
	}
	
	inline const Vector3& getTranslation() const
	{
		return m_translation;
	}
	
	inline Vector3& modifyTranslation()
	{
		m_matrixValid = false;
		m_isIdentity = false;
		return m_translation;
	}
	
	inline void setScale(const Vector3& p_scale)
	{
		if (m_scale != p_scale)
		{
			m_scale = p_scale;

			m_matrixValid = false;
			m_isUniformScale = false;
			m_isIdentity = false;
		}
	}
	
	inline const Vector3& getScale() const
	{
		return m_scale;
	}
	
	inline Vector3& modifyScale()
	{
		m_isIdentity = false;
		m_matrixValid = false;
		m_isUniformScale = false;
		return m_scale;
	}
	
	inline void setUniformScale(real p_scale)
	{
		if (m_scale.x != p_scale ||
		    m_scale.y != p_scale ||
			m_scale.z != p_scale)
		{
			m_matrixValid = false;
			m_isIdentity = false;

			m_scale = Vector3::allOne * p_scale;
		}
		m_isUniformScale = true;
	}
	
	inline real getUniformScale() const
	{
		return m_scale.x;
	}
	
	inline bool isIdentity() const
	{
		return m_isIdentity;
	}
	
	inline bool isUniformScale() const
	{
		return m_isUniformScale;
	}
	
	/* \brief Apply transformation to input vector. */
	inline Vector3 applyForward(const Vector3& p_input) const
	{
		if (m_isIdentity)
		{
			return p_input;
		}
		
		Vector3 output(p_input.x * m_scale.x,
		               p_input.y * m_scale.y,
		               p_input.z * m_scale.z);
		output = m_rotation.applyRotation(output);
		output += m_translation;
		return output;
	}
	
	/* \brief Apply the inverse transformation to input vector. */
	inline Vector3 applyInverse(const Vector3& p_input) const
	{
		if (m_isIdentity)
		{
			return p_input;
		}
		
		if (m_isUniformScale)
		{
			return m_rotation.applyInverseRotation(p_input - m_translation) / 
				getUniformScale();
		}
		
		Vector3 output = m_rotation.applyInverseRotation(p_input - m_translation);
		real scaleXY = m_scale.x * m_scale.y;
		real scaleXZ = m_scale.x * m_scale.z;
		real scaleYZ = m_scale.y * m_scale.z;
		real invDet = 1.0f / (scaleXY * m_scale.z);
		output.x *= invDet * scaleYZ;
		output.y *= invDet * scaleXZ;
		output.z *= invDet * scaleXY;
		return output;
	}
	
	inline const Matrix44& getMatrix() const
	{
#if 0
		// If turning on this part remove the const reference from the return type!
		return Matrix44::getSRT(m_scale, 
		                        m_rotation.getRotationVector(), 
		                        m_translation);
#else
		if(m_matrixValid == false)
		{
			//TT_Printf("Creating matrix...\n");
			
			// Translate
			m_matrix = Matrix44::getTranslation(m_translation.x, m_translation.y, m_translation.z);
			
			// Rotate
			if (m_rotMatrixValid == false)
			{
				m_rotMatrix = m_rotation.getRotationMatrix();
				m_rotMatrixValid = true;
			}
			m_matrix = m_rotMatrix * m_matrix;
			
			// Scale
			m_matrix.scale(m_scale.x, m_scale.y, m_scale.z);
			
			m_matrixValid = true;
		}
		
		return m_matrix;
#endif
	}
	
private:
	Quaternion m_rotation;
	Vector3 m_translation;
	Vector3 m_scale;
	
	// Cached Transform Matrix
	mutable Matrix44 m_matrix;
	mutable bool m_matrixValid;
	
	// Cached Rotation Matrix
	mutable Matrix33 m_rotMatrix;
	mutable bool m_rotMatrixValid;
	
	bool m_isIdentity;
	bool m_isUniformScale;
};

inline bool operator==(const Transform& p_lhs, const Transform& p_rhs)
{
	if (p_lhs.getRotation()    == p_rhs.getRotation()    &&
	    p_lhs.getTranslation() == p_rhs.getTranslation() &&
	    p_lhs.getScale()       == p_rhs.getScale())
	{
		return true;
	}
	return false;
}


inline bool operator!=(const Transform& p_lhs, const Transform& p_rhs)
{
	return (p_lhs == p_rhs) == false;
}

// Namespace
}
}

#endif // !defined INC_TT_MATH_TRANSFORMATION_H
