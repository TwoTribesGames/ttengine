
#include "HLSL_to_PSSL.h"
 
static const float4 maxGlowColor = float4(0.5, 0.5, 0.5, 0.5);

// Shader input (set by application)
CONST_BUFFER_BEGIN(VShaderConstants)
float4 inverseScreenSize;
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
 
	Out.Pos = In.Position;
	Out.Tex = inverseScreenSize.zw * inverseScreenSize.xy + In.TexCoord;

	return Out;
}


float4 ps_light_glow(VS_OUTPUT In) : SV_TARGET
{	
	return min(SAMPLE_TEXTURE_2D(ScreenBuffer, In.Tex), maxGlowColor);
}


technique LightGlow
{
    pass P0
    {
        VertexShader = compile vs_2_0 vs_main();
        PixelShader  = compile ps_2_0 ps_light_glow();
    }
}
