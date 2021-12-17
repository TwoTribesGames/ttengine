#if !defined(INC_TOKI_GAME_CAMERA_H)
#define INC_TOKI_GAME_CAMERA_H


#include <tt/code/fwd.h>
#include <tt/engine/renderer/enums.h>
#include <tt/engine/scene/fwd.h>
#include <tt/math/interpolation.h>
#include <tt/math/Point2.h>
#include <tt/math/Rect.h>
#include <tt/math/Vector2.h>
#include <tt/math/Vector3.h>
#include <tt/platform/tt_types.h>

#include <toki/game/entity/fwd.h>
#include <toki/game/fwd.h>

namespace toki {
namespace game {

class Camera
{
public:
	explicit Camera(const std::string&               p_configKeyBase,
	                tt::engine::renderer::ViewPortID p_viewport = tt::engine::renderer::ViewPortID_TV);
	
	void selectCameraFromViewPort();
	
	void update(real                     p_deltaTime,
	            const tt::math::Vector2& p_extraOffset           = tt::math::Vector2::zero,
	            bool                     p_applyEffectMgrEffects = false);
	
	void updateWithCurrentState();
	
	void renderDebug() const;
	
	CameraEffectPtr createCameraEffect();
	
	tt::math::Vector2 screenToWorld(const tt::math::Point2&  p_screenPos) const;
	tt::math::Point2  worldToScreen(const tt::math::Vector2& p_worldPos)  const;
	bool isOnScreen(const tt::math::Vector2& p_worldPos2D, real p_radius) const;
	bool isOnScreen(const tt::math::VectorRect& p_worldRect) const;
	bool isFullyOnScreen(const tt::math::Vector2& p_worldPos2D, real p_radius) const;
	bool isFullyOnScreen(const tt::math::VectorRect& p_worldRect) const;
	
	// Returns the FOV needed to fit p_height on screen
	real getFOVForWorldHeight(real p_height) const;
	
	tt::math::Point2 getViewPortSize() const;
	
	inline const tt::math::Vector2& getTargetPosition()             const { return m_target.getPosition();  }
	inline const tt::math::Vector2& getCurrentPosition()            const { return m_currentPosition;       }
	inline const tt::math::Vector2& getCurrentPositionWithEffects() const { return m_current.getPosition(); }
	
	inline real getTargetFOV()  const { return m_target .getFOV(); }
	inline real getCurrentFOV() const { return m_current.getFOV(); }
	
	inline real getTargetRotation()  const { return m_target .getRotation(); }
	inline real getCurrentRotation() const { return m_current.getRotation(); }
	
	inline const tt::math::VectorRect& getCurrentVisibleRect()   const { return m_currentVisibleRect;   }
	inline const tt::math::VectorRect& getCurrentCullingRect()   const { return m_currentCullingRect;   }
	inline const tt::math::VectorRect& getCurrentUncullingRect() const { return m_currentUncullingRect; }
	
	inline bool isFollowingEntity() const                             { return m_followEntity.isEmpty() == false; }
	inline void setFollowEntity(const entity::EntityHandle& p_entity) { m_followEntity = p_entity; }
	inline void resetFollowEntity() { m_followEntity.invalidate(); }
	inline const entity::EntityHandle& getFollowEntity() const        { return m_followEntity;     }
	
	inline bool isFreeScrollModeEnabled() const  { return m_freeScrollModeEnabled;     }
	void setFreeScrollMode(bool p_enable);
	
	inline bool isRenderingHudOnly() const      { return m_renderHudOnly;     }
	inline void setRenderHudOnly(bool p_enable) { m_renderHudOnly = p_enable; }
	
	void setPosition          (const tt::math::Vector2& p_pos, bool p_instant = false);
	void setPositionWithEasing(const tt::math::Vector2& p_pos, real p_duration, tt::math::interpolation::EasingType p_easingType);
	
	void setFOV               (real p_fovYDegrees,             bool p_instant = false);
	void setFOVWithEasing     (real p_fovYDegrees,             real p_duration, tt::math::interpolation::EasingType p_easingType);
	void setFOVToDefault();
	
	inline bool isDebugFOVEnabled() const { return m_debugFOVEnabled; }
	void setDebugFOVEnabled(bool p_enabled);
	void setDebugFOV(real p_debugFOV);
	real getDebugFOV() const { return m_debugFOV; }
	
	void setLevelAspectRatio(real p_texelsPerTile);
	void restoreAspectRatio();
	void setRotation          (real p_rotationInRadians, bool p_instant = false);
	void setRotationWithEasing(real p_rotationInRadians, real p_duration, tt::math::interpolation::EasingType p_easingType);
	void resetRotation();
	
	inline void setCameraDistance(real p_distance) { m_cameraDistance = p_distance; }
	inline real getCameraDistance() const          { return m_cameraDistance;       }
	
	inline bool isScrollingEnabled() const          { return m_scrollingEnabled;      }
	inline void setScrollingEnabled(bool p_enabled) { m_scrollingEnabled = p_enabled; }
	
	void setScrollOffset(const tt::math::Vector2& p_offset, bool p_instant = false);
	
	void setFollowSpeedToDefault();
	
	inline void setFollowSpeed(real p_speed) { m_followSpeed = p_speed; }
	inline real getFollowSpeed() const       { return m_followSpeed;    }
	
