

#include <tt/platform/tt_error.h>
#include <tt/engine/renderer/directx.h>
#include <tt/engine/renderer/D3DResourceRegistry.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/engine/renderer/Shader.h>
#include <tt/engine/renderer/Texture.h>
#include <tt/math/Matrix44.h>
#include <tt/math/Vector4.h>
#include <tt/fs/File.h>
#include <tt/mem/mem.h>




namespace tt {
namespace engine {
namespace renderer {


Shader* Shader::ms_activeShader = 0;
const ShaderHandle Shader::invalidHandle = 0;

//--------------------------------------------------------------------------------------------------
// Public member functions


Shader::~Shader()
{
	safeRelease (m_vertexConstantTable);
	safeRelease (m_pixelConstantTable);
	safeRelease (m_compiledVertexShader);
	safeRelease (m_compiledPixelShader);
	safeRelease (m_vertexShader);
	safeRelease (m_pixelShader);

	D3DResourceRegistry::unregisterResource(this);
}


void Shader::select()
{
	if (getRenderDevice(true) == nullptr) return;
	
	Renderer::getInstance()->checkFromRenderThread();
	
	if (m_vertexShader == 0 && m_pixelShader == 0)
	{
		deviceCreated();
		deviceReset();
	}
	
	if (ms_activeShader != this)
	{
		checkD3DSucceeded( getRenderDevice()->SetVertexShader (m_vertexShader) );
		checkD3DSucceeded( getRenderDevice()->SetPixelShader  (m_pixelShader)  );
		
		ms_activeShader = this;

		setShaderParameters();
	}
}


bool Shader::setParameter(const std::string& p_name, real p_value)
{
	return setParameter(p_name, math::Vector4(p_value,0,0,0));
}


bool Shader::setParameter(const std::string& p_name, const math::Vector4& p_value)
{
	bool result(false);

	ShaderHandle handle = getVertexUniformHandle(p_name);
	if (handle != invalidHandle)
	{
		setVertexParam(handle, p_value);
		result = true;
	}
	handle = getPixelUniformHandle(p_name);
	if (handle != invalidHandle)
	{
		setPixelParam(handle, p_value);
		result = true;
	}
	return result;
}


bool Shader::setParameter(const std::string& p_name, const math::Matrix44& p_value)
{
	bool result(false);
	
	ShaderHandle handle = getVertexUniformHandle(p_name);
	if (handle != invalidHandle)
	{
		setVertexParam(handle, p_value);
		result = true;
	}
	handle = getPixelUniformHandle(p_name);
	if (handle != invalidHandle)
	{
		setPixelParam(handle, p_value);
		result = true;
	}
	
	return result;
}


void Shader::setVertexParam(ShaderHandle p_handle, const math::Vector4& p_value)
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


void Shader::setPixelParam(ShaderHandle p_handle, const math::Vector4& p_value)
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


void Shader::setPixelParam(ShaderHandle p_handle, const math::Matrix44& p_value)
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
	ShaderHandle handle = getHandle(p_name, m_vertexConstantTable);
	if (handle != invalidHandle)
	{
		return m_vertexConstantTable->GetSamplerIndex(handle);
	}

	handle = getHandle(p_name, m_pixelConstantTable);
	if (handle != invalidHandle)
	{
		return m_pixelConstantTable->GetSamplerIndex(handle);
	}

