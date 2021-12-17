#include <tt/engine/shaders/shaders.h>


namespace tt {
namespace engine {
namespace shaders {
	

const char* fullscreenQuad()
{
	return
	"varying vec2 TexCoord;"
	"void main() {"
	"	gl_Position = vec4(gl_Vertex.x * 0.25, gl_Vertex.y * 0.25, 0, 1);"
	"	TexCoord.x = (gl_Vertex.x + 4.0) / 8.0;"
	"	TexCoord.y = (gl_Vertex.y + 4.0) / 8.0;"
	"}";
}


// Fast Gauss Blur
// A couple of tricks are applied to achieve a 9 tap gauss filter with only 5 texture fetches
// First the outer taps are discarded, as they contribute near to nothing, secondly hardware linear
// filtering is used to average 2 pixels with 1 fetch. Finally we precompute the offsets in the
// vertex shader and we unroll the for loop in the pixel shader.
// Detailed description of this implementation:
// http://rastergrid.com/blog/2010/09/efficient-gaussian-blur-with-linear-sampling/

const char* vertexFastGaussH()
{
	return
	
	"uniform vec2 inverseScreenSize;"
	
	"varying vec4 TexCoordPos;"
	"varying vec3 TexCoordNeg;"
	
	"const vec2 fg_offset = vec2(1.3846153846, 3.2307692308);"
	
	"void main() {"
	"	gl_Position = vec4(gl_Vertex.x * 0.25, gl_Vertex.y * 0.25, 0, 1);"
	
	"	vec2 baseTexCoord = vec2((gl_Vertex.x + 4.0) / 8.0, (gl_Vertex.y + 4.0) / 8.0);"
	
	"	TexCoordPos.x = baseTexCoord.x;"
	"	TexCoordPos.y = baseTexCoord.y;"
	"	TexCoordPos.z = baseTexCoord.x + fg_offset.x * inverseScreenSize.x;"
	"	TexCoordPos.w = baseTexCoord.x + fg_offset.y * inverseScreenSize.x;"
	
	"	TexCoordNeg.x = baseTexCoord.x - fg_offset.x * inverseScreenSize.x;"
	"	TexCoordNeg.y = baseTexCoord.y;"
	"	TexCoordNeg.z = baseTexCoord.x - fg_offset.y * inverseScreenSize.x;"
	
	"}";
}


const char* vertexFastGaussV()
{
	return
	"uniform vec2 inverseScreenSize;"
	
	"varying vec4 TexCoordPos;"
	"varying vec3 TexCoordNeg;"
	
	"const vec2 fg_offset = vec2(1.3846153846, 3.2307692308);"
	
	"void main() {"
	"	gl_Position = vec4(gl_Vertex.x * 0.25, gl_Vertex.y * 0.25, 0, 1);"
	
	"	vec2 baseTexCoord = vec2((gl_Vertex.x + 4.0) / 8.0, (gl_Vertex.y + 4.0) / 8.0);"
	
	"	TexCoordPos.x = baseTexCoord.x;"
	"	TexCoordPos.y = baseTexCoord.y;"
	"	TexCoordPos.z = baseTexCoord.y + fg_offset.x * inverseScreenSize.y;"
	"	TexCoordPos.w = baseTexCoord.y + fg_offset.y * inverseScreenSize.y;"
	
	"	TexCoordNeg.x = baseTexCoord.x;"
	"	TexCoordNeg.y = baseTexCoord.y - fg_offset.x * inverseScreenSize.y;"
	"	TexCoordNeg.z = baseTexCoord.y - fg_offset.y * inverseScreenSize.y;"
	
	"}";
}


const char* textured()
{
	return
	"uniform sampler2D Texture0;"
	"varying vec2 TexCoord;"
	"void main() {"
	"	gl_FragColor = texture2D(Texture0, TexCoord);"
	"}";
}


const char* texturedFiltered()
{
	return
	"uniform sampler2D FilteredScreenSampler;"
	"varying vec2 TexCoord;"
	"void main() {"
	"	gl_FragColor = texture2D(FilteredScreenSampler, TexCoord);"
	"}";
}


const char* texturedShift()
{
	return
	"uniform sampler2D Texture0;"
	"varying vec2 TexCoord;"
	"void main() {"
	"	gl_FragColor = texture2D(Texture0, TexCoord).brga;"
	"}";
}


const char* boxBlur()
{
	return
	"uniform sampler2D FilteredScreenSampler;"
	"varying vec2 TexCoord;"
	
	"uniform float radius;"
	"uniform vec2 inverseScreenSize;"
	
	"void main()"
	"{"
	"	vec4 color = texture2D(FilteredScreenSampler, TexCoord + inverseScreenSize * radius * vec2(-0.5,-0.5));"
	"	color += texture2D(FilteredScreenSampler, TexCoord + inverseScreenSize * radius * vec2(0.5,-0.5));"
	"	color += texture2D(FilteredScreenSampler, TexCoord + inverseScreenSize * radius * vec2(-0.5,0.5));"
	"	color += texture2D(FilteredScreenSampler, TexCoord + inverseScreenSize * radius * vec2(0.5,0.5));"
	"	color *= 0.25;"

	"	gl_FragColor = color;"
	"}";
}


const char* boxBlurLite()
{
	return
	"uniform sampler2D FilteredScreenSampler;"
	"varying vec2 TexCoord;"
	
	"uniform float radius;"
	"uniform vec2 inverseScreenSize;"
	
	"void main()"
	"{"
	"	vec4 color = texture2D(FilteredScreenSampler, TexCoord + inverseScreenSize * radius * vec2(-0.5,-0.5));"
	"	color += texture2D(FilteredScreenSampler, TexCoord + inverseScreenSize * radius * vec2(0.5,-0.5));"
	"	color += texture2D(FilteredScreenSampler, TexCoord + inverseScreenSize * radius * vec2(0.0, 1.0));"
	"	color *= 0.333333333;"
	
	"	gl_FragColor = vec4(color.rgb, 1);"
	"}";
}


const char* brightPass()
{
	return
	"uniform sampler2D FilteredScreenSampler;"
	"varying vec2 TexCoord;"
	
	"uniform float power;"
	
	"void main()"
	"{"
	"	vec3 color = texture2D(FilteredScreenSampler, TexCoord).rgb;"

	"	float I = 0.299 * color.r + 0.587 * color.g + 0.114 * color.b;"
		
	"	gl_FragColor = vec4(pow(abs(I), power) * color, 1);"
	"}";
}

	

const char* colorPass()
{
	return
	"uniform sampler2D FilteredScreenSampler;"
	"varying vec2 TexCoord;"
	
	"void main()"
	"{"
	"	vec3 color = texture2D(FilteredScreenSampler, TexCoord).rgb;"
	
	"	float I = abs(color.r - color.g) + abs(color.g - color.b) + abs(color.g - color.r);"
	
	"	gl_FragColor = vec4(clamp(I, 0.0, 1.0) * color, 1);"
	"}";
}

const char* radialBlur()
{
	return
	
	"uniform sampler2D OriginalSampler;"
	"uniform sampler2D FilteredScreenSampler;"
	"uniform vec2      inverseScreenSize;"
	""
	"varying vec2 TexCoord;"
	""
	"vec3 deform( in vec2 p )"
	"{"
	 "	vec2 uv;"
	"	"
	"	vec2 q = vec2( sin(1.1+p.x),sin(1.2+p.y) );"
	"	"
	"	float a = atan(q.y,q.x);"
	"	float r = sqrt(dot(q,q));"
	"	"
	"	uv.x = sin(0.0+1.0)+p.x*sqrt(r*r+1.0);"
	"	uv.y = sin(0.6+1.1)+p.y*sqrt(r*r+1.0);"
	"	"
	"	return texture2D(OriginalSampler,uv*.5).xyz;"
	"}"
	""
	"void main(void)"
	"{"
	"	vec2 p = -1.0 + 2.0 * TexCoord.xy;"
	"	vec2 s = p;"
	"	"
	"	const int stepsCount = 50;"
	"	"
	"	vec3 total = vec3(0.0);"
	"	vec2 d = (vec2(0.0,0.0) - p) / float(stepsCount);"
	"	float w = 1.0;"
	"	for( int i=0; i < stepsCount; i++ )"
	"	{"
	//"		vec3 res = deform(s);"
	"		vec3 res = texture2D(FilteredScreenSampler, (1.0 + s) * 0.5).xyz;"
	//"		res = smoothstep(0.1,1.0,res*res);"
	"		total += w*res;"
	"		w *= .99;"
	"		s += d;"
	"	}"
	"	total /= float(stepsCount);"
	"	float r = 1.0;"//1.5/(1.0+dot(p,p));"
    "	gl_FragColor = vec4( total*r,1.0);"
	"}";
}
	
	
const char* colorSeparation()
{
	return
	"uniform sampler2D OriginalSampler;"
	"uniform sampler2D SeparationWeight;"
	"uniform vec2      inverseScreenSize;"
	"uniform float     SeparationScale;"
	"uniform float     ExtraWhiteScale;"
	""
	"varying vec2 TexCoord;"
	""
	"void main(void)"
	"{"
	"	vec3 weight = texture2D(SeparationWeight, TexCoord).rgb;"
	"	vec3 col;"
	"	"
	"	col.r = texture2D(OriginalSampler, vec2(TexCoord.x + ((weight.x - 0.5) * SeparationScale) * inverseScreenSize.x, TexCoord.y)).x;"
	"	col.g = texture2D(OriginalSampler, vec2(TexCoord.x + ((weight.y - 0.5) * SeparationScale) * inverseScreenSize.x, TexCoord.y)).y;"
	"	col.b = texture2D(OriginalSampler, vec2(TexCoord.x + ((weight.z - 0.5) * SeparationScale) * inverseScreenSize.x, TexCoord.y)).z;"
	"	"
	"	col *= ExtraWhiteScale;"
	"	gl_FragColor = vec4(col,1.0);"
	"}";
}


const char* colorSeparationLite()
{
	return
	"uniform sampler2D OriginalSampler;"
	"uniform vec2      inverseScreenSize;"
	"uniform float     SeparationScale;"
	"uniform float     ExtraWhiteScale;"
	""
	"varying vec2 TexCoord;"
	""
	"void main(void)"
	"{"
	"   float offset = max(abs(TexCoord.x - 0.5) - 0.10, 0.0) * 1.25;"
	"   vec3 weight = vec3(0.5 - offset, 0.5, 0.5 + offset);"
	"	vec3 col;"
	"	"
	"	col.r = texture2D(OriginalSampler, vec2(TexCoord.x + ((weight.x - 0.5) * SeparationScale) * inverseScreenSize.x, TexCoord.y)).x;"
	"	col.g = texture2D(OriginalSampler, vec2(TexCoord.x + ((weight.y - 0.5) * SeparationScale) * inverseScreenSize.x, TexCoord.y)).y;"
	"	col.b = texture2D(OriginalSampler, vec2(TexCoord.x + ((weight.z - 0.5) * SeparationScale) * inverseScreenSize.x, TexCoord.y)).z;"
	"	"
	"	col *= ExtraWhiteScale;"
	"   "
	//"   col = weight;"
	"	gl_FragColor = vec4(col,1.0);"
	"}";
}


const char* mixEffect()
{
	return
		
	"uniform sampler2D OriginalSampler;"
	"uniform sampler2D FilteredScreenSampler;"
	
	"varying vec2 TexCoord;"
	
	"void main()"
	"{"
	"	vec3 original  = texture2D(OriginalSampler,       TexCoord).rgb;"
	"	vec3 processed = texture2D(FilteredScreenSampler, TexCoord).rgb;"
	"	gl_FragColor = vec4(mix(original, processed, 0.2), 1);"
	"}";
}


const char* inGameEffect()
{
	return
	
	"uniform sampler2D OriginalSampler;"
	"uniform sampler2D FilteredScreenSampler;"
	
	"varying vec2 TexCoord;"
	
	"void main()"
	"{"
	"	vec3 original  = texture2D(OriginalSampler, TexCoord).rgb;"
	"	vec3 processed = texture2D(FilteredScreenSampler, TexCoord).rgb;"
	
	"	float I = 0.299 * original.r + 0.587 * original.g + 0.114 * original.b;"

	"	gl_FragColor = vec4(mix(original + processed, original, I), 1);"
	"}";
}
	

// Fast Gaussian blur - Horizontal Pass
// Offsets are precomputed in texture coordinates, loop is unrolled

const char* fastGaussH()
{
	return
	
	"uniform sampler2D FilteredScreenSampler;"
	"const vec3 fg_weight = vec3(0.2270270270, 0.3162162162, 0.0702702703);"
	
	"varying vec4 TexCoordPos;"
	"varying vec3 TexCoordNeg;"
	
	"void main()"
	"{"
	
	"	vec4 color = texture2D(FilteredScreenSampler, TexCoordPos.xy) * fg_weight.x;"
	
	"	color += texture2D(FilteredScreenSampler, TexCoordPos.zy) * fg_weight.y;"
	"	color += texture2D(FilteredScreenSampler, TexCoordPos.wy) * fg_weight.z;"
	
	"	color += texture2D(FilteredScreenSampler, TexCoordNeg.xy) * fg_weight.y;"
	"	color += texture2D(FilteredScreenSampler, TexCoordNeg.zy) * fg_weight.z;"
	
	"	gl_FragColor = color;"
	"}";
}
	
// Fast Gaussian blur - Vertical Pass

const char* fastGaussV()
{
	return
	
	"uniform sampler2D FilteredScreenSampler;"
	"const vec3 fg_weight = vec3(0.2270270270, 0.3162162162, 0.0702702703);"
	
	"varying vec4 TexCoordPos;"
	"varying vec3 TexCoordNeg;"
	
	"void main()"
	"{"
	
	"	vec4 color = texture2D(FilteredScreenSampler, TexCoordPos.xy) * fg_weight.x;"
	
	"	color += texture2D(FilteredScreenSampler, TexCoordPos.xz) * fg_weight.y;"
	"	color += texture2D(FilteredScreenSampler, TexCoordPos.xw) * fg_weight.z;"
	
	"	color += texture2D(FilteredScreenSampler, TexCoordNeg.xy) * fg_weight.y;"
	"	color += texture2D(FilteredScreenSampler, TexCoordNeg.xz) * fg_weight.z;"
	
	"	gl_FragColor = color;"
	"}";
}


}
}
}
