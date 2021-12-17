#ifndef INC_TT_ENGINE_OPENGLHEADERS_H
#define INC_TT_ENGINE_OPENGLHEADERS_H


#if defined(TT_PLATFORM_OSX_IPHONE)
// iOS builds: using OpenGL ES 1 or 2
#	define TT_OPENGLES_VERSION 1 // Can be 1 or 2. (Don't mix functions of these two versions!)
#else
#	define TT_OPENGLES_VERSION 0 // No ES version. (Mac OSX uses regular OpenGL.)
#endif


#if TT_OPENGLES_VERSION == 1
#	include <OpenGLES/ES1/gl.h>
#	include <OpenGLES/ES1/glext.h>
#elif TT_OPENGLES_VERSION == 2
#	include <OpenGLES/ES2/gl.h>
#	include <OpenGLES/ES2/glext.h>
#elif TT_OPENGLES_VERSION == 0
#	include <OpenGL/gl.h>
#	include <OpenGL/glext.h>
#else
#	error Unsupported OpenGL version.
#endif


#endif  // !defined(INC_TT_ENGINE_OPENGLHEADERS_H)
