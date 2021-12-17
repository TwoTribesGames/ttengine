#if !defined(INC_TT_MATH_QUATERNION_H)
#define INC_TT_MATH_QUATERNION_H


#include <ostream>

#include <tt/platform/tt_types.h>
#include <tt/platform/tt_error.h>
#include <tt/math/Vector3.h>
#include <tt/math/Matrix33.h>
#include <tt/math/math.h>
#include <tt/streams/BOStream.h>
#include <tt/streams/BIStream.h>



namespace tt {
namespace xml {
class XmlNode;
}
}


namespace tt {
namespace math {

class Quaternion;
Quaternion operator*(const Quaternion& p_lhs, const Quaternion& p_rhs);


class Quaternion
{
public:
	inline explicit Quaternion(real p_w = 1.0f, real p_x = 0.0f, real p_y = 0.0f, real p_z = 0.0f)
	:
	m_w(p_w),
	m_x(p_x),
	m_y(p_y),
	m_z(p_z)
	{
		// TODO: Clamp value in -1, 1 range & warn
		/*TT_ASSERTMSG(realGreaterEqual(m_w, -1.0f) && 
			         realLessEqual(m_w, 1.0f),
		             "m_w (%f) should be >= -1 && <= 1",
		             (float)m_w);*/
	}
	
	// Construct from axis-angle pair
	inline Quaternion(real p_thetaRad, Vector3 p_axis)
	:
	m_w(0.0f),
	m_x(0.0f),
	m_y(0.0f),
	m_z(0.0f)
	{
		// TODO: (With fixed values it could be done with rad >>= 1)
		real halfRad = p_thetaRad * 0.5f;
		
		// Compute angle part
		m_w = math::cos(halfRad);
		
		// Compute axis part
		real sinHalfTheta = math::sin(halfRad);
		
		p_axis.normalize();
		
		m_x = p_axis.x * sinHalfTheta;
		m_y = p_axis.y * sinHalfTheta;
		m_z = p_axis.z * sinHalfTheta;
		
		normalize();
	}
	
	inline Quaternion operator-() const
	{
		return Quaternion(-m_w, -m_x, -m_y, -m_z);
	}
	
	inline Quaternion& operator += (const Quaternion& p_rhs)
	{
		m_w += p_rhs.m_w; 
		m_x += p_rhs.m_x; 
		m_y += p_rhs.m_y;
		m_z += p_rhs.m_z;
		return *this;
	}
	
	inline Quaternion& operator -= (const Quaternion& p_rhs)
	{
		m_w -= p_rhs.m_w; 
		m_x -= p_rhs.m_x; 
		m_y -= p_rhs.m_y;
		m_z -= p_rhs.m_z;
		return *this;
	}
	
	inline Quaternion& operator *= (const Quaternion& p_rhs)
	{
		real w = m_w * p_rhs.m_w - m_x * p_rhs.m_x - 
		         m_y * p_rhs.m_y - m_z * p_rhs.m_z;
		real x = m_w * p_rhs.m_x + m_x * p_rhs.m_w + 
		         m_z * p_rhs.m_y - m_y * p_rhs.m_z;
		real y = m_w * p_rhs.m_y + m_y * p_rhs.m_w + 
		         m_x * p_rhs.m_z - m_z * p_rhs.m_x;
		real z = m_w * p_rhs.m_z + m_z * p_rhs.m_w + 
		         m_y * p_rhs.m_x - m_x * p_rhs.m_y;
		
		m_w = w;
		m_x = x;
		m_y = y;
		m_z = z;
		
		tt::math::clamp(m_w, -1.0f, 1.0f);
		
		return *this;
	}


