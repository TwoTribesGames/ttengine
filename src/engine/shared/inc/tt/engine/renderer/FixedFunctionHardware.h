#if !defined(INC_TT_ENGINE_RENDERER_FIXEDFUNCTIONHARDWARE_H)
#define INC_TT_ENGINE_RENDERER_FIXEDFUNCTIONHARDWARE_H


#include <tt/engine/renderer/enums.h>
#include <tt/engine/renderer/fwd.h>
#include <tt/math/fwd.h>


namespace tt {
namespace engine {
namespace renderer {


class FixedFunctionHardware
{
public:
	static void disableTexture(u32 p_channel);
	
	static void setProjectionMatrix(const math::Matrix44& p_projection);
	static void setViewMatrix      (const math::Matrix44& p_view);
	static void setWorldMatrix     (const math::Matrix44& p_world);
	
	static void setTextureMatrix(const math::Matrix44& p_matrix, s32 p_channel = 0);
	
	static void setFogEnabled(bool             p_enable);
	static void setFogMode   (FogMode          p_mode);
	static void setFogColor  (const ColorRGBA& p_color);
	static void setFogSetting(FogSetting       p_setting, real p_value);
	
	static void setMaterial(const MaterialProperties& p_properties);
	static void setLight       (s32 p_lightIndex, const LightProperties& p_properties);
	static void setLightEnabled(s32 p_lightIndex, bool p_enabled);
	static void setLightingEnabled(bool p_enabled);
	static void setAmbientLightColor(const ColorRGBA& p_color);
	
	static void setTextureStage    (s32 p_stage, const TextureStageData& p_stageData);
	static void disableTextureStage(s32 p_stage);
};


// Namespace end
}
}
}


#endif  // !defined(INC_TT_ENGINE_RENDERER_FIXEDFUNCTIONHARDWARE_H)
