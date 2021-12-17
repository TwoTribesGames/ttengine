#if !defined(INC_TT_ENGINE_RENDERER_FIXEDFUNCTION_H)
#define INC_TT_ENGINE_RENDERER_FIXEDFUNCTION_H


#include <tt/engine/renderer/enums.h>
#include <tt/engine/renderer/fwd.h>
#include <tt/math/fwd.h>


namespace tt {
namespace engine {
namespace renderer {


class FixedFunction
{
public:
	static bool initialize(const EngineID& p_shader);
	static void destroy();
	
	static void disableTexture(u32 p_channel);
	
	static void setProjectionMatrix(const math::Matrix44& p_projection);
	static void setViewMatrix      (const math::Matrix44& p_view);
	static void setWorldMatrix     (const math::Matrix44& p_world);
	static void setCameraPosition  (const math::Vector3&  p_position);
	
	static void setTextureMatrix(const math::Matrix44& p_matrix, s32 p_channel = 0);
	
	static void setFogEnabled(bool             p_enable);
	static void setFogMode   (FogMode          p_mode);
	static void setFogColor  (const ColorRGBA& p_color);
	static void setFogSetting(FogSetting       p_setting, real p_value);
	
	static void setPremultipliedAlpha(bool p_enabled);
	
	static void setMaterial(const MaterialProperties& p_properties);
	static void setLight       (s32 p_lightIndex, const LightProperties& p_properties);
	static void setLightEnabled(s32 p_lightIndex, bool p_enabled);
	static void setLightingEnabled(bool p_enabled);
	static void setAmbientLightColor(const ColorRGBA& p_color);
	static void setVertexColorEnabled(bool p_enabled);
	
	static void setTextureStage(s32 p_stage, const TextureStageData& p_stageData);
	static void disableTextureStage(s32 p_stage);
	static void resetTextureStages(s32 p_startStage = 0);
	
	static void setActive();
	static bool isInitialized() { return ms_shader != 0; }

#ifndef TT_BUILD_FINAL
	static void setOverdrawModeEnabled(bool p_enable);
	static void setOverdrawColor(const ColorRGBA& p_color);

	static void setMipmapVisualizationEnabled(bool p_enable);
	static void toggleShaderEmulation();
#endif
	static inline bool isShaderEmulationUsed() { return ms_useShaderEmulation; }

private:
	FixedFunction();
	~FixedFunction();
	
	static void setupShaderVariables();
	
	static ShaderPtr      ms_shader;
	static TexturePtr     ms_whiteTexture;
	static math::Vector4  ms_fogSettings;
	static math::Vector4  ms_fogColor;
	static FogMode        ms_fogMode;
	static bool           ms_useShaderEmulation;
	static bool           ms_lightingEnabled;
	
	static const s32 maxSupportedStages = 4;
	static math::Matrix44 ms_textureMatrix[maxSupportedStages];
	static math::Vector4  ms_textureStageEnabled;
	
#if !defined(TT_BUILD_FINAL)
	static TexturePtr       ms_mipVisualizeTexture;
	static math::Vector4    ms_debugFeatures;
#endif
};


// Namespace end
}
}
}


#endif  // !defined(INC_TT_ENGINE_RENDERER_FIXEDFUNCTION_H)
