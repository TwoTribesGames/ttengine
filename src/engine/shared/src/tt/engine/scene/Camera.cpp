#include <tt/engine/scene/Camera.h>
#include <tt/engine/renderer/Renderer.h>


#include <tt/engine/renderer/FixedFunction.h>
#include <tt/engine/renderer/MatrixStack.h>
#include <tt/engine/scene/Instance.h>
#include <tt/engine/physics/Ray.h>
#include <tt/math/math.h>
#include <tt/math/projection.h>
#include <tt/fs/File.h>


namespace tt {
namespace engine {
namespace scene {


Camera::~Camera()
{
}


void Camera::select()
{
	// Load the projection & view matrices
#if !defined(TT_BUILD_FINAL)
	if (m_debugFOVEnabled)
	{
		renderer::FixedFunction::setProjectionMatrix(m_projectionDebugFOV);
	}
	else
#endif
	{
		renderer::FixedFunction::setProjectionMatrix(m_projection);
	}
	renderer::FixedFunction::setViewMatrix(m_worldMatrix);
	renderer::FixedFunction::setCameraPosition(m_worldPosition);
}


void Camera::setFOV(real p_fov)
{
	TT_ASSERT(m_projectionType == ProjectionType_Perspective);
	TT_ASSERT(p_fov > 0.0f && p_fov < 180.0f);
	
	m_fov = math::degToRad(p_fov);
	
	computeProjectionMatrix();
	
	// TODO: Only update when requested (dirty flag)
	computeProjectionViewMatrix();
	m_frustum.updateRatio(m_fov, m_aspect);
}


void Camera::setDebugFOV(real p_fov)
{
	TT_ASSERT(m_projectionType == ProjectionType_Perspective);
	TT_ASSERT(p_fov > 0.0f && p_fov < 180.0f);
	
	m_debugFOV = math::degToRad(p_fov);
	
	computeProjectionMatrix();
	
	// TODO: Only update when requested (dirty flag)
	computeProjectionViewMatrix();
	m_frustum.updateRatio(m_debugFOV, m_aspect);
}


void Camera::setDebugFOVEnabled(bool p_enabled)
{
#if !defined(TT_BUILD_FINAL)
	m_debugFOVEnabled = p_enabled;
	m_frustum.updateRatio(m_debugFOVEnabled ? m_debugFOV : m_fov, m_aspect);
#else
	m_debugFOVEnabled = false;
	(void)p_enabled;
#endif
}


void Camera::setNearFar(real p_near, real p_far)
{
	TT_ASSERTMSG(p_near < p_far, "Near (%f) should be smaller than Far (%f)!", p_near, p_far);
	
	// Store value
	m_nearPlane = p_near;
	m_farPlane  = p_far;
	
	// Update projection matrix
	computeProjectionMatrix();
	
	m_frustum.updatePlanes(m_nearPlane, m_farPlane);
	computeProjectionViewMatrix();
}


void Camera::setNear(real p_near)
{
	setNearFar(p_near, m_farPlane);
}


void Camera::setFar(real p_far)
{
	setNearFar(m_nearPlane, p_far);
}


void Camera::setViewPort(real p_x, real p_y, real p_width, real p_height)
{
	m_x = p_x;
	m_y = p_y;
	m_width = p_width;
	m_height = p_height;
	m_aspect = p_width / p_height;
	
	// Update projection matrix
	computeProjectionMatrix();
	computeProjectionViewMatrix();
	m_frustum.updateRatio(m_fov, m_aspect);
}


bool Camera::isVisible(const renderer::Sphere& p_sphere) const
{
	if(m_projectionType == ProjectionType_Perspective)
	{
		return m_frustum.containsSphere(p_sphere.getPosition(), p_sphere.getRadius());
	}
	else
	{
		// Orthographic projection
		return isSphereInOrtho(p_sphere.getPosition(), p_sphere.getRadius());
	}
}


bool Camera::isVisible(const math::Vector3& p_pos, real p_radius) const
{
	if(m_projectionType == ProjectionType_Perspective)
	{
		return m_frustum.containsSphere(p_pos, p_radius);
	}
	else
	{
		// Orthographic projection
		return isSphereInOrtho(p_pos, p_radius);
	}
}


bool Camera::isVisible(const math::VectorRect& p_rect, real p_furthestZ) const
{
	const real cameraZ = getPosition().z;
	if(cameraZ <= p_furthestZ)
	{
		// Z is behind camera
		return false;
	}

	math::VectorRect cullRect;
	if(m_projectionType == ProjectionType_Perspective)
	{
		cullRect = m_frustum.getCullRect(getPosition().z - p_furthestZ);
	}
	else
	{
		cullRect.setWidth(m_width);
		cullRect.setHeight(m_height);
	}
	cullRect.setCenterPosition(math::Vector2(getPosition().x, getPosition().y));

	return p_rect.intersects(cullRect);
}


void Camera::update(const animation::AnimationPtr& p_animation, Instance*)
{
	TT_ASSERTMSG(p_animation == 0, "This camera anim is not supported");
	
	// Only update if something changed
	if(m_lookAtChanged || m_worldPosition != getPosition())
	{
		// Get the at position
		m_worldPosition = getPosition();
		
		computeViewMatrix();
		computeProjectionViewMatrix();
		
		m_lookAtChanged = false;
		m_updateInverseProjection = true;
	}
}


void Camera::convert3Dto2D(const math::Vector3& p_pos, s32& p_x, s32& p_y, bool p_allowDebugFOV) const
{
#if !defined(TT_BUILD_FINAL)
	const math::Matrix44& projectionView = m_debugFOVEnabled && p_allowDebugFOV ? m_projectionViewDebugFOV :
	                                                                              m_projectionView;
#else
	const math::Matrix44& projectionView = m_projectionView;
	(void)p_allowDebugFOV;
#endif
	
	// Convert to screen 3D
	math::Vector3 screen_pos = p_pos * projectionView;
	
	// TODO: TEST THIS
	real w = (p_pos.x * projectionView.m_14) +
			 (p_pos.y * projectionView.m_24) +
			 (p_pos.z * projectionView.m_34) +
			 projectionView.m_44;
	
	// FIXME: A Vector4 * Matrix44 should do the same.
	
	// Now apply the matrix
	real x = ((screen_pos.x + w) / (2 * w));
	real y = ((screen_pos.y + w) / (2 * w));
	
	//TT_Printf("Camera::convert3Dto2D: x: %f, y: %f\n", (float)x, (float)y);
	
	// convert to pixel coordinates
	p_x = static_cast<s32>(x * (m_width - 1));
	p_y = static_cast<s32>((m_height - 1) - (y * (m_height - 1)));
}


bool Camera::convert3Dto2D(math::VectorRect& p_box_OUT, const real p_depth, bool p_allowDebugFOV) const
{
	if (math::realEqual(p_depth, 0.0f))
	{
		// No perspective correction required when depth is 0
		return true;
	}
	
	// FIXME: Rewrite this into a clear/simplified method
	s32 x0, y0, x1, y1;
	convert3Dto2D(math::Vector3(p_box_OUT.getLeft(), p_box_OUT.getTop(), p_depth), x0, y0, p_allowDebugFOV);
	math::Vector3 tl(getWorldFromScreen((float)x0, (float)y0));
	
	convert3Dto2D(math::Vector3(p_box_OUT.getRight(), p_box_OUT.getBottom(), p_depth), x1, y1, p_allowDebugFOV);
	math::Vector3 br(getWorldFromScreen((float)x1, (float)y1));
	
	// FIXME: Apparently the coordinates can 'overshoot' causing the tl to be bigger than br.
	// In that case the rectangle is invalid. Perhaps rewrite this in something more elegant?
	bool isValid = true;
	if (tl.y < br.y)
	{
		p_box_OUT.setTopBottom(tl.y, br.y);
	}
	else
	{
		p_box_OUT.setTopBottom(br.y, tl.y);
		isValid = false;
	}
	
	if (tl.x < br.x)
	{
		p_box_OUT.setLeftRight(tl.x, br.x);
	}
	else
	{
		p_box_OUT.setLeftRight(br.x, tl.x);
		isValid = false;
	}
	
	return isValid;
}


bool Camera::scrPosToWorldLine(s32 p_x, s32 p_y, math::Vector3* p_near, math::Vector3* p_far)
{
	TT_NULL_ASSERT(p_near);

	if (m_updateInverseProjection)
	{
		// the code further down relies on the OpenGL way of doing projection matrices,
		// so we force an OpenGL matrix here to make picking behaviour consistent cross-platform.
		tt::math::Matrix44 perspectiveView;
		math::makePerspectiveOpenGL(perspectiveView, m_aspect, m_fov, m_nearPlane, m_farPlane);
		perspectiveView = m_worldMatrix * perspectiveView;

		m_inverseProjectionView = perspectiveView.getInverse();
		m_updateInverseProjection = false;
	}

	// Normalize screen coords.
	real x = (p_x - m_x) / (m_width  - 1);
	real y = (p_y + m_y - m_height) / -(m_height - 1);

	bool insideViewport = true;
	if (x < 0 || y < 0 || x > 1 || y > 1)
	{
		insideViewport = false; // (Outside viewport!)
	}

	// Becomes a +- 1 cube
	x = (x - 0.5f) * 2;
	y = (y - 0.5f) * 2;

	// Get the inverse matrix of the product of the projection matrix and the camera matrix
	const math::Matrix44& m = m_inverseProjectionView;

	// The point on the NEAR plane is (x, y, -FX32_ONE, FX32_ONE)
	// The point on the FAR plane is (x, y,  FX32_ONE, FX32_ONE)
	// Apply the inverse matrix and calculate the point in the world coordinate system
	real wNear = m.m_44 + (x * m.m_14 + y * m.m_24);

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

	real invWNear = 1.0f / (wNear - m.m_34);

	p_near->x = nearVec.x * invWNear;
	p_near->y = nearVec.y * invWNear;
	p_near->z = nearVec.z * invWNear;

	if (p_far != 0)
	{
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

			//TT_Printf("invWFar: %f, m_farPlane: %f\n", realToFloat(invWFar), realToFloat(m_farPlane));

			p_far->x = farVec.x * invWFar;
			p_far->y = farVec.y * invWFar;
			p_far->z = farVec.z * invWFar;

			/*TT_Printf("farVec x: %f, y: %f, z: %f\n", 
				realToFloat(farVec.x), realToFloat(farVec.y), realToFloat(farVec.z));
			TT_Printf("p_far x: %f, y: %f, z: %f\n", 
				realToFloat(p_far->x), realToFloat(p_far->y), realToFloat(p_far->z));*/
		}
	}

