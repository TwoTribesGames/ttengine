#if !defined(INC_TT_PRES_ANIM2D_TRANSFORM_H)
#define INC_TT_PRES_ANIM2D_TRANSFORM_H

#include <tt/math/Matrix44.h>
#include <tt/math/Vector3.h>


namespace tt {
namespace pres {
namespace anim2d {


struct Transform
{
	tt::math::Vector3 translation;
	tt::math::Vector3 scale;
	real              rotation;
	
	Transform()
	:
	translation(0.0f, 0.0f, 0.0f),
	scale(1.0f, 1.0f, 1.0f),
	rotation(0.0f)
	{}
	
	void reset() { *this = Transform(); }
	
	math::Matrix44 calcMatrix() const
	{
		const real cosAngle(math::cos(rotation));
		const real sinAngle(math::sin(rotation));
		return math::Matrix44(scale.x *  cosAngle, scale.x * sinAngle, 0.0f         , 0.0f,
		                      scale.y * -sinAngle, scale.y * cosAngle, 0.0f         , 0.0f,
		                      0.0f               , 0.0f              , scale.z      , 0.0f,
		                      translation.x      , translation.y     , translation.z, 1.0f);
		
		/* // The code below can be used to verify the matrix above.
		math::Matrix44 test;
		test.translate(translation);
		test.rotateZ(rotation);
		test.scale(scale);
		
		TT_ASSERT(math::realEqual(mtx.m_11, test.m_11));
		TT_ASSERT(math::realEqual(mtx.m_12, test.m_12));
		TT_ASSERT(math::realEqual(mtx.m_13, test.m_13));
		TT_ASSERT(math::realEqual(mtx.m_14, test.m_14));
		
		TT_ASSERT(math::realEqual(mtx.m_21, test.m_21));
		TT_ASSERT(math::realEqual(mtx.m_22, test.m_22));
		TT_ASSERT(math::realEqual(mtx.m_23, test.m_23));
		TT_ASSERT(math::realEqual(mtx.m_24, test.m_24));
		
		TT_ASSERT(math::realEqual(mtx.m_31, test.m_31));
		TT_ASSERT(math::realEqual(mtx.m_32, test.m_32));
		TT_ASSERT(math::realEqual(mtx.m_33, test.m_33));
		TT_ASSERT(math::realEqual(mtx.m_34, test.m_34));
		
		TT_ASSERT(math::realEqual(mtx.m_42, test.m_42));
		TT_ASSERT(math::realEqual(mtx.m_43, test.m_43));
		TT_ASSERT(math::realEqual(mtx.m_44, test.m_44));
		TT_ASSERT(math::realEqual(mtx.m_41, test.m_41));
		
		return mtx;
		*/
	}
	
	math::Matrix44 multiplyWithMatrix(const math::Matrix44& p_rhs) const
	{
		const real cosAngle(math::cos(rotation));
		const real sinAngle(math::sin(rotation));
		
		return math::Matrix44(scale.x *  cosAngle * p_rhs.m_11 + scale.x * sinAngle * p_rhs.m_21,
		                      scale.x *  cosAngle * p_rhs.m_12 + scale.x * sinAngle * p_rhs.m_22,
		                      scale.x *  cosAngle * p_rhs.m_13 + scale.x * sinAngle * p_rhs.m_23,
		                      scale.x *  cosAngle * p_rhs.m_14 + scale.x * sinAngle * p_rhs.m_24,
		                      
		                      scale.y * -sinAngle * p_rhs.m_11 + scale.y * cosAngle * p_rhs.m_21,
		                      scale.y * -sinAngle * p_rhs.m_12 + scale.y * cosAngle * p_rhs.m_22,
		                      scale.y * -sinAngle * p_rhs.m_13 + scale.y * cosAngle * p_rhs.m_23,
		                      scale.y * -sinAngle * p_rhs.m_14 + scale.y * cosAngle * p_rhs.m_24,
		                      
		                      scale.z * p_rhs.m_31,
		                      scale.z * p_rhs.m_32,
		                      scale.z * p_rhs.m_33,
		                      scale.z * p_rhs.m_34,
		                      
		                      translation.x * p_rhs.m_11 + translation.y * p_rhs.m_21 + 
		                      translation.z * p_rhs.m_31 + p_rhs.m_41,
		                      translation.x * p_rhs.m_12 + translation.y * p_rhs.m_22 + 
		                      translation.z * p_rhs.m_32 + p_rhs.m_42,
		                      translation.x * p_rhs.m_13 + translation.y * p_rhs.m_23 + 
		                      translation.z * p_rhs.m_33 + p_rhs.m_43,
		                      translation.x * p_rhs.m_14 + translation.y * p_rhs.m_24 + 
		                      translation.z * p_rhs.m_34 + p_rhs.m_44);
	}
};

//namespace end
}
}
}

#endif // !defined(INC_TT_PRES_ANIM2D_TRANSFORM_H)
