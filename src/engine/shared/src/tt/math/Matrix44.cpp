#include <tt/math/Matrix44.h>

namespace tt {
namespace math {

// Constant vector definitions
const Matrix44 Matrix44::zero(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);
const Matrix44 Matrix44::identity(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1); 

Matrix44 Matrix44::getMayaTextureMatrix(real p_scaleS, real p_scaleT,
										real p_rotation,
										real p_translateS, real p_translateT)
{
	Matrix44 result;

	real rotationInRadians(degToRad(p_rotation));

	real cosR(math::cos(rotationInRadians));
	real sinR(math::sin(rotationInRadians));

	result.m_11 =  p_scaleS * cosR;
	result.m_12 = -p_scaleT * sinR;

	result.m_21 =  p_scaleS * sinR;
	result.m_22 =  p_scaleT * cosR;

	result.m_41 = p_scaleS * (-0.5f * sinR - 0.5f * cosR + 0.5f - p_translateS);
	result.m_42 = p_scaleT * ( 0.5f * sinR - 0.5f * cosR - 0.5f + p_translateT) + 1.0f;

	return result;
}

}
}
