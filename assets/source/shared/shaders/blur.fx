#include "HLSL_to_PSSL.h"

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
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD0;
};


VS_OUTPUT vs_main( VS_INPUT In )
{
	VS_OUTPUT Out = (VS_OUTPUT) 0;
 
	Out.Pos   = In.Position;
	Out.Tex.x = In.TexCoord.x + inverseScreenSize.z * inverseScreenSize.x;
	Out.Tex.y = In.TexCoord.y + inverseScreenSize.w * inverseScreenSize.y;

	return Out;
}


float4 ps_boxblur(VS_OUTPUT In) : SV_TARGET
{
	float2 halfPixelOffset = 0.5 * inverseScreenSizePS.xy;
	
	float4 result = SAMPLE_TEXTURE_2D(ScreenBuffer, In.Tex + float2(-halfPixelOffset.x,  halfPixelOffset.y));
	result +=       SAMPLE_TEXTURE_2D(ScreenBuffer, In.Tex + float2(-halfPixelOffset.x, -halfPixelOffset.y));
	result +=       SAMPLE_TEXTURE_2D(ScreenBuffer, In.Tex + float2( halfPixelOffset.x, -halfPixelOffset.y));
	result +=       SAMPLE_TEXTURE_2D(ScreenBuffer, In.Tex + float2( halfPixelOffset.x,  halfPixelOffset.y));
	
	return result * 0.25;
}


float4 ps_triblur(VS_OUTPUT In) : SV_TARGET
{
	float2 halfPixelOffset = 0.5 * inverseScreenSizePS.xy;
	
	float4 result = SAMPLE_TEXTURE_2D(ScreenBuffer, In.Tex + float2(-halfPixelOffset.x, -halfPixelOffset.y));
	result +=       SAMPLE_TEXTURE_2D(ScreenBuffer, In.Tex + float2( halfPixelOffset.x, -halfPixelOffset.y));
	result +=       SAMPLE_TEXTURE_2D(ScreenBuffer, In.Tex + float2(                 0,  halfPixelOffset.y));
	
	return result * 0.33333;
}


technique BoxBlur
{
    pass P0
    {
        VertexShader = compile vs_2_0 vs_main();
        PixelShader  = compile ps_2_0 ps_boxblur();
    }
}


technique TriBlur
{
    pass P0
    {
        VertexShader = compile vs_2_0 vs_main();
        PixelShader  = compile ps_2_0 ps_triblur();
    }
}
