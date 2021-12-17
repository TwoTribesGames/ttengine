#include <tt/math/Vector3.h>

#include <toki/game/Game.h>
#include <toki/game/script/wrappers/CameraEffectWrapper.h>
#include <toki/game/script/wrappers/CameraWrapper.h>
#include <toki/game/script/sqbind_bindings.h>
#include <toki/AppGlobal.h>

namespace toki {
namespace game {
namespace script {
namespace wrappers {

//--------------------------------------------------------------------------------------------------
// Public member functions

wrappers::CameraEffectWrapper CameraWrapper::createCameraEffect()
{
	toki::game::Camera& camera = AppGlobal::getGame()->getCamera();
	
	return wrappers::CameraEffectWrapper(camera.createCameraEffect(),
	                                     wrappers::CameraEffectWrapper::CameraType_Normal);
}


bool CameraWrapper::isMainCamera()
{
	return AppGlobal::getGame()->isDrcCameraMain() == false;
}


bool CameraWrapper::isSubCamera()
{
	return isMainCamera() == false;
}


void CameraWrapper::makeMainCamera()
{
	AppGlobal::getGame()->setDrcCameraAsMain(false);
}


bool CameraWrapper::isFollowingEntity()
{
	return AppGlobal::getGame()->getCamera().isFollowingEntity();
}


void CameraWrapper::setFollowEntity(const EntityWrapper* p_entity)
{
	if (p_entity != 0)
	{
		AppGlobal::getGame()->getCamera().setFollowEntity(p_entity->getHandle());
		return;
	}
	// else
	AppGlobal::getGame()->getCamera().resetFollowEntity();
}


const tt::math::Vector2& CameraWrapper::getTargetPosition()
{
	return AppGlobal::getGame()->getCamera().getTargetPosition();
}


const tt::math::Vector2& CameraWrapper::getPosition()
{
	return AppGlobal::getGame()->getCamera().getCurrentPositionWithEffects();
}


void CameraWrapper::setPosition(const tt::math::Vector2& p_position, real p_duration,
                                tt::math::interpolation::EasingType p_easingType)
{
	//TT_ASSERTMSG(AppGlobal::getGame()->getCamera().isFollowingEntity() == false,
	//             "Setting camera to a position while following entity!");
	setFollowEntity(0); // FIXME: Remove this line and add in the assert above.
	AppGlobal::getGame()->getCamera().setPositionWithEasing(p_position, p_duration, p_easingType);
}


real CameraWrapper::getCameraDistance()
{
	return AppGlobal::getGame()->getCamera().getCameraDistance();
}


const tt::math::VectorRect& CameraWrapper::getVisibleRect()
{
	return AppGlobal::getGame()->getCamera().getCurrentVisibleRect();
}


tt::math::VectorRect CameraWrapper::getScreenSpaceRect()
{
	const toki::game::Camera& cam = AppGlobal::getGame()->getCamera();
	
	tt::math::Vector2 scrSize(cam.getViewPortSize());
	scrSize /= scrSize.y; // Normalized space
	return tt::math::VectorRect(tt::math::Vector2(-scrSize.x * 0.5f, -scrSize.y * 0.5f),
		scrSize.x, scrSize.y);
}


void CameraWrapper::setFOV(real p_fovYDegrees, real p_duration,
                           tt::math::interpolation::EasingType p_easingType)
{
	AppGlobal::getGame()->getCamera().setFOVWithEasing(p_fovYDegrees, p_duration, p_easingType);
}


real CameraWrapper::getTargetFOV()
{
	return AppGlobal::getGame()->getCamera().getTargetFOV();
}


void CameraWrapper::setRotation(real p_rotationInDegrees, real p_duration,
                                tt::math::interpolation::EasingType p_easingType)
{
	AppGlobal::getGame()->getCamera().setRotationWithEasing(tt::math::degToRad(p_rotationInDegrees),
	                                                        p_duration, p_easingType);
}


real CameraWrapper::getCurrentRotation()
{
	return tt::math::radToDeg(AppGlobal::getGame()->getCamera().getCurrentRotation());
}


real CameraWrapper::getTargetRotation()
{
	return tt::math::radToDeg(AppGlobal::getGame()->getCamera().getTargetRotation());
}


void CameraWrapper::setFollowSpeed(real p_followSpeed)
{
	AppGlobal::getGame()->getCamera().setFollowSpeed(p_followSpeed);
}


real CameraWrapper::getFollowSpeed()
{
	return AppGlobal::getGame()->getCamera().getFollowSpeed();
}


void CameraWrapper::setRenderHudOnly(bool p_enabled)
{
	TT_Printf("CameraWrapper::setRenderHudOnly: %d\n", p_enabled);
	
	toki::game::Camera& camera = AppGlobal::getGame()->getCamera();
	camera.setRenderHudOnly(p_enabled);
}


bool CameraWrapper::isRenderingHudOnly()
{
	return AppGlobal::getGame()->getCamera().isRenderingHudOnly();
}


tt::math::Vector2 CameraWrapper::screenToWorld(tt::math::Vector2 p_screenPos)
{
	const toki::game::Camera& cam = AppGlobal::getGame()->getCamera();
	
	// Translate Toki Tori screen space to hud screen space.
	const tt::math::Point2 scrSize(cam.getViewPortSize());
	
	p_screenPos.y  = -p_screenPos.y;
	p_screenPos   *=  scrSize.y;
	p_screenPos   +=  tt::math::Vector2(scrSize.x * 0.5f, scrSize.y * 0.5f);
	
	return cam.screenToWorld(tt::math::Point2(p_screenPos));
}


tt::math::Vector2 CameraWrapper::worldToScreen(const tt::math::Vector2& p_worldPos)
{
	const toki::game::Camera& cam = AppGlobal::getGame()->getCamera();
	
	tt::math::Vector2 screenPos(cam.worldToScreen(p_worldPos));
	
	const tt::math::Point2 scrSize(cam.getViewPortSize());
	
	screenPos -= tt::math::Vector2(scrSize.x * 0.5f, scrSize.y * 0.5f); // Center of screen is (0,0)
	screenPos /= scrSize.y;     // Normalized space
	screenPos.y = -screenPos.y; // Flip Y
	return screenPos;
}


bool CameraWrapper::isOnScreen(const tt::math::Vector2& p_worldPosition, real p_radius)
{
	return AppGlobal::getGame()->getCamera().isOnScreen(p_worldPosition, p_radius);
}


bool CameraWrapper::isFullyOnScreen(const tt::math::Vector2& p_worldPosition, real p_radius)
{
	return AppGlobal::getGame()->getCamera().isFullyOnScreen(p_worldPosition, p_radius);
}


bool CameraWrapper::isScrollingEnabled()
{
	return AppGlobal::getGame()->getCamera().isScrollingEnabled();
}


void CameraWrapper::setScrollingEnabled(bool p_enabled)
{
	AppGlobal::getGame()->getCamera().setScrollingEnabled(p_enabled);
}


void CameraWrapper::bind(const tt::script::VirtualMachinePtr& p_vm)
{
	TT_SQBIND_SETVM(p_vm);
	
	TT_SQBIND_INIT_NO_INSTANCING_NAME(CameraWrapper, "Camera");
	TT_SQBIND_STATIC_METHOD(CameraWrapper, createCameraEffect);
	TT_SQBIND_STATIC_METHOD(CameraWrapper, isMainCamera);
	TT_SQBIND_STATIC_METHOD(CameraWrapper, isSubCamera);
	TT_SQBIND_STATIC_METHOD(CameraWrapper, makeMainCamera);
	TT_SQBIND_STATIC_METHOD(CameraWrapper, isFollowingEntity);
	TT_SQBIND_STATIC_METHOD(CameraWrapper, setFollowEntity);
	TT_SQBIND_STATIC_METHOD(CameraWrapper, getTargetPosition);
	TT_SQBIND_STATIC_METHOD(CameraWrapper, getPosition);
	TT_SQBIND_STATIC_METHOD(CameraWrapper, setPosition);
	TT_SQBIND_STATIC_METHOD(CameraWrapper, getCameraDistance);
	TT_SQBIND_STATIC_METHOD(CameraWrapper, getVisibleRect);
	TT_SQBIND_STATIC_METHOD(CameraWrapper, getScreenSpaceRect);
	TT_SQBIND_STATIC_METHOD(CameraWrapper, setFOV);
	TT_SQBIND_STATIC_METHOD(CameraWrapper, getTargetFOV);
	TT_SQBIND_STATIC_METHOD(CameraWrapper, setRotation);
	TT_SQBIND_STATIC_METHOD(CameraWrapper, getCurrentRotation);
	TT_SQBIND_STATIC_METHOD(CameraWrapper, getTargetRotation);
	TT_SQBIND_STATIC_METHOD(CameraWrapper, setFollowSpeed);
	TT_SQBIND_STATIC_METHOD(CameraWrapper, getFollowSpeed);
	TT_SQBIND_STATIC_METHOD(CameraWrapper, setRenderHudOnly);
	TT_SQBIND_STATIC_METHOD(CameraWrapper, isRenderingHudOnly);
	TT_SQBIND_STATIC_METHOD(CameraWrapper, screenToWorld);
	TT_SQBIND_STATIC_METHOD(CameraWrapper, worldToScreen);
	TT_SQBIND_STATIC_METHOD(CameraWrapper, isOnScreen);
	TT_SQBIND_STATIC_METHOD(CameraWrapper, isFullyOnScreen);
	TT_SQBIND_STATIC_METHOD(CameraWrapper, isScrollingEnabled);
	TT_SQBIND_STATIC_METHOD(CameraWrapper, setScrollingEnabled);
}


// ------------------------------------------------------------------------------------------------

wrappers::CameraEffectWrapper DrcCameraWrapper::createCameraEffect()
{
	toki::game::Camera& camera = AppGlobal::getGame()->getCamera();
	
	return wrappers::CameraEffectWrapper(camera.createCameraEffect(),
	                                     wrappers::CameraEffectWrapper::CameraType_DRC);
}


bool DrcCameraWrapper::isEnabled()
{
	return AppGlobal::getGame()->isDrcCameraEnabled();
}


void DrcCameraWrapper::setEnabled(bool p_enabled)
{
	return AppGlobal::getGame()->setDrcCameraEnabled(p_enabled);
}


bool DrcCameraWrapper::isMainCamera()
{
	return AppGlobal::getGame()->isDrcCameraMain();
}


bool DrcCameraWrapper::isSubCamera()
{
	return isMainCamera() == false;
}


void DrcCameraWrapper::makeMainCamera()
{
	AppGlobal::getGame()->setDrcCameraAsMain(true);
}


bool DrcCameraWrapper::isFollowingEntity()
{
	return AppGlobal::getGame()->getDrcCamera().isFollowingEntity();
}


void DrcCameraWrapper::setFollowEntity(const EntityWrapper* p_entity)
{
	if (p_entity != 0)
	{
		AppGlobal::getGame()->getDrcCamera().setFollowEntity(p_entity->getHandle());
		return;
	}
	// else
	AppGlobal::getGame()->getDrcCamera().resetFollowEntity();
}


const tt::math::Vector2& DrcCameraWrapper::getTargetPosition()
{
	return AppGlobal::getGame()->getDrcCamera().getTargetPosition();
}


const tt::math::Vector2& DrcCameraWrapper::getPosition()
{
	return AppGlobal::getGame()->getDrcCamera().getCurrentPositionWithEffects();
}


void DrcCameraWrapper::setPosition(const tt::math::Vector2& p_position, real p_duration,
                                   tt::math::interpolation::EasingType p_easingType)
{
	setFollowEntity(0);
	AppGlobal::getGame()->getDrcCamera().setPositionWithEasing(p_position, p_duration, p_easingType);
}


real DrcCameraWrapper::getCameraDistance()
{
	return AppGlobal::getGame()->getDrcCamera().getCameraDistance();
}


const tt::math::VectorRect& DrcCameraWrapper::getVisibleRect()
{
	return AppGlobal::getGame()->getDrcCamera().getCurrentVisibleRect();
}


void DrcCameraWrapper::setFOV(real p_fovYDegrees, real p_duration,
                                      tt::math::interpolation::EasingType p_easingType)
{
	AppGlobal::getGame()->getDrcCamera().setFOVWithEasing(p_fovYDegrees, p_duration, p_easingType);
}


real DrcCameraWrapper::getTargetFOV()
{
	return AppGlobal::getGame()->getDrcCamera().getTargetFOV();
}


void DrcCameraWrapper::setRotation(real p_rotationInDegrees, real p_duration,
                                   tt::math::interpolation::EasingType p_easingType)
{
	AppGlobal::getGame()->getDrcCamera().setRotationWithEasing(tt::math::degToRad(p_rotationInDegrees),
	                                                           p_duration, p_easingType);
}


real DrcCameraWrapper::getCurrentRotation()
{
	return tt::math::radToDeg(AppGlobal::getGame()->getDrcCamera().getCurrentRotation());
}


real DrcCameraWrapper::getTargetRotation()
{
	return tt::math::radToDeg(AppGlobal::getGame()->getDrcCamera().getTargetRotation());
}


void DrcCameraWrapper::setFollowSpeed(real p_followSpeed)
{
	AppGlobal::getGame()->getDrcCamera().setFollowSpeed(p_followSpeed);
}


real DrcCameraWrapper::getFollowSpeed()
{
	return AppGlobal::getGame()->getDrcCamera().getFollowSpeed();
}


void DrcCameraWrapper::setRenderHudOnly(bool p_enabled)
{
	TT_Printf("DrcCameraWrapper::setRenderHudOnly: %d\n", p_enabled);
	
	toki::game::Camera& camera = AppGlobal::getGame()->getDrcCamera();
	camera.setRenderHudOnly(p_enabled);
}


bool DrcCameraWrapper::isRenderingHudOnly()
{
	return AppGlobal::getGame()->getDrcCamera().isRenderingHudOnly();
}


tt::math::Vector2 DrcCameraWrapper::screenToWorld(tt::math::Vector2 p_screenPos)
{
	const toki::game::Camera& cam = AppGlobal::getGame()->getDrcCamera();
	
	// Translate Toki Tori screen space to hud screen space.
	const tt::math::Point2 scrSize(cam.getViewPortSize());
	
	p_screenPos.y  = -p_screenPos.y;
	p_screenPos   *=  scrSize.y;
	p_screenPos   +=  tt::math::Vector2(scrSize.x * 0.5f, scrSize.y * 0.5f);
	
	return cam.screenToWorld(tt::math::Point2(p_screenPos));
}


tt::math::Vector2 DrcCameraWrapper::worldToScreen(const tt::math::Vector2& p_worldPos)
{
	const toki::game::Camera& cam = AppGlobal::getGame()->getDrcCamera();
	
	tt::math::Vector2 screenPos(cam.worldToScreen(p_worldPos));
	
	const tt::math::Point2 scrSize(cam.getViewPortSize());
	
	screenPos -= tt::math::Vector2(scrSize.x * 0.5f, scrSize.y * 0.5f); // Center of screen is (0,0)
	screenPos /= scrSize.y;     // Normalized space
	screenPos.y = -screenPos.y; // Flip Y
	return screenPos;
}


bool DrcCameraWrapper::isOnScreen(const tt::math::Vector2& p_worldPosition, real p_radius)
{
	return AppGlobal::getGame()->getDrcCamera().isOnScreen(p_worldPosition, p_radius);
}


bool DrcCameraWrapper::isScrollingEnabled()
{
	return AppGlobal::getGame()->getDrcCamera().isScrollingEnabled();
}


void DrcCameraWrapper::setScrollingEnabled(bool p_enabled)
{
	AppGlobal::getGame()->getDrcCamera().setScrollingEnabled(p_enabled);
}


void DrcCameraWrapper::bind(const tt::script::VirtualMachinePtr& p_vm)
{
	TT_SQBIND_SETVM(p_vm);
	
	TT_SQBIND_INIT_NO_INSTANCING_NAME(DrcCameraWrapper, "DrcCamera");
	TT_SQBIND_STATIC_METHOD(DrcCameraWrapper, createCameraEffect);
	TT_SQBIND_STATIC_METHOD(DrcCameraWrapper, isEnabled);
	TT_SQBIND_STATIC_METHOD(DrcCameraWrapper, setEnabled);
	TT_SQBIND_STATIC_METHOD(DrcCameraWrapper, isMainCamera);
	TT_SQBIND_STATIC_METHOD(DrcCameraWrapper, isSubCamera);
	TT_SQBIND_STATIC_METHOD(DrcCameraWrapper, makeMainCamera);
	TT_SQBIND_STATIC_METHOD(DrcCameraWrapper, isFollowingEntity);
	TT_SQBIND_STATIC_METHOD(DrcCameraWrapper, setFollowEntity);
	TT_SQBIND_STATIC_METHOD(DrcCameraWrapper, getTargetPosition);
	TT_SQBIND_STATIC_METHOD(DrcCameraWrapper, getPosition);
	TT_SQBIND_STATIC_METHOD(DrcCameraWrapper, setPosition);
	TT_SQBIND_STATIC_METHOD(DrcCameraWrapper, getCameraDistance);
	TT_SQBIND_STATIC_METHOD(DrcCameraWrapper, getVisibleRect);
	TT_SQBIND_STATIC_METHOD(DrcCameraWrapper, setFOV);
	TT_SQBIND_STATIC_METHOD(DrcCameraWrapper, getTargetFOV);
	TT_SQBIND_STATIC_METHOD(DrcCameraWrapper, setRotation);
	TT_SQBIND_STATIC_METHOD(DrcCameraWrapper, getCurrentRotation);
	TT_SQBIND_STATIC_METHOD(DrcCameraWrapper, getTargetRotation);
	TT_SQBIND_STATIC_METHOD(DrcCameraWrapper, setFollowSpeed);
	TT_SQBIND_STATIC_METHOD(DrcCameraWrapper, getFollowSpeed);
	TT_SQBIND_STATIC_METHOD(DrcCameraWrapper, setRenderHudOnly);
	TT_SQBIND_STATIC_METHOD(DrcCameraWrapper, isRenderingHudOnly);
	TT_SQBIND_STATIC_METHOD(DrcCameraWrapper, screenToWorld);
	TT_SQBIND_STATIC_METHOD(DrcCameraWrapper, worldToScreen);
	TT_SQBIND_STATIC_METHOD(DrcCameraWrapper, isOnScreen);
	TT_SQBIND_STATIC_METHOD(DrcCameraWrapper, isScrollingEnabled);
	TT_SQBIND_STATIC_METHOD(DrcCameraWrapper, setScrollingEnabled);
}


// Namespace end
}
}
}
}
