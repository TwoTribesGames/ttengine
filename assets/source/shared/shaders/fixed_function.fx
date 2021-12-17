
#include "HLSL_to_PSSL.h"

CONST_BUFFER_BEGIN(VShaderConstants)
float4x4 u_projection;
float4x4 u_view;
float4x4 u_world;
float4x4 u_textureMtx[1];
float4   u_fogSettings; // [Enable, Start, End, Density]
float4   u_mainTextureSize;
bool     u_useVtxColor = false; 
CONST_BUFFER_END


CONST_BUFFER_BEGIN(PShaderConstants)
float4 u_fogColor;
float4 u_overdrawColor;
float4 u_debugFeatures; // x = overdraw, y = mip visualize
CONST_BUFFER_END


sampler2D diffuse;
sampler2D mipVisualization;


struct VS_INPUT
{
	float4 Position		: POSITION;
	float4 Color		: COLOR0;
	float2 UV			: TEXCOORD0;
};

struct VS_OUTPUT
{
	float4 Position		: SV_POSITION;
	float4 Color		: COLOR0;
	float2 UV			: TEXCOORD0;
	float  Fog          : TEXCOORD1;
};

struct VS_OUTPUT_MIPMAP_DEBUG
{
	float4 Position		: SV_POSITION;
	float4 Color		: COLOR0;
	float2 UV			: TEXCOORD0;
	float  Fog          : TEXCOORD1;
	float2 MipUV		: TEXCOORD2;
};

//------------------------------------------------------------------
// Helper functions

float computeFog(float4 p_eye)
{
	float fogDistance = abs(p_eye.z);
	
	// Compute linear fog
	float fog = (u_fogSettings.z - fogDistance) / (u_fogSettings.z - u_fogSettings.y);
	
	return saturate(fog);
}


float3 computeFogColor(float4 p_color, float p_fog)
{
	// Make fog behave correctly in combination with premultiplied alpha
	// This is impossible with the Fixed Function pipeline
	// u_fogColor.a is 0 for premultiplied, 1 for straight alpha
	float premultiply = max(p_color.a, u_fogColor.a);
	float3 fogColor = u_fogColor.rgb * premultiply;

	return lerp(fogColor, p_color.rgb, p_fog);
}

//------------------------------------------------------------------
// Entry points

VS_OUTPUT vs_main( VS_INPUT p_input )
{
	VS_OUTPUT Output;

	// Position in camera space
	float4 eye = mul(mul(p_input.Position, u_world), u_view);
	
	Output.Position = mul(eye, u_projection);
	Output.Color    = u_useVtxColor ? p_input.Color : float4(1,1,1,1);
	Output.UV       = mul(float4(p_input.UV,0,1), u_textureMtx[0]).xy;
	Output.Fog      = u_fogSettings.x > 0.5 ? computeFog(eye) : 1.0;

	return Output;
}


VS_OUTPUT_MIPMAP_DEBUG vs_mipmap_debug( VS_INPUT p_input )
{
	VS_OUTPUT_MIPMAP_DEBUG Output;
	
	// Position in camera space
	float4 eye = mul(mul(p_input.Position, u_world), u_view);
	
	Output.Position = mul(eye, u_projection);
	Output.Color    = u_useVtxColor ? p_input.Color : float4(1,1,1,1);
	Output.UV       = mul(float4(p_input.UV,0,1), u_textureMtx[0]).xy;
	Output.Fog      = u_fogSettings.x > 0.5 ? computeFog(eye) : 1.0;
	
	Output.MipUV = Output.UV * float2(u_mainTextureSize.x / 8.0, u_mainTextureSize.y / 8.0);
	
	return Output;
}

float4 ps_main( VS_OUTPUT p_input ) : SV_TARGET
{
	float4 color = SAMPLE_TEXTURE_2D(diffuse,  p_input.UV) * p_input.Color;
	return float4(computeFogColor(color, p_input.Fog), color.a);
}


float4 ps_debug( VS_OUTPUT p_input ) : SV_TARGET
{
	if (u_debugFeatures.x > 0.0f) return u_overdrawColor;
	
	float4 color = SAMPLE_TEXTURE_2D(diffuse,  p_input.UV) * p_input.Color;
	return float4(computeFogColor(color, p_input.Fog), color.a);
}


float4 ps_mipmap_debug( VS_OUTPUT_MIPMAP_DEBUG p_input ) : SV_TARGET
{
	if (u_debugFeatures.x > 0.0f)
		return u_overdrawColor;

	float4 color = SAMPLE_TEXTURE_2D(diffuse,  p_input.UV) * p_input.Color;

	if (u_debugFeatures.y > 0.0f)
	{
		float4 mipColor = SAMPLE_TEXTURE_2D(mipVisualization, p_input.MipUV);
		color.rgb = lerp(color.rgb, mipColor.rgb * color.a, mipColor.a);
		return color;
	}

	return float4(computeFogColor(color, p_input.Fog), color.a);
}

//------------------------------------------------------------------
// Shader definitions


technique FixedFunction
{
    pass P0
    {
        VertexShader = compile vs_2_0 vs_main();
        PixelShader  = compile ps_2_0 ps_main();
    }
}


technique FixedFunctionDebug
{
    pass P0
    {
        VertexShader = compile vs_2_0 vs_main();
        PixelShader  = compile ps_2_0 ps_debug();
    }
}

// Unfortunately only works on Windows for now
technique FixedFunctionMipMapDebug
{
    pass P0
    {
        VertexShader = compile vs_2_0 vs_mipmap_debug();
        PixelShader  = compile ps_2_0 ps_mipmap_debug();
    }
}
