#include <tt/code/bufferutils.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/engine/renderer/ViewPort.h>
#include <tt/engine/scene/Camera.h>
#include <tt/math/ExponentialGrowth.h>
#include <tt/math/math.h>

#include <toki/game/entity/effect/EffectMgr.h>
#include <toki/game/entity/Entity.h>
#include <toki/game/CameraEffect.h>
#include <toki/game/Camera.h>
#include <toki/game/DebugView.h>
#include <toki/game/Game.h>
#include <toki/level/helpers.h>
#include <toki/level/LevelData.h>
#include <toki/AppGlobal.h>
#include <toki/cfg.h>


namespace toki {
namespace game {

static const real g_snapDistance = 0.00001f;  // when to snap to target pos (squared distance)


//--------------------------------------------------------------------------------------------------
// Public member functions

Camera::Camera(const std::string& p_configKeyBase, tt::engine::renderer::ViewPortID p_viewport)
:
m_configKeyBase(p_configKeyBase),
m_positionMoveMode(PositionMoveMode_None),
m_fovMode(FOVMode_None),
m_rotationMode(RotationMode_None),
m_currentPosition(tt::math::Vector2::zero),
m_current(),
m_target(),
m_speed(tt::math::Vector2::zero),
m_followSpeed(0.0f),
m_followEntity(entity::EntityHandle()),
m_freeScrollModeEnabled(false),
m_renderHudOnly(false),
m_easingPosition(),
m_easingFOV(),
m_easingRotation(),
m_currentFOVWithEffects(0.0f),
m_cullingFOV(0.0f),
m_debugFOVEnabled(false),
m_debugFOV(0.0f),
m_currentVisibleRect(),
m_currentCullingRect(),
m_currentUncullingRect(),
m_cullingScale(cfg()->getRealDirect("toki.camera.game.culling_scale")),
m_uncullingScale(cfg()->getRealDirect("toki.camera.game.unculling_scale")),
m_scrollingEnabled(false),
m_scrollLength(0.0f),
m_currentScrollOffset(tt::math::Vector2::zero),
m_targetScrollOffset(tt::math::Vector2::zero),
m_engineCamera(),
m_cameraDistance(25.0f),
m_viewport(p_viewport),
m_cameraEffects()
{
	TT_ASSERT(m_cullingScale > 0.0f);
	TT_ASSERT(m_uncullingScale > 0.0f);
	TT_ASSERT(m_cullingScale > m_uncullingScale);
	
	clear();
	
	selectCameraFromViewPort();
	
	// Prevent unnecessary memory allocations
	m_cameraEffects.reserve(30);
}


void Camera::selectCameraFromViewPort()
{
	if (m_viewport == tt::engine::renderer::ViewPortID_1)
	{
		const tt::math::Vector2& pos(m_current.getPosition());
		
		m_engineCamera = tt::engine::renderer::Renderer::getInstance()->getMainCamera(m_viewport);
		m_engineCamera->setPosition(tt::math::Vector3(pos.x, pos.y, m_cameraDistance));
		m_engineCamera->setLookAt  (tt::math::Vector3(pos.x, pos.y, 0.0f            ));
		
		setEngineFOV(m_currentFOVWithEffects);
	}
	else
	{
		m_engineCamera.reset();
	}
}


void Camera::update(real                     p_deltaTime,
                    const tt::math::Vector2& p_extraOffset,
                    bool                     p_applyEffectMgrEffects)
{
	tt::math::Vector2 effectOffset   = p_extraOffset;
	real              effectFOV      = 0.0f;
	real              effectRotation = 0.0f;
	{
		// Update all camera effects first
		for (CameraEffects::iterator it = m_cameraEffects.begin(); it != m_cameraEffects.end(); )
		{
			CameraEffectPtr effect = (*it).lock();
			if (effect == 0)
			{
				it = m_cameraEffects.erase(it);
			}
			else
			{
				effect->update(p_deltaTime);
				
				effectOffset   += effect->getActualOffset();
				effectFOV      += effect->getActualFOV();
				effectRotation += effect->getActualRotation();
				
				++it;
			}
		}
	}
	
	// Following entity logic
	{
		if (isFollowingEntity() && isFreeScrollModeEnabled() == false)
		{
			entity::Entity* entity = m_followEntity.getPtr();
			if (entity != 0)
			{
				setPosition(entity->getCenterPosition());
			}
		}
	}
	
	// Save the current FOV
	const real preUpdateFov = m_currentFOVWithEffects;
	
	updatePosition(p_deltaTime);
	updateFOV(p_deltaTime);
	updateRotation(p_deltaTime);
	m_currentFOVWithEffects = p_applyEffectMgrEffects ? getFOVWithEffect() : m_current.getFOV();
	
	updateScrollOffset(p_deltaTime);
	
	const tt::math::Vector2 pos((isFreeScrollModeEnabled()) ? m_currentPosition : 
	                            getPosition(effectOffset + m_currentScrollOffset, p_applyEffectMgrEffects));
	
	m_current.setPosition(pos);
	
	// Update engine camera with current values.
	m_engineCamera->setPosition(tt::math::Vector3(pos.x, pos.y, m_cameraDistance));
	m_engineCamera->setLookAt  (tt::math::Vector3(pos.x, pos.y, 0.0f            ));
	{
		const real currentRotation = p_applyEffectMgrEffects ? effectRotation + m_current.getRotation() :
		                             m_current.getRotation();
		
		const real x = sin(currentRotation);
		const real y = cos(currentRotation);
		m_engineCamera->setUpVector(tt::math::Vector3(x, y, 0.0f));
	}
	
	if (isFreeScrollModeEnabled())
	{
		changeFOVWhileKeepingMouseScreenPosition(preUpdateFov);
	}
	else
	{
		setEngineFOV(m_currentFOVWithEffects + effectFOV);
	}
	
	m_engineCamera->update();
	
	updateCullingRectangles();
}


void Camera::updateWithCurrentState()
{
	const tt::math::Vector2 pos(m_currentPosition);
	m_engineCamera->setPosition(tt::math::Vector3(pos.x, pos.y, m_cameraDistance));
	m_engineCamera->setLookAt  (tt::math::Vector3(pos.x, pos.y, 0.0f            ));
	m_engineCamera->setFOV(m_current.getFOV());
	m_engineCamera->update();
	
	updateCullingRectangles();
}


CameraEffectPtr Camera::createCameraEffect()
{
	CameraEffectPtr effect(new CameraEffect);
	m_cameraEffects.push_back(effect);
	return effect;
}


tt::math::Vector2 Camera::screenToWorld(const tt::math::Point2& p_screenPos) const
{
	// Paraphrased implementation from engine Camera (but using viewport size instead of screen size)
	TT_NULL_ASSERT(m_engineCamera);
	
	const tt::math::Point2  scrSize(getViewPortSize());
	const tt::math::Vector3 camPos (m_engineCamera->getActualPosition());
	
	const real screenWidth  = static_cast<real>(scrSize.x);
	const real screenHeight = static_cast<real>(scrSize.y);
	const real dist         = camPos.z;
	
	// Calculate the width and height of the camera area at the world Z position
	const real fov = m_engineCamera->isDebugFOVEnabled() ? 
		m_engineCamera->getDebugFOVInRad() : m_engineCamera->getFOVInRad();
	const real camHeight = 2.0f * (tt::math::tan(0.5f * fov) * dist);
	const real camWidth  = m_engineCamera->getAspectRatio() * camHeight;
	
	// Translate the screen-space position to a camera-space position
	const real screenX = p_screenPos.x - (screenWidth  * 0.5f);
	const real screenY = p_screenPos.y - (screenHeight * 0.5f);
	
	// Scale the position according to the camera area
	const real worldX = (screenX / screenWidth ) * camWidth;
	const real worldY = (screenY / screenHeight) * camHeight;
	
	// Rotate according to the camera rotation (based on up vector)
	const real angleSin = m_engineCamera->getUp().x;
	const real angleCos = m_engineCamera->getUp().y;
	
	const real rotatedResultX = (worldX * angleCos) + (worldY * -angleSin);
	const real rotatedResultY = (worldX * angleSin) + (worldY *  angleCos);
	
	return tt::math::Vector2(camPos.x + rotatedResultX, camPos.y - rotatedResultY);
	
	/* Original implementation:
	TT_NULL_ASSERT(m_engineCamera);
	const tt::math::Vector3 pos(m_engineCamera->getWorldFromScreen(static_cast<real>(p_screenPos.x),
	                                                               static_cast<real>(p_screenPos.y)));
	return tt::math::Vector2(pos.x, pos.y);
	*/
}


tt::math::Point2 Camera::worldToScreen(const tt::math::Vector2& p_worldPos) const
{
	tt::math::Point2 screenPos;
	// NOTE: allow for Debug FOV here
	m_engineCamera->convert3Dto2D(tt::math::Vector3(p_worldPos.x, p_worldPos.y, 0.0f), screenPos.x, screenPos.y, true);
	return screenPos;
}


bool Camera::isOnScreen(const tt::math::Vector2& p_worldPos2D, real p_radius) const
{
	return m_currentVisibleRect.intersects(p_worldPos2D, p_radius);
}


bool Camera::isOnScreen(const tt::math::VectorRect& p_worldRect) const
{
	return m_currentVisibleRect.intersects(p_worldRect);
}


bool Camera::isFullyOnScreen(const tt::math::Vector2& p_worldPos2D, real p_radius) const
{
	return m_currentVisibleRect.contains(p_worldPos2D, p_radius);
}


bool Camera::isFullyOnScreen(const tt::math::VectorRect& p_worldRect) const
{
	return m_currentVisibleRect.contains(p_worldRect);
}


real Camera::getFOVForWorldHeight(real p_height) const
{
	return tt::math::radToDeg(tt::math::atan2(p_height * 0.5f, m_cameraDistance) * 2.0f);
}


tt::math::Point2 Camera::getViewPortSize() const
{
	const tt::engine::renderer::ViewPortContainer& viewPorts(tt::engine::renderer::ViewPort::getViewPorts());
	tt::engine::renderer::ViewPortContainer::size_type viewPortIndex =
			static_cast<tt::engine::renderer::ViewPortContainer::size_type>(m_viewport);
	if (m_viewport >= 0 && viewPortIndex < viewPorts.size())
	{
		return tt::math::Point2(viewPorts[viewPortIndex].getSettings().width,
		                        viewPorts[viewPortIndex].getSettings().height);
	}
	else
	{
		TT_PANIC("Renderer does not have viewport %d. Using screen size as camera viewport size.",
		         m_viewport);
		tt::engine::renderer::Renderer* renderer = tt::engine::renderer::Renderer::getInstance();
		return tt::math::Point2(renderer->getScreenWidth(), renderer->getScreenHeight());
	}
}


void Camera::setFreeScrollMode(bool p_enable)
{
	if (m_freeScrollModeEnabled != p_enable)
	{
		m_freeScrollModeEnabled = p_enable;
		if (p_enable)
		{
			// Set position with all effects applied, so we continue scrolling from that location.
			m_currentPosition = m_current.getPosition();
		}
	}
}


void Camera::setPosition(const tt::math::Vector2& p_pos, bool p_instant)
{
	if (p_instant)
	{
		m_target.setPosition(p_pos);
		m_currentPosition  = p_pos;
		m_positionMoveMode = PositionMoveMode_None;
	}
	else if (m_target.setPosition(p_pos))
	{
		// Target position changed, move to target
		m_positionMoveMode = PositionMoveMode_MoveToTarget;
	}
}


// FIXME: Code duplication with Camera::setPosition
void Camera::setPositionWithEasing(const tt::math::Vector2& p_pos, real p_duration,
                                   tt::math::interpolation::EasingType p_easingType)
{
	if (p_duration <= 0.0f)
	{
		m_target.setPosition(p_pos);
		m_currentPosition  = p_pos;
		m_positionMoveMode = PositionMoveMode_None;
	}
	else if (m_target.setPosition(p_pos))
	{
		// Target position changed, update to target value
		m_easingPosition = tt::math::interpolation::Easing<tt::math::Vector2>(
			m_currentPosition,
			m_target.getPosition(),
			p_duration, p_easingType);
		m_positionMoveMode = PositionMoveMode_MoveToTargetWithEasing;
	}
}


void Camera::setFOV(real p_fovYDegrees, bool p_instant)
{
	setFOVWithEasing(p_fovYDegrees, p_instant ? 0.0f : 0.5f, tt::math::interpolation::EasingType_CubicInOut);
}


void Camera::setFOVWithEasing(real p_fovYDegrees, real p_duration,
                              tt::math::interpolation::EasingType p_easingType)
{
	if (p_duration <= 0.0f)
	{
		const real previousFov = m_current.getFOV();
		m_current.setFOV(p_fovYDegrees);
		m_target .setFOV(p_fovYDegrees);
		
		if (isFreeScrollModeEnabled())
		{
			changeFOVWhileKeepingMouseScreenPosition(previousFov);
		}
		
		m_fovMode = FOVMode_None;
	}
	else if (m_target.setFOV(p_fovYDegrees))
	{
		// Target FOV changed, move to target
		m_easingFOV = tt::math::interpolation::Easing<real>(
			m_current.getFOV(),
			m_target .getFOV(),
			p_duration, p_easingType);
		m_fovMode = FOVMode_UpdateToTargetWithEasing;
	}
}


void Camera::setFOVToDefault()
{
	setFOV(cfg()->getRealDirect(m_configKeyBase + ".fov"));
}


void Camera::setDebugFOVEnabled(bool p_enabled)
{
#if !defined(TT_BUILD_FINAL)
	m_debugFOVEnabled = p_enabled;
	m_engineCamera->setDebugFOVEnabled(p_enabled);
#else
	m_debugFOVEnabled = false;
	(void)p_enabled;
#endif
}


void Camera::setDebugFOV(real p_debugFOV)
{
	m_debugFOV = p_debugFOV;
	tt::math::clamp(m_debugFOV, m_current.getMinFOV(), m_current.getMaxFOV());
	m_engineCamera->setDebugFOV(m_debugFOV);
}


void Camera::setLevelAspectRatio(real p_texelsPerTile)
{
	const real levelWidth  = AppGlobal::getGame()->getAttributeLayer()->getWidth()  * p_texelsPerTile;
	const real levelHeight = AppGlobal::getGame()->getAttributeLayer()->getHeight() * p_texelsPerTile;
	m_engineCamera->setViewPort(0,0,levelWidth, levelHeight);
}


void Camera::restoreAspectRatio()
{
	using tt::engine::renderer::Renderer;
	const real screenWidth  = static_cast<real>(Renderer::getInstance()->getScreenWidth());
	const real screenHeight = static_cast<real>(Renderer::getInstance()->getScreenHeight());
	m_engineCamera->setViewPort(0,0, screenWidth, screenHeight);
}


void Camera::setRotation(real p_rotationInRadians, bool p_instant)
{
	setRotationWithEasing(p_rotationInRadians, p_instant ? 0.0f : 0.5f, tt::math::interpolation::EasingType_CubicInOut);
}


void Camera::setRotationWithEasing(real p_rotationInRadians, real p_duration, tt::math::interpolation::EasingType p_easingType)
{
	if (p_duration <= 0.0f)
	{
		m_current.setRotation(p_rotationInRadians);
		m_target .setRotation(p_rotationInRadians);
		
		m_rotationMode = RotationMode_None;
	}
	else if (m_target.setRotation(p_rotationInRadians))
	{
		// Target rotation changed, update to target value
		m_easingRotation = tt::math::interpolation::Easing<real>(
			m_current.getRotation(),
			m_target .getRotation(),
			p_duration, p_easingType);
		m_rotationMode = RotationMode_UpdateToTargetWithEasing;
	}
}


void Camera::setScrollOffset(const tt::math::Vector2& p_offset, bool p_instant)
{
	m_targetScrollOffset = p_offset * m_scrollLength;
	
	if (p_instant)
	{
		m_currentScrollOffset = m_targetScrollOffset;
	}
}


void Camera::setFollowSpeedToDefault()
{
	m_followSpeed = 0.125f;
}


void Camera::syncWith(const Camera& p_source)
{
	m_cameraEffects = p_source.m_cameraEffects;
	
	m_positionMoveMode      = p_source.m_positionMoveMode;
	m_fovMode               = p_source.m_fovMode;
	m_rotationMode          = p_source.m_rotationMode;
	m_currentPosition       = p_source.m_currentPosition;
	m_current               = p_source.m_current;
	m_target                = p_source.m_target;
	m_speed                 = p_source.m_speed;
	m_followSpeed           = p_source.m_followSpeed;
	m_followEntity          = p_source.m_followEntity;
	m_freeScrollModeEnabled = p_source.m_freeScrollModeEnabled;
	m_easingPosition        = p_source.m_easingPosition;
	m_easingFOV             = p_source.m_easingFOV;
	m_easingRotation        = p_source.m_easingRotation;
	m_currentFOVWithEffects = p_source.m_currentFOVWithEffects;
	m_cullingFOV            = p_source.m_cullingFOV;
	m_debugFOVEnabled       = p_source.m_debugFOVEnabled;
	m_debugFOV              = p_source.m_debugFOV;
	m_currentVisibleRect    = p_source.m_currentVisibleRect;
	m_currentCullingRect    = p_source.m_currentCullingRect;
	m_currentUncullingRect  = p_source.m_currentUncullingRect;
	m_scrollingEnabled      = p_source.m_scrollingEnabled;
	m_scrollLength          = p_source.m_scrollLength;
	m_currentScrollOffset   = p_source.m_currentScrollOffset;
	m_targetScrollOffset    = p_source.m_targetScrollOffset;
	// Do not sync m_renderHudOnly because we want to set this per camera.
	
	m_engineCamera->setPosition   (p_source.m_engineCamera->getPosition());
	m_engineCamera->setLookAt     (p_source.m_engineCamera->getLookAt());
	m_engineCamera->setFOV        (p_source.m_engineCamera->getFOV());
	m_engineCamera->update();
}


void Camera::clear()
{
	m_cameraEffects.clear();
	
	//m_target.pos.setValues(tt::engine::renderer::Renderer::getInstance()->getScreenWidth() * 0.5f,
	//                       -tt::engine::renderer::Renderer::getInstance()->getScreenHeight() * 0.5f,
	//                       0.0f);
	
	m_positionMoveMode      = PositionMoveMode_None;
	m_fovMode               = FOVMode_None;
	m_rotationMode          = RotationMode_None;
	m_speed                 = tt::math::Vector2::zero;
	m_followSpeed           = 0.125f;
	m_followEntity.invalidate();
	m_freeScrollModeEnabled = false;
	m_renderHudOnly         = false;
	
	m_target.setFOVLimits(cfg()->getRealDirect(m_configKeyBase + ".min_fov"),
	                      cfg()->getRealDirect(m_configKeyBase + ".max_fov"));
	m_target.setFOV      (cfg()->getRealDirect(m_configKeyBase + ".fov"));
	
	m_current               = m_target;
	m_currentPosition       = m_current.getPosition();
	m_currentFOVWithEffects = m_current.getFOV(); // Ignoring effect here because lazy.
	m_debugFOVEnabled       = false;
	m_debugFOV              = 0.0f;
	m_cullingFOV            = 0.0f;
	m_scrollingEnabled      = true;
	m_scrollLength          = cfg()->getRealDirect(m_configKeyBase + ".scroll_length"),
	m_currentScrollOffset   = tt::math::Vector2::zero;
	m_targetScrollOffset    = tt::math::Vector2::zero;
}


void Camera::serialize(tt::code::BufferWriteContext* p_context) const
{
	TT_NULL_ASSERT(p_context);
	
	namespace bu = tt::code::bufferutils;
	
	m_target.serialize(p_context);
	bu::put(m_speed,                 p_context);
	bu::put(m_followSpeed,           p_context);
	bu::putHandle(m_followEntity,    p_context);
	bu::put(m_freeScrollModeEnabled, p_context);
	bu::put(m_renderHudOnly,         p_context);
	bu::put(m_scrollingEnabled,      p_context);
	bu::put(m_scrollLength,          p_context);
	
	bu::putEnum<u8>(m_fovMode,       p_context);
	bu::putEasing(m_easingFOV,       p_context);
	
	// Don't serialize movemode and easing settings;
	// when unserializing the camera should always be at target position
	
	// Don't serialize m_currentScrollOffset and m_targetScrollOffset
	
	// Shouldn't serialize the m_cameraEffects, because lifetime of them is handled in script
}


void Camera::unserialize(tt::code::BufferReadContext* p_context)
{
	TT_NULL_ASSERT(p_context);
	
	namespace bu = tt::code::bufferutils;
	
	m_target.unserialize(p_context);
	m_speed                 = bu::get<tt::math::Vector2         >(p_context);
	m_followSpeed           = bu::get<real                      >(p_context);
	m_followEntity          = bu::getHandle<game::entity::Entity>(p_context);
	m_freeScrollModeEnabled = bu::get<bool                      >(p_context);
	m_renderHudOnly         = bu::get<bool                      >(p_context);
	m_scrollingEnabled      = bu::get<bool                      >(p_context);
	m_scrollLength          = bu::get<real                      >(p_context);
	
	m_fovMode               = bu::getEnum<u8, FOVMode>(p_context);
	m_easingFOV             = bu::getEasing<real>(p_context);
	
	m_current               = m_target;
	m_currentPosition       = m_current.getPosition();
	m_positionMoveMode      = PositionMoveMode_None;
	m_rotationMode          = RotationMode_None;
	
	m_currentScrollOffset   = tt::math::Vector2::zero;
	m_targetScrollOffset    = tt::math::Vector2::zero;
	
	// Camera effects are created by the CameraEffectWrappers
	m_cameraEffects.clear();
	
	// Do an update so that all rectangles and values are set correctly
	update(0.0f);
}


//----------------------------------------------------------------------------------------------------------------
// Private member functions

tt::math::Vector2 Camera::getWorldVisibleSize(real p_fov, real p_cameraDistance) const
{
	TT_NULL_ASSERT(m_engineCamera);
	const real worldVisibleHeight = (tt::math::tan(tt::math::degToRad(p_fov) * 0.5f) * p_cameraDistance) * 2.0f;
	return tt::math::Vector2(worldVisibleHeight * m_engineCamera->getAspectRatio(), worldVisibleHeight);
}


tt::math::VectorRect Camera::getWorldVisibleRect(const tt::math::Vector2& p_pos,
                                                 real                     p_fov,
                                                 real                     p_cameraDistance) const
{
	const tt::math::Vector2 visibleSize(getWorldVisibleSize(p_fov, p_cameraDistance));
	return tt::math::VectorRect(p_pos - (visibleSize * 0.5f), visibleSize.x, visibleSize.y);
}


void Camera::changeFOVWhileKeepingMouseScreenPosition(real p_previousFov)
{
	setEngineFOV(p_previousFov);
	m_engineCamera->update();
	
	const input::Controller::State::EditorState& editorState(AppGlobal::getController(tt::input::ControllerIndex_One).cur.editor);
	const tt::math::Vector2 preWorldPos(screenToWorld(editorState.pointer));
	
	setEngineFOV(m_currentFOVWithEffects);
	m_engineCamera->update();
	
	const tt::math::Vector2 postWorldPos(screenToWorld(editorState.pointer));
	const tt::math::Vector2 worldPosDiff(preWorldPos - postWorldPos);
	
	const real lenghtSqrt = worldPosDiff.lengthSquared();
	if (lenghtSqrt > 0.0001f)
	{
		m_currentPosition += worldPosDiff;
		m_target.setPosition(m_target.getPosition() + worldPosDiff);
		
		// Re-apply to engine camera.
		const tt::math::Vector2& pos = m_currentPosition;
		m_engineCamera->setPosition(tt::math::Vector3(pos.x, pos.y, m_cameraDistance));
		m_engineCamera->setLookAt  (tt::math::Vector3(pos.x, pos.y, 0.0f            ));
	}
}


void Camera::updatePosition(real p_deltaTime)
{
	tt::math::Vector2&       currentPosition = m_currentPosition;
	const tt::math::Vector2& targetPosition  = m_target.getPosition();
	
	if (m_positionMoveMode == PositionMoveMode_InitWithSpeed)
	{
		const tt::math::Vector2 diff(targetPosition - currentPosition);
		m_speed            = p_deltaTime > 0.0f ? diff * (1.0f / p_deltaTime) : tt::math::Vector2::zero;
		m_positionMoveMode = PositionMoveMode_MoveWithSpeed;
	}
	
	switch (m_positionMoveMode)
	{
	case PositionMoveMode_None:
		break;
		
	case PositionMoveMode_MoveToTarget:
		{
			// Note: Don't use AppGlobal::getFixedDeltaTime() for this ExponentialGrowth.
			// We want to define speedDecay as *= m_followSpeed as if it was applied each frame at 60 FPS.
			// This is already handing different fixed frame rates because it uses p_deltaTime in getGrowth.
			// (See: ExponentialGrowth.h)
			if (AppGlobal::getFixedDeltaTimeScale() <= 0.0)
			{
				// ExponentialGrowth cannot have a fixed time <= 0; so early out when the scale is <= 0
				return;
			}
			const tt::math::ExponentialGrowth targetDecay(m_followSpeed, (1.0f / 60.0f) * AppGlobal::getFixedDeltaTimeScale());
			const real targetDecayWithDeltaTime = targetDecay.getGrowth(p_deltaTime);
			currentPosition += (targetPosition - currentPosition) * targetDecayWithDeltaTime;
			
			// Set to target if close enough.
			if ((targetPosition - currentPosition).lengthSquared() < g_snapDistance)
			{
				currentPosition    = targetPosition;
				m_positionMoveMode = PositionMoveMode_None;
			}
		}
		break;
		
	case PositionMoveMode_MoveToTargetWithEasing:
		{
			m_easingPosition.update(p_deltaTime);
			currentPosition = m_easingPosition.getValue();
			
			// Set to target if close enough.
			if ((targetPosition - currentPosition).lengthSquared() < g_snapDistance)
			{
				currentPosition    = targetPosition;
				m_positionMoveMode = PositionMoveMode_None;
			}
		}
		break;
		
	case PositionMoveMode_MoveWithSpeed:
		{
			// Note: Don't use AppGlobal::getFixedDeltaTime() for this ExponentialGrowth.
			// We want to define speedDecay as *= 0.95f as if it was applied each frame at 60 FPS.
			// This is already handing different fixed frame rates because it uses p_deltaTime in getGrowth.
			// (See: ExponentialGrowth.h)
			static const tt::math::ExponentialGrowth speedDecay( 0.95f, 1.0f / 60.f);
			currentPosition += (m_speed * p_deltaTime);
			m_speed *= speedDecay.getGrowth(p_deltaTime);
			if (m_speed.lengthSquared() < 0.0001f)
			{
				m_speed.setValues(0.0f, 0.0f);
				m_target.setPosition(currentPosition);
				m_positionMoveMode = PositionMoveMode_None;
			}
		}
		break;
		
	case PositionMoveMode_InitWithSpeed:
		TT_PANIC("MoveMode_InitWithSpeed should be handled before this switch");
		break;
		
	default:
		TT_PANIC("Unknown camera position move mode: %d", m_positionMoveMode);
		break;
	}
}


void Camera::updateFOV(real p_deltaTime)
{
	const real targetFOV  = m_target.getFOV();
	
	switch (m_fovMode)
	{
	case FOVMode_None:
		break;
		
	case FOVMode_UpdateToTargetWithEasing:
		{
			m_easingFOV.update(p_deltaTime);
			m_current.setFOV(m_easingFOV.getValue());
		}
		break;
		
	default:
		TT_PANIC("Unknown camera FOV mode: %d", m_fovMode);
		break;
	}
	
	if (tt::math::fabs(targetFOV - m_current.getFOV()) < 0.01f)
	{
		m_current.setFOV(targetFOV);
		m_fovMode = FOVMode_None;
	}
}


void Camera::updateRotation(real p_deltaTime)
{
	const real targetRotation  = m_target.getRotation();
	
	switch (m_rotationMode)
	{
	case RotationMode_None:
		break;
		
	case RotationMode_UpdateToTargetWithEasing:
		{
			m_easingRotation.update(p_deltaTime);
			m_current.setRotation(m_easingRotation.getValue());
		}
		break;
		
	default:
		TT_PANIC("Unknown camera rotation mode: %d", m_rotationMode);
		break;
	}
	
	if (tt::math::fabs(targetRotation - m_current.getRotation()) < 0.0001f)
	{
		m_current.setRotation(targetRotation);
		m_rotationMode = RotationMode_None;
	}
}


real Camera::getFOVWithEffect() const
{
	const real defaultFov = m_current.getFOV();
	if (AppGlobal::hasGame())
	{
		Game* game = AppGlobal::getGame();
		
		// I would rather not have this tv or drc logic here.
		switch (m_viewport)
		{
		case tt::engine::renderer::ViewPortID_TV:
			return game->getEffectMgr().getCameraFov(defaultFov);
		case tt::engine::renderer::ViewPortID_DRC:
			return game->getEffectMgr().getDrcCameraFov(defaultFov);
		default:
			TT_PANIC("Unknown viewport: %d\n", m_viewport);
			break;
		}
	}
	return defaultFov;
}


void Camera::updateScrollOffset(real p_deltaTime)
{
	tt::math::Vector2&      currentPosition = m_currentScrollOffset;
	
	const tt::math::Vector2 targetPosition = (m_scrollingEnabled) ?
	                                         m_targetScrollOffset :
	                                         tt::math::Vector2::zero; // Move to zero when scrolling is disabled.
	
	if (currentPosition == targetPosition)
	{
		return;
	}
	
	// Note: Don't use AppGlobal::getFixedDeltaTime() for this ExponentialGrowth.
	// We want to define speedDecay as *= m_followSpeed as if it was applied each frame at 60 FPS.
	// This is already handing different fixed frame rates because it uses p_deltaTime in getGrowth.
	// (See: ExponentialGrowth.h)
	const tt::math::ExponentialGrowth targetDecay(m_followSpeed, 1.0f / 60.0f);
	const real targetDecayWithDeltaTime = targetDecay.getGrowth(p_deltaTime);
	currentPosition += (targetPosition - currentPosition) * targetDecayWithDeltaTime;
	
	// Set to target if close enough.
	if ((targetPosition - currentPosition).lengthSquared() < g_snapDistance)
	{
		currentPosition = targetPosition;
	}
}


tt::math::Vector2 Camera::getPosition(const tt::math::Vector2& p_offset,
                                      bool                     p_applyEffectFromEffectMgr) const
{
	const tt::math::Vector2 cleanPos(m_currentPosition + p_offset);
	
	if (p_applyEffectFromEffectMgr && AppGlobal::hasGame())
	{
		Game* game = AppGlobal::getGame();
		
		// I would rather not have this tv or drc logic here.
		switch (m_viewport)
		{
		case tt::engine::renderer::ViewPortID_TV:
			return game->getEffectMgr().getCameraPosition(cleanPos);
		case tt::engine::renderer::ViewPortID_DRC:
			return game->getEffectMgr().getDrcCameraPosition(cleanPos);
		default:
			TT_PANIC("Unknown viewport: %d\n", m_viewport);
			break;
		}
	}
	return cleanPos;
}


void Camera::setEngineFOV(real p_newFOV)
{
#if defined(TT_BUILD_FINAL)
	// Clamp FOV based on FOV min/max of current state
	tt::math::clamp(p_newFOV, m_current.getMinFOV(), m_current.getMaxFOV());
	
	static const real widescreenAspectRatio = 16.0f / 9.0f;
	const real aspectRatio = m_engineCamera->getAspectRatio();
	if (aspectRatio >= widescreenAspectRatio || AppGlobal::shouldTakeLevelScreenshot())
	{
		m_cullingFOV = p_newFOV;
		m_engineCamera->setFOV(p_newFOV);
		return;
	}
	
	const real cameraDistance = m_engineCamera->getPosition().z;
	
	// Compute width at z = 0 with current fov and 16:9 aspect ratio
	const real originalHeight = 2 * tt::math::tan(0.5f * tt::math::degToRad(p_newFOV)) * cameraDistance;
	const real originalWidth = originalHeight * widescreenAspectRatio;
	
	// Compute the fov needed to get the same width with the current aspect ratio
	const real currentHeight = originalWidth / aspectRatio;
	const real currentFOV = 2 * tt::math::atan((0.5f * currentHeight) / cameraDistance);
	m_cullingFOV = tt::math::radToDeg(currentFOV);
	
	m_engineCamera->setFOV(m_cullingFOV);
#else
	// Clamp FOV based on FOV min/max of current state
	tt::math::clamp(p_newFOV, m_current.getMinFOV(), m_current.getMaxFOV());
	real debugFOV = m_debugFOV;
	tt::math::clamp(debugFOV, m_current.getMinFOV(), m_current.getMaxFOV());
	
	static const real widescreenAspectRatio = 16.0f / 9.0f;
	const real aspectRatio = m_engineCamera->getAspectRatio();
	if (aspectRatio >= widescreenAspectRatio || AppGlobal::shouldTakeLevelScreenshot())
	{
		m_cullingFOV = p_newFOV;
		m_debugFOVEnabled ? m_engineCamera->setDebugFOV(debugFOV) : m_engineCamera->setFOV(m_cullingFOV);
		return;
	}
	
	const real cameraDistance = m_engineCamera->getPosition().z;
	
	// Compute culling FOV
	{
		// Compute width at z = 0 with current fov and 16:9 aspect ratio
		const real originalHeight = 2 * tt::math::tan(0.5f * tt::math::degToRad(p_newFOV)) * cameraDistance;
		const real originalWidth = originalHeight * widescreenAspectRatio;
		
		// Compute the fov needed to get the same width with the current aspect ratio
		const real currentHeight = originalWidth / aspectRatio;
		const real currentFOV = 2 * tt::math::atan((0.5f * currentHeight) / cameraDistance);
		m_cullingFOV = tt::math::radToDeg(currentFOV);
	}
	
	// Compute debug FOV
	if (m_debugFOVEnabled)
	{
		// Compute width at z = 0 with current fov and 16:9 aspect ratio
		const real originalHeight = 2 * tt::math::tan(0.5f * tt::math::degToRad(debugFOV)) * cameraDistance;
		const real originalWidth = originalHeight * widescreenAspectRatio;
		
		// Compute the fov needed to get the same width with the current aspect ratio
		const real currentHeight = originalWidth / aspectRatio;
		debugFOV = 2 * tt::math::atan((0.5f * currentHeight) / cameraDistance);
		debugFOV = tt::math::radToDeg(debugFOV);
	}
	
	m_debugFOVEnabled ? m_engineCamera->setDebugFOV(debugFOV) : m_engineCamera->setFOV(m_cullingFOV);
#endif
}


void Camera::updateCullingRectangles()
{
	// Use the actual camera position for the culling
	const tt::math::Vector3& pos3D(m_engineCamera->getActualPosition());
	const tt::math::Vector2 pos(pos3D.x, pos3D.y);
	
	// Update culling rectangles (should be a fixed 4:3 (1:0.75) rectangle scaled in height with aspect ratio)
	m_currentVisibleRect = getWorldVisibleRect(pos, m_cullingFOV, m_cameraDistance);
	
	m_currentCullingRect.setWidth(m_currentVisibleRect.getWidth()   * m_cullingScale);
	m_currentCullingRect.setHeight(m_currentVisibleRect.getHeight() * m_cullingScale * m_engineCamera->getAspectRatio() * 0.75f);
	m_currentCullingRect.setCenterPosition(pos);
	
	m_currentUncullingRect.setWidth(m_currentVisibleRect.getWidth()   * m_uncullingScale);
	m_currentUncullingRect.setHeight(m_currentVisibleRect.getHeight() * m_uncullingScale * m_engineCamera->getAspectRatio() * 0.75f);
	m_currentUncullingRect.setCenterPosition(pos);
}


void Camera::renderDebug() const
{
#if !defined(TT_BUILD_FINAL)
	toki::DebugRenderMask mask = AppGlobal::getDebugRenderMask();
	tt::engine::renderer::Renderer* renderer = tt::engine::renderer::Renderer::getInstance();
	const tt::engine::debug::DebugRendererPtr debug(renderer->getDebug());
	if (mask.checkFlag(DebugRender_RenderCullingRects))
	{
		// TODO: Inverse these rects
		debug->renderSolidRect(tt::engine::renderer::ColorRGBA(255, 255, 255, 30), m_currentCullingRect);
		debug->renderSolidRect(tt::engine::renderer::ColorRGBA(255, 255, 255, 20), m_currentUncullingRect);
	}
	
	if (m_debugFOVEnabled)
	{
		// Render border around 'game' screen.
		debug->renderRect(tt::engine::renderer::ColorRGB::white, m_currentVisibleRect);
	}
#endif
}


//----------------------------------------------------------------------------------------------------------------
// Camera::State public member functions

bool Camera::State::setPosition(const tt::math::Vector2& p_position)
{
	if (m_position == p_position)
	{
		return false;
	}
	
	m_position = p_position;
	return true;
}


bool Camera::State::setFOV(real p_fov)
{
	if (p_fov == m_fov)
	{
		return false;
	}
	
	m_fov = p_fov;
	
	tt::math::clamp(m_fov, m_minFOV, m_maxFOV);
	return true;
}


void Camera::State::setFOVLimits(const real p_minFOV, const real p_maxFOV)
{
	static const real fovHardMin = 0.1f;    // absolute minimum setting for FOV
	static const real fovHardMax = 180.0f;  // absolute maximum setting for FOV
	
	m_minFOV = p_minFOV;
	m_maxFOV = p_maxFOV;
	
	TT_ASSERTMSG(m_minFOV >= fovHardMin && m_minFOV <= fovHardMax,
	             "Minimum field of view of %f is invalid. Must be in range %.1f - %.1f.",
	             m_minFOV, fovHardMin, fovHardMax);
	TT_ASSERTMSG(m_maxFOV >= fovHardMin && m_maxFOV <= fovHardMax,
	             "Maximum field of view of %f is invalid. Must be in range %.1f - %.1f.",
	             m_maxFOV, fovHardMin, fovHardMax);
	
	tt::math::clamp(m_minFOV, 0.1f, 180.0f);
	tt::math::clamp(m_maxFOV, 0.1f, 180.0f);
	
	if (m_minFOV > m_maxFOV)
	{
		TT_PANIC("Minimum field of view (%f) is larger than maximum (%f). "
		         "Swapping values.", m_minFOV, m_maxFOV);
		std::swap(m_minFOV, m_maxFOV);
	}
}


bool Camera::State::setRotation(real p_rotation)
{
	if (p_rotation == m_rotation)
	{
		return false;
	}
	
	m_rotation = p_rotation;
	return true;
}


/*
void Camera::State::recalculateCullingRectangle(real p_cameraDistance, real p_aspectRatio)
{
	if (m_dirty == false)
	{
		// No changes, no need to recalc
		return;
	}
	
	const real visibleHeight = (tt::math::tan(tt::math::degToRad(m_fov) * 0.5f) * p_cameraDistance) * 2.0f;
	const real visibleWidth  = visibleHeight * p_aspectRatio;
	
	m_cullingRectangle.setPosition(tt::math::Vector2(m_position.x - (visibleWidth  * 0.5f), 
	                                                 m_position.y - (visibleHeight * 0.5f)));
	
	m_cullingRectangle.setWidth (visibleWidth);
	m_cullingRectangle.setHeight(visibleHeight);
	m_dirty = false;
}
*/


void Camera::State::serialize(tt::code::BufferWriteContext* p_context) const
{
	TT_NULL_ASSERT(p_context);
	
	namespace bu = tt::code::bufferutils;
	bu::put(m_position,         p_context);	// Vector3
	bu::put(m_minFOV,           p_context);	// real
	bu::put(m_maxFOV,           p_context);	// real
	bu::put(m_fov,              p_context);	// real
	bu::put(m_rotation,         p_context);	// real
}


void Camera::State::unserialize(tt::code::BufferReadContext* p_context)
{
	TT_NULL_ASSERT(p_context);
	
	*this = State();
	
	namespace bu = tt::code::bufferutils;
	m_position         = bu::get<tt::math::Vector2   >(p_context);
	m_minFOV           = bu::get<real                >(p_context);
	m_maxFOV           = bu::get<real                >(p_context);
	m_fov              = bu::get<real                >(p_context);
	m_rotation         = bu::get<real                >(p_context);
}

// Namespace end
}
}
