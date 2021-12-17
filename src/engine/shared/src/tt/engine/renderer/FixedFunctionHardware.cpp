
#include <tt/engine/renderer/FixedFunctionHardware.h>

#include <tt/engine/opengl_headers.h>
#include <tt/engine/renderer/gpu_capabilities.h>
#include <tt/engine/renderer/ColorRGBA.h>
#include <tt/engine/renderer/LightProperties.h>
#include <tt/engine/renderer/MaterialProperties.h>
#include <tt/engine/renderer/MatrixStack.h>
#include <tt/engine/renderer/MultiTexture.h>
#include <tt/engine/renderer/Shader.h>
#include <tt/engine/renderer/Texture.h>
#include <tt/engine/renderer/TexturePainter.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/engine/renderer/GLStateCache.h>
#include <tt/math/Matrix44.h>
#include <tt/math/Vector3.h>
#include <tt/platform/tt_error.h>
#include <tt/str/toStr.h>


namespace tt {
namespace engine {
namespace renderer {


//--------------------------------------------------------------------------------------------------
// Public member functions

// OpenGL Fixed Function Pipeline


void FixedFunctionHardware::disableTexture(u32 p_channel)
{
	Texture::unbindTexture(p_channel);
}


void FixedFunctionHardware::setProjectionMatrix(const math::Matrix44& p_projection)
{
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(reinterpret_cast<const GLfloat*>(&p_projection));
	glMatrixMode(GL_MODELVIEW);
}

static math::Matrix44 viewMatrix;

void FixedFunctionHardware::setViewMatrix(const math::Matrix44& p_view)
{
	viewMatrix = p_view;
	glLoadMatrixf(reinterpret_cast<const GLfloat*>(&p_view));
}


void FixedFunctionHardware::setWorldMatrix(const math::Matrix44& p_world)
{
	math::Matrix44 worldViewMatrix = p_world * viewMatrix;
	glLoadMatrixf(reinterpret_cast<const GLfloat*>(&worldViewMatrix));
}


void FixedFunctionHardware::setTextureMatrix(const math::Matrix44& p_matrix, s32 p_channel)
{
	// FIXME: No support for channel yet
	(void) p_channel;
	
	glMatrixMode(GL_TEXTURE);
	glLoadMatrixf(reinterpret_cast<const GLfloat*>(&p_matrix));
	glMatrixMode(GL_MODELVIEW);
}


void FixedFunctionHardware::setFogEnabled(bool p_enable)
{
	Renderer::getInstance()->stateCache()->setState(GL_FOG, p_enable);
}


void FixedFunctionHardware::setFogMode(FogMode p_mode)
{
	static const GLenum fogModes[] =
	{
		GL_LINEAR,     //<!FogMode_Linear
		GL_EXP,        //<!FogMode_Exponential
		GL_EXP2        //<!FogMode_ExponentialSquared
	};
	TT_STATIC_ASSERT(sizeof(fogModes) / sizeof(GLenum) == FogMode_Count);
	
	glFogi(GL_FOG_MODE, fogModes[p_mode]);
}


void FixedFunctionHardware::setFogColor(const ColorRGBA& p_color)
{
	const math::Vector4 fogColor = p_color.normalized();
	
	glFogfv(GL_FOG_COLOR, reinterpret_cast<const GLfloat*>(&fogColor));
}


void FixedFunctionHardware::setFogSetting(FogSetting p_setting, real p_value)
{
	static const GLenum fogSettings[] =
	{
		GL_FOG_START,       //<!FogSetting_Start
		GL_FOG_END,         //<!FogSetting_End
		GL_FOG_DENSITY      //<!FogSetting_Density
	};
	TT_STATIC_ASSERT(sizeof(fogSettings) / sizeof(GLenum) == FogSetting_Count);
	
	glFogf(fogSettings[p_setting], p_value);
}


void FixedFunctionHardware::setMaterial(const MaterialProperties& p_material)
{
	const math::Vector4 ambient  = p_material.ambient.normalized();
	const math::Vector4 diffuse  = p_material.diffuse.normalized();
	const math::Vector4 specular = p_material.specular.normalized();
	
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT , reinterpret_cast<const GLfloat*>(&ambient));
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE , reinterpret_cast<const GLfloat*>(&diffuse));
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, reinterpret_cast<const GLfloat*>(&specular));
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, p_material.specularPower);
}


