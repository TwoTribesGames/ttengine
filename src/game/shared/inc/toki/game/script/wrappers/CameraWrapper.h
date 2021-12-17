#if !defined(INC_TOKITORI_GAME_SCRIPT_WRAPPERS_CAMERAWRAPPER_H)
#define INC_TOKITORI_GAME_SCRIPT_WRAPPERS_CAMERAWRAPPER_H


#include <tt/math/interpolation.h>
#include <tt/math/Vector2.h>
#include <tt/script/helpers.h>

#include <toki/game/script/wrappers/fwd.h>

namespace toki {
namespace game {
namespace script {
namespace wrappers {

/*! \brief 'Camera' in Squirrel. */
class CameraWrapper
{
public:
	/*! \brief Creates a CameraEffect object for this camera. Lifetime of this object is controlled in script. */
	static wrappers::CameraEffectWrapper createCameraEffect();
	
	/*! \brief is Camera the main camera? */
	static bool isMainCamera();
	
	/*! \brief is Camera the sub camera? */
	static bool isSubCamera();
	
	/*! \brief Camera the main camera. (Used for the main_only and sub_only render passes.) */
	static void makeMainCamera();
	
	/*! \brief Returns if the camera is following an entity */
	static bool isFollowingEntity();
	
	/*! \brief Sets the entity this camera should follow (or null for no entity following) */
	static void setFollowEntity(const EntityWrapper* p_entity);
	
	/*! \brief Returns the target position of the camera */
	static const tt::math::Vector2& getTargetPosition();
	
	/*! \brief Returns the current position of the camera (including effects) */
	static const tt::math::Vector2& getPosition();
	
	/*! \brief Sets the position of the camera. If p_duration <= 0.0, the camera is instantly set to the target position. */
	static void setPosition(const tt::math::Vector2& p_position, real p_duration,
	                        tt::math::interpolation::EasingType p_easingType);
	
	/*! \brief Returns the camera distance */
	static real getCameraDistance();
	
	/*! \brief Returns the currently visible rect. */
	static const tt::math::VectorRect& getVisibleRect();
	
	/*! \brief Returns the normalized screenspace rect. */
	static tt::math::VectorRect getScreenSpaceRect();
	
	/*! \brief Sets the field of view of the camera in degrees. If p_duration <= 0.0, the camera FOV is instantly set to the target FOV. */
	static void setFOV(real p_fovYDegrees, real p_duration,
	                           tt::math::interpolation::EasingType p_easingType);
	
	/*! \brief Returns the target field of view of the camera in degrees */
	static real getTargetFOV();
	
	/*! \brief Sets the rotation of the camera in degrees. If p_duration <= 0.0, the camera is instantly set to that rotation. */
	static void setRotation(real p_rotationInDegrees, real p_duration,
	                        tt::math::interpolation::EasingType p_easingType);
	
	/*! \brief Returns the current rotation of the camera in degrees */
	static real getCurrentRotation();
	
	/*! \brief Returns the target rotation of the camera in degrees */
	static real getTargetRotation();
	
	/*! \brief Sets the speed at which the camera animates from its current position to the target position. */
	static void setFollowSpeed(real p_followSpeed);
	
	/*! \brief Returns the speed at which the camera animates from its current position to the target position. */
	static real getFollowSpeed();
	
	/*! \brief Enable of disable "hud only" rendering. */
	static void setRenderHudOnly(bool p_enabled);
	
	/*! \brief Returns if "hud only" rendering is enabled. */
	static bool isRenderingHudOnly();
	
	/*! \brief Returns world position for a screen position. */
	static tt::math::Vector2 screenToWorld(tt::math::Vector2 p_screenPos);
	
	/*! \brief Returns screen position for a world position. */
	static tt::math::Vector2 worldToScreen(const tt::math::Vector2& p_worldPos);
	
	/*! \brief Returns if circle intersects with the visible rectangle of this camera. */
	static bool isOnScreen(const tt::math::Vector2& p_worldPosition, real p_radius);
	
	/*! \brief Returns if circle is completely enclosed with the visible rectangle of this camera. */
	static bool isFullyOnScreen(const tt::math::Vector2& p_worldPosition, real p_radius);
	
	/*! \brief Returns if scrolling of this camera is enabled. */
	static bool isScrollingEnabled();
	