	///////////////////////////////////////////////////////////////////////////
	// Modifiers
	///////////////////////////////////////////////////////////////////////////	
	inline Quaternion& rotate(const Quaternion& p_rhs)
	{
		real w = m_w * p_rhs.m_w - m_x * p_rhs.m_x - 
		         m_y * p_rhs.m_y - m_z * p_rhs.m_z;
		real x = m_w * p_rhs.m_x + m_x * p_rhs.m_w + 
		         p_rhs.m_y * m_z - p_rhs.m_z * m_y;
		real y = m_w * p_rhs.m_y + m_y * p_rhs.m_w + 
		         p_rhs.m_z * m_x - p_rhs.m_x * m_z;
		real z = m_w * p_rhs.m_z + m_z * p_rhs.m_w + 
		         p_rhs.m_x * m_y - p_rhs.m_y * m_x;
		
		m_w = w;
		m_x = x;
		m_y = y;
		m_z = z;
		
		tt::math::clamp(m_w, -1.0f, 1.0f);		
		return *this;
	}
	
	inline void setValues(real p_w, real p_x, real p_y, real p_z)
	{
		m_w = p_w;
		m_x = p_x;
		m_y = p_y;
		m_z = p_z;
	}
	
	inline real getX() const { return m_x; }
	inline real getY() const { return m_y; }
	inline real getZ() const { return m_z; }
	inline real getW() const { return m_w; }
	
	inline Quaternion& setIdentity()
	{
		m_w = 1.0f;
		m_x = 0.0f;
		m_y = 0.0f;
		m_z = 0.0f;
		return *this;
	}
	
	inline Quaternion& conjugate()
	{
		m_x = -m_x;
		m_y = -m_y;
		m_z = -m_z;
		return *this;
	}
	
	inline Quaternion& inverse()
	{
		if (realEqual(getMagnitude(), 1.0f) == false)
		{
			TT_WARN("Normalization was needed before inversion.");
			normalize();
		}
		
		// Inverse is equal to conjugate for normalized quaternion
		conjugate();		
		return *this;
	}
	
	// Normalization
	inline Quaternion& normalize()
	{
		real mag = getMagnitude();
		
		if (mag > 0.0f)
		{
			if (realEqual(mag, 1.0f) == false)
			{
				real oneOverMag = 1.0f / (mag + floatTolerance);
				
				m_w *= oneOverMag;
				m_x *= oneOverMag;
				m_y *= oneOverMag;
				m_z *= oneOverMag;
			}
		}
		else
		{
			TT_PANIC("Division by zero.");
			
			// For release builds
			setIdentity();
		}
		return *this;
	}

	inline Quaternion& rotateX(real p_thetaRad)
	{
		rotate(getRotationX(p_thetaRad));
		return *this;
	}
	
	inline Quaternion& rotateY(real p_thetaRad)
	{
		rotate(getRotationY(p_thetaRad));
		return *this;
	}
	
	inline Quaternion& rotateZ(real p_thetaRad)
	{
		rotate(getRotationZ(p_thetaRad));
		return *this;
	}
	
	///////////////////////////////////////////////////////////////////////////
	// Getters
	///////////////////////////////////////////////////////////////////////////	
	inline real getMagnitude() const
	{
		// Quaternion magnitude
		return math::sqrt(m_w * m_w + m_x * m_x + m_y * m_y + m_z * m_z);
	}
	
	static inline Quaternion getRotationX(real p_thetaRad)
	{
		return Quaternion(p_thetaRad, Vector3::unitX);
	}
	
	static inline Quaternion getRotationY(real p_thetaRad)
	{
		return Quaternion(p_thetaRad, Vector3::unitY);
	}
	
	static inline Quaternion getRotationZ(real p_thetaRad)
	{
		return Quaternion(p_thetaRad, Vector3::unitZ);
	}
	
	inline real getRotationAngle() const
	{
		// Return the rotation angle
		return getTimesTwo(math::acos(m_w));
	}
	
	inline Vector3 getRotationAxis() const
	{
		// Compute sin^2(theta/2).  Remember that w = cos(theta/2),
		// and sin^2(x) + cos^2(x) = 1
		real sinThetaOver2Sq = 1 - m_w * m_w;
		
		// Protect against numerical imprecision
		if (sinThetaOver2Sq <= 0.0f) 
		{
			// Identity quaternion, or numerical imprecision.  Just
			// return any valid vector, since it doesn't matter
			return Vector3(1.0f, 0.0f, 0.0f);
		}
		
		// Compute 1 / sin(theta/2)
		real oneOverSinThetaOver2 = 1.0f / sqrt(sinThetaOver2Sq);
		
		// Return axis of rotation
		return Vector3(m_x * oneOverSinThetaOver2, 
					   m_y * oneOverSinThetaOver2, 
					   m_z * oneOverSinThetaOver2);
	}
	
