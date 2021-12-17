///////////////////////////////////////////////////////////////////////////////
///
///  File name     : Camera.h
///  Platform      : Windows (DX 9)
///  Project       : Rubiks' Cube
///  Author        : Adrian Brown, M.C. Bouterse (marco@twotribes.com)
///  Company       : Two Tribes (www.twotribes.com)
///  Created On    : 2008-01-14
///  Description   : Camera Class
///

#include <OpenGLES/ES1/gl.h>
#include <OpenGLES/ES1/glext.h>

// Include complement header file
#include <tt/engine/scene/Camera.h>

// include additional files
#include <tt/platform/tt_printf.h>
#include <tt/math/math.h>
#include <tt/engine/file/File.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/engine/Memory.h>
#include <tt/engine/renderer/MatrixStack.h>
#include <tt/engine/scene/Instance.h>
#include <tt/engine/physics/Ray.h>

#include <tt/math/math.h>
#include <tt/math/tt_cmath.h>

// namespace definition
namespace tt {
namespace engine {
namespace scene {

Camera::ProjectionType Camera::m_projectionType = Camera::ProjectionType_Perspective;
static const real defaultNear = 1.0f;
static const real defaultFar  = 4096.0f;

/*! \brief Base constructor for Camera

	The constructor will automatically set the reference count to 1. All initial 
	data is set to defaults and the base constructor is called to add this to 
	the object lists

    \return Nothing.
*/
Camera::Camera() 
: 
SceneObject(Type_Camera),
m_lookAt(0,0,0),
m_x(0),
m_y(0),
m_width(640),
m_height(480),
m_mode(Mode_FieldOfView),
m_left(-320),
m_right(320),
m_top(240),
m_bottom(-240),
m_fov(0.0f),
m_nearPlane(defaultNear),
m_farPlane(defaultFar),
m_worldPosition(0,0,0),
m_instance(0)
{
	// Set the camera details
	setPosition(0.0f, 0.0f, 5.0f);
	uploadViewport();
}

/*! \brief Destructor for Camera
	
	Frees up the memory and removes from list

    \return Nothing.
*/
Camera::~Camera()
{
}

void Camera::setPerspectiveProjection(s32 p_width, s32 p_height, 
									  real p_near, real p_far, 
									  real p_fovy)
{
	// Store Settings
	m_projectionType = ProjectionType_Perspective;
	m_nearPlane = p_near;
	m_farPlane  = p_far;
	m_width     = static_cast<real>(p_width);
	m_height    = static_cast<real>(p_height);
	m_fov       = math::degToRad(p_fovy);

	// Compute aspect ratio
	m_aspect = m_width / m_height;

	//m_projection.m_43 *= 2;
	
	glViewport(0, 0, m_width, m_height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();	
	
	double ymax = m_nearPlane * tt::math::tan(m_fov * 0.5f);
	double ymin = - ymax;
	double xmin = ymin * (m_aspect);
	double xmax = ymax * (m_aspect);
	glFrustumf(xmin, xmax, ymin, ymax, m_nearPlane, m_farPlane);
	//glFrustumf(-2,2,-2,2,1,4096);
}

void Camera::setOrthographicProjection(s32 p_width, s32 p_height, 
									  real p_near, real p_far)
{
	// Store Settings
	m_projectionType = ProjectionType_Orthogonal;
	m_nearPlane = p_near;
	m_farPlane  = p_far;
	m_width     = static_cast<real>(p_width);
	m_height    = static_cast<real>(p_height);
	
	m_aspect = m_width / m_height;

	glViewport(0, 0, m_width, m_height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrthof(0, m_width, 0, m_height, m_nearPlane, m_farPlane);
}

void Camera::resetPerspective()
{
	// Store Settings
	m_projectionType = ProjectionType_Perspective;

	// Simulating DS matrix setup
	//m_projection.m_43 *= 2;
	
	glViewport(0, 0, m_width, m_height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
		 
	double ymax = m_nearPlane * tt::math::tan(m_fov * 0.5f);
	double ymin = - ymax;
	double xmin = ymin * (m_aspect);
	double xmax = ymax * (m_aspect);
	glFrustumf(xmin, xmax, ymin, ymax, m_nearPlane, m_farPlane);
}

void Camera::resetOrtho()
{
	// Store Settings
	m_projectionType = ProjectionType_Orthogonal;

	glViewport(0, 0, m_width, m_height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrthof(0, m_width, 0, m_height, m_nearPlane, m_farPlane);	
}


bool Camera::load(file::File* p_file)
{
	// Load the base part of the object
	if (SceneObject::load(p_file) == false)
	{
		return false;
	}

	// Now we can load the field of view
	if (p_file->read(&m_fov, sizeof(m_fov)) != sizeof(m_fov) )
	{
		return false;
	}
	m_fov = math::degToRad(m_fov);

	// Now we can load the ranges
	if (p_file->read(&m_nearPlane, sizeof(m_nearPlane)) != sizeof(m_nearPlane) )
	{
		return false;
	}

	if (p_file->read(&m_farPlane, sizeof(m_farPlane)) != sizeof(m_farPlane) )
	{
		return false;
	}

	// Load the target - not used here
	s16 targetid;
	if (p_file->read(&targetid, sizeof(targetid)) != sizeof(targetid) )
	{
		return false;
	}

	return true;
}

bool Camera::isPointInFrustum(const math::Vector3& p_point) const
{
	using tt::math::Vector3;
	real pcz, pcx, pcy, aux;

	// compute vector from camera position to p_point
	Vector3 v(p_point - m_worldPosition);

	// compute and test the Z coordinate
	pcz = dotProduct(v, -m_radarZ);
	if (pcz > m_farPlane || pcz < m_nearPlane)
	{
		//TT_Printf("Reject: Z\n");
		return false;
	}

	// compute and test the Y coordinate
	pcy = dotProduct(v, m_radarY);
	aux = pcz * m_radarTangent;
	if (pcy > aux || pcy < -aux)
	{
		//TT_Printf("Reject: Y\n");
		return false;
	}
		
	// compute and test the X coordinate
	pcx = dotProduct(v, m_radarX);
	aux = aux * m_aspect;
	if (pcx > aux || pcx < -aux)
	{
		//TT_Printf("Reject: X\n");
		return false;
	}

	return true;
}


bool Camera::isSphereInFrustum(const math::Vector3& p_pos, real p_radius) const
{
	using tt::math::Vector3;

	real d1, d2;
	real az, ax, ay, zz1, zz2;

	// compute vector from camera position to p_pos
	Vector3 v(p_pos - m_worldPosition);

	az = dotProduct(v, -m_radarZ);
	if (az > (m_farPlane + p_radius) || az < (m_nearPlane - p_radius))
	{
		//TT_Printf("Reject: Z\n");
		return false;
	}

	ax = dotProduct(v, m_radarX);
	zz1 = az * m_radarTangent * m_aspect;
	d1 = m_radarSphereFactorX * p_radius;
	if (ax > (zz1 + d1) || ax < (-zz1 - d1))
	{
		//TT_Printf("Reject: X\n");
		return false;
	}

	ay = dotProduct(v, m_radarY);
	zz2 = az * m_radarTangent;
	d2 = m_radarSphereFactorY * p_radius;
	if (ay > (zz2 + d2) || ay < (-zz2 - d2))
	{
		//TT_Printf("Reject: Y\n");
		return false;
	}

	// FIXME: Perhaps use intersection?
	/*
	if (az > farD - radius || az < nearD+radius)
		result = INTERSECT;
	if (ay > zz2-d2 || ay < -zz2+d2)
		result = INTERSECT;
	if (ax > zz1-d1 || ax < -zz1+d1)
		result = INTERSECT;
	*/
	return true;
}


/*! \brief Updates the camera
	
	Recalculates the camera matrix based on the position details.  
	NOTE: This should check to see if the camera has moved first. */
void Camera::update(Instance* p_instance,
					const animation::AnimationPtr& p_animation)
{
	using tt::math::Vector3;

	TT_ASSERTMSG(p_animation == 0, "This camera anim is not supported");

	// Get the at position
	m_worldPosition = getPosition();

	// Normalise a look vector
	Vector3 vLook(0,0,0);
	if (m_instance != 0)
	{
		vLook = m_worldPosition - m_instance->getActualPosition();
	}
	else
	{
		vLook = m_worldPosition - getLookAt();
	}
	
	if(vLook.length() > 0.0f)
	{
		vLook.normalize();
	}
	else
	{
		vLook = math::Vector3::forward;
	}
	
	// Compute up vector
	m_up = Vector3(0,1,0);

	// Are we too close to coplanear?
	if (math::realEqual(vLook.x, 0) && math::realEqual(vLook.z, 0))
	{
		m_up.setValues(-vLook.y, 0, 0);
	}
	
	math::Vector3 vRight = crossProduct(m_up, vLook);
	vRight.normalize();
	
	m_up = crossProduct(vLook, vRight);

	m_worldMatrix.setIdentity();
	
	// Now set the matrix
	m_worldMatrix.m_11 = vRight.x;
	m_worldMatrix.m_12 = m_up.x;
	m_worldMatrix.m_13 = vLook.x;
	m_worldMatrix.m_14 = 0.0f;
	
	m_worldMatrix.m_21 = vRight.y;
	m_worldMatrix.m_22 = m_up.y;
	m_worldMatrix.m_23 = vLook.y;
	m_worldMatrix.m_24 = 0.0f;
	
	m_worldMatrix.m_31 = vRight.z;
	m_worldMatrix.m_32 = m_up.z;
	m_worldMatrix.m_33 = vLook.z;
	m_worldMatrix.m_34 = 0.0f;
	
	m_worldMatrix.m_41 = -dotProduct(vRight, m_worldPosition);
	m_worldMatrix.m_42 = -dotProduct(m_up, m_worldPosition);
	m_worldMatrix.m_43 = -dotProduct(vLook, m_worldPosition);
	m_worldMatrix.m_44 = 1.0f;

	m_projectionView = m_worldMatrix * m_projection;
	m_inverseProjectionView = m_projectionView.getInverse();

	// FIXME: Only do this when camera is changed
	// Frustrum calculations, Radar approach as described in an article in Game Programming Gems 5
	// taken from http://zach.in.tu-clausthal.de/teaching/cg_literatur/lighthouse3d_view_frustum_culling/index.html
	m_radarTangent = tt::math::tan(m_fov * 0.5f);

	m_radarZ = m_worldPosition - m_lookAt;
	m_radarZ.normalize();

	m_radarX = crossProduct(m_up, m_radarZ);
	m_radarX.normalize();
	m_radarY = crossProduct(m_radarZ, m_radarX);

	m_radarSphereFactorY = 1.0f / tt::math::cos(m_fov);

	// compute half of the the horizontal field of view and sphereFactorX 
	real anglex = tt::math::atan(m_radarTangent * m_aspect);
	m_radarSphereFactorX = 1.0f / tt::math::cos(anglex);
}


void Camera::calculatePerspectiveMatrix()
{
	// Build the projection matrix
	switch (m_mode)
	{
		case Mode_FieldOfView:
		{
			real width  = math::cos(m_fov / 2.0f) / math::sin(m_fov / 2.0f);
			real height = (m_width / m_height) * width;

			real fq = m_farPlane / (m_farPlane - m_nearPlane);
			real fqn = fq * m_nearPlane;

			// Work out the matrix
			m_projection.setIdentity();
			m_projection.m_11 = width;
			m_projection.m_22 = height;
			m_projection.m_33 = fq;
			m_projection.m_34 = 1.0f;
			m_projection.m_43 = -fqn;
			m_projection.m_44 = 0.0f;
		}
		break;

		case Mode_OffCentre:
		{
			real fq = m_farPlane / (m_farPlane - m_nearPlane);
			real fqn = fq * m_nearPlane;

			// Work out the matrix
			m_projection.setIdentity();
			m_projection.m_11 = (m_nearPlane * 2) / 
								((m_right * 10) - (m_left * 10));
			m_projection.m_22 = (m_nearPlane * 2) / 
								((m_top * 10) - (m_bottom * 10));
			m_projection.m_31 = ((m_left * 10.0f) + (m_right * 10)) / 
								((m_left * 10) - (m_right * 10));
			m_projection.m_32 = ((m_top * 10.0f) + (m_bottom * 10)) / 
								((m_bottom * 10) - (m_top * 10));
			m_projection.m_33 = fq;
			m_projection.m_34 = 1.0f;
			m_projection.m_43 = -fqn;
			m_projection.m_44 = 0.0f;
		}
		break;
	}

	resetProjectionMatrixType();
}


/*! \brief Sets the camera matrices to hardware
	
	Loads the project / view matrix to the hardware, also clears the world matrix to identity */
void Camera::uploadCameraMatrices()
{
	uploadViewMatrix();
}


void Camera::uploadOrthoMatrix()
{
	using renderer::MatrixStack;

	if (m_projectionType != ProjectionType_Orthogonal)
	{
		MatrixStack::getInstance()->setMode(MatrixStack::Mode_Projection);
		MatrixStack::getInstance()->load44(m_orthoMatrix);
		MatrixStack::getInstance()->setMode(MatrixStack::Mode_Position);
		
		m_projectionType = ProjectionType_Orthogonal;
	}
}

void Camera::uploadProjectionMatrix()
{
	using renderer::MatrixStack;

	if (m_projectionType != ProjectionType_Perspective)
	{
		MatrixStack::getInstance()->setMode(MatrixStack::Mode_Projection);
		MatrixStack::getInstance()->load44(m_projection);
		MatrixStack::getInstance()->setMode(MatrixStack::Mode_Position);

		m_projectionType = ProjectionType_Perspective;
	}
}

void Camera::uploadViewMatrix()
{
	renderer::MatrixStack::getInstance()->load44(m_worldMatrix);
}

void Camera::uploadViewport()
{
	// NOT IMPLEMENTED
}


void Camera::renderObject(Instance*)
{
	// We dont really render a camera
}

/*! \brief Get View Matrix
	
	Returns the current view matrix

    \return The current view matrix of this camera.
*/
math::Matrix44 Camera::getViewMatrix()
{
	return m_worldMatrix;
}


/*! \brief Get Projection Matrix
	
	Returns the current projection matrix

    \return The current projection matrix of this camera.
*/
math::Matrix44 Camera::getProjectionMatrix()
{
	return m_projection;
}


/*! \brief Converts a 3D point into 2D screen coordinates
	
	Calculates the 2D screen coordinates of a 3d point.  

	\param v - The 3D Position
	\param x - The X coordinate
	\param y - The Y coordinate

    \return Nothing
*/
void Camera::convert3Dto2D(const math::Vector3& p_pos, s32& p_x, s32& p_y) const
{
	// Convert to screen 3D
	math::Vector3 screen_pos = p_pos * m_projectionView;
	
	// TODO: TEST THIS
	real w = (p_pos.x * m_projectionView.m_14) + 
			 (p_pos.y * m_projectionView.m_24) + 
			 (p_pos.z * m_projectionView.m_34) + 
			  m_projectionView.m_44;

	// FIXME: A Vector4 * Matrix44 should do the same.
	
	// Now apply the matrix
	real x = ((screen_pos.x + w) / (2 * w));
	real y = ((screen_pos.y + w) / (2 * w));
	
	//TT_Printf("Camera::convert3Dto2D: x: %f, y: %f\n", (float)x, (float)y);
	
	// convert to pixel coordinates
	p_x = static_cast<s32>(x * 255);         // Replace 255 with (m_width  - 1)?
	p_y = static_cast<s32>(191 - (y * 191)); // Replace 191 with (m_height - 1)?
	/*TT_Printf("Camera::convert3Dto2D: p_x: %f, p_y: %f\n",
	          (float)p_x, (float)p_y);*/
}

bool Camera::scrPosToWorldLine(int p_x, int p_y, 
                               tt::math::Vector3* p_near, 
                               tt::math::Vector3* p_far) const
{	
	TT_NULL_ASSERT(p_near);

	/* OLD CODE
	// conversion to normalized screen coordinate system from BG screen coordinate system
    NNS_G3dGlbGetViewPort(&x1, &y1, &x2, &y2);
    dx = x2 - x1;
    dy = y2 - y1;
	
    x = FX_Div((px - x1) << FX32_SHIFT, dx << FX32_SHIFT);
    y = FX_Div((py + y1 - 191) << FX32_SHIFT, -dy << FX32_SHIFT);
	*/
	
    // Normalize screen coords.
    real x = (p_x - m_x) / (m_width  - 1);
	real y = (p_y + m_y - m_height) / -(m_height - 1);
	
	/* OLD CODE
	if (x < 0 || y < 0 || x > FX32_ONE || y > FX32_ONE)
    {
        rval = -1;
    }
    else
    {
        rval = 0;
    }
	*/

	bool insideViewport = true;
	if (x < 0 || y < 0 || x > 1 || y > 1)
	{
		insideViewport = false; // (Outside viewport!)
	}
	
	/* OLD CODE
    // Becomes a +- 1 cube
    x = (x - FX32_HALF) * 2;
    y = (y - FX32_HALF) * 2;
	*/
	
    // Becomes a +- 1 cube
    x = (x - 0.5f) * 2;
    y = (y - 0.5f) * 2;
	
	// Get the inverse matrix of the matrix that multiplied the projection matrix and the camera matrix
	/* OLD CODE
    m = NNS_G3dGlbGetInvVP();
	*/
	const math::Matrix44& m = m_inverseProjectionView;
	
    // The point on the NEAR plane is (x, y, -FX32_ONE, FX32_ONE)
    // The point on the FAR plane is (x, y,  FX32_ONE, FX32_ONE)
    // Apply the inverse matrix and calculate the point in the world coordinate system
	/* OLD CODE
	wNear   = m->_33 + (fx32)(((fx64)x * m->_03 + (fx64)y * m->_13) >> FX32_SHIFT);
    FX_InvAsync(wNear - m->_23);
	*/
	real wNear = m.m_44 + (x * m.m_14 + y * m.m_24);
    
	//FX_InvAsync((wNear - m.m_34).getValue());
	/*
	typedef math::Fixed<64, 32> realHighPrecision;
	realHighPrecision invWNear = realHighPrecision(1) / 
	                             realHighPrecision(wNear - m.m_34);
	*/
	
	math::Vector3 nearVec;
    nearVec.x = m.m_41 + (x * m.m_11 + y * m.m_21);
    nearVec.y = m.m_42 + (x * m.m_12 + y * m.m_22);
    nearVec.z = m.m_43 + (x * m.m_13 + y * m.m_23);
	
	real wFar = 0;
	
	math::Vector3 farVec;
    if (p_far != 0)
    {
        farVec.x = nearVec.x + m.m_31;
        farVec.y = nearVec.y + m.m_32;
        farVec.z = nearVec.z + m.m_33;
        wFar     = wNear     + m.m_34;
    }
	
    nearVec.x -= m.m_31;
    nearVec.y -= m.m_32;
    nearVec.z -= m.m_33;
	
	//fx64c invWNear = FX_GetInvResultFx64c();
	real invWNear = 1.0f / (wNear - m.m_34);
	
	/*
	if (p_far)
	{
		FX_InvAsync(wFar.getValue());
	}*/
	
	typedef math::Fixed<64, 32> realHighPrecision;
	p_near->x = nearVec.x * invWNear;
	p_near->y = nearVec.y * invWNear;
	p_near->z = nearVec.z * invWNear;
	
    if (p_far != 0)
    {
        //fx64c invWFar = FX_GetInvResultFx64c();
		/*
		realHighPrecision invWFar = realHighPrecision(1) / 
		                            realHighPrecision(wFar);
		*/
		if (wFar != 0)
		{
			real invWFar = 1 / wFar;
			
			p_far->x = farVec.x * invWFar;
			p_far->y = farVec.y * invWFar;
			p_far->z = farVec.z * invWFar;
		}
		else
		{
			real invWFar = 2 * m_farPlane;
			
			TT_Printf("invWFar: %f, m_farPlane: %f\n", realToFloat(invWFar), realToFloat(m_farPlane));
			
			p_far->x = farVec.x * invWFar;
			p_far->y = farVec.y * invWFar;
			p_far->z = farVec.z * invWFar;
			
			TT_Printf("farVec x: %f, y: %f, z: %f\n", 
			          realToFloat(farVec.x), realToFloat(farVec.y), realToFloat(farVec.z));
			TT_Printf("p_far x: %f, y: %f, z: %f\n", 
			          realToFloat(p_far->x), realToFloat(p_far->y), realToFloat(p_far->z));
		}
    }
	
    return insideViewport;
}


bool Camera::scrPosToWorldLine(int p_x, int p_y, physics::Ray& p_ray_OUT) const
{
	math::Vector3 start;
	math::Vector3 end;
	bool ret = scrPosToWorldLine(p_x, p_y, &start, &end);
	p_ray_OUT.setOriginEnd(start, end);
	return ret;
}


/*! \brief Gets the actual position
	
	Returns a Vector3 containing the actual position of the object in 
	world space, this takes into account animation and default positions.

    \return The Vector3 of the position
*/
math::Vector3 Camera::getActualPosition() const
{
	return m_worldPosition;
}


real Camera::getPixelPerfectDistance() const
{
	using renderer::Renderer;

	return getFullScreenDistance(static_cast<real>(
	                               Renderer::getInstance()->getScreenHeight()));
}


real Camera::getFullScreenDistance(real p_height) const
{
	return (p_height * 0.5f) / math::tan(0.5f * m_fov);
}


math::Vector3 Camera::getWorldFromScreen(real p_screenX, real p_screenY,
                                         real p_worldZ) const
{
	using renderer::Renderer;

	Renderer* renderer = Renderer::getInstance();
	
	real screenWidth  = static_cast<real>(renderer->getScreenWidth());
	real screenHeight = static_cast<real>(renderer->getScreenHeight());
	
	real dist = m_worldPosition.z - p_worldZ;
	
	// Calculate the width and height of the camera area at the world Z position
	real camHeight = 2.0f * (math::tan(0.5f * m_fov) * dist);
	real camWidth  = m_aspect * camHeight;
	
	// Translate the screen-space position to a camera-space position
	p_screenX -= screenWidth  * 0.5f;
	p_screenY -= screenHeight * 0.5f;
	
	// Scale the position according to the camera area
	real worldX = (p_screenX / screenWidth ) * camWidth;
	real worldY = (p_screenY / screenHeight) * camHeight;
	
	return math::Vector3(m_worldPosition.x + worldX, m_worldPosition.y - worldY, p_worldZ);
}


// Factory function
CameraPtr Camera::createPerspective(const math::Vector3& p_position, 
									const math::Vector3& p_lookAt, 
									s32  p_width, s32  p_height, 
									real p_near,  real p_far, 
									real p_fov)
{
	Camera* camera = TT_New Camera;
	camera->setPerspectiveProjection(p_width, p_height, p_near, p_far, p_fov);
	camera->setPosition(p_position);
	camera->setLookAt(p_lookAt);
	camera->update();
	
	return CameraPtr(camera);
}


CameraPtr Camera::createOrtho(const math::Vector3& p_position, 
                              const math::Vector3& p_lookAt, 
                              s32  p_width, s32  p_height, 
							  real p_near,  real p_far)
{
	Camera* camera = TT_New Camera;
	camera->setOrthographicProjection(p_width, p_height, p_near, p_far);
	camera->setPosition(p_position);
	camera->setLookAt(p_lookAt);
	camera->update();
	
	return CameraPtr(camera);
}

} // End of namespace scene
} // End of namespace engine
} // End of namespace tt
