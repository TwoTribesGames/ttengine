
#include "HLSL_to_PSSL.h"

// Shader input (set by application)
CONST_BUFFER_BEGIN(VShaderConstants)
float4 inverseScreenSize;
CONST_BUFFER_END

CONST_BUFFER_BEGIN(PShaderConstants)
float4 lookupStrength;
float4 clearColor;
CONST_BUFFER_END

sampler2D ScreenBuffer;
sampler3D ColorLookup3DOne;
sampler3D ColorLookup3DTwo;


struct VS_INPUT
{
   float4 Position : POSITION;
   float4 Color    : COLOR;
   float2 TexCoord : TEXCOORD;
};

struct VS_OUTPUT
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD0;
};


VS_OUTPUT vs_main( VS_INPUT In )
{
	VS_OUTPUT Out = (VS_OUTPUT) 0;
 
	Out.Pos = In.Position;
	Out.Tex = inverseScreenSize.zw * inverseScreenSize.xy + In.TexCoord;

	return Out;
}


float4 ps_normal(VS_OUTPUT In) : SV_TARGET
{
	return SAMPLE_TEXTURE_2D(ScreenBuffer, In.Tex);
}


float4 ps_clear() : SV_TARGET
{
	return clearColor;
}


float3 applyVignette(float3 color, float2 uv )
{
	float vignetteStrength = 0.35; // Darkness of effect (higher = darker)
	float vignetteScale    = 70.0; // Overall reach of effect (lower = greater reach)
	
	uv = clamp(uv, 0.0, 1.0);
	float vignette = uv.x * uv.y * ( 1.0 - uv.x ) * ( 1.0 - uv.y );
	vignette = clamp( pow( vignetteScale * vignette, vignetteStrength ), 0.0, 1.0 );
	color *= vignette;
	return color;
}


float2 warpUV( float2 uv, float2 curve )
{
	uv = uv * 2.0 - 1.0;
	float2 offset = abs( uv.yx ) / curve;
	uv = uv + uv * offset * offset;
	uv = uv * 0.5 + 0.5;
	return uv;
}


float4 ps_colorgrading(VS_OUTPUT In) : SV_TARGET
{
	// Apply chromatic abberation and curvature
	float2 baseCurve = float2(8.5, (8.5 * 2.0)/3.0); // Controls curvature
	float chromaStrength = 0.275; // Strength of chromatic abberation
	
	float2 curveR = baseCurve * (1.0 - chromaStrength * 0.5);
	float2 curveG = baseCurve;
	float2 curveB = baseCurve * (1.0 + chromaStrength);
	
	float2 uvR = warpUV(In.Tex, curveR);
	float2 uvG = warpUV(In.Tex, curveG);
	float2 uvB = warpUV(In.Tex, curveB);
	
	float3 color = float3(1.0, 1.0, 1.0);
	color.r = SAMPLE_TEXTURE_2D(ScreenBuffer, uvR).r;
	color.g = SAMPLE_TEXTURE_2D(ScreenBuffer, uvG).g;
	color.b = SAMPLE_TEXTURE_2D(ScreenBuffer, uvB).b;
	
	// Apply color grading on result
	float3 scale  = 0.96875;
	float3 offset = 0.015625;
	
	float3 lookup = scale * color + offset;
	
	float3 lookupColorOne = SAMPLE_TEXTURE_3D(ColorLookup3DOne, lookup).rgb;
	float3 lookupColorTwo = SAMPLE_TEXTURE_3D(ColorLookup3DTwo, lookup).rgb;
	
	float3 lookupResult = lerp(lookupColorTwo, lookupColorOne, saturate(lookupStrength.x));
	
	// Apply vignette at end
	color = applyVignette(lerp(lookupResult, color, saturate(lookupStrength.y)), In.Tex);
	
	// Clamp within 0-1 range. If outside screen, make pixel black
	color *= (uvG.x >= 0.0 && uvG.y >= 0.0 && uvG.x <= 1.0 && uvG.y <= 1.0);
	
	return float4(color, 1.0);
}


technique Identity
{
    pass P0
    {
        VertexShader = compile vs_2_0 vs_main();
        PixelShader  = compile ps_2_0 ps_normal();
    }
}

technique Clear
{
    pass P0
    {
        VertexShader = compile vs_2_0 vs_main();
        PixelShader  = compile ps_2_0 ps_clear();
    }
}

technique ColorGrading
{
    pass P0
    {
        VertexShader = compile vs_2_0 vs_main();
        PixelShader  = compile ps_2_0 ps_colorgrading();
    }
}
