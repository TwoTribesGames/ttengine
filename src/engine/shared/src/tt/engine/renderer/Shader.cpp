#include <tt/code/Buffer.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/engine/renderer/Shader.h>
#include <tt/engine/renderer/Texture.h>
#include <tt/fs/File.h>
#include <tt/math/Matrix44.h>
#include <tt/math/Vector4.h>
#include <tt/platform/tt_error.h>


namespace tt {
namespace engine {
namespace renderer {
	
	
Shader* Shader::ms_activeShader = 0;
const ShaderHandle Shader::invalidHandle = -1;
	

//--------------------------------------------------------------------------------------------------
// Public member functions

Shader::~Shader()
{
#if TT_OPENGLES_VERSION != 1
	if(m_program != 0 && Renderer::hasInstance())
	{
		glDeleteProgram(m_program);
		glDeleteShader(m_vertexShader);
		glDeleteShader(m_fragmentShader);
	}
#endif
}
	
	
void Shader::select()
{
	if (ms_activeShader != this)
	{
#if TT_OPENGLES_VERSION == 1
		TT_PANIC("Not supported by OpenGL ES 1");
#else
		TT_NULL_ASSERT(m_program);
	
		glUseProgram(m_program);
	
		TT_CHECK_OPENGL_ERROR();
#endif
		ms_activeShader = this;
	}
}
	
bool Shader::setParameter(const std::string& p_name, real p_value)
{
	ShaderHandle handle = getUniform(p_name.c_str());
	if (handle != invalidHandle)
	{
		select();
		glUniform1f(handle, p_value);
		return true;
	}
	return false;
}
	
	
bool Shader::setParameter(const std::string& p_name, const math::Vector4& p_value)
{
	ShaderHandle handle = getUniform(p_name.c_str());
	if (handle != invalidHandle)
	{
		select();
		glUniform4f(handle, p_value.x, p_value.y, p_value.z, p_value.w);
		return true;
	}
	return false;
}

	
bool Shader::setParameter(const std::string &p_name, const math::Matrix44 &p_value)
{
	ShaderHandle handle = getUniform(p_name.c_str());
	if (handle != invalidHandle)
	{
		select();
		glUniformMatrix4fv(handle, 1, false, reinterpret_cast<const GLfloat*>(&p_value));
		return true;
	}
	return false;
}
	
void Shader::setVertexParam(ShaderHandle p_handle, const math::Vector4&  p_value)
{
	TT_ASSERT(p_handle != invalidHandle);
	if (m_vertexParameters[p_handle] == p_value)
	{
		return;
	}
	
	m_vertexParameters[p_handle] = p_value;

	if (this == ms_activeShader)
	{
		setVertexUniform(p_handle, 4, &m_vertexParameters[p_handle]);
	}
}
	
void Shader::setVertexParam(ShaderHandle p_handle, const math::Matrix44& p_value)
{
	TT_ASSERT(p_handle != invalidHandle);
	
	m_vertexMatrixParams[p_handle] = p_value;
	
	if (this == ms_activeShader)
	{
		setVertexUniform(p_handle, 16, &m_vertexMatrixParams[p_handle]);
	}
}
	
void Shader::setPixelParam (ShaderHandle p_handle, const math::Vector4&  p_value)
{
	TT_ASSERT(p_handle != invalidHandle);
	if (m_pixelParameters[p_handle] == p_value)
	{
		return;
	}
	
	m_pixelParameters[p_handle] = p_value;
	
	if (this == ms_activeShader)
	{
		setPixelUniform(p_handle, 4, &m_pixelParameters[p_handle]);
	}
}
	
void Shader::setPixelParam (ShaderHandle p_handle, const math::Matrix44& p_value)
{
	TT_ASSERT(p_handle != invalidHandle);
	
	m_pixelMatrixParams[p_handle] = p_value;
	
	if (this == ms_activeShader)
	{
		setPixelUniform(p_handle, 16, &m_pixelMatrixParams[p_handle]);
	}
}
	
s32 Shader::getSamplerIndex(const std::string& p_name)
{
	ShaderHandle handle = getUniform(p_name.c_str());
	if (handle == invalidHandle)
	{
		return -1;
	} 
	u32 samplerIndex(0);
	for (Samplers::iterator it = m_samplers.begin(); it != m_samplers.end(); ++it)
	{
		if (*it == handle)
		{
			return samplerIndex;
		}
		++samplerIndex;
	}
	
	// Add new sampler
	m_samplers.push_back(handle);
	select();
	glUniform1i(handle, samplerIndex);
	
	return samplerIndex;
}
	
	
ShaderHandle Shader::getVertexUniformHandle(const std::string& p_name) const
{
	return getUniform(p_name);
}


ShaderHandle Shader::getPixelUniformHandle (const std::string& p_name) const
{
	return getUniform(p_name);
}


void Shader::setVertexUniform (ShaderHandle p_handle, u32 p_count, const void* p_values)
{
	TT_ASSERT(p_handle != Shader::invalidHandle);
	if (this != ms_activeShader)
	{
		return;
	}

	if (p_count == 4)
	{
		glUniform4fv(p_handle, 1, static_cast<const GLfloat*>(p_values));
	}
	else if (p_count == 16)
	{
		glUniformMatrix4fv(p_handle, 1, GL_FALSE, static_cast<const GLfloat*>(p_values));
	}
	else if (p_count == 1)
	{
		glUniform1fv(p_handle, 1, static_cast<const GLfloat*>(p_values));
	}
}
	
	
void Shader::setPixelUniform  (ShaderHandle p_handle, u32 p_count, const void* p_values)
{
	TT_ASSERT(p_handle != Shader::invalidHandle);
	if (this != ms_activeShader)
	{
		return;
	}

	if (p_count == 4)
	{
		glUniform4fv(p_handle, 1, static_cast<const GLfloat*>(p_values));
	}
	else if (p_count == 1)
	{
		glUniform1fv(p_handle, 1, static_cast<const GLfloat*>(p_values));
	}
}


Shader* Shader::create(const fs::FilePtr&, const EngineID& p_id, u32)
{
	return new Shader(p_id);
}


bool Shader::load(const fs::FilePtr& p_file)
{
	m_uniformMap.clear();
	m_vertexParameters.clear();
	m_pixelParameters.clear();
	m_vertexMatrixParams.clear();
	m_pixelMatrixParams.clear();
	
	u32 vertexShaderSize(0);
	if (p_file->read(&vertexShaderSize, sizeof(vertexShaderSize)) != sizeof(vertexShaderSize))
	{
		return false;
	}
	std::vector<char> vertexShaderSource(vertexShaderSize);
	if (p_file->read(&vertexShaderSource[0], vertexShaderSize) != static_cast<fs::size_type>(vertexShaderSize))
	{
		return false;
	}
	std::string vertexSource(vertexShaderSource.begin(), vertexShaderSource.end());
	
	u32 fragmentShaderSize(0);
	if (p_file->read(&fragmentShaderSize, sizeof(fragmentShaderSize)) != sizeof(fragmentShaderSize))
	{
		return false;
	}
	std::vector<char> fragmentShaderSource(fragmentShaderSize);
	if (p_file->read(&fragmentShaderSource[0], fragmentShaderSize) != static_cast<fs::size_type>(fragmentShaderSize))
	{
		return false;
	}
	std::string fragmentSource(fragmentShaderSource.begin(), fragmentShaderSource.end());
	
	// Create shader program by linking fragment and vertex shader
	
	//TT_Printf("VERTEX SHADER:  \n %s\n\n", vertexSource.c_str());
	//TT_Printf("FRAGMENT SHADER:\n %s\n\n", fragmentSource.c_str());
	
	setShaderSource(vertexSource.c_str(), fragmentSource.c_str());
	
	if (build() == false)
	{
		return false;
	}
	
	
	TT_CHECK_OPENGL_ERROR();
	
	return true;
}
	
	
void Shader::resetActiveShader()
{
	glUseProgram(0);
	ms_activeShader = 0;
}
	

ShaderHandle Shader::getUniform(const std::string& p_uniformName) const
{
	TT_NULL_ASSERT(m_program);
	
	UniformMap::iterator it = m_uniformMap.find(p_uniformName);
	if (it != m_uniformMap.end())
	{
		return it->second;
	}
	
	ShaderHandle handle = glGetUniformLocation(m_program, p_uniformName.c_str());
	m_uniformMap[p_uniformName] = handle;
	
	return handle;
}


// Checks for errors in the fragment program
bool Shader::hardwareSupportsLargeShaders()
{
#if TT_OPENGLES_VERSION == 1
	return false;
#elif 0 // Not needed in OpenLG 2.1+
	// Source of this code:
	// http://petewarden.com/notes/archives/2005/06/fragment_progra_3.html
	// NOTE! This code uses the OpenGL 1 fragment programs extension.
	//       We use the OpenGL 2 fragment programs.
	//       We can't compile our programs and change it against the hardware limits.
	//       We can only check the hardware limits against some magic number.
	//       There is also nothing like GL_PROGRAM_UNDER_NATIVE_LIMITS_ARB in OpenGL 2.

	// Make sure the card supports fragment programs at all, by searching for the extension string
	const char *extensions = reinterpret_cast<const char *>( glGetString( GL_EXTENSIONS ) );
	const bool cardSupportsARB = ( strstr( extensions, "GL_ARB_fragment_program" ) != NULL );
	
	// If it doesn't support them, no program can run, so report the problem and return false
	if (!cardSupportsARB)
	{
		TT_PANIC("Card does not support the ARB_fragment_program extension");
		return false;
	}

	GLint maxAluInstructions;
	glGetProgramivARB(GL_FRAGMENT_PROGRAM_ARB, GL_MAX_PROGRAM_ALU_INSTRUCTIONS_ARB, &maxAluInstructions);
	
	GLint maxNativeAluInstructions;
	glGetProgramivARB(GL_FRAGMENT_PROGRAM_ARB, GL_MAX_PROGRAM_NATIVE_ALU_INSTRUCTIONS_ARB, &maxNativeAluInstructions);
	
	GLint maxTextureIndirections;
	glGetProgramivARB(GL_FRAGMENT_PROGRAM_ARB, GL_MAX_PROGRAM_TEX_INDIRECTIONS_ARB, &maxTextureIndirections);
	
	GLint maxNativeTextureIndirections;
	glGetProgramivARB(GL_FRAGMENT_PROGRAM_ARB, GL_MAX_PROGRAM_NATIVE_TEX_INDIRECTIONS_ARB, &maxNativeTextureIndirections);
	
	GLint maxTextureInstructions;
	glGetProgramivARB(GL_FRAGMENT_PROGRAM_ARB, GL_MAX_PROGRAM_TEX_INSTRUCTIONS_ARB, &maxTextureInstructions);
	
	bool result = maxAluInstructions           >= 512 &&
	              maxNativeAluInstructions     >= 512 &&
	              maxTextureIndirections       >= 512 &&
	              maxNativeTextureIndirections >= 512 &&
	              maxTextureInstructions       >= 512;
	
	TT_WARNING(result,
	           "Hardware to limited - maxAluInstructions: %d, maxNativeAluInstructions: %d, "
	           "maxTextureIndirections: %d, maxNativeTextureIndirections: %d, "
	           "maxTextureInstructions: %d.", 
	           maxAluInstructions,     maxNativeAluInstructions,
	           maxTextureIndirections, maxNativeTextureIndirections,
	           maxTextureInstructions);
	return result;
#else
	// we assume OpenGL 2.1+
	return true;
#endif
}	


//--------------------------------------------------------------------------------------------------
// Private member functions

Shader::Shader(const EngineID& p_id)
:
m_id(p_id),
m_program(0),
m_vertexShader(0),
m_fragmentShader(0)
{
}
	

bool Shader::compile(GLuint p_shader)
{
#if TT_OPENGLES_VERSION == 1
	return false;
#else
	glCompileShader(p_shader);
	
	GLint status(GL_FALSE);
	glGetShaderiv(p_shader, GL_COMPILE_STATUS, &status);
	
	if(status != GL_TRUE)
	{
		GLint logLength(0);
		glGetShaderiv(p_shader, GL_INFO_LOG_LENGTH, &logLength);
		
		char* log = new char[logLength];
		glGetShaderInfoLog(p_shader, logLength, 0, log);
		
		TT_PANIC("Shader Compilation Failed!\n\nShader Log:\n%s", log);
		
		delete [] log;
		
		return false;
	}
	return true;
#endif
}
	
void Shader::setShaderSource(const char* p_vertexSource, const char* p_fragmentSource)
{
#if TT_OPENGLES_VERSION != 1
	if(m_program == 0)
	{
		m_program        = glCreateProgram();
		m_vertexShader   = glCreateShader(GL_VERTEX_SHADER);
		m_fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	}
	
	setSource(p_vertexSource,   m_vertexShader);
	setSource(p_fragmentSource, m_fragmentShader);
#endif
}


bool Shader::build()
{
#if TT_OPENGLES_VERSION == 1
	return false;
#else
	// Compile and link shaders
	if(compile(m_vertexShader) == false)
	{
		return false;
	}
	if(compile(m_fragmentShader) == false)
	{
		return false;
	}
	
	glAttachShader(m_program, m_vertexShader);
	glAttachShader(m_program, m_fragmentShader);
	
	return link();
#endif
}


bool Shader::link()
{
#if TT_OPENGLES_VERSION == 1
	return false;
#else
	if(m_program == 0) return false;
	
	glLinkProgram(m_program);
	
	GLint status(GL_FALSE);
	glGetProgramiv(m_program, GL_LINK_STATUS, &status);
	
	if(status != GL_TRUE)
	{
		GLint logLength(0);
		glGetProgramiv(m_program, GL_INFO_LOG_LENGTH, &logLength);
		
		char* log = new char[logLength];
		glGetProgramInfoLog(m_program, logLength, 0, log);
		
		TT_PANIC("Shader Linking Failed!\n\nProgram Log:\n%s", log);
		
		delete [] log;
		
		return false;
	}
	
	return true;
#endif
}


bool Shader::validate()
{
#if TT_OPENGLES_VERSION == 1
	return false;
#else
	if(m_program == 0) return false;
	
	glValidateProgram(m_program);
	GLint status = GL_FALSE;
	glGetProgramiv(m_program, GL_VALIDATE_STATUS, &status);
	
	if (status != GL_TRUE)
	{
		GLint logLength(0);
		glGetProgramiv(m_program, GL_INFO_LOG_LENGTH, &logLength);
		
		char* log = new char[logLength];
		glGetProgramInfoLog(m_program, logLength, 0, log);
		
		TT_PANIC("Shader Validation Failed!\n\nProgram Log:\n%s", log);
		
		delete [] log;
		
		return false;
	}
	return true;
#endif
}
	
	
void Shader::setSource(const char* p_source, GLuint p_shader)
{
#if TT_OPENGLES_VERSION == 1
	(void) p_source; (void) p_shader;
#else
	glShaderSource(p_shader, 1, &p_source, 0);
	
	TT_CHECK_OPENGL_ERROR();
#endif
}


// Namespace end
}
}
}
