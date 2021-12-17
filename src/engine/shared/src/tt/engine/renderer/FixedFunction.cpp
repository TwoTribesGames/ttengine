
#include <tt/engine/renderer/FixedFunction.h>


#include <tt/engine/renderer/gpu_capabilities.h>
#include <tt/engine/renderer/ColorRGBA.h>
#include <tt/engine/renderer/LightProperties.h>
#include <tt/engine/renderer/FixedFunctionHardware.h>
#include <tt/engine/renderer/MaterialProperties.h>
#include <tt/engine/renderer/MatrixStack.h>
#include <tt/engine/renderer/MultiTexture.h>
#include <tt/engine/renderer/Shader.h>
#include <tt/engine/renderer/Texture.h>
#include <tt/engine/renderer/TextureStageData.h>
#include <tt/engine/renderer/TexturePainter.h>
#include <tt/math/Matrix44.h>
#include <tt/math/Vector3.h>
#include <tt/platform/tt_error.h>
#include <tt/str/toStr.h>


namespace tt {
namespace engine {
namespace renderer {

enum ShaderVariables
{
	// Vertex Shader
	ShaderVariable_Projection,
	ShaderVariable_View,
	ShaderVariable_World,
	ShaderVariable_NormalMatrix,
	ShaderVariable_TextureMatrix0,
	ShaderVariable_TextureMatrix1,
	ShaderVariable_FogSettings,
	ShaderVariable_MainTextureSize,
	ShaderVariable_UseVertexColor,
	
	// Pixel Shader
	ShaderVariable_FogColor,
	ShaderVariable_OverdrawColor,
	ShaderVariable_DebugFeatures,
	
