#if !defined(INC_TT_ENGINE_SCENE_CAMERA_H)
#define INC_TT_ENGINE_SCENE_CAMERA_H

// Include necessary files
#include <tt/platform/tt_types.h>
#include <tt/math/Vector3.h>
#include <tt/math/Matrix44.h>
#include <tt/math/Rect.h>
#include <tt/engine/scene/fwd.h>
#include <tt/engine/renderer/fwd.h>
#include <tt/engine/animation/fwd.h>
#include <tt/engine/scene/SceneObject.h>
#include <tt/engine/scene/Frustum.h>


namespace tt {
namespace engine {
namespace scene {


class Camera : public SceneObject
{
public:
	// Camera projection types
	enum ProjectionType
	{
		ProjectionType_Orthogonal,
		ProjectionType_Perspective
	};
	
	virtual ~Camera();
	
	/*! \brief Sets the camera matrices to hardware
	Loads the project / view matrix to the hardware, also clears the world matrix to identity */
	void select();
	
	inline void setLookAt(const math::Vector3& p_lookAt)
	{
		m_lookAt = p_lookAt;
		m_lookAtChanged = true;
	}
	inline void setLookAt(real p_x, real p_y, real p_z)
	{
		m_lookAt.setValues(p_x, p_y, p_z);
		m_lookAtChanged = true;
	}
	inline const math::Vector3& getLookAt() const {return m_lookAt;}
	
	// Properties that influence projection
	void setFOV(real p_fov);      // NOTE: Field-of-View in Degrees!!
	void setNearFar(real p_near, real p_far);
	void setNear(real p_near);
	void setFar(real p_far);
	
	// Debug FOV
	void setDebugFOV(real p_fov); // NOTE: Field-of-View in Degrees!!
	void setDebugFOVEnabled(bool p_enabled);
	bool isDebugFOVEnabled() const { return m_debugFOVEnabled; }
	
	void setViewPort(real p_x, real p_y, real p_width, real p_height);
	
	inline real getFOVInRad() const { return m_fov; }
	inline real getFOV() const {return math::radToDeg(m_fov);}
	inline real getDebugFOVInRad() const { return m_debugFOV; }
	inline real getDebugFOV() const {return math::radToDeg(m_debugFOV);}
	inline real getNear() const {return m_nearPlane;}
	inline real getFar() const {return m_farPlane;}
	inline real getAspectRatio() const {return m_aspect;}
	
	inline real getWidth()  const {return m_width;}
	inline real getHeight() const {return m_height;}
	inline real getX() const {return m_x;}
	inline real getY() const {return m_y;}
	
	inline void setInstance(const InstancePtr& p_instance) {m_instance = p_instance;}
	
	inline const math::Matrix44& getViewMatrix() const        { return m_worldMatrix;        }
	inline const math::Matrix44& getViewMatrixInverse() const { return m_worldMatrixInverse; }
	
	bool isVisible(const renderer::Sphere& p_sphere) const;
	bool isVisible(const math::Vector3& p_pos, real p_radius) const;
	bool isVisible(const math::VectorRect& p_rect, real p_maxZ) const;
	
	virtual void update(const animation::AnimationPtr& p_animation = animation::AnimationPtr(),
		                Instance* p_instance = 0);
	
	/*! \brief Converts a 3D point into 2D screen coordinates
	
		Calculates the 2D screen coordinates of a 3d point.  
		
		\param v - The 3D Position
		\param x - The X coordinate
		\param y - The Y coordinate
		\param p_allowDebugFOV - When set to true, it uses the debugFOV when enabled (non final builds only)
		
		\return Nothing
	*/
	void convert3Dto2D(const math::Vector3& v, s32 &x, s32 &y, bool p_allowDebugFOV = false) const;
	
	/*! \brief Converts a bounding box (worldspace) so that it is perspective corrected.
	
		\param p_box_OUT - The box to be corrected
		\param p_depth - The depth
		\param p_allowDebugFOV - When set to true, it uses the debugFOV when enabled (non final builds only)
		
		\return Whether or not this rectangle is still visible
	*/
	bool convert3Dto2D(math::VectorRect& p_box_OUT, const real p_depth, bool p_allowDebugFOV = false) const;
	
