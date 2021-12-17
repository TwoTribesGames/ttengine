#if !defined(INC_TT_MATH_PROJECTION_H)
#define INC_TT_MATH_PROJECTION_H

#include <tt/platform/tt_types.h>

namespace tt {
namespace math {

class Matrix44;


/*! \brief Create a orthographic projection matrix centered around (0,0)
    \param p_result The resulting projection matrix will be stored in this matrix
    \param p_width  The width  of the view frustum (X axis)
    \param p_height The height of the view frustum (Y-axis)
    \param p_near   The location of the near clipping plane of the view frustum
    \param p_far    The location of the far  clipping plane of the view frustum */
void makeOrtho(math::Matrix44& p_result, real p_width, real p_height, real p_near, real p_far);


/*! \brief Create a perspective projection matrix centered around (0,0)
           OpenGL-Style (z ranges from -w to w from near to far clip plane.)
    \param p_result The resulting projection matrix will be stored in this matrix
    \param p_aspect The aspect ratio (width divided by height) of the perspective view
    \param p_fov    The field-of-view on the Y-axis of the perspective view (in radians)
    \param p_near   The location of the near clipping plane of the view frustum
    \param p_far    The location of the far  clipping plane of the view frustum */
void makePerspectiveOpenGL(math::Matrix44& p_result, real p_aspect, real p_fov, real p_near, real p_far);


/*! \brief Create a perspective projection matrix centered around (0,0)
           DirectX-Style (z ranges from 0 to w.)
    \param p_result The resulting projection matrix will be stored in this matrix
    \param p_aspect The aspect ratio (width divided by height) of the perspective view
    \param p_fov    The field-of-view on the Y-axis of the perspective view (in radians)
    \param p_near   The location of the near clipping plane of the view frustum
    \param p_far    The location of the far  clipping plane of the view frustum */
void makePerspectiveDirectX(math::Matrix44& p_result, real p_aspect, real p_fov, real p_near, real p_far);

// Namespace end
}
}


#endif  // !defined(INC_TT_MATH_PROJECTION_H)