	ShaderVariable_Count,
};
ShaderHandle g_shaderVarHandles[ShaderVariable_Count];

static void buildShaderHandleTable(const ShaderPtr& p_shader)
{
	g_shaderVarHandles[ShaderVariable_Projection     ] = p_shader->getVertexUniformHandle("u_projection");
	g_shaderVarHandles[ShaderVariable_View           ] = p_shader->getVertexUniformHandle("u_view");
	g_shaderVarHandles[ShaderVariable_World          ] = p_shader->getVertexUniformHandle("u_world");
	g_shaderVarHandles[ShaderVariable_NormalMatrix   ] = p_shader->getVertexUniformHandle("u_normalMatrix");
	g_shaderVarHandles[ShaderVariable_TextureMatrix0 ] = p_shader->getVertexUniformHandle("u_textureMtx[0]");
	g_shaderVarHandles[ShaderVariable_TextureMatrix1 ] = p_shader->getVertexUniformHandle("u_textureMtx[1]");
	g_shaderVarHandles[ShaderVariable_FogSettings    ] = p_shader->getVertexUniformHandle("u_fogSettings");
	g_shaderVarHandles[ShaderVariable_MainTextureSize] = p_shader->getVertexUniformHandle("u_mainTextureSize");
	g_shaderVarHandles[ShaderVariable_UseVertexColor ] = p_shader->getVertexUniformHandle("u_useVtxColor");
	
	g_shaderVarHandles[ShaderVariable_FogColor       ] = p_shader->getPixelUniformHandle("u_fogColor");
	g_shaderVarHandles[ShaderVariable_OverdrawColor  ] = p_shader->getPixelUniformHandle("u_overdrawColor");
	g_shaderVarHandles[ShaderVariable_DebugFeatures  ] = p_shader->getPixelUniformHandle("u_debugFeatures");
}


ShaderPtr      FixedFunction::ms_shader;
TexturePtr     FixedFunction::ms_whiteTexture;
math::Vector4  FixedFunction::ms_fogSettings;
math::Vector4  FixedFunction::ms_fogColor;
FogMode        FixedFunction::ms_fogMode;
bool           FixedFunction::ms_useShaderEmulation = false;
bool           FixedFunction::ms_lightingEnabled    = false;
math::Matrix44 FixedFunction::ms_textureMatrix[FixedFunction::maxSupportedStages];
math::Vector4  FixedFunction::ms_textureStageEnabled;

#if !defined(TT_BUILD_FINAL)
TexturePtr FixedFunction::ms_mipVisualizeTexture;
math::Vector4 FixedFunction::ms_debugFeatures = math::Vector4::zero;
#endif


// FIXME: Can we solve this in a better way?
#if defined(TT_PLATFORM_OSX) || defined(TT_PLATFORM_LNX)
const bool matrixNeedsTranspose = true;
#else
const bool matrixNeedsTranspose = false;
#endif



//--------------------------------------------------------------------------------------------------
// Public member functions


bool FixedFunction::initialize(const EngineID& p_shader)
{
	if (ms_shader != 0)
	{
		// Already initialized
		return true;
	}
	
	ms_useShaderEmulation = hasShaderSupport();
	
	if (ms_useShaderEmulation == false)
	{
		TT_Printf("Shaders not available: running in Fixed Function mode...\n");
		return true;
	}
	
	ms_shader = ShaderCache::get(p_shader, false);
	
	if (ms_shader == 0)
	{
		TT_PANIC("FixedFunction shader cannot be found. A FF shader is needed with ID [%s]"
			"Falling back to fixed function pipeline if available.",
			p_shader.toDebugString().c_str());
		ms_useShaderEmulation = false;
		return false;
	}
	
	buildShaderHandleTable(ms_shader);
	
	setupShaderVariables();
	
	// Create small white texture to use when color only is wanted
	ms_whiteTexture = Texture::createForText(4, 4);
	TT_NULL_ASSERT(ms_whiteTexture);
	
	{
		TexturePainter painter = ms_whiteTexture->lock();
		painter.clearToWhite();
	}
	
#if !defined(TT_BUILD_FINAL) && defined(TT_PLATFORM_WIN)
	// FIXME: Implement on other platforms as well
	ms_mipVisualizeTexture = Texture::createForText(32,32);
	ms_mipVisualizeTexture->createMipVisualizationTexture();
	TT_NULL_ASSERT(ms_mipVisualizeTexture);
#endif
	
	return true;
}


void FixedFunction::destroy()
{
	ms_shader.reset();
	ms_whiteTexture.reset();
	
#if !defined(TT_BUILD_FINAL) && defined(TT_PLATFORM_WIN)
	ms_mipVisualizeTexture.reset();
#endif
}


void FixedFunction::disableTexture(u32 p_channel)
{
	if (ms_useShaderEmulation)
	{
		if (ms_whiteTexture != 0)
		{
			ms_whiteTexture->select(p_channel);
		}
	}
	else
	{
		FixedFunctionHardware::disableTexture(p_channel);
	}
}


void FixedFunction::setProjectionMatrix(const math::Matrix44& p_projection)
{
	if (ms_useShaderEmulation)
	{
		ms_shader->setVertexParam(
			g_shaderVarHandles[ShaderVariable_Projection], matrixNeedsTranspose ? p_projection.getTranspose() : p_projection);
	}
	else
	{
		FixedFunctionHardware::setProjectionMatrix(p_projection);
	}
}


void FixedFunction::setViewMatrix(const math::Matrix44& p_view)
{
	if (ms_useShaderEmulation)
	{
		ms_shader->setVertexParam(
			g_shaderVarHandles[ShaderVariable_View], matrixNeedsTranspose ? p_view.getTranspose() : p_view);
	}
	else
	{
		FixedFunctionHardware::setViewMatrix(p_view);
	}
}


void FixedFunction::setWorldMatrix(const math::Matrix44& p_world)
{
	if (ms_useShaderEmulation)
	{
		if (ms_lightingEnabled)
		{
			math::Matrix44 normalMatrix = p_world.getInverse();
			
			// Instead of a double transpose for OpenGL we just skip this
			if (matrixNeedsTranspose == false)
			{
				normalMatrix.transpose();
			}
			ms_shader->setVertexParam(g_shaderVarHandles[ShaderVariable_NormalMatrix], normalMatrix);
		}
		ms_shader->setVertexParam(
			g_shaderVarHandles[ShaderVariable_World], matrixNeedsTranspose ? p_world.getTranspose() : p_world);
	}
	else
	{
		FixedFunctionHardware::setWorldMatrix(p_world);
	}
}



void FixedFunction::setCameraPosition(const math::Vector3& p_position)
{
	if (ms_useShaderEmulation)
	{
		const math::Vector4 cameraPosition(p_position, 1.0f);
		ms_shader->setParameter("u_cameraPosition", cameraPosition);
	}
}


void FixedFunction::setTextureMatrix(const math::Matrix44& p_matrix, s32 p_channel)
{
	if (p_matrix == ms_textureMatrix[p_channel]) return;
	
	if (ms_useShaderEmulation)
	{
		ms_shader->setVertexParam(
			g_shaderVarHandles[ShaderVariable_TextureMatrix0 + p_channel], matrixNeedsTranspose ? p_matrix.getTranspose() : p_matrix);
	}
	else
	{
		FixedFunctionHardware::setTextureMatrix(p_matrix, p_channel);
	}
	
	ms_textureMatrix[p_channel] = p_matrix;
}


void FixedFunction::setFogEnabled(bool p_enable)
{
	if (ms_useShaderEmulation)
	{
		ms_fogSettings.x = p_enable ? 1.0f : 0.0f;
		ms_shader->setVertexParam(g_shaderVarHandles[ShaderVariable_FogSettings], ms_fogSettings);
	}
	else
	{
		FixedFunctionHardware::setFogEnabled(p_enable);
	}
}


void FixedFunction::setFogMode(FogMode p_mode)
{
	if (ms_useShaderEmulation)
	{
		// Do nothing -- Only linear fog supported now
	}
	else
	{
		FixedFunctionHardware::setFogMode(p_mode);
	}
	
	ms_fogMode = p_mode;
}


void FixedFunction::setFogColor(const ColorRGBA& p_color)
{
	if (ms_useShaderEmulation)
	{
		const math::Vector4 fogColor = p_color.normalized();
		
		ms_fogColor.x = fogColor.x;
		ms_fogColor.y = fogColor.y;
		ms_fogColor.z = fogColor.z;
		
		ms_shader->setPixelParam(g_shaderVarHandles[ShaderVariable_FogColor], ms_fogColor);
	}
	else
	{
		FixedFunctionHardware::setFogColor(p_color);
	}
}


void FixedFunction::setFogSetting(FogSetting p_setting, real p_value)
{
	if (ms_useShaderEmulation)
	{
		switch(p_setting)
		{
		case FogSetting_Start  : ms_fogSettings.y = p_value; break;
		case FogSetting_End    : ms_fogSettings.z = p_value; break;
		case FogSetting_Density: ms_fogSettings.w = p_value; break;
		default:
			TT_PANIC("Invalid fog setting (%d)", p_setting);
		}
		
		ms_shader->setVertexParam(g_shaderVarHandles[ShaderVariable_FogSettings], ms_fogSettings);
	}
	else
	{
		FixedFunctionHardware::setFogSetting(p_setting, p_value);
	}
}


void FixedFunction::setPremultipliedAlpha(bool p_enabled)
{
	if (ms_useShaderEmulation)
	{
		ms_fogColor.w = p_enabled ? 0.0f : 1.0f;
		ms_shader->setPixelParam(g_shaderVarHandles[ShaderVariable_FogColor], ms_fogColor);
	}
}


void FixedFunction::setMaterial(const MaterialProperties& p_material)
{
	if (ms_useShaderEmulation)
	{
		const math::Vector4 ambient = p_material.ambient.normalized();
		const math::Vector4 diffuse = p_material.diffuse.normalized();
		math::Vector4 specular = p_material.specular.normalized();
		specular.w = p_material.specularPower;
		
		ms_shader->setParameter("u_material.ambient" , ambient);
		ms_shader->setParameter("u_material.diffuse" , diffuse);
		ms_shader->setParameter("u_material.specular", specular);
	}
	else
	{
		FixedFunctionHardware::setMaterial(p_material);
	}
}


void FixedFunction::setLight(s32 p_lightIndex, const LightProperties& p_properties)
{
	if (ms_useShaderEmulation)
	{
		const std::string lightName = "u_light[" + str::toStr(p_lightIndex) + "]";
		ms_shader->setParameter(lightName + ".color"      , p_properties.color);
		ms_shader->setParameter(lightName + ".posDir"     , p_properties.positionDirection);
		ms_shader->setParameter(lightName + ".attenuation", p_properties.attenuation);
	}
	else
	{
		FixedFunctionHardware::setLight(p_lightIndex, p_properties);
	}
}


void FixedFunction::setLightEnabled(s32 p_lightIndex, bool p_enabled)
{
	if (ms_useShaderEmulation)
	{
		static math::Vector4 lightEnabled(0,0,0,0);
		
		switch (p_lightIndex)
		{
		case 0: lightEnabled.x = p_enabled ? 1.0f : 0.0f; break;
		case 1: lightEnabled.y = p_enabled ? 1.0f : 0.0f; break;
		case 2: lightEnabled.z = p_enabled ? 1.0f : 0.0f; break;
		case 3: lightEnabled.w = p_enabled ? 1.0f : 0.0f; break;
		default:
			TT_PANIC("Invalid light index (%d), maximum supported lights exceeded.\n", p_lightIndex);
			break;
		}
		ms_shader->setParameter("u_lightEnabled", lightEnabled);
	}
	else
	{
		FixedFunctionHardware::setLightEnabled(p_lightIndex, p_enabled);
	}
}
	
	
void FixedFunction::setLightingEnabled(bool p_enabled)
{
	if (ms_useShaderEmulation)
	{
		ms_shader->setParameter("u_lightingEnabled", p_enabled ? 1.0f : 0.0f);
		ms_lightingEnabled = p_enabled;
	}
	else
	{
		FixedFunctionHardware::setLightingEnabled(p_enabled);
	}
}


void FixedFunction::setAmbientLightColor(const tt::engine::renderer::ColorRGBA &p_color)
{
	if (ms_useShaderEmulation)
	{
		ms_shader->setParameter("u_ambientLight", p_color.normalized());
	}
	else
	{
		FixedFunctionHardware::setAmbientLightColor(p_color);
	}
}


void FixedFunction::setVertexColorEnabled(bool p_enabled)
{
	if (ms_useShaderEmulation)
	{
		// OpenGL at least MUST set the a single float for a bool uniform
		const float enabled = p_enabled ? 1.0f : 0.0f;
		ms_shader->setVertexUniform(g_shaderVarHandles[ShaderVariable_UseVertexColor], 1, (const void*)&enabled);
	}
}


void FixedFunction::setTextureStage(s32 p_stage, const TextureStageData& p_stageData)
{
	const TexturePtr& texture = p_stageData.getTexture();
	
#if !defined(TT_BUILD_FINAL) && defined(TT_PLATFORM_WIN)
	// FIXME: Implement on other platforms as well
	// Do this BEFORE setting texture slot 0 to prevent overwriting premultiplied alpha setting
	if (p_stage == 0)
	{
		if (texture != 0 && ms_debugFeatures.y > 0.0f)
		{
			math::Vector4 textureSize(1.0f * texture->getWidth(), 1.0f * texture->getHeight());
			ms_shader->setVertexParam(g_shaderVarHandles[ShaderVariable_MainTextureSize], textureSize);
			ms_mipVisualizeTexture->select(1);
		}
	}
#endif
	// Set texture & texture matrix
	if (texture != 0)
	{
		texture->select(p_stage);
		setTextureMatrix(p_stageData.getMatrix(), p_stage);
	}
	else
	{
		disableTexture(p_stage);
	}
	
	if (ms_useShaderEmulation)
	{
		switch (p_stage)
		{
		case 0: ms_textureStageEnabled.x = 1.0f; break;
		case 1: ms_textureStageEnabled.y = 1.0f; break;
		case 2: ms_textureStageEnabled.z = 1.0f; break;
		case 3: ms_textureStageEnabled.w = 1.0f; break;
		default:
			TT_PANIC("Unsupported texture stage (%d), only %d stages supported.",
				p_stage, maxSupportedStages);
		}
			ms_shader->setParameter("u_textureStageEnabled", ms_textureStageEnabled);
	}
	else
	{
		FixedFunctionHardware::setTextureStage(p_stage, p_stageData);
	}
	
	MultiTexture::setStageEnabled(p_stage, true);
}


void FixedFunction::disableTextureStage(s32 p_stage)
{
	if (MultiTexture::isStageEnabled(p_stage))
	{
		if (ms_useShaderEmulation)
		{
			switch (p_stage)
			{
			case 0: ms_textureStageEnabled.x = 0.0f; break;
			case 1: ms_textureStageEnabled.y = 0.0f; break;
			case 2: ms_textureStageEnabled.z = 0.0f; break;
			case 3: ms_textureStageEnabled.w = 0.0f; break;
			default:
				TT_PANIC("Unsupported texture stage (%d), only %d stages supported.",
					p_stage, maxSupportedStages);
			}
			ms_shader->setParameter("u_textureStageEnabled", ms_textureStageEnabled);
		}
		else
		{
			FixedFunctionHardware::disableTextureStage(p_stage);
		}
		MultiTexture::setStageEnabled(p_stage, false);
	}
}


void FixedFunction::resetTextureStages(s32 p_startStage)
{
	s32 firstStage(p_startStage);
	if (firstStage == 0)
	{
		static TextureStageData stageData;
		FixedFunction::setTextureStage(0, stageData);
		++firstStage;
	}
	
	for (s32 i = firstStage; i < static_cast<s32>(MultiTexture::getChannelCount()); ++i)
	{
		disableTextureStage(i);
	}
}


void FixedFunction::setActive()
{
	if (ms_useShaderEmulation)
	{
		if (ms_shader != 0)
		{
			ms_shader->select();
			setTextureMatrix(math::Matrix44::identity);
		}
		else
		{
			TT_PANIC("No shader loaded for fixed function!");
			ms_useShaderEmulation = false;
		}
	}
	else
	{
		Shader::resetActiveShader();
	}
}


#ifndef TT_BUILD_FINAL
void FixedFunction::setOverdrawModeEnabled(bool p_enable)
{
	ms_debugFeatures.x = p_enable ? 1.0f : 0.0f;
	if (ms_shader != 0)
	{
		ms_shader->setPixelParam(g_shaderVarHandles[ShaderVariable_DebugFeatures], ms_debugFeatures);
	}
}


void FixedFunction::setOverdrawColor(const ColorRGBA& p_color)
{
	if (ms_shader != 0)
	{
		ms_shader->setPixelParam(g_shaderVarHandles[ShaderVariable_OverdrawColor], p_color.normalized());
	}
}


void FixedFunction::setMipmapVisualizationEnabled(bool p_enable)
{
	ms_debugFeatures.y = p_enable ? 1.0f : 0.0f;
	if (ms_shader != 0)
	{
		ms_shader->setPixelParam(g_shaderVarHandles[ShaderVariable_DebugFeatures], ms_debugFeatures);
	}
}


void FixedFunction::toggleShaderEmulation()
{
#if defined(TT_PLATFORM_WIN) || defined(TT_PLATFORM_OSX) || defined(TT_PLATFORM_LNX)
	ms_useShaderEmulation = !ms_useShaderEmulation;
	setActive();
#endif
}

#endif // TT_BUILD_FINAL


//--------------------------------------------------------------------------------------------------
// Private member functions

void FixedFunction::setupShaderVariables()
{
	ms_shader->setParameter("u_world"              , math::Matrix44::identity);
	ms_shader->setParameter("u_textureMtx[0]"      , math::Matrix44::identity);
	ms_shader->setParameter("u_textureStageEnabled", math::Vector4(1,0,0,0));
	setFogEnabled(false);
	ms_fogColor = math::Vector4(1,1,1,1);
	
	
	TT_ASSERT(ms_shader->getSamplerIndex("diffuse") == 0 || ms_shader->getSamplerIndex("diffuse[0]") == 0);
	
	for (s32 channel = 0; channel < maxSupportedStages; ++channel)
	{
		// NOTE: OpenGL implementation requires that getSamplerIndex() has been called at least once
		//       to bind index value to the shader sampler variable
		const std::string samplerName("diffuse[" + tt::str::toStr(channel) + "]");
		const s32 samplerIndex = ms_shader->getSamplerIndex(samplerName);
		TT_ASSERT(samplerIndex == -1 || samplerIndex == channel);
	}
	
	
#ifndef TT_BUILD_FINAL
	setOverdrawModeEnabled(false);
	setMipmapVisualizationEnabled(false);
#endif
}


// Namespace end
}
}
}
