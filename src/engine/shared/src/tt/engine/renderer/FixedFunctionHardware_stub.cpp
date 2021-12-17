
#include <tt/engine/renderer/FixedFunctionHardware.h>


namespace tt {
namespace engine {
namespace renderer {


//--------------------------------------------------------------------------------------------------
// Public member functions

// Stubs for Fixed Function Pipeline on platforms that only support shaders


void FixedFunctionHardware::disableTexture(u32) { }

void FixedFunctionHardware::setProjectionMatrix(const math::Matrix44&)      { }
void FixedFunctionHardware::setViewMatrix      (const math::Matrix44&)      { }
void FixedFunctionHardware::setWorldMatrix     (const math::Matrix44&)      { }
void FixedFunctionHardware::setTextureMatrix   (const math::Matrix44&, s32) { }

void FixedFunctionHardware::setFogEnabled(bool)             { }
void FixedFunctionHardware::setFogMode   (FogMode)          { }
void FixedFunctionHardware::setFogColor  (const ColorRGBA&) { }
void FixedFunctionHardware::setFogSetting(FogSetting, real) { }

void FixedFunctionHardware::setMaterial         (const MaterialProperties&)              { }
void FixedFunctionHardware::setLight            (s32, const LightProperties&)            { }
void FixedFunctionHardware::setLightEnabled     (s32, bool)                              { }
void FixedFunctionHardware::setLightingEnabled  (bool)                                   { }
void FixedFunctionHardware::setAmbientLightColor(const tt::engine::renderer::ColorRGBA&) { }

void FixedFunctionHardware::setTextureStage    (s32, const TextureStageData&) { }
void FixedFunctionHardware::disableTextureStage(s32)                          { }


// Namespace end
}
}
}