	inline void initSpeedFromPos(const tt::math::Vector2& p_pos)
	{
		m_target.setPosition(p_pos);
		m_positionMoveMode = PositionMoveMode_InitWithSpeed;
	}
	
	void syncWith(const Camera& p_source);
	void clear();
	
	void serialize  (tt::code::BufferWriteContext* p_context) const;
	void unserialize(tt::code::BufferReadContext*  p_context);
	
private:
	typedef std::vector<CameraEffectWeakPtr> CameraEffects;
	
	class State
	{
	public:
		inline State()
		:
		m_position(tt::math::Vector2::zero),
		m_minFOV(0.1f),
		m_maxFOV(180.f),
		m_fov(60.0f),
		m_rotation(0.0f)
		{ }
		
		inline const tt::math::Vector2& getPosition() const { return m_position; } 
		// returns true if position has been updated
		bool setPosition(const tt::math::Vector2& p_position);
		
		inline real getFOV() const { return m_fov; }
		inline real getMinFOV() const { return m_minFOV; }
		inline real getMaxFOV() const { return m_maxFOV; }
		
		// returns true if fov has been updated
		bool setFOV(real p_fov);
		
		// FIXME: Perhaps add this to the constructor
		void setFOVLimits(real p_fovMin, real p_fovMax);
		
		inline real getRotation() const { return m_rotation; } 
		// returns true if rotation has been updated
		bool setRotation(real p_position);
		
		void serialize  (tt::code::BufferWriteContext* p_context) const;
		void unserialize(tt::code::BufferReadContext*  p_context);
		
	private:
		tt::math::Vector2 m_position;
		
		real              m_minFOV;
		real              m_maxFOV;
		real              m_fov;
		real              m_rotation;
	};
	
	enum PositionMoveMode
	{
		PositionMoveMode_None,
		PositionMoveMode_MoveToTarget,
		PositionMoveMode_MoveToTargetWithEasing,
		PositionMoveMode_MoveWithSpeed,
		
		PositionMoveMode_InitWithSpeed // Start speed is calculated with first update. (Because it needs the deltatime.)
	};
	
	enum FOVMode
	{
		FOVMode_None,
		FOVMode_UpdateToTargetWithEasing
	};
	
	enum RotationMode
	{
		RotationMode_None,
		RotationMode_UpdateToTargetWithEasing
	};
	
	/*! \return The width and height of what is visible of Z=0 in world space, with the specified camera settings. */
	tt::math::Vector2    getWorldVisibleSize(real p_fov, real p_cameraDistance) const;
	
	/*! \return Same as getWorldVisibleSize, but as a rectangle, at the specified (center) position. */
	tt::math::VectorRect getWorldVisibleRect(const tt::math::Vector2& p_pos, real p_fov, real p_cameraDistance) const;
	
	void changeFOVWhileKeepingMouseScreenPosition(real p_previousFov);
	
	void updatePosition(real p_deltaTime);
	void updateFOV(real p_deltaTime);
	void updateRotation(real p_deltaTime);
	real getFOVWithEffect() const;
	void updateScrollOffset(real p_deltaTime);
	tt::math::Vector2 getPosition(const tt::math::Vector2& p_effectOffset, bool p_applyEffectFromEffectMgr) const;
	
	void setEngineFOV(real p_newFOV);
	
	void updateCullingRectangles();
	
	// No copying
	Camera(const Camera&);
	Camera& operator=(const Camera&);
	
	
	const std::string    m_configKeyBase;
	PositionMoveMode     m_positionMoveMode;
	FOVMode              m_fovMode;
	RotationMode         m_rotationMode;
	tt::math::Vector2    m_currentPosition;  // Clean camera position (without camera effects)
	State                m_current;          // The state containing the complete camera position (with camera effects)
	State                m_target;           // The state containing the clean camera position (no camera effects)
	tt::math::Vector2    m_speed;
	real                 m_followSpeed;
	entity::EntityHandle m_followEntity;
	bool                 m_freeScrollModeEnabled;
	bool                 m_renderHudOnly;
	tt::math::interpolation::Easing<tt::math::Vector2> m_easingPosition;
	tt::math::interpolation::Easing<real             > m_easingFOV;
	tt::math::interpolation::Easing<real             > m_easingRotation;
	real                 m_currentFOVWithEffects;
	real                 m_cullingFOV;
	
	bool                 m_debugFOVEnabled;
	real                 m_debugFOV;
	
	// Culling
	tt::math::VectorRect m_currentVisibleRect;
	tt::math::VectorRect m_currentCullingRect;
	tt::math::VectorRect m_currentUncullingRect;
	real                 m_cullingScale;
	real                 m_uncullingScale;
	
	// Scroll parameters
	bool              m_scrollingEnabled;
	real              m_scrollLength;
	tt::math::Vector2 m_currentScrollOffset;
	tt::math::Vector2 m_targetScrollOffset;
	
	tt::engine::scene::CameraPtr m_engineCamera;
	real                         m_cameraDistance;
	
	const tt::engine::renderer::ViewPortID m_viewport;
	
	CameraEffects m_cameraEffects;
};

// Namespace end
}
}


#endif  // !defined(INC_TOKI_GAME_CAMERA_H)
