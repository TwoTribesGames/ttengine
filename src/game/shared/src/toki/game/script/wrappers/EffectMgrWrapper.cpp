#include <toki/game/entity/effect/EffectMgr.h>
#include <toki/game/script/wrappers/EffectMgrWrapper.h>
#include <toki/game/script/wrappers/EffectRectWrapper.h>
#include <toki/game/script/wrappers/TextureWrapper.h>
#include <toki/game/script/sqbind_bindings.h>
#include <toki/game/Game.h>
#include <toki/AppGlobal.h>


namespace toki {
namespace game {
namespace script {
namespace wrappers {

//--------------------------------------------------------------------------------------------------
// Public member functions

void EffectMgrWrapper::addFogColor(const EffectRectWrapper*              p_effectRect,
                                   real                                  p_baseStrength,
                                   const tt::engine::renderer::ColorRGB& p_color)
{
	if (p_effectRect == 0)
	{
		TT_NULL_ASSERT(p_effectRect);
		return;
	}
	
	entity::effect::EffectMgr& mgr = AppGlobal::getGame()->getEffectMgr();
	mgr.addFogColor(p_effectRect->getHandle(), p_baseStrength, p_color);
}


void EffectMgrWrapper::addFogNearFar(const EffectRectWrapper* p_effectRect,
                                     real                     p_baseStrength,
                                     real                     p_near,
                                     real                     p_far)
{
	if (p_effectRect == 0)
	{
		TT_NULL_ASSERT(p_effectRect);
		return;
	}
	
	entity::effect::EffectMgr& mgr = AppGlobal::getGame()->getEffectMgr();
	mgr.addFogNearFar(p_effectRect->getHandle(), p_baseStrength, p_near, p_far);
}


void EffectMgrWrapper::addCameraOffset(const EffectRectWrapper* p_effectRect, real p_baseStrength,
                                       const tt::math::Vector2& p_offset)
{
	if (p_effectRect == 0)
	{
		TT_NULL_ASSERT(p_effectRect);
		return;
	}
	
	entity::effect::EffectMgr& mgr = AppGlobal::getGame()->getEffectMgr();
	mgr.addCameraOffset(p_effectRect->getHandle(), p_baseStrength, p_offset);
}


void EffectMgrWrapper::addDrcCameraOffset(const EffectRectWrapper* p_effectRect, real p_baseStrength,
                                          const tt::math::Vector2& p_offset)
{
	if (p_effectRect == 0)
	{
		TT_NULL_ASSERT(p_effectRect);
		return;
	}
	
	entity::effect::EffectMgr& mgr = AppGlobal::getGame()->getEffectMgr();
	mgr.addDrcCameraOffset(p_effectRect->getHandle(), p_baseStrength, p_offset);
}


void EffectMgrWrapper::addCameraPosition(const EffectRectWrapper* p_effectRect, real p_baseStrength,
                                         const tt::math::Vector2& p_position)
{
	if (p_effectRect == 0)
	{
		TT_NULL_ASSERT(p_effectRect);
		return;
	}
	
	entity::effect::EffectMgr& mgr = AppGlobal::getGame()->getEffectMgr();
	mgr.addCameraPosition(p_effectRect->getHandle(), p_baseStrength, p_position);
}


void EffectMgrWrapper::addDrcCameraPosition(const EffectRectWrapper* p_effectRect, real p_baseStrength,
                                            const tt::math::Vector2& p_position)
{
	if (p_effectRect == 0)
	{
		TT_NULL_ASSERT(p_effectRect);
		return;
	}
	
	entity::effect::EffectMgr& mgr = AppGlobal::getGame()->getEffectMgr();
	mgr.addDrcCameraPosition(p_effectRect->getHandle(), p_baseStrength, p_position);
}


void EffectMgrWrapper::addLightAmbient(const EffectRectWrapper* p_effectRect, real p_baseStrength,
                                       real p_ambient)
{
	if (p_effectRect == 0)
	{
		TT_NULL_ASSERT(p_effectRect);
		return;
	}
	
	entity::effect::EffectMgr& mgr = AppGlobal::getGame()->getEffectMgr();
	mgr.addLightAmbient(p_effectRect->getHandle(), p_baseStrength, p_ambient);
}


void EffectMgrWrapper::addCameraFov(const EffectRectWrapper* p_effectRect, real p_baseStrength,
                                    real p_fov)
{
	if (p_effectRect == 0)
	{
		TT_NULL_ASSERT(p_effectRect);
		return;
	}
	entity::effect::EffectMgr& mgr = AppGlobal::getGame()->getEffectMgr();
	mgr.addCameraFov(p_effectRect->getHandle(), p_baseStrength, p_fov);
}


void EffectMgrWrapper::addDrcCameraFov(const EffectRectWrapper* p_effectRect, real p_baseStrength,
                                       real p_fov)
{
	if (p_effectRect == 0)
	{
		TT_NULL_ASSERT(p_effectRect);
		return;
	}
	entity::effect::EffectMgr& mgr = AppGlobal::getGame()->getEffectMgr();
	mgr.addDrcCameraFov(p_effectRect->getHandle(), p_baseStrength, p_fov);
}


void EffectMgrWrapper::addColorGrading(const EntityWrapper* p_entityForErrorReport,
                                       const EffectRectWrapper* p_effectRect, real p_baseStrength,
                                       const TextureWrapper* p_texture)
{
	if (p_effectRect == 0 || p_entityForErrorReport == 0 || p_texture == 0)
	{
		TT_NULL_ASSERT(p_effectRect);
		TT_NULL_ASSERT(p_entityForErrorReport);
		TT_NULL_ASSERT(p_texture);
		return;
	}
	entity::effect::EffectMgr& mgr = AppGlobal::getGame()->getEffectMgr();
	mgr.addColorGrading(p_entityForErrorReport->getHandle(), p_effectRect->getHandle(), p_baseStrength, p_texture->getTexture());
}


void EffectMgrWrapper::bind(const tt::script::VirtualMachinePtr& p_vm)
{
	TT_SQBIND_SETVM(p_vm);
	
	TT_SQBIND_INIT_NO_INSTANCING_NAME(EffectMgrWrapper, "EffectMgr");
	TT_SQBIND_STATIC_METHOD(EffectMgrWrapper, addFogColor);
	TT_SQBIND_STATIC_METHOD(EffectMgrWrapper, addFogNearFar);
	TT_SQBIND_STATIC_METHOD(EffectMgrWrapper, addCameraOffset);
	TT_SQBIND_STATIC_METHOD(EffectMgrWrapper, addDrcCameraOffset);
	TT_SQBIND_STATIC_METHOD(EffectMgrWrapper, addCameraPosition);
	TT_SQBIND_STATIC_METHOD(EffectMgrWrapper, addDrcCameraPosition);
	TT_SQBIND_STATIC_METHOD(EffectMgrWrapper, addLightAmbient);
	TT_SQBIND_STATIC_METHOD(EffectMgrWrapper, addCameraFov);
	TT_SQBIND_STATIC_METHOD(EffectMgrWrapper, addDrcCameraFov);
	TT_SQBIND_STATIC_METHOD(EffectMgrWrapper, addColorGrading);
}


// Namespace end
}
}
}
}