	return insideViewport;
}


bool Camera::scrPosToWorldLine(s32 p_x, s32 p_y, physics::Ray& p_ray_OUT)
{
	math::Vector3 start;
	math::Vector3 end;

	bool ret = scrPosToWorldLine(p_x, p_y, &start, &end);
	
	p_ray_OUT.setOriginEnd(start, end);
	
	return ret;
}


real Camera::getPixelPerfectDistance() const
{
	return getFullScreenDistance(
		static_cast<real>(renderer::Renderer::getInstance()->getScreenHeight()));
}


real Camera::getFullScreenDistance(real p_height) const
{
	if(m_projectionType == ProjectionType_Perspective)
	{
		return (p_height * 0.5f) / math::tan(0.5f * m_fov);
	}
	else
	{
		return m_farPlane;
	}
}


real Camera::getPixelPerfectScale() const
{
	if(m_projectionType == ProjectionType_Perspective)
	{
		return math::tan(0.5f * m_fov) / (static_cast<real>(renderer::Renderer::getInstance()->getScreenHeight() * 0.5f));
	}
	else
	{
		return 1.0f;
	}
}


math::Vector3 Camera::getWorldFromScreen(real p_screenX, real p_screenY, real p_worldZ) const
{
	// Get renderer instance
	renderer::Renderer* renderer = renderer::Renderer::getInstance();
	
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
	
	if (math::realEqual(m_up.x, 1.0f))
	{
		TT_ASSERT(tt::math::realEqual(m_up.y, 0.0f) && tt::math::realEqual(m_up.z, 0.0f));
		
		real tmp = worldY;
		worldY = worldX;
		worldX = -tmp; // Also flip it.
	}
	else if (math::realEqual(m_up.y, 1.0f) == false)
	{
		// Rotated camera
		real rotation = math::acos(math::dotProduct(math::Vector3::up, m_up));
		if(m_up.x < 0) rotation = -rotation;
		math::Vector2 rot(worldX, worldY);
		rot.rotate(rotation);
		worldX = rot.x;
		worldY = rot.y;
	}
	
	return math::Vector3(m_worldPosition.x + worldX, m_worldPosition.y - worldY, p_worldZ);
}


// Factory functions
CameraPtr Camera::createPerspective(const math::Vector3& p_position, const math::Vector3& p_lookAt, 
									real p_near, real p_far, real p_fov)
{
	real width  = real(renderer::Renderer::getInstance()->getScreenWidth());
	real height = real(renderer::Renderer::getInstance()->getScreenHeight());

	return CameraPtr(new Camera(p_position, p_lookAt, width, height, p_near, p_far, p_fov));
}


CameraPtr Camera::createOrtho(const math::Vector3& p_position, const math::Vector3& p_lookAt,
							  real p_near, real p_far)
{
	real width  = real(renderer::Renderer::getInstance()->getScreenWidth());
	real height = real(renderer::Renderer::getInstance()->getScreenHeight());

	return CameraPtr(new Camera(p_position, p_lookAt, width, height, p_near, p_far));
}


////////////////////////////////////////////
// Protected

void Camera::renderObject(renderer::RenderContext&)
{
	m_frustum.render(this);
}


bool Camera::load(const fs::FilePtr& p_file)
{
	// Load the base part of the object
	if (SceneObject::load(p_file) == false)
	{
		return false;
	}

	// Now we can load the field of view
	if (p_file->read(&m_fov, sizeof(m_fov)) != sizeof(m_fov))
	{
		return false;
	}
	m_fov = math::degToRad(m_fov);

	// Now we can load the ranges
	if (p_file->read(&m_nearPlane, sizeof(m_nearPlane)) != sizeof(m_nearPlane))
	{
		return false;
	}

	if (p_file->read(&m_farPlane, sizeof(m_farPlane)) != sizeof(m_farPlane))
	{
		return false;
	}

	// Load the target - not used here
	s16 targetid;
	if (p_file->read(&targetid, sizeof(targetid)) != sizeof(targetid))
	{
		return false;
	}

	return true;
}


////////////////////////////////////////////
// Private

// Constructor for orthographic projections
Camera::Camera(const math::Vector3& p_pos, const math::Vector3& p_lookAt,
			   real p_width, real p_height, real p_near, real p_far)
:
SceneObject(Type_Camera),
m_fov(0.0f),
m_debugFOV(0.0f),
m_debugFOVEnabled(false),
m_nearPlane(p_near),
m_farPlane(p_far),
m_x(0),
m_y(0),
m_width(p_width),
m_height(p_height),
m_aspect(p_width / p_height),
m_worldPosition(p_pos),
m_lookAt(p_lookAt),
m_up(math::Vector3::zero),
m_requestedUp(0.0f, 1.0f, 0.0f),
m_projectionView(),
m_updateInverseProjection(true),
m_projectionDebugFOV(),
m_projectionViewDebugFOV(),
m_inverseProjectionView(),
m_projectionType(ProjectionType_Orthogonal),
m_dualProjectionEnabled(false),
m_worldMatrixInverse(),
m_instance(),
m_frustum(),
m_lookAtChanged(true)
{
	setPosition(p_pos);
	computeProjectionMatrix();
}


// Constructor for perspective projections
Camera::Camera(const math::Vector3& p_pos, const math::Vector3& p_lookAt,
               real p_width, real p_height, real p_near, real p_far, real p_fov)
:
SceneObject(Type_Camera),
m_fov(math::degToRad(p_fov)),
m_debugFOV(m_fov),
m_debugFOVEnabled(false),
m_nearPlane(p_near),
m_farPlane(p_far),
m_x(0),
m_y(0),
m_width(p_width),
m_height(p_height),
m_aspect(p_width / p_height),
m_worldPosition(p_pos),
m_lookAt(p_lookAt),
m_up(math::Vector3::zero),
m_requestedUp(0.0f, 1.0f, 0.0f),
m_projectionView(),
m_updateInverseProjection(true),
m_projectionDebugFOV(),
m_projectionViewDebugFOV(),
m_inverseProjectionView(),
m_projectionType(ProjectionType_Perspective),
m_dualProjectionEnabled(false),
m_worldMatrixInverse(),
m_instance(),
m_frustum(),
m_lookAtChanged(true)
{
	setPosition(p_pos);
	m_frustum.updateRatio(m_fov, m_aspect);
	m_frustum.updatePlanes(m_nearPlane, m_farPlane);
	computeProjectionMatrix();
}


void Camera::computeViewMatrix()
{
	// Normalize a look vector
	math::Vector3 vLook(0,0,0);
	
	if (m_instance != 0)
	{
		// Looking at instance
		vLook = m_worldPosition - m_instance->getActualPosition();
	}
	else
	{
		vLook = m_worldPosition - m_lookAt;
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
	m_up = m_requestedUp;

	// Are we too close to coplanear?
	if (math::realEqual(vLook.x, 0) && math::realEqual(vLook.z, 0))
	{
		m_up.setValues(-vLook.y, 0, 0);
	}
	
	math::Vector3 vRight = crossProduct(m_up, vLook);
	vRight.normalize();
	
	// Re-compute up
	m_up = crossProduct(vLook, vRight);
	
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
	m_worldMatrix.m_42 = -dotProduct(m_up  , m_worldPosition);
	m_worldMatrix.m_43 = -dotProduct(vLook , m_worldPosition);
	m_worldMatrix.m_44 = 1.0f;
	
	m_worldMatrixInverse = m_worldMatrix.getInverse();
}


void Camera::computeProjectionViewMatrix()
{
	// Re-compute auxilary matrices (Only update when needed!)
	m_projectionView = m_worldMatrix * m_projection;
#if !defined(TT_BUILD_FINAL)
	m_projectionViewDebugFOV = m_worldMatrix * m_projectionDebugFOV;
#endif
	m_updateInverseProjection = true;
}


bool Camera::isSphereInOrtho(const math::Vector3& p_pos, real p_radius) const
{
	// Check sphere against near/far plane
	if(p_pos.z < -(m_farPlane + p_radius) || p_pos.z > -(m_nearPlane - p_radius))
	{
		return false;
	}
	
	// Compute max y
	real limit((m_height / 2) + p_radius);

	// Check sphere against y boundaries
	if(p_pos.y < -limit || p_pos.y > limit)
	{
		return false;
	}

	// Compute max x
	limit = (m_width / 2) + p_radius;

	// Check sphere against x boundaries
	if(p_pos.x < -limit || p_pos.x > limit)
	{
		return false;
	}

	return true;
}


void Camera::computeProjectionMatrix()
{
	if(m_projectionType == ProjectionType_Perspective)
	{
#if defined(TT_PLATFORM_WIN)
		math::makePerspectiveDirectX(
#else
		math::makePerspectiveOpenGL( 
#endif
		                             m_projection, m_aspect, m_fov, m_nearPlane, m_farPlane);
	
#if !defined(TT_BUILD_FINAL)
#if defined(TT_PLATFORM_WIN)
		math::makePerspectiveDirectX(
#else
		math::makePerspectiveOpenGL( 
#endif
		                             m_projectionDebugFOV, m_aspect, m_debugFOV, m_nearPlane, m_farPlane);
#endif
	}
	else
	{
#if defined(TT_PLATFORM_WIN)
		// NOTE: Swap near/far plane to get correct ortho projection for DirectX
		math::makeOrtho(m_projection, m_width, m_height, m_farPlane, m_nearPlane);
#else
		math::makeOrtho(m_projection, m_width, m_height, m_nearPlane, m_farPlane);
#endif
	}
}


// Namespace end
}
}
}