	return -1;
}


ShaderHandle Shader::getVertexUniformHandle(const std::string& p_name) const
{
	if (m_vertexConstantTable != 0)
	{
		return m_vertexConstantTable->GetConstantByName(0, p_name.c_str());
	}
	return invalidHandle;
}


ShaderHandle Shader::getPixelUniformHandle(const std::string& p_name) const
{
	if (m_pixelConstantTable != 0)
	{
		return m_pixelConstantTable->GetConstantByName(0, p_name.c_str());
	}
	return invalidHandle;
}


void Shader::setVertexUniform(ShaderHandle p_handle, u32 p_count, const void* p_values)
{
	TT_ASSERT(p_handle != invalidHandle);

	if (p_handle != invalidHandle)
	{
		checkD3DSucceeded( m_vertexConstantTable->SetFloatArray(
			getRenderDevice(true), p_handle, static_cast<const FLOAT*>(p_values), p_count) );
	}
}


void Shader::setPixelUniform(ShaderHandle p_handle, u32 p_count, const void* p_values)
{
	TT_ASSERT(p_handle != invalidHandle);
	TT_NULL_ASSERT(m_pixelConstantTable);

	if (p_handle != invalidHandle)
	{
		checkD3DSucceeded( m_pixelConstantTable->SetFloatArray(
			getRenderDevice(true), p_handle, static_cast<const FLOAT*>(p_values), p_count) );
	}
}


Shader* Shader::create(const fs::FilePtr&, const EngineID& p_id, u32)
{
	return new Shader(p_id);
}


bool Shader::load(const fs::FilePtr& p_file)
{
	DWORD vertexShaderSize(0);
	if (p_file->read(&vertexShaderSize, sizeof(vertexShaderSize)) != sizeof(vertexShaderSize))
	{
		return false;
	}
	
	safeRelease(m_compiledVertexShader);
	safeRelease(m_vertexConstantTable);
	m_vertexParameters.clear();
	m_pixelParameters.clear();
	m_vertexMatrixParams.clear();
	m_pixelMatrixParams.clear();
	
	if (checkD3DSucceeded( D3DXCreateBuffer(vertexShaderSize, &m_compiledVertexShader) ) == false)
	{
		return false;
	}

	if (p_file->read(m_compiledVertexShader->GetBufferPointer(), vertexShaderSize) !=
		static_cast<fs::size_type>(vertexShaderSize))
	{
		return false;
	}

	if (checkD3DSucceeded(D3DXGetShaderConstantTable(static_cast<DWORD*>(
			m_compiledVertexShader->GetBufferPointer()), &m_vertexConstantTable)) == false)
	{
		return false;
	}

	DWORD pixelShaderSize(0);
	if (p_file->read(&pixelShaderSize, sizeof(pixelShaderSize)) != sizeof(pixelShaderSize))
	{
		return false;
	}

	safeRelease(m_compiledPixelShader);
	safeRelease(m_pixelConstantTable);

	if (checkD3DSucceeded(D3DXCreateBuffer(pixelShaderSize, &m_compiledPixelShader) ) == false)
	{
		return false;
	}

	if (p_file->read(m_compiledPixelShader->GetBufferPointer(), pixelShaderSize) !=
		static_cast<fs::size_type>(pixelShaderSize))
	{
		return false;
	}

	if (checkD3DSucceeded(D3DXGetShaderConstantTable(static_cast<DWORD*>(
			m_compiledPixelShader->GetBufferPointer()), &m_pixelConstantTable)) == false)
	{
		return false;
	}

	// If a D3D device exists, create the actual shaders,
	// otherwise they will be created when the device is created
	if (getRenderDevice() != 0)
	{
		deviceCreated();
		deviceReset();
	}

	return true;
}


s32 Shader::getMemSize() const
{
	s32 totalSize(0);
	if (m_compiledVertexShader != 0)
	{
		totalSize += m_compiledVertexShader->GetBufferSize();
	}
	if (m_compiledPixelShader != 0)
	{
		totalSize += m_compiledPixelShader->GetBufferSize();
	}
	return totalSize;
}


void Shader::resetActiveShader()
{
	if (getRenderDevice() != 0)
	{
		checkD3DSucceeded( getRenderDevice()->SetVertexShader (0) );
		checkD3DSucceeded( getRenderDevice()->SetPixelShader  (0) );
	}
	ms_activeShader = 0;
}


void Shader::deviceCreated()
{
	safeRelease (m_vertexShader);
	safeRelease (m_pixelShader);
	
	IDirect3DDevice9* device = getRenderDevice(true);

	if(m_compiledVertexShader != 0)
	{
		checkD3DSucceeded( device->CreateVertexShader(
			static_cast<DWORD*>(m_compiledVertexShader->GetBufferPointer()), &m_vertexShader) );
	}

	if(m_compiledPixelShader != 0)
	{
		checkD3DSucceeded( device->CreatePixelShader(
			static_cast<DWORD*>(m_compiledPixelShader->GetBufferPointer()), &m_pixelShader) );
	}
}


void Shader::deviceReset()
{
	IDirect3DDevice9* device = getRenderDevice(true);

	if (m_vertexConstantTable != 0)
	{
		checkD3DSucceeded(m_vertexConstantTable->SetDefaults(device));
	}

	if (m_pixelConstantTable != 0)
	{
		checkD3DSucceeded(m_pixelConstantTable->SetDefaults(device));
	}
}


void Shader::deviceDestroyed()
{
	safeRelease (m_vertexShader);
	safeRelease (m_pixelShader);
}


//--------------------------------------------------------------------------------------------------
// Private member functions

Shader::Shader(const EngineID& p_id)
:
m_compiledVertexShader(0),
m_compiledPixelShader(0),
m_vertexShader(0),
m_pixelShader(0),
m_vertexConstantTable(0),
m_pixelConstantTable(0),
m_id(p_id)
{
	D3DResourceRegistry::registerResource(this);
}


ShaderHandle Shader::getHandle(const std::string& p_name, ID3DXConstantTable* p_table) const
{
	if(p_table != 0)
	{
		return p_table->GetConstantByName(0, p_name.c_str());
	}
	return invalidHandle;
}


void Shader::setShaderParameters()
{
	for (ShaderParams::iterator it = m_vertexParameters.begin(); it != m_vertexParameters.end(); ++it)
	{
		setVertexUniform(it->first, 4, &it->second);
	}

	for (ShaderParams::iterator it = m_pixelParameters.begin(); it != m_pixelParameters.end(); ++it)
	{
		setPixelUniform(it->first, 4, &it->second);
	}

	for (ShaderMatrixParams::iterator it = m_vertexMatrixParams.begin(); it != m_vertexMatrixParams.end(); ++it)
	{
		setVertexUniform(it->first, 16, &it->second);
	}

	for (ShaderMatrixParams::iterator it = m_pixelMatrixParams.begin(); it != m_pixelMatrixParams.end(); ++it)
	{
		setPixelUniform(it->first, 16, &it->second);
	}
}


// Namespace end
}
}
}
