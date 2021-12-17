#if !defined(INC_TOKI_GAME_SCRIPT_WRAPPERS_EFFECTMGRWRAPPER_H)
#define INC_TOKI_GAME_SCRIPT_WRAPPERS_EFFECTMGRWRAPPER_H

#include <tt/engine/renderer/fwd.h>
#include <tt/script/VirtualMachine.h>

#include <toki/game/script/wrappers/fwd.h>

namespace toki {
namespace game {
namespace script {
namespace wrappers {

/*! \brief 'EffectMgr' in Squirrel. */
class EffectMgrWrapper
{
public:
	/*! \brief Add fog color effect. */
	static void addFogColor(const EffectRectWrapper*              p_effectRect,
	                        real                                  p_baseStrength,
	                        const tt::engine::renderer::ColorRGB& p_color);
	
	/*! \brief Add fog near/far effect. */
	static void addFogNearFar(const EffectRectWrapper* p_effectRect,
	                          real                     p_baseStrength,
	                          real                     p_near,
	                          real                     p_far);
	
	/*! \brief Add Camera offset effect. */
	static void addCameraOffset(const EffectRectWrapper* p_effectRect, real p_baseStrength,
	                            const tt::math::Vector2& p_offset);
	
	/*! \brief Add DrcCamera offset effect. */
	static void addDrcCameraOffset(const EffectRectWrapper* p_effectRect, real p_baseStrength,
	                               const tt::math::Vector2& p_offset);
	
	/*! \brief Add Camera position effect. */
	static void addCameraPosition(const EffectRectWrapper* p_effectRect, real p_baseStrength,
	                              const tt::math::Vector2& p_position);
	
	/*! \brief Add DrcCamera position effect. */
	static void addDrcCameraPosition(const EffectRectWrapper* p_effectRect, real p_baseStrength,
	                                 const tt::math::Vector2& p_position);
	
	/*! \brief Add Light ambient effect. */
	static void addLightAmbient(const EffectRectWrapper* p_effectRect, real p_baseStrength,
	                            real p_ambient);
	
	/*! \brief Add Camera FOV effect. */
	static void addCameraFov(const EffectRectWrapper* p_effectRect, real p_baseStrength,
	                         real p_fov);
	
	/*! \brief Add DrcCamera FOV effect. */
	static void addDrcCameraFov(const EffectRectWrapper* p_effectRect, real p_baseStrength,
	                            real p_fov);
	
	/*! \brief Add ColorGrading effect. */
	static void addColorGrading(const EntityWrapper* p_entityForErrorReport, const EffectRectWrapper* p_effectRect,
	                            real p_baseStrength, const TextureWrapper* p_texture);
	
	static void bind(const tt::script::VirtualMachinePtr& p_vm);
	
private:
};

// Namespace end
}
}
}
}


#endif  // !defined(INC_TOKI_GAME_SCRIPT_WRAPPERS_EFFECTMGRWRAPPER_H)