	inline Matrix33 getRotationMatrix() const
	{
		/*
		return Matrix33(1 - (2 * m_y * m_y) - (2 * m_z * m_z),
		                    (2 * m_x * m_y) + (2 * m_w * m_z),
		                    (2 * m_x * m_z) - (2 * m_w * m_y),
		                
		                    (2 * m_x * m_y) - (2 * m_w * m_z),
		                1 - (2 * m_x * m_x) - (2 * m_z * m_z),
		                    (2 * m_y * m_z) + (2 * m_w * m_x),
		                
		                    (2 * m_x * m_z) + (2 * m_w * m_y),
		                    (2 * m_y * m_z) - (2 * m_w * m_x),
		                1 - (2 * m_x * m_x) - (2 * m_y * m_y));
		*/
		real twoW = m_w + m_w;
		real twoX = m_x + m_x;
		real twoY = m_y + m_y;
		real twoZ = m_z + m_z;
		
		real twoWmulX = twoW * m_x;
		real twoWmulY = twoW * m_y;
		real twoWmulZ = twoW * m_z;
		real twoXmulX = twoX * m_x;
		real twoXmulY = twoX * m_y;
		real twoXmulZ = twoX * m_z;
		real twoYmulY = twoY * m_y;
		real twoYmulZ = twoY * m_z;
		real twoZmulZ = twoZ * m_z;
		
		return Matrix33(1 - (twoYmulY) - (twoZmulZ), 
		                    (twoXmulY) + (twoWmulZ),
		                    (twoXmulZ) - (twoWmulY),
		                    
		                    (twoXmulY) - (twoWmulZ), 
		                1 - (twoXmulX) - (twoZmulZ),
		                    (twoYmulZ) + (twoWmulX),
		                    
		                    (twoXmulZ) + (twoWmulY), 
		                    (twoYmulZ) - (twoWmulX), 
		                1 - (twoXmulX) - (twoYmulY));
	}
	
	
	// Convert an inertial-to-object quaternion to euler angles
	inline Vector3 getRotationVector() const
	{
		//// Original code uses h, p and b.
		//// 
		//// h -> y
		//// p -> x
		//// b -> z
		//Vector3 angles(Vector3::zero);
		//
		//// Extract sin(pitch)
		//real sp = -2.0f * (m_y * m_z + m_w * m_x);
		//
		//// Check for Gimbal lock
		//// NOTE: this was if (fabs(sp) > 0.9999f) but that doesn't work with Fixed.
		//if (math::realGreaterThan(math::fabs(sp), 1))
		//{
		//	// Looking straight up or down
		//	angles.x = halfPi * sp;
		//	
		//	// Compute heading
		//	angles.y = math::atan2(-m_x * m_z - m_w * m_y, 
		//	                       0.5f - m_y * m_y - m_z * m_z);
		//	// Set bank to zero
		//	angles.z = 0.0f;
		//}
		//else 
		//{
		//	// Compute angles
		//	angles.x = math::asin(sp);
		//	angles.y = atan2(m_x * m_z - m_w * m_y, 
		//	                 0.5f - m_x * m_x - m_y * m_y);
		//	angles.z = atan2(m_x * m_y - m_w * m_z, 
		//	                 0.5f - m_x * m_x - m_z * m_z);
		//}
		//return angles;

		/*heading = atan2(2*qy*qw-2*qx*qz , 1 - 2*qy2 - 2*qz2)
		attitude = asin(2*qx*qy + 2*qz*qw)
		bank = atan2(2*qx*qw-2*qy*qz , 1 - 2*qx2 - 2*qz2)
		
		except when qx*qy + qz*qw = 0.5 (north pole)
		which gives:
		heading = 2 * atan2(x,w)
		bank = 0
		and when qx*qy + qz*qw = -0.5 (south pole)
		which gives:
		heading = -2 * atan2(x,w)
		bank = 0
		*/

		real h = math::atan2(2 * m_y * m_w - 2 * m_x * m_z, 1 - 2 * m_y * m_y - 2 * m_z * m_z);
		real a = math::asin (2 * m_x * m_y + 2 * m_z * m_w);
		real b = math::atan2(2 * m_x * m_w - 2 * m_y * m_z, 1 - 2 * m_x * m_x - 2 * m_z * m_z);

		// Exceptions:
		if(math::realGreaterEqual(m_x * m_y + m_z * m_w, real(0.5f)))
		{
			//TT_Printf("North Pole\n");
			h = 2 * math::atan2(m_x, m_w);
			a = math::halfPi;
			b = 0.0f;
		}
		if(math::realLessEqual(m_x * m_y + m_z * m_w, real(-0.5f)))
		{
			//TT_Printf("South Pole\n");
			h = -2 * math::atan2(m_x, m_w);
			a = -math::halfPi;
			b = 0.0f;
		}

		return Vector3(b, h, a);
	}
	
	
	inline Vector3 applyRotation(const Vector3& p_input) const
	{
	#if 0
		//FIXME: is this the best implementation for this?
		//  http://www.euclideanspace.com/maths/algebra/realNormedAlgebra/quaternions/transforms/index.htm
		//  also see: http://www.sjbrown.co.uk/?article=quaternions
		
		tt::math::Quaternion input(0, p_input.x, p_input.y, p_input.z);
		
		tt::math::Quaternion conj(*this);
		conj.conjugate();
		
		tt::math::Quaternion output = ::operator*((*this), ::operator*(input, conj));
		return output.getRotationAxis();
	#else
		Matrix33 mtx = getRotationMatrix();
		return p_input * mtx;
	#endif
	}
	
	
	inline Vector3 applyInverseRotation(const Vector3& p_input) const
	{
	#if 0
		//FIXME: is this the best implementation for this?
		//  http://www.euclideanspace.com/maths/algebra/realNormedAlgebra/quaternions/transforms/index.htm
		//  also see: http://www.sjbrown.co.uk/?article=quaternions
		
		tt::math::Quaternion input(0, p_input.x, p_input.y, p_input.z);
		
		tt::math::Quaternion inv(*this);
		//inv.inverse();
		
		tt::math::Quaternion conj(inv);
		conj.conjugate();
		
		tt::math::Quaternion output = ::operator*(inv, ::operator*(input, conj));
		return output.getRotationAxis();
	#else
		Quaternion inv(*this);
		inv.inverse();
		Matrix33 mtx = inv.getRotationMatrix();
		return p_input * mtx;
	#endif
	}
	
