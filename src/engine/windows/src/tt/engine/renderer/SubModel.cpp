#include <tt/engine/renderer/SubModel.h>

#include <tt/platform/tt_error.h>
#include <tt/fs/File.h>
#include <tt/engine/debug/DebugRenderer.h>
#include <tt/engine/debug/DebugStats.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/engine/renderer/directx.h>
#include <tt/engine/renderer/D3DResourceRegistry.h>
#include <tt/engine/renderer/MatrixStack.h>
#include <tt/engine/renderer/Material.h>
#include <tt/engine/renderer/Texture.h>
#include <tt/engine/scene/Model.h>


namespace tt {
namespace engine {
namespace renderer {


SubModel::SubModel()
:
m_model(0),
m_triangleCount(0),
m_vertexBuffer(0),
m_indexBuffer(0)
{
	// Register this resource
	D3DResourceRegistry::registerResource(this);
}


SubModel::SubModel(const SubModel& p_model)
:
m_material(p_model.m_material),
m_model(0), // Need to update this one after copying!!
m_vertices(p_model.m_vertices),
m_indices(p_model.m_indices),
m_triangleCount(p_model.m_triangleCount),
m_vertexBuffer(0),
m_indexBuffer(0)
{
	// Register this resource
	D3DResourceRegistry::registerResource(this);

	// Create own buffers
	updateBuffers();
}


SubModel::~SubModel()
{
	// Un-Register this resource
	D3DResourceRegistry::unregisterResource(this);
	Renderer::addToDeathRow(m_vertexBuffer);
	Renderer::addToDeathRow(m_indexBuffer);
}


bool SubModel::load(const fs::FilePtr& p_file, scene::Model* p_model)
{
	// Store pointer to the model
	m_model = p_model;

	// Now we can load our information
	EngineID matID(0,0);
	if(matID.load(p_file) == false)
	{
		return false;
	}

	// Read the vertex type
	u32 vertexType = 0;
	if (p_file->read(&vertexType, sizeof(vertexType)) != sizeof(vertexType))
	{
		return false;
	}
	m_vertices.setVertexType(vertexType);

	// Read number of vertices
	s32 vertexCount = 0;
	if (p_file->read(&vertexCount, sizeof(vertexCount)) != sizeof(vertexCount))
	{
		return false;
	}

	TT_ASSERTMSG(vertexCount > 0, "[ENGINE] SubModel contains no vertices.");

	// Read number of faces (= nr of indices / 3)
	// Maybe we want to read index count instead
	if (p_file->read(&m_triangleCount, sizeof(m_triangleCount)) != sizeof(m_triangleCount))
	{
		return false;
	}

	s32 packed_data = 0;
	if (p_file->read(&packed_data, sizeof(packed_data)) != sizeof(packed_data))
	{
		return false;
	}

	// Load the material
	if(matID.valid())
	{
		m_material = MaterialCache::get(matID, false);

		if(m_material != 0 && m_material->checkFlag(Material::Flag_Transparent))
		{
			p_model->setFlag(scene::SceneObject::Flag_Transparent);
		}
	}
	if(m_material == 0)
	{
		// Get default material
		m_material = MaterialPtr(new Material(ColorRGBA(255,0,255,255)));
	}

	if(p_model->checkFlag(scene::SceneObject::Flag_DisableLighting))
	{
		m_material->setFlag(Material::Flag_DisableLighting);
	}

	if (packed_data != 0)
	{
		TT_PANIC("Packed data not supported for windows build");
	}
	else if(vertexCount > 0)
	{
		// Here we load a vertex buffer instead of an array of vertices
		if(m_vertices.load(p_file, vertexCount) == false)
		{
			return false;
		}

		// Load the indices, we can still read the whole buffer at once
		m_indices.resize(m_triangleCount * 3);
		TT_ASSERTMSG(m_indices.size() < std::numeric_limits<u16>::max(), 
			"Too many vertices in submodel of model %s",
			m_model->getEngineID().toDebugString().c_str());

		s32 indexSize(static_cast<s32>(sizeof(u16) * m_indices.size()));

		if (p_file->read(&m_indices[0], indexSize) != indexSize)
		{
			return false;
		}
	}

	// Create DirectX buffers
	return updateBuffers();
}


bool SubModel::updateBuffers()
{
	const VertexBuffer::PositionBuffer& positions(m_vertices.getPositions());
	
	// Check if we have any vertices
	if(positions.empty()) return true;
	
	// Release resources if already used
	if(m_vertexBuffer != 0)
	{
		m_vertexBuffer->Release();
	}

	if(m_indexBuffer != 0)
	{
		m_indexBuffer->Release();
	}

	// Compose the FVF
	s32 fvf = D3DFVF_XYZ;

	if(m_vertices.hasProperty(VertexBuffer::Property_Normal))
	{
		fvf |= D3DFVF_NORMAL;
	}
	if(m_vertices.hasProperty(VertexBuffer::Property_Diffuse))
	{
		fvf |= D3DFVF_DIFFUSE;
	}

	s32 count(0);
	for(s32 i = 0; i < 4; ++i)
	{
		if ((m_vertices.getVertexType() & (VertexBuffer::Property_Texture0 << i)) != 0)
		{
			fvf |= D3DFVF_TEXCOORDSIZE2(i);
			++count;
		}
	}

	// This will put in D3DFVF_TEXn (where n is the number of tex coords)
	fvf |= (count << D3DFVF_TEXCOUNT_SHIFT);

	// Create Vertex Buffer
	const size_t vertexCount = static_cast<size_t>(positions.size());
	
	if(FAILED(getRenderDevice()->CreateVertexBuffer(
								static_cast<UINT>(vertexCount * m_vertices.getVertexSize()),
								0, fvf, D3DPOOL_MANAGED, &m_vertexBuffer, 0)))
	{
		TT_PANIC("Failed to create vertex buffer for SubModel");
		return false;
	}

	// Lock the vertex buffer.
	u8* vertices = 0;
	if(FAILED(m_vertexBuffer->Lock(0, 0, reinterpret_cast<void**>(&vertices), 0)))
	{
		TT_PANIC("Failed to lock vertex buffer");
		return false;
	}

	// Copy all vertices into the buffer
	const bool hasNormal   = m_vertices.hasProperty(VertexBuffer::Property_Normal  );
	const bool hasDiffuse  = m_vertices.hasProperty(VertexBuffer::Property_Diffuse );
	const bool hasTexture0 = m_vertices.hasProperty(VertexBuffer::Property_Texture0);
	const bool hasTexture1 = m_vertices.hasProperty(VertexBuffer::Property_Texture1);
	const bool hasTexture2 = m_vertices.hasProperty(VertexBuffer::Property_Texture2);
	const bool hasTexture3 = m_vertices.hasProperty(VertexBuffer::Property_Texture3);
	
	const VertexBuffer::Position* positionsPtr = &positions[0];
	const VertexBuffer::Normal*   normals      = hasNormal   ? &m_vertices.getNormals()[0]    : 0;
	const ColorRGBA*              colors       = hasDiffuse  ? &m_vertices.getColors()[0]     : 0;
	const VertexBuffer::TexCoord* texCoords0   = hasTexture0 ? &m_vertices.getTexCoords(0)[0] : 0;
	const VertexBuffer::TexCoord* texCoords1   = hasTexture1 ? &m_vertices.getTexCoords(1)[0] : 0;
	const VertexBuffer::TexCoord* texCoords2   = hasTexture2 ? &m_vertices.getTexCoords(2)[0] : 0;
	const VertexBuffer::TexCoord* texCoords3   = hasTexture3 ? &m_vertices.getTexCoords(3)[0] : 0;
	
	for (size_t i = 0; i < vertexCount; ++i)
	{
		// Copy position
		memcpy(vertices, positionsPtr, sizeof(VertexBuffer::Position));

		++positionsPtr;
		vertices += sizeof(VertexBuffer::Position);

		// Copy normal
		if (hasNormal)
		{
			memcpy(vertices, normals, sizeof(VertexBuffer::Normal));

			++normals;
			vertices += sizeof(VertexBuffer::Normal);
		}

		// Copy Color
		if (hasDiffuse)
		{
			D3DCOLOR d3dcolor = D3DCOLOR_ARGB(colors->a, colors->r, colors->g, colors->b);
			memcpy(vertices, &d3dcolor, sizeof(D3DCOLOR));

			++colors;
			vertices += sizeof(D3DCOLOR);
		}

		// Copy Texture Coords
		if (hasTexture0)
		{
			memcpy(vertices, texCoords0, sizeof(VertexBuffer::TexCoord));

			++texCoords0;
			vertices += sizeof(VertexBuffer::TexCoord);
		}
		if (hasTexture1)
		{
			memcpy(vertices, texCoords1, sizeof(VertexBuffer::TexCoord));

			++texCoords1;
			vertices += sizeof(VertexBuffer::TexCoord);
		}
		if (hasTexture2)
		{
			memcpy(vertices, texCoords2, sizeof(VertexBuffer::TexCoord));

			++texCoords2;
			vertices += sizeof(VertexBuffer::TexCoord);
		}
		if (hasTexture3)
		{
			memcpy(vertices, texCoords3, sizeof(VertexBuffer::TexCoord));

			++texCoords3;
			vertices += sizeof(VertexBuffer::TexCoord);
		}
	}

	m_vertexBuffer->Unlock();

	// Create index buffer
	if (FAILED(getRenderDevice()->CreateIndexBuffer(static_cast<UINT>(m_indices.size() * sizeof(u16)), 0,
			D3DFMT_INDEX16 ,D3DPOOL_MANAGED, &m_indexBuffer, 0)))
	{
		TT_PANIC("Failed to create index buffer for SubModel");
		return false;
	}

	void* indices = 0;
	if(FAILED(m_indexBuffer->Lock(0, 0, &indices, 0)))
	{
		TT_PANIC("Failed to lock index buffer");
		return false;
	}
	// Copy indices
	memcpy(indices, &m_indices[0], m_indices.size() * sizeof(u16));

	m_indexBuffer->Unlock();

	return true;
}


void SubModel::render(RenderContext& p_renderContext)
{
	Renderer* renderer = Renderer::getInstance();
	IDirect3DDevice9* renderDevice = getRenderDevice();
	
	bool resetBackFaceCulling = false;
	const bool oldLightSetting(renderer->isLightingEnabled());
	
	// Do not render shadow volumes on windows
	if(m_model != 0 && m_model->getType() == scene::SceneObject::Type_ShadowModel)
	{
		TT_PANIC("Shadow models are not supported anymore");
		return;
	}
	
	if (m_material != 0 && p_renderContext.pass != RenderPass_ShadowVolumes)
	{
		if (m_material->checkFlag(Material::Flag_DisableLighting))
		{
			renderer->setLighting(false);
		}
		
		m_material->select(p_renderContext);
		
		// Double sided?
		if (renderer->isCullingEnabled() && m_material->checkFlag(Material::Flag_DoubleSided))
		{
			resetBackFaceCulling = true;
			renderer->setCullingEnabled(false);
		}
	}
	
	// Increase the polygons rendered count
	debug::DebugStats::addToPolygonsRendered(m_triangleCount);
	
	// Set up the correct format
	renderer->setVertexType(m_vertices.getVertexType());
	
	/*
	// FIXME: Should not be needed
	if (m_material != 0 && m_material->checkFlag(Material::Flag_DisableLighting))
	{
		// Overwrite setting from setVertexType()
		FP(renderDevice)->SetRenderState(FPRS(D3DRS_LIGHTING), FALSE);
	}
	*/
	
	// Update the world matrix
	MatrixStack::getInstance()->updateWorldMatrix();
	
	renderDevice->SetIndices(m_indexBuffer);
	
	renderDevice->SetStreamSource(0,                           // Stream ID
	                              m_vertexBuffer,              // Vertex Buffer
	                              0,                           // Offset
	                              m_vertices.getVertexSize()); // Stride
	
	renderDevice->DrawIndexedPrimitive(
								D3DPT_TRIANGLELIST,		// Draw triangles
								0,						// Start at vertex 0
								0,						// Minimum index
								static_cast<UINT>(m_vertices.getPositions().size()),		// Nr of vertices
								0,						// Start at index 0
								m_triangleCount);       // Triangles to draw
	
	if (p_renderContext.pass != RenderPass_ShadowVolumes)
	{
		if (resetBackFaceCulling)
		{
			renderer->setCullingEnabled(true);
		}
		renderer->setLighting(oldLightSetting);
	}
}


void SubModel::renderNormals()
{
	if (m_vertices.getNormals().size() == m_vertices.getPositions().size())
	{
		Renderer* renderer = Renderer::getInstance();
		const bool oldLightSetting(renderer->isLightingEnabled());
		renderer->setLighting(false);
	
		for(std::size_t i = 0; i < m_vertices.getPositions().size(); ++i)
		{
			renderer->getDebug()->renderLine(
				ColorRGB::white,
				m_vertices.getPositions().at(i),
				m_vertices.getPositions().at(i) + m_vertices.getNormals().at(i));
		}
		renderer->getDebug()->flush();
		renderer->setLighting(oldLightSetting);
	}
}


void SubModel::deviceCreated()
{
	updateBuffers();
}


void SubModel::deviceDestroyed()
{
	// Release DirectX resources
	safeRelease(m_vertexBuffer);
	safeRelease(m_indexBuffer);
}

// Namespace end
}
}
}
