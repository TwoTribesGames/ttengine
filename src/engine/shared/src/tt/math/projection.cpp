#include <tt/math/projection.h>
#include <tt/math/Matrix44.h>
#include <tt/platform/tt_error.h>


namespace tt {
namespace math {

// For an explanation of the matrices see: http://www.songho.ca/opengl/gl_projectionmatrix.html
// Our matrices are transposed versions of the OpenGL matrices


void makeOrtho(math::Matrix44& p_result, real p_width, real p_height, real p_near, real p_far)
{
	TT_ASSERTMSG(p_result.m_12 == 0 &&
	             p_result.m_13 == 0 &&
	             p_result.m_14 == 0 &&
	             p_result.m_21 == 0 &&
	             p_result.m_23 == 0 &&
	             p_result.m_24 == 0 &&
	             p_result.m_31 == 0 &&
	             p_result.m_32 == 0 &&
	             p_result.m_41 == 0 &&
	             p_result.m_42 == 0,
		"Matrix must be either all zero, identity or previous projection matrix");

	const real invDepth = 1 / (p_far-p_near);

	p_result.m_11 = 2 / p_width;
	p_result.m_22 = 2 / p_height;
	p_result.m_33 = -2 * invDepth;
	p_result.m_34 =  0;
	p_result.m_43 =  -(p_far + p_near) * invDepth;
	p_result.m_44 =  1;
}


static void makePerspectiveSharedPart(math::Matrix44& p_result, real p_aspect, real p_fov, real p_33, real p_43)
{
	TT_ASSERTMSG(p_result.m_12 == 0 &&
	             p_result.m_13 == 0 &&
	             p_result.m_14 == 0 &&
	             p_result.m_21 == 0 &&
	             p_result.m_23 == 0 &&
	             p_result.m_24 == 0 &&
	             p_result.m_31 == 0 &&
	             p_result.m_32 == 0 &&
	             p_result.m_41 == 0 &&
	             p_result.m_42 == 0,
		"Matrix must be either all zero, identity or previous projection matrix");
	
	TT_ASSERT(p_aspect > 0);
	TT_ASSERT(p_fov > 0 && p_fov < pi);
	
	const real coTangent = 1.0f / tanf(p_fov * 0.5f);
	
	p_result.m_11 = coTangent / p_aspect;
	p_result.m_22 = coTangent;
	p_result.m_33 = p_33;
	p_result.m_34 = -1;
	p_result.m_43 = p_43;
	p_result.m_44 = 0;
}


void makePerspectiveOpenGL(math::Matrix44& p_result, real p_aspect, real p_fov, real p_near, real p_far)
{
	const real invDepth = 1 / (p_far-p_near);
	
	makePerspectiveSharedPart(p_result, p_aspect, p_fov, 
	                          -(p_far + p_near) * invDepth,       // 33
	                          -(2 *  p_far * p_near) * invDepth); // 43
}


void makePerspectiveDirectX(math::Matrix44& p_result, real p_aspect, real p_fov, real p_near, real p_far)
{
	const real invDepth = 1 / (p_near-p_far);
	makePerspectiveSharedPart(p_result, p_aspect, p_fov,
	                          p_far * invDepth,           // 33
	                          p_near * p_far * invDepth); // 43
}


}
}
