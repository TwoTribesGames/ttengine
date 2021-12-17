#include <tt/engine/renderer/gpu_capabilities.h>
#include <tt/engine/opengl_headers.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/platform/tt_printf.h>
#include <tt/platform/tt_error.h>

#include <cstdio>

namespace tt {
namespace engine {
namespace renderer {
	
	
static bool isGLSLVersionSupported(int p_major, int p_minor)
{
	const char* glslVersion = reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION));
	TT_Printf("Found GLSL Version: %s\n", glslVersion);
	
	int majorVersion(0);
	int minorVersion(0);
	
	if(glslVersion != 0 && std::strlen(glslVersion) > 0)
	{
		std::sscanf(glslVersion, "%d.%d", &majorVersion, &minorVersion);
	}
	
	return (majorVersion > p_major || (majorVersion == p_major && minorVersion >= p_minor));
}


bool hasShaderSupport()
{
	return isGLSLVersionSupported(1,2);
}


u32 getMaxTextureWidth()
{
	GLint maxTextureSize(0);
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
	
	TT_CHECK_OPENGL_ERROR();
	
	return static_cast<u32>(maxTextureSize);
}


NPOTSupport getNonPowerOfTwoSupport()
{
#if TT_OPENGLES_VERSION == 0
	
	if (getMaxTextureWidth() < 8192)
	{
		// Assume pre-DX10
		return NPOTSupport_Limited;
	}
	
	return NPOTSupport_Full;
	
#elif TT_OPENGLES_VERSION == 1
	// All iOS devices support this: GL_APPLE_texture_2D_limited_npot
	return NPOTSupport_Limited;
	
#elif TT_OPENGLES_VERSION == 2
	// Could check here for GL_ARB_texture_non_power_of_two or GL_OES_texture_npot
	// to get full npot support
	return NPOTSupport_Limited;
	
#elif TT_OPENGLES_VERSION >= 3
	return NPOTSupport_Full;
#endif
}


bool hasStencilBufferSupport()
{
	static const GLint stencilBitsNeeded = 8;

	GLint bitsAvailable(0);
	glGetIntegerv(GL_STENCIL_BITS, &bitsAvailable);

	return bitsAvailable >= stencilBitsNeeded;
}


// Namespace end
}
}
}
