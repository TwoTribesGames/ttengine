 
#include "HLSL_to_PSSL.h"

// A couple of tricks are applied to achieve a 9 tap gauss filter with only 5 texture fetches
// (1) The outer taps are discarded, as they contribute near to nothing
// (2) Hardware linear filtering is used to average 2 pixels with 1 fetch (sample between pixel boundaries)
// (3) The offsets are precomputed in the vertex shader
// (4) The loop in the pixel shader is unrolled
// Detailed description of this implementation: http://rastergrid.com/blog/2010/09/efficient-gaussian-blur-with-linear-sampling/

// Offset and weights for texel samples (weights must always add up to 1.0)
// Array initializers do not work on OSX 10.6 :(
//static const float fg_offset[3] = {0.0, 1.3846153846, 3.2307692308};
//static const float fg_weight[3] = {0.2270270270, 0.3162162162, 0.0702702703};

static const float fg_offset_1 = 1.3846153846;
static const float fg_offset_2 = 3.2307692308;

static const float fg_weight_0 = 0.2270270270;
static const float fg_weight_1 = 0.3162162162;
static const float fg_weight_2 = 0.0702702703;


// Shader input (set by application)
CONST_BUFFER_BEGIN(VShaderConstants)
float4 inverseScreenSize;
CONST_BUFFER_END

CONST_BUFFER_BEGIN(PShaderConstants)
float4 inverseScreenSizePS;
CONST_BUFFER_END

sampler2D ScreenBuffer;


struct VS_INPUT
{
	float4 Position : POSITION;
	float4 Color    : COLOR;
	float2 TexCoord : TEXCOORD;
};

struct VS_OUTPUT
{
	float4 Position     : SV_POSITION;
	float4 TexCoordPos  : TEXCOORD0;
	float4 TexCoordNeg  : TEXCOORD1;
};


VS_OUTPUT vs_fastGaussH( VS_INPUT In )
{
	VS_OUTPUT Out = (VS_OUTPUT) 0;
 
	Out.Position = In.Position;
	
	// Offset by half pixel size to align texels exactly with pixels
	Out.TexCoordPos.x = In.TexCoord.x + inverseScreenSize.z * inverseScreenSize.x; // Regular x
	Out.TexCoordPos.y = In.TexCoord.y + inverseScreenSize.w * inverseScreenSize.y; // Regular y
	Out.TexCoordPos.z = In.TexCoord.x + (inverseScreenSize.z + fg_offset_1) * inverseScreenSize.x; // x offset 1
	Out.TexCoordPos.w = In.TexCoord.x + (inverseScreenSize.z + fg_offset_2) * inverseScreenSize.x; // x offset 2
	
	Out.TexCoordNeg.y = In.TexCoord.y + inverseScreenSize.w * inverseScreenSize.y; // Regular y
	Out.TexCoordNeg.x = In.TexCoord.x + (inverseScreenSize.z - fg_offset_1) * inverseScreenSize.x; // x offset -1
	Out.TexCoordNeg.z = In.TexCoord.x + (inverseScreenSize.z - fg_offset_2) * inverseScreenSize.x; // x offset -2

	return Out;
}


VS_OUTPUT vs_fastGaussV( VS_INPUT In )
{
	VS_OUTPUT Out = (VS_OUTPUT) 0;
 
	Out.Position = In.Position;
	
	// Offset by half pixel size to align texels exactly with pixels
	Out.TexCoordPos.x = In.TexCoord.x + inverseScreenSize.z * inverseScreenSize.x; // Regular x
	Out.TexCoordPos.y = In.TexCoord.y + inverseScreenSize.w * inverseScreenSize.y; // Regular y
	Out.TexCoordPos.z = In.TexCoord.y + (inverseScreenSize.w + fg_offset_1) * inverseScreenSize.y; // y offset 1
	Out.TexCoordPos.w = In.TexCoord.y + (inverseScreenSize.w + fg_offset_2) * inverseScreenSize.y; // y offset 2
	
	Out.TexCoordNeg.x = In.TexCoord.x + inverseScreenSize.z * inverseScreenSize.x; // Regular x
	Out.TexCoordNeg.y = In.TexCoord.y + (inverseScreenSize.w - fg_offset_1) * inverseScreenSize.y; // y offset -1
	Out.TexCoordNeg.z = In.TexCoord.y + (inverseScreenSize.w - fg_offset_2) * inverseScreenSize.y; // y offset -2

	return Out;
}


// Fast Gaussian blur - Horizontal Pass
// Offsets are precomputed in texture coordinates, loop is unrolled
float4 ps_fastGaussH(VS_OUTPUT In) : SV_TARGET
{
	// Center tap
    float4 output = SAMPLE_TEXTURE_2D(ScreenBuffer, In.TexCoordPos.xy) * fg_weight_0;
	
	// Taps to the right
	output += SAMPLE_TEXTURE_2D(ScreenBuffer, In.TexCoordPos.zy) * fg_weight_1;
	output += SAMPLE_TEXTURE_2D(ScreenBuffer, In.TexCoordPos.wy) * fg_weight_2;
	
	// Taps to the left
	output += SAMPLE_TEXTURE_2D(ScreenBuffer, In.TexCoordNeg.xy) * fg_weight_1;
	output += SAMPLE_TEXTURE_2D(ScreenBuffer, In.TexCoordNeg.zy) * fg_weight_2;
	
	return output;
}

// Fast Gaussian blur - Vertical Pass
// Offsets are precomputed in texture coordinates, loop is unrolled
float4 ps_fastGaussV(VS_OUTPUT In) : SV_TARGET
{
	// Center tap
    float4 output = SAMPLE_TEXTURE_2D(ScreenBuffer, In.TexCoordPos.xy) * fg_weight_0;
	
	// Taps to the right
	output += SAMPLE_TEXTURE_2D(ScreenBuffer, In.TexCoordPos.xz) * fg_weight_1;
	output += SAMPLE_TEXTURE_2D(ScreenBuffer, In.TexCoordPos.xw) * fg_weight_2;
	
	// Taps to the left
	output += SAMPLE_TEXTURE_2D(ScreenBuffer, In.TexCoordNeg.xy) * fg_weight_1;
	output += SAMPLE_TEXTURE_2D(ScreenBuffer, In.TexCoordNeg.xz) * fg_weight_2;
	
	return output;
}


technique GaussBlurH
{
    pass P0
    {
        // compiler directives
        VertexShader = compile vs_2_0 vs_fastGaussH();
        PixelShader  = compile ps_2_0 ps_fastGaussH();
    }
}


technique GaussBlurV
{
	pass P0
    {
        // compiler directives
        VertexShader = compile vs_2_0 vs_fastGaussV();
        PixelShader  = compile ps_2_0 ps_fastGaussV();
    }
}