void FixedFunctionHardware::setLight(s32 p_lightIndex, const LightProperties& p_properties)
{
	const GLenum lightIndex(GL_LIGHT0 + p_lightIndex);
	glLightfv(lightIndex, GL_DIFFUSE , reinterpret_cast<const GLfloat*>(&p_properties.color));
	glLightfv(lightIndex, GL_SPECULAR, reinterpret_cast<const GLfloat*>(&p_properties.color));
	glLightfv(lightIndex, GL_POSITION, reinterpret_cast<const GLfloat*>(&p_properties.positionDirection));
	
	glLightf(lightIndex, GL_CONSTANT_ATTENUATION , p_properties.attenuation.x);
	glLightf(lightIndex, GL_LINEAR_ATTENUATION   , p_properties.attenuation.y);
	glLightf(lightIndex, GL_QUADRATIC_ATTENUATION, p_properties.attenuation.z);
	
	// NOTE: OpenGL does not have a range parameter
}


void FixedFunctionHardware::setLightEnabled(s32 p_lightIndex, bool p_enabled)
{
	const GLenum lightIndex(GL_LIGHT0 + p_lightIndex);
	Renderer::getInstance()->stateCache()->setState(lightIndex, p_enabled);
}


void FixedFunctionHardware::setLightingEnabled(bool p_enabled)
{
	Renderer::getInstance()->stateCache()->setState(GL_LIGHTING, p_enabled);
	Renderer::getInstance()->stateCache()->setState(GL_NORMALIZE, p_enabled);

	// FIXME: Add color material setting here or enable it for everything (?)
}


void FixedFunctionHardware::setAmbientLightColor(const tt::engine::renderer::ColorRGBA &p_color)
{
	const math::Vector4 ambient = p_color.normalized();
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, reinterpret_cast<const GLfloat*>(&ambient));
}


