#if !defined(INC_TT_ENGINE_RENDERER_SHADER_H)
#define INC_TT_ENGINE_RENDERER_SHADER_H

#define NOMINMAX
#include <d3dx9shader.h>
#include <string>

#include <tt/engine/cache/ResourceCache.h>
#include <tt/engine/renderer/D3DResource.h>
#include <tt/engine/renderer/fwd.h>
#include <tt/math/fwd.h>


namespace tt {
namespace engine {
namespace renderer {

typedef D3DXHANDLE ShaderHandle;


// Resource Management
typedef cache::ResourceCache<Shader> ShaderCache;


class Shader : public D3DResource
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
	
	void setVertexUniform (ShaderHandle p_handle, u32 p_count, const void* p_values);
	void setPixelUniform  (ShaderHandle p_handle, u32 p_count, const void* p_values);

	static Shader* create(const fs::FilePtr& p_file, const EngineID& p_id, u32 p_flags);
	bool load(const fs::FilePtr& p_file);

	s32 getMemSize() const;
	inline EngineID getEngineID() const { return m_id; }

	static void resetActiveShader();

	virtual void deviceCreated();
	virtual void deviceReset();
	virtual void deviceDestroyed();

private:
	explicit Shader(const EngineID& p_id);

	bool compileShader(const std::string&   p_shaderAsText,
		               const std::string&   p_profile,
		               LPD3DXBUFFER*        p_compiledShader_OUT,
		               LPD3DXCONSTANTTABLE* p_constantTable_OUT);

	ShaderHandle getHandle(const std::string& p_name, ID3DXConstantTable* p_table) const;

	void setShaderParameters();


	LPD3DXBUFFER            m_compiledVertexShader;
	LPD3DXBUFFER            m_compiledPixelShader;
	IDirect3DVertexShader9* m_vertexShader;
	IDirect3DPixelShader9*  m_pixelShader;
	ID3DXConstantTable*     m_vertexConstantTable;
	ID3DXConstantTable*     m_pixelConstantTable;
	EngineID                m_id;

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
