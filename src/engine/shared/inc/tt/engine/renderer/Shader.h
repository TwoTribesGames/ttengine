#if !defined(INC_TT_ENGINE_RENDERER_SHADER_H)
#define INC_TT_ENGINE_RENDERER_SHADER_H


#include <map>
#include <string>
#include <vector>
#include <tt/engine/opengl_headers.h>
#include <tt/engine/cache/ResourceCache.h>
#include <tt/engine/renderer/fwd.h>
#include <tt/engine/EngineID.h>
#include <tt/math/fwd.h>
#include <tt/platform/tt_types.h>


namespace tt {
namespace engine {
namespace renderer {
	
typedef GLint ShaderHandle;

	
// Resource Management
typedef cache::ResourceCache<Shader> ShaderCache;

	

class Shader
{
public:
	static const file::FileType fileType = file::FileType_Shader;
	static const bool hasResourceHeader = true;
	static const ShaderHandle invalidHandle;
	
	~Shader();
	
	void select();
	
	bool setParameter(const std::string& p_name, real p_value);
	bool setParameter(const std::string& p_name, const math::Vector4& p_value);
	bool setParameter(const std::string& p_name, const math::Matrix44& p_value);

	void setVertexParam(ShaderHandle p_handle, const math::Vector4&  p_value);
	void setVertexParam(ShaderHandle p_handle, const math::Matrix44& p_value);
	void setPixelParam (ShaderHandle p_handle, const math::Vector4&  p_value);
	void setPixelParam (ShaderHandle p_handle, const math::Matrix44& p_value);
	
	s32 getSamplerIndex(const std::string& p_name);
	
	ShaderHandle getVertexUniformHandle(const std::string& p_name) const;
	ShaderHandle getPixelUniformHandle (const std::string& p_name) const;
	
	void setVertexUniform (ShaderHandle p_location, u32 p_count, const void* p_values);
	void setPixelUniform  (ShaderHandle p_location, u32 p_count, const void* p_values);
	
	static Shader* create(const fs::FilePtr& p_file, const EngineID& p_id, u32 p_flags);
	bool load(const fs::FilePtr& p_file);
	
	inline s32 getMemSize() const { return sizeof(Shader); }
	inline EngineID getEngineID() const {return m_id;}
	
	static void resetActiveShader();
	
	// OpenGl specific
	inline GLuint   getProgram() const {return m_program;}
	ShaderHandle getUniform(const std::string& p_uniformName) const;
		
	static bool hardwareSupportsLargeShaders();
	
	
private:
	Shader(const EngineID& p_id);
	bool compile(GLuint p_shader);
	void setShaderSource(const char* p_vertexSource, const char* p_fragmentSource);
	bool build();
	bool link();
	bool validate();
	void setSource(const char* p_source, GLuint p_shader);
	
	EngineID m_id;
	GLuint m_program;
	GLuint m_vertexShader;
	GLuint m_fragmentShader;
	
	typedef std::vector<ShaderHandle> Samplers;
	Samplers m_samplers;
	
	typedef std::map<std::string, ShaderHandle> UniformMap;
	mutable UniformMap m_uniformMap;
	
	typedef std::map<ShaderHandle, math::Vector4> ShaderParams;
	typedef std::map<ShaderHandle, math::Matrix44> ShaderMatrixParams;
	ShaderParams m_vertexParameters;
	ShaderParams m_pixelParameters;
	ShaderMatrixParams m_vertexMatrixParams;
	ShaderMatrixParams m_pixelMatrixParams;
	
	static Shader* ms_activeShader;
};

// Namespace end
}
}
}


#endif  // !defined(INC_TT_ENGINE_RENDERER_SHADER_H)