void FixedFunctionHardware::setTextureStage(s32 p_stage, const TextureStageData& p_stageData)
{
	TT_MINMAX_ASSERT(p_stage, 0, static_cast<s32>(MultiTexture::getChannelCount() - 1));
	
	// Activate this stage
	Texture::setActiveChannel(static_cast<s32>(p_stage));
	
		//--- APPLY TEXTURE STAGE PARAMETERS ---
	// Texture coordinates set index
	// OpenGL texture coordinate sets do not exist, texcoord sets are actually bound to the corresponding stage:
	// (set 0 = stage 0, set 1 = stage 1) and therefore handled by the data layer
	
	Texture::setActiveClientChannel(p_stageData.getTexCoordIndex());
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	
	/*! \brief Lookup table matching shared enum TextureBlendOperation. */
	static const GLenum blendOp[] =
	{
		GL_REPLACE,     // TextureBlendOperation_Disable     - NOT DIRECTLY SUPPORTED -
		GL_REPLACE,     // TextureBlendOperation_SelectArg1  - NOT DIRECTLY SUPPORTED -
		GL_REPLACE,     // TextureBlendOperation_SelectArg2  - NOT DIRECTLY SUPPORTED -
		GL_MODULATE,    // TextureBlendOperation_Modulate
		GL_ADD,         // TextureBlendOperation_Add
		GL_SUBTRACT,    // TextureBlendOperation_Substract
		GL_MODULATE,    // TextureBlendOperation_Modulate2X  - NOT DIRECTLY SUPPORTED -
		GL_DECAL        // TextureBlendOperation_Decal
	};
	TT_STATIC_ASSERT(sizeof(blendOp) / sizeof(GLenum) == TextureBlendOperation_Count);
	
	// Setup the RGBA blend operations
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB  , blendOp[p_stageData.getColorBlendOperation()]);
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, blendOp[p_stageData.getAlphaBlendOperation()]);
	
	const real scaleColor = (p_stageData.getColorBlendOperation() == TextureBlendOperation_Modulate2X);
	glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE, scaleColor ? 2.0f : 1.0f);
	
	const real scaleAlpha = (p_stageData.getAlphaBlendOperation() == TextureBlendOperation_Modulate2X);
	glTexEnvf(GL_TEXTURE_ENV, GL_ALPHA_SCALE, scaleAlpha ? 2.0f : 1.0f);
	
	// TextureBlendOperation_Disable => In OpenGL this is done on a higher level, so this is already done
	
	// Set up correct stage for inverting color/alpha
	const bool invertColor1 = p_stageData.getColorSource1() >= TextureBlendSource_InverseOperandStart;
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, invertColor1 ? GL_ONE_MINUS_SRC_COLOR : GL_SRC_COLOR);
	
	const bool invertColor2 = p_stageData.getColorSource2() >= TextureBlendSource_InverseOperandStart;
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, invertColor2 ? GL_ONE_MINUS_SRC_COLOR : GL_SRC_COLOR);
	
	const bool invertAlpha1 = p_stageData.getAlphaSource1() >= TextureBlendSource_InverseOperandStart;
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, invertAlpha1 ? GL_ONE_MINUS_SRC_ALPHA : GL_SRC_ALPHA);
	
	const bool invertAlpha2 = p_stageData.getAlphaSource2() >= TextureBlendSource_InverseOperandStart;
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, invertAlpha2 ? GL_ONE_MINUS_SRC_ALPHA : GL_SRC_ALPHA);

	/*! \brief Lookup table matching shared enum TextureBlendSource. */
	static const GLenum blendSrc[] =
	{
		GL_TEXTURE,          // TextureBlendSource_Texture
		GL_PRIMARY_COLOR,    // TextureBlendSource_Diffuse
		GL_PREVIOUS,         // TextureBlendSource_Previous
		GL_CONSTANT,         // TextureBlendSource_Constant
		
		GL_TEXTURE,          // TextureBlendSource_OneMinusTexture
		GL_PRIMARY_COLOR,    // TextureBlendSource_OneMinusDiffuse
		GL_PREVIOUS,         // TextureBlendSource_OneMinusPrevious
		GL_CONSTANT,         // TextureBlendSource_OneMinusConstant
	};
	TT_STATIC_ASSERT(sizeof(blendSrc) / sizeof(GLenum) == TextureBlendSource_Count);
	
	// Color/alpha blend operation
	const bool useColorArg2 = (p_stageData.getColorBlendOperation() == TextureBlendOperation_SelectArg2);
	const bool useAlphaArg2 = (p_stageData.getAlphaBlendOperation() == TextureBlendOperation_SelectArg2);
	
	if (useColorArg2)
	{
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, blendSrc[p_stageData.getColorSource2()]);
	}
	else
	{
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, blendSrc[p_stageData.getColorSource1()]);
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, blendSrc[p_stageData.getColorSource2()]);
	}
	
	if (useAlphaArg2)
	{
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, blendSrc[p_stageData.getAlphaSource2()]);
	}
	else
	{
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, blendSrc[p_stageData.getAlphaSource1()]);
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_ALPHA, blendSrc[p_stageData.getAlphaSource2()]);
	}
	
	// Constant color
	math::Vector4 color = p_stageData.getConstant().normalized();
	glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, reinterpret_cast<GLfloat*>(&color));
	
	TT_CHECK_OPENGL_ERROR();
}


void FixedFunctionHardware::disableTextureStage(s32 p_stage)
{
	TT_MINMAX_ASSERT(p_stage, 0, static_cast<s32>(MultiTexture::getChannelCount() - 1));
	// FIXME: How to disable with OpenGL?
	TextureStageData defaultStage;
	defaultStage.setAlphaBlendOperation(TextureBlendOperation_Disable);
	defaultStage.setColorBlendOperation(TextureBlendOperation_Disable);
	setTextureStage(p_stage, defaultStage);
	Texture::unbindTexture(p_stage);
}

// Namespace end
}
}
}
