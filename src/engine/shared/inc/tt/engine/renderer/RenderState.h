#if !defined(INC_TT_ENGINE_RENDERER_RENDERSTATE_H)
#define INC_TT_ENGINE_RENDERER_RENDERSTATE_H


#include <tt/engine/renderer/enums.h>
#include <tt/engine/renderer/ColorRGBA.h>
#include <tt/platform/tt_types.h>


namespace tt {
namespace engine {
namespace renderer {


struct RenderState
{
	// Culling
	CullFrontOrder    cullFrontOrder;
	CullMode          cullMode;
	bool              cullingEnabled;
	
	// Blend Modes    
	BlendFactor       srcColorFactor;
	BlendFactor       dstColorFactor;
	BlendFactor       srcAlphaFactor;
	BlendFactor       dstAlphaFactor;
	bool              separateAlphaBlendEnabled;
	bool              premultipliedAlphaEnabled;

	// Alpha Test
	bool              alphaTestEnabled;
	AlphaTestFunction alphaTestFunction;
	u8                alphaTestValue;

	// Lighting
	bool              lightingEnabled;

	// Fog
	FogMode           fogMode;
	ColorRGBA         fogColor;
	real              fogSettings[FogSetting_Count];
	bool              fogEnabled;

	// Depth Test
	bool              depthWriteEnabled;
	bool              depthTestEnabled;

	FillMode          fillMode;
	u32               colorMask;

	bool                stencilEnabled;
	s32                 stencilReference;
	s32                 stencilMask;
	StencilTestFunction stencilFunc[2];
	StencilOperation    stencilOpFail[2];
	StencilOperation    stencilOpDepthFail[2];
	StencilOperation    stencilOpPass[2];
	
	RenderState()
	:
	cullFrontOrder(CullFrontOrder_ClockWise),
	cullMode(CullMode_Back),
	cullingEnabled(true),
	srcColorFactor(BlendFactor_SrcAlpha),
	dstColorFactor(BlendFactor_InvSrcAlpha),
	srcAlphaFactor(BlendFactor_Zero),
	dstAlphaFactor(BlendFactor_Zero),
	separateAlphaBlendEnabled(false),
	premultipliedAlphaEnabled(false),
	alphaTestEnabled(false),
	alphaTestFunction(AlphaTestFunction_Never),
	alphaTestValue(0),
	lightingEnabled(false),
	fogMode(FogMode_Linear),
	fogColor(ColorRGB::white),
	fogEnabled(false),
	depthWriteEnabled(true),
	depthTestEnabled(true),
	fillMode(FillMode_Solid),
	colorMask(ColorMask_All),
	stencilEnabled(false),
	stencilReference(0),
	stencilMask(255)
	{
		fogSettings[FogSetting_Start  ] = 0.0f;
		fogSettings[FogSetting_End    ] = 1.0f;
		fogSettings[FogSetting_Density] = 1.0f;
		
		stencilFunc[StencilSide_Front] = StencilTestFunction_Never;
		stencilFunc[StencilSide_Back ] = StencilTestFunction_Never;
		
		stencilOpFail     [StencilSide_Front] = StencilOperation_Keep;
		stencilOpDepthFail[StencilSide_Front] = StencilOperation_Keep;
		stencilOpPass     [StencilSide_Front] = StencilOperation_Keep;
		stencilOpFail     [StencilSide_Back ] = StencilOperation_Keep;
		stencilOpDepthFail[StencilSide_Back ] = StencilOperation_Keep;
		stencilOpPass     [StencilSide_Back ] = StencilOperation_Keep;
	}
};


// Namespace end
}
}
}


#endif  // !defined(INC_TT_ENGINE_RENDERER_RENDERSTATE_H)