	/*! \brief calculates the world coordinates of the points on the near and 
	           far clip plane for a given screen position.
	    \param p_x screen position x
		\param p_y screen position y
		\param p_near pointer to a Vector3 for the result of the world position on the near clip plane.
		\param p_far pointer to a Vector3 for the result of the world position on the far clip plane.
		             If p_far == 0, no calculations for this point will be done.
		\return If the point in inside of the viewport.*/
	bool scrPosToWorldLine(s32 p_x, s32 p_y, math::Vector3* p_near, math::Vector3* p_far);
	bool scrPosToWorldLine(s32 p_x, s32 p_y, physics::Ray& p_ray_OUT);
	
	inline const math::Vector3& getActualPosition() const
	{
		return m_worldPosition;
	}
	
	real getPixelPerfectDistance() const;
	real getFullScreenDistance(real p_height) const;
	
	real getPixelPerfectScale() const;
	
	/*! \brief Translates a screen position to a world position. */
	math::Vector3 getWorldFromScreen(real p_screenX, real p_screenY, real p_worldZ = 0.0f) const;
	
	/*! \brief Start AA Pass */
	inline void startPass(bool p_top) {m_frustum.setPass(p_top);}
	
	// Factory functions
	static CameraPtr createPerspective(const math::Vector3& p_position,
		                               const math::Vector3& p_lookAt = math::Vector3::zero,
	                                   real p_near = 1.0f,
	                                   real p_far = 4096.0f,
	                                   real p_fov = 60.0f);
	
	static CameraPtr createOrtho(const math::Vector3& p_position,
	                             const math::Vector3& p_lookAt = math::Vector3::zero,
	                             real p_near = 1.0f,
	                             real p_far = 4096.0f);
	
	inline ProjectionType getProjectionType() {return m_projectionType;}
	void toggleProjectionMode() { m_dualProjectionEnabled = !m_dualProjectionEnabled; }
	
	inline void visualize()
	{
		renderer::RenderContext rc;
		renderObject(rc);
	}
	
	inline void setUpVector(const tt::math::Vector3& p_up)
	{
		m_requestedUp = p_up;
	}
	
	inline const math::Vector3& getUp() const { return m_up; }
	
protected:
	virtual void renderObject(renderer::RenderContext& p_renderContext);
	virtual bool load(const fs::FilePtr& p_file);
	
private:
	friend class renderer::Renderer;
	friend class renderer::ShadowSource;
	friend class renderer::UpScaler;
	friend class SceneObject;
	
	Camera(const math::Vector3& p_pos, const math::Vector3& p_lookAt,
		real p_width, real p_height, real p_near, real p_far);
	Camera(const math::Vector3& p_pos, const math::Vector3& p_lookAt,
		real p_width, real p_height, real p_near, real p_far, real p_fov);
	
	bool isSphereInOrtho(const math::Vector3& p_pos, real p_radius) const;
	
	void computeViewMatrix();
	void computeProjectionMatrix();
	void computeProjectionViewMatrix();
	
	real m_fov;
	real m_debugFOV;
	bool m_debugFOVEnabled;
	
	real m_nearPlane;
	real m_farPlane;
	real m_x;
	real m_y;
	real m_width;
	real m_height;
	real m_aspect;
	
	math::Vector3 m_worldPosition;
	math::Vector3 m_lookAt;
	math::Vector3 m_up;
	math::Vector3 m_requestedUp;
	
	math::Matrix44 m_projection;
	math::Matrix44 m_projectionView;
	bool m_updateInverseProjection;
	
	math::Matrix44 m_projectionDebugFOV;
	math::Matrix44 m_projectionViewDebugFOV;
	
	math::Matrix44 m_inverseProjectionView;
	ProjectionType m_projectionType;
	bool m_dualProjectionEnabled;
	
	math::Matrix44 m_worldMatrixInverse;
	
	InstancePtr m_instance;
	
	Frustum m_frustum;
	
	bool m_lookAtChanged;
};

// Namespace end
}
} 
}

#endif // INC_TT_ENGINE_SCENE_CAMERA_H
