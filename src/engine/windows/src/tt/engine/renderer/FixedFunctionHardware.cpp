
#include <tt/engine/renderer/FixedFunctionHardware.h>

#include <tt/engine/renderer/directx.h>
#include <tt/engine/renderer/ColorRGBA.h>
#include <tt/engine/renderer/LightProperties.h>
#include <tt/engine/renderer/MaterialProperties.h>
#include <tt/engine/renderer/MatrixStack.h>
#include <tt/engine/renderer/MultiTexture.h>
#include <tt/engine/renderer/Texture.h>
#include <tt/math/Matrix44.h>
#include <tt/math/Vector3.h>
#include <tt/platform/tt_error.h>


namespace tt {
namespace engine {
namespace renderer {

// DirectX Fixed Function Pipeline

//--------------------------------------------------------------------------------------------------
// Helper functions

static void setTransform(D3DTRANSFORMSTATETYPE p_type, const math::Matrix44& p_matrix)
{
	checkD3DSucceeded(
		getRenderDevice(true)->SetTransform(p_type, reinterpret_cast<const D3DMATRIX*>(&p_matrix))
	);
}


static void setRenderState(D3DRENDERSTATETYPE p_type, DWORD p_value, IDirect3DDevice9* p_device)
{
	TT_NULL_ASSERT(p_device);
	checkD3DSucceeded( p_device->SetRenderState(p_type, p_value) );
}


static void setRenderState(D3DRENDERSTATETYPE p_type, DWORD p_value)
{
	setRenderState(p_type, p_value, getRenderDevice(true));
}


static void setTextureStageState(DWORD                    p_stage,
	                             D3DTEXTURESTAGESTATETYPE p_type,
	                             DWORD                    p_value,
	                             IDirect3DDevice9*        p_device)
{
	checkD3DSucceeded( p_device->SetTextureStageState(p_stage, p_type, p_value) );
}


//--------------------------------------------------------------------------------------------------
// Public member functions


void FixedFunctionHardware::disableTexture(u32 p_channel)
{
	// FIXME: This actually disables a texture stage instead of just binding a 0 texture
	//        Check with OpenGL usage
	IDirect3DDevice9* device = getRenderDevice(true);
	if (device != 0)
	{
		checkD3DSucceeded( device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_DISABLE) );
		checkD3DSucceeded( device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE) );
		checkD3DSucceeded( device->SetTexture(0,0) );
	}
}


void FixedFunctionHardware::setProjectionMatrix(const math::Matrix44& p_projection)
{
	setTransform(D3DTS_PROJECTION, p_projection);
}


void FixedFunctionHardware::setViewMatrix(const math::Matrix44& p_view)
{
	setTransform(D3DTS_VIEW, p_view);
}


void FixedFunctionHardware::setWorldMatrix(const math::Matrix44& p_world)
{
	setTransform(D3DTS_WORLD, p_world);
}


void FixedFunctionHardware::setTextureMatrix(const math::Matrix44& p_matrix, s32 p_channel)
{
	math::Matrix44 texMatrix(p_matrix);
	
	// Convert to Windows style texture matrix
	texMatrix.m_31 = texMatrix.m_41;
	texMatrix.m_32 = texMatrix.m_42;
	
	setTransform(static_cast<D3DTRANSFORMSTATETYPE>(D3DTS_TEXTURE0 + p_channel), texMatrix);
}


void FixedFunctionHardware::setFogEnabled(bool p_enable)
{
	setRenderState(D3DRS_FOGENABLE, p_enable ? TRUE : FALSE);
}


void FixedFunctionHardware::setFogMode(FogMode p_mode)
{
	static const DWORD fogModes[] =
	{
		D3DFOG_LINEAR,      //<!FogMode_Linear
		D3DFOG_EXP,         //<!FogMode_Exponential
		D3DFOG_EXP2         //<!FogMode_ExponentialSquared
	};
	TT_STATIC_ASSERT(sizeof(fogModes) / sizeof(DWORD) == FogMode_Count);
	
	setRenderState(D3DRS_FOGVERTEXMODE, fogModes[p_mode]);
}


void FixedFunctionHardware::setFogColor(const ColorRGBA& p_color)
{
	setRenderState(D3DRS_FOGCOLOR, D3DCOLOR_RGBA(p_color.r, p_color.g, p_color.b, p_color.a));
}


void FixedFunctionHardware::setFogSetting(FogSetting p_setting, real p_value)
{
	static const D3DRENDERSTATETYPE fogSettings[] =
	{
		D3DRS_FOGSTART,      //<!FogSetting_Start
		D3DRS_FOGEND,        //<!FogSetting_End
		D3DRS_FOGDENSITY     //<!FogSetting_Density
	};
	TT_STATIC_ASSERT(sizeof(fogSettings) / sizeof(D3DRENDERSTATETYPE) == FogSetting_Count);
	
	setRenderState(fogSettings[p_setting], *reinterpret_cast<DWORD*>(&p_value) );
}


void FixedFunctionHardware::setMaterial(const MaterialProperties& p_material)
{
	// TODO: Add caching / checking for currently active material
	D3DMATERIAL9 material = {};
	
	math::Vector4 color = p_material.ambient.normalized();
	material.Ambient = *reinterpret_cast<D3DCOLORVALUE*>(&color);
	
	color = p_material.diffuse.normalized();
	material.Diffuse = *reinterpret_cast<D3DCOLORVALUE*>(&color);
	
	color = p_material.specular.normalized();
	material.Specular = *reinterpret_cast<D3DCOLORVALUE*>(&color);
	
	// NOTE: Emissive is not supported because we have data that has incorrect emissive values
	// color = p_material.emissive.normalized();
	material.Emissive = D3DXCOLOR(0,0,0,0);
	
	material.Power = p_material.specularPower;
	
	checkD3DSucceeded( getRenderDevice(true)->SetMaterial(&material) );
}


void FixedFunctionHardware::setLight(s32 p_lightIndex, const LightProperties& p_properties)
{
	D3DLIGHT9 light = {};
	light.Diffuse  = *reinterpret_cast<const D3DCOLORVALUE*>(&p_properties.color);
	light.Specular = *reinterpret_cast<const D3DCOLORVALUE*>(&p_properties.color);
	
	if (p_properties.positionDirection.w > 0.0f)
	{
		light.Position.x = p_properties.positionDirection.x;
		light.Position.y = p_properties.positionDirection.y;
		light.Position.z = p_properties.positionDirection.z;
		light.Type       = D3DLIGHT_POINT;
	}
	else
	{
		// NOTE: Flipping the sign because DirectX wants vector from light source
		light.Direction.x = -p_properties.positionDirection.x;
		light.Direction.y = -p_properties.positionDirection.y;
		light.Direction.z = -p_properties.positionDirection.z;
		light.Type        = D3DLIGHT_DIRECTIONAL;
	}
	
	light.Attenuation0 = p_properties.attenuation.x;
	light.Attenuation1 = p_properties.attenuation.y;
	light.Attenuation2 = p_properties.attenuation.z;
	light.Range        = p_properties.attenuation.w;
	
	checkD3DSucceeded( getRenderDevice(true)->SetLight(p_lightIndex, &light) );
}


void FixedFunctionHardware::setLightEnabled(s32 p_lightIndex, bool p_enabled)
{
	// FIXME: Because of app destruction order this can be called after device cleanup
	if (getRenderDevice() != 0)
	{
		checkD3DSucceeded( getRenderDevice(true)->LightEnable(p_lightIndex, p_enabled) );
	}
}


void FixedFunctionHardware::setLightingEnabled(bool p_enabled)
{
	IDirect3DDevice9* device = getRenderDevice(true);
	if (device != 0)
	{
		setRenderState(D3DRS_LIGHTING             , p_enabled ? TRUE : FALSE);
		setRenderState(D3DRS_NORMALIZENORMALS     , p_enabled ? TRUE : FALSE);
		setRenderState(D3DRS_SPECULARENABLE       , p_enabled ? TRUE : FALSE);
		setRenderState(D3DRS_LOCALVIEWER          , p_enabled ? TRUE : FALSE);
		setRenderState(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1           );
		setRenderState(D3DRS_SHADEMODE            , D3DSHADE_GOURAUD        );
	}
}


void FixedFunctionHardware::setAmbientLightColor(const tt::engine::renderer::ColorRGBA &p_color)
{
	setRenderState(D3DRS_AMBIENT, D3DCOLOR_RGBA(p_color.r, p_color.g, p_color.b, p_color.a));
}


void FixedFunctionHardware::setTextureStage(s32 p_stage, const TextureStageData& p_stageData)
{
	TT_MINMAX_ASSERT(p_stage, 0, static_cast<s32>(MultiTexture::getChannelCount() - 1));
	
	IDirect3DDevice9* device = getRenderDevice(true);
	if (device == 0) return;
	
	/*! \brief Lookup table matching shared enum TextureBlendOperation */
	static const D3DTEXTUREOP blendOp[] =
	{
		D3DTOP_DISABLE,           //<!TextureBlendOperation_Disable,
		D3DTOP_SELECTARG1,        //<!TextureBlendOperation_SelectArg1
		D3DTOP_SELECTARG2,        //<!TextureBlendOperation_SelectArg2
		D3DTOP_MODULATE,          //<!TextureBlendOperation_Modulate,
		D3DTOP_ADD,               //<!TextureBlendOperation_Add,
		D3DTOP_SUBTRACT,          //<!TextureBlendOperation_Subtract
		D3DTOP_MODULATE2X,        //<!TextureBlendOperation_Modulate2X
		D3DTOP_BLENDTEXTUREALPHA, //<!TextureBlendOperation_Decal
	};
	TT_STATIC_ASSERT(sizeof(blendOp) / sizeof(D3DTEXTUREOP) == TextureBlendOperation_Count);
	
	/*! \brief Lookup table matching shared enum TextureBlendSource */
	static const u32 blendSrc[] =
	{
		D3DTA_TEXTURE,                           //<!TextureBlendSource_Texture,
		D3DTA_DIFFUSE,                           //<!TextureBlendSource_Diffuse,
		D3DTA_CURRENT,                           //<!TextureBlendSource_Previous,
		D3DTA_CONSTANT,                          //<!TextureBlendSource_Constant,
		D3DTA_TEXTURE  | D3DTA_COMPLEMENT,       //<!TextureBlendSource_OneMinusTexture,
		D3DTA_DIFFUSE  | D3DTA_COMPLEMENT,       //<!TextureBlendSource_OneMinusDiffuse,
		D3DTA_CURRENT  | D3DTA_COMPLEMENT,       //<!TextureBlendSource_OneMinusPrevious,
		D3DTA_CONSTANT | D3DTA_COMPLEMENT,       //<!TextureBlendSource_OneMinusConstant,
	};
	TT_STATIC_ASSERT(sizeof(blendSrc) / sizeof(u32) == TextureBlendSource_Count);
	
	// Set the color operation and sources for this texture stage
	setTextureStageState(p_stage, D3DTSS_TEXCOORDINDEX, p_stageData.getTexCoordIndex()                , device);
	setTextureStageState(p_stage, D3DTSS_COLOROP      , blendOp [p_stageData.getColorBlendOperation()], device);
	setTextureStageState(p_stage, D3DTSS_ALPHAOP      , blendOp [p_stageData.getAlphaBlendOperation()], device);
	setTextureStageState(p_stage, D3DTSS_COLORARG1    , blendSrc[p_stageData.getColorSource1       ()], device);
	setTextureStageState(p_stage, D3DTSS_COLORARG2    , blendSrc[p_stageData.getColorSource2       ()], device);
	setTextureStageState(p_stage, D3DTSS_ALPHAARG1    , blendSrc[p_stageData.getAlphaSource1       ()], device);
	setTextureStageState(p_stage, D3DTSS_ALPHAARG2    , blendSrc[p_stageData.getAlphaSource2       ()], device);
	
	// Setup the constant color for this stage
	const ColorRGBA& color(p_stageData.getConstant());
	setTextureStageState(p_stage, D3DTSS_CONSTANT, D3DCOLOR_ARGB(color.a, color.r, color.g, color.b), device);
}


void FixedFunctionHardware::disableTextureStage(s32 p_stage)
{
	TT_MINMAX_ASSERT(p_stage, 0, static_cast<s32>(MultiTexture::getChannelCount() - 1));
	
	IDirect3DDevice9* device = getRenderDevice(true);
	
	if (device != 0)
	{
		setTextureStageState(p_stage, D3DTSS_COLOROP, D3DTOP_DISABLE, device);
		setTextureStageState(p_stage, D3DTSS_ALPHAOP, D3DTOP_DISABLE, device);
	}
}


// Namespace end
}
}
}