	// Constants
	static const Quaternion identity;
	
public:
	real m_w;
	real m_x;
	real m_y;
	real m_z;
};


inline Quaternion conjugate(const Quaternion& p_rhs)
{
	return Quaternion(p_rhs.m_w, -p_rhs.m_x, -p_rhs.m_y, -p_rhs.m_z);
}


inline real dotProduct(const Quaternion& p_lhs, const Quaternion& p_rhs)
{
	return p_lhs.m_w * p_rhs.m_w + p_lhs.m_x * p_rhs.m_x +
	       p_lhs.m_y * p_rhs.m_y + p_lhs.m_z * p_rhs.m_z;
}


// Following function adapted from gamemath.com
inline Quaternion slerp(const Quaternion& p_src,
                        const Quaternion& p_dst,
                        real p_time)
{
	// Check for out-of range parameter and return edge points if so
	if (realLessEqual(p_time, real(0)))    return p_src;
	if (realGreaterEqual(p_time, real(1))) return p_dst;
	
	// Compute "cosine of angle between quaternions" using dot product
	real cosOmega = dotProduct(p_src, p_dst);
	
	// If negative dot, use -p_dst.  Two quaternions q and -q
	// represent the same rotation, but may produce
	// different slerp.  We chose q or -q to rotate using
	// the acute angle.
	Quaternion dst(p_dst);
	
	if (realLessThan(cosOmega, real(0))) 
	{
		dst = -dst;
		cosOmega = -cosOmega;
	}
	
	// We should have two unit quaternions, so dot should be <= 1.0
	TT_ASSERT(realLessEqual(cosOmega, real(1)));
	
	// Compute interpolation fraction, checking for quaternions
	// almost exactly the same
	real k0, k1;
	if (realGreaterEqual(cosOmega, real(1)))
	{
		// Very close - just use linear interpolation,
		// which will protect against a divide by zero
		k0 = 1.0f - p_time;
		k1 = p_time;
	} 
	else 
	{
		// Compute the sin of the angle using the
		// trig identity sin^2(omega) + cos^2(omega) = 1
		real sinOmega = math::sqrt(real(1) - (cosOmega * cosOmega));
		
		// Compute the angle from its sin and cosine
		real omega = math::atan2(sinOmega, cosOmega);
		
		// Compute inverse of denominator, so we only have
		// to divide once
		real oneOverSinOmega = real(1) / sinOmega;
		
		// Compute interpolation parameters
		k0 = math::sin((real(1) - p_time) * omega) * oneOverSinOmega;
		k1 = math::sin(p_time * omega) * oneOverSinOmega;
	}
	
	// Return interpolated quaternion
	return Quaternion(k0 * p_src.m_w + k1 * dst.m_w,
	                  k0 * p_src.m_x + k1 * dst.m_x,
	                  k0 * p_src.m_y + k1 * dst.m_y,
	                  k0 * p_src.m_z + k1 * dst.m_z);
}


inline Quaternion nlerp(const Quaternion& p_src,
                        const Quaternion& p_dst,
                        real              p_time)
{
	real timeLeft = 1.0f - p_time;
	
	Quaternion result(p_src.m_w * timeLeft + p_dst.m_w * p_time,
	                  p_src.m_x * timeLeft + p_dst.m_x * p_time,
	                  p_src.m_y * timeLeft + p_dst.m_y * p_time,
	                  p_src.m_z * timeLeft + p_dst.m_z * p_time);
	
	return result.normalize();
}

inline tt::streams::BOStream& operator<<(tt::streams::BOStream& s,
                                         const Quaternion& q)
{
	s << q.m_w;
	s << q.m_x;
	s << q.m_y;
	s << q.m_z;
	return s;
}


inline tt::streams::BIStream& operator>>(tt::streams::BIStream& s,
                                         Quaternion&  q)
{
	s >> q.m_w;
	s >> q.m_x;
	s >> q.m_y;
	s >> q.m_z;
	return s;
}


inline tt::math::Quaternion operator*(const Quaternion& p_lhs, 
                                      const Quaternion& p_rhs)
{
	real w = p_lhs.m_w * p_rhs.m_w - p_lhs.m_x * p_rhs.m_x -
	         p_lhs.m_y * p_rhs.m_y - p_lhs.m_z * p_rhs.m_z;

	tt::math::clamp(w, -1.0f, 1.0f);
	
	return Quaternion(w,
                p_lhs.m_w * p_rhs.m_x + p_lhs.m_x * p_rhs.m_w +
                p_lhs.m_z * p_rhs.m_y - p_lhs.m_y * p_rhs.m_z,
                p_lhs.m_w * p_rhs.m_y + p_lhs.m_y * p_rhs.m_w +
                p_lhs.m_x * p_rhs.m_z - p_lhs.m_z * p_rhs.m_x,
                p_lhs.m_w * p_rhs.m_z + p_lhs.m_z * p_rhs.m_w +
                p_lhs.m_y * p_rhs.m_x - p_lhs.m_x * p_rhs.m_y);
}


inline bool operator==(const Quaternion& p_lhs, 
                       const Quaternion& p_rhs)
{
	return(p_lhs.m_w == p_rhs.m_w && p_lhs.m_x == p_rhs.m_x &&
	       p_lhs.m_y == p_rhs.m_y && p_lhs.m_z == p_rhs.m_z);
}


inline bool operator!=(const Quaternion& p_lhs, 
                       const Quaternion& p_rhs)
{
	return (p_lhs == p_rhs) == false;
}


bool parseQuaternion(const xml::XmlNode* p_node, Quaternion* p_result);

// Namespace end
}
}


#endif  // !defined(INC_TT_MATH_QUATERNION_H)