	/*! \brief Enable or disable scrolling of this camera. */
	static void setScrollingEnabled(bool p_enabled);
	
	static void bind(const tt::script::VirtualMachinePtr& p_vm);
};


/*! \brief 'DrcCamera' in Squirrel. */
class DrcCameraWrapper
{
public:
	/*! \brief Creates a CameraEffect object for this camera. Lifetime of this object is controlled in script. */
	static CameraEffectWrapper createCameraEffect();
	
	/*! \brief Returns if the DRC camera is enabled. */
	static bool isEnabled();
	
	/*! \brief Enable of disable the DRC camera. */
	static void setEnabled(bool p_enabled);
	
	/*! \brief is DRC the main camera? */
	static bool isMainCamera();
	
	/*! \brief is DRC the sub camera? */
	static bool isSubCamera();
	
	/*! \brief make DRC the main camera. (Used for the main_only and sub_only render passes.) */
	static void makeMainCamera();
	
	/*! \brief Returns if the DRC camera is following an entity */
	static bool isFollowingEntity();
	
	/*! \brief Sets the entity this camera should follow (or null for no entity following) */
	static void setFollowEntity(const EntityWrapper* p_entity);
	
	/*! \brief Returns the target position of the DRC camera */
	static const tt::math::Vector2& getTargetPosition();
	
	/*! \brief Returns the current position of the DRC camera (including effects) */
	static const tt::math::Vector2& getPosition();
	
	/*! \brief Sets the position of the DRC camera. If p_duration <= 0.0, the camera is instantly set to the target position. */
	static void setPosition(const tt::math::Vector2& p_position, real p_duration,
	                        tt::math::interpolation::EasingType p_easingType);
	
	/*! \brief Returns the camera distance */
	static real getCameraDistance();
	
	/*! \brief Returns the currently visible rect. */
	static const tt::math::VectorRect& getVisibleRect();
	
	/*! \brief Sets the field of view of the DRC camera in degrees. If p_duration <= 0.0, the DRC camera FOV is instantly set to the target FOV. */
	static void setFOV(real p_fovYDegrees, real p_duration,
	                           tt::math::interpolation::EasingType p_easingType);
	
	/*! \brief Returns the target field of view of the DRC camera in degrees */
	static real getTargetFOV();
	
	/*! \brief Sets the rotation of the DRC camera in degrees. If p_duration <= 0.0, the camera is instantly set to that rotation. */
	static void setRotation(real p_rotationInDegrees, real p_duration,
	                        tt::math::interpolation::EasingType p_easingType);
	
	/*! \brief Returns the current rotation of the DRC camera in degrees */
	static real getCurrentRotation();
	
	/*! \brief Returns the target rotation of the DRC camera in degrees */
	static real getTargetRotation();
	
	/*! \brief Sets the speed at which the DRC camera animates from its current position to the target position. */
	static void setFollowSpeed(real p_followSpeed);
	
	/*! \brief Returns the speed at which the DRC camera animates from its current position to the target position. */
	static real getFollowSpeed();
	
	/*! \brief Enable of disable "hud only" rendering. */
	static void setRenderHudOnly(bool p_enabled);
	
	/*! \brief Returns if "hud only" rendering is enabled. */
	static bool isRenderingHudOnly();
	
	/*! \brief Returns world position for a screen position. */
	static tt::math::Vector2 screenToWorld(tt::math::Vector2 p_screenPos);
	
	/*! \brief Returns screen position for a world position. */
	static tt::math::Vector2 worldToScreen(const tt::math::Vector2& p_worldPos);
	
	/*! \brief Returns if circle intersects with the culling rectangle of this camera. */
	static bool isOnScreen(const tt::math::Vector2& p_worldPosition, real p_radius);
	
	/*! \brief Returns if scrolling of this camera is enabled. */
	static bool isScrollingEnabled();
	
	/*! \brief Enable of disable scrolling. */
	static void setScrollingEnabled(bool p_enabled);
	
	static void bind(const tt::script::VirtualMachinePtr& p_vm);
};


// Namespace end
}
}
}
}

#endif // !defined(INC_TOKITORI_GAME_SCRIPT_WRAPPERS_CAMERAWRAPPER_H)
