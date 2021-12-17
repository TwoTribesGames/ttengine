#include <tt/engine/debug/DebugStats.h>
#include <tt/engine/debug/DebugRenderer.h>
#include <tt/engine/opengl_headers.h>
#include <tt/engine/renderer/Material.h>
#include <tt/engine/renderer/MatrixStack.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/engine/renderer/GLStateCache.h>
#include <tt/engine/renderer/SubModel.h>
#include <tt/engine/renderer/Texture.h>
#include <tt/engine/renderer/TextureStageData.h>
#include <tt/engine/scene/Model.h>
#include <tt/mem/util.h>
//#include <tt/platform/tt_printf.h>


namespace tt {
namespace engine {
namespace renderer {

//--------------------------------------------------------------------------------------------------
// Public member functions

SubModel::SubModel()
:
m_material(),
m_model(0),
m_vertices(),
m_indices(),
m_triangleCount(0)
{
	// Clear OpenGl buffer values
	mem::zero32(m_buffers, maxBuffers * sizeof(GLuint));
}


SubModel::SubModel(const SubModel& p_model)
:
m_material(p_model.m_material),
m_model(0), // Need to update this one after copying!
m_vertices(p_model.m_vertices),
m_indices(p_model.m_indices),
m_triangleCount(p_model.m_triangleCount)
{
	// Clear OpenGL buffer values
	mem::zero32(m_buffers, maxBuffers * sizeof(GLuint));
	
	// Create own buffers
	updateBuffers();
}


SubModel::~SubModel()
{
	destroyBuffers();
}


SubModel& SubModel::operator=(const SubModel& p_rhs)
{
	if (&p_rhs != this)
	{
		m_material      = p_rhs.m_material;
		m_model         = p_rhs.m_model;
		m_vertices      = p_rhs.m_vertices;
		m_indices       = p_rhs.m_indices;
		m_triangleCount = p_rhs.m_triangleCount;
		
		// Destroy old buffers and recreate them
		destroyBuffers();
		updateBuffers();
	}
	
	return *this;
}


bool SubModel::load(const fs::FilePtr& p_file, scene::Model* p_model)
{
	// Store pointer to the model
	m_model = p_model;
	TT_NULL_ASSERT(m_model);
	
	// Now we can load our information
	EngineID matID(0, 0);
	if (matID.load(p_file) == false)
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
	
	// NOTE: This should be removed, unless we're going to pack data for OSX
	s32 packedData = 0;
	if (p_file->read(&packedData, sizeof(packedData)) != sizeof(packedData))
	{
		return false;
	}
	
	// Load the material
	if (matID.valid())
	{
		m_material = MaterialCache::get(matID, false);
		
		if (m_material != 0 && m_material->checkFlag(Material::Flag_Transparent))
		{
			p_model->setFlag(scene::SceneObject::Flag_Transparent);
		}
	}
	
	if (m_material == 0)
	{
		// Get default material
		m_material.reset(new Material(ColorRGBA(255, 0, 255, 255)));
	}
	
	if (p_model->checkFlag(scene::SceneObject::Flag_DisableLighting))
	{
		m_material->setFlag(Material::Flag_DisableLighting);
	}
	
	if (packedData != 0)
	{
		TT_PANIC("Packed data not supported on OS X.");
	}
	else if (vertexCount > 0)
	{
		// Here we load a vertex buffer instead of an array of vertices
		if (m_vertices.load(p_file, vertexCount) == false)
		{
			return false;
		}
		
		// Load the indices, we can still read the whole buffer at once
		m_indices.resize(m_triangleCount * 3);
		
		// NOTE: If I remember correctly this was a RVL limit, does not aplly to OS X / Windows
		//       I might be wrong, but I think the assert is wrong. The limitation is that the index
		//       is a u16 so at most 2^16 unique vertices can be indexed, so the vertex buffer is limited
		//       to 2^16 entries. The index list however can be larger, because of vertex re-use.
		//       Anyway, the best location to check is during data export, making sure that each index
		//       fits in a u16.
		
		/*TT_ASSERTMSG(m_indices.size() < std::numeric_limits<u16>::max(), 
		             "Too many vertices in submodel of model '%s'",
		             m_model->getEngineID().toString().c_str());*/
		
		s32 indexSize = static_cast<s32>(sizeof(u16) * m_indices.size());
		if (p_file->read(&m_indices[0], indexSize) != indexSize)
		{
			return false;
		}
	}
	
	// Create OpenGL buffers
	return updateBuffers();
}


bool SubModel::updateBuffers()
{
	// Cannot create buffer if no model data is available
	if (m_vertices.getPositions().empty())
	{
		return false;
	}
	
	// Only generate once
	if (m_buffers[0] == 0)
	{
		glGenBuffers(maxBuffers, m_buffers);
	}
	
	// Create VBO data
	glBindBuffer(GL_ARRAY_BUFFER, m_buffers[0]);
	glBufferData(GL_ARRAY_BUFFER,
				 m_vertices.getPositions().size() * sizeof(VertexBuffer::PositionBuffer::value_type),
				 &m_vertices.getPositions()[0], GL_STATIC_DRAW);
	
	if(m_vertices.hasProperty(VertexBuffer::Property_Normal))
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_buffers[1]);
		glBufferData(GL_ARRAY_BUFFER,
					 m_vertices.getNormals().size() * sizeof(VertexBuffer::NormalBuffer::value_type),
					 &m_vertices.getNormals()[0], GL_STATIC_DRAW);
	}
	
	if(m_vertices.hasProperty(VertexBuffer::Property_Diffuse))
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_buffers[2]);
		glBufferData(GL_ARRAY_BUFFER,
					 m_vertices.getColors().size() * sizeof(VertexBuffer::ColorBuffer::value_type),
					 &m_vertices.getColors()[0], GL_STATIC_DRAW);
	}
	
	for(s32 i = 0; i < 4; ++i)
	{
		if(m_vertices.hasProperty(static_cast<VertexBuffer::Property>(VertexBuffer::Property_Texture0 << i)))
		{
			glBindBuffer(GL_ARRAY_BUFFER, m_buffers[3+i]);
			glBufferData(GL_ARRAY_BUFFER,
						 m_vertices.getTexCoords(i).size() * sizeof(VertexBuffer::TexCoordBuffer::value_type),
						 &m_vertices.getTexCoords(i)[0], GL_STATIC_DRAW);
		}
	}
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_buffers[maxBuffers-1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(IndexBuffer::value_type), &m_indices[0], GL_STATIC_DRAW);
	
	// Restore state
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	
	TT_CHECK_OPENGL_ERROR();
	
	return true;
}


void SubModel::render(RenderContext& p_renderContext)
{
	// DO NOT RENDER SHADOW VOLUMES ON OS X
	// FIXME: Find way to implement Nitro-style shadow volumes using OpenGL?
	if (m_model != 0 && m_model->getType() == scene::SceneObject::Type_ShadowModel)
	{
		return;
	}
	
	Renderer* renderer = Renderer::getInstance();
	const bool oldLightSetting   = renderer->isLightingEnabled();
	const bool oldCullingSetting = renderer->isCullingEnabled();
	
	if (m_material != 0 && p_renderContext.pass != RenderPass_ShadowVolumes)
	{
		m_material->select(p_renderContext);
		
		// Double sided?
		renderer->setCullingEnabled(m_material->checkFlag(Material::Flag_DoubleSided) == false);
	}
	
	// Increase the polygons rendered count
	debug::DebugStats::addToPolygonsRendered(m_triangleCount);
	
	// Set up the correct format
	renderer->setVertexType(m_vertices.getVertexType());
	
	// Overwrite setVertexType settings
	if(m_material != 0 && p_renderContext.pass != RenderPass_ShadowVolumes &&
	   m_material->checkFlag(Material::Flag_DisableLighting))
	{
		renderer->setLighting(false);
	}
	
	// Update the world matrix
	MatrixStack::getInstance()->updateWorldMatrix();
	
	// Set the various vertex input arrays
	glBindBuffer(GL_ARRAY_BUFFER, m_buffers[0]);
	glVertexPointer(3, GL_FLOAT, 0, 0);
	
	if (m_vertices.hasProperty(VertexBuffer::Property_Normal))
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_buffers[1]);
		glNormalPointer(GL_FLOAT, 0, 0);
		
		// If lighting + normal then make the textureblend mode BlendOp_Modulate2X.
		// (WIN does this in setVertexType OSX needs to do it per material)
		// This is done to emulate Wii X2 Lighting.
		if (m_material != 0 && renderer->isLightingEnabled())
		{
			glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE, 2.0f);
		}
	}
	
	if (m_vertices.hasProperty(VertexBuffer::Property_Diffuse))
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_buffers[2]);
		glColorPointer(4, GL_UNSIGNED_BYTE, 0, 0);
	}
	
	// NOTE: In OpenGL Texture Coordinate sharing is set up in the texture coordinate streams
	const s32 textureStageCount = m_material->getTextureStageCount();
	for (s32 stage = 0; stage < textureStageCount; ++stage)
	{
		Texture::setActiveClientChannel(stage);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		
		const s32 texcoordIndex = m_material->getTextureStage(stage).getTexCoordIndex();
		TT_ASSERTMSG(m_vertices.hasProperty(static_cast<VertexBuffer::Property>(VertexBuffer::Property_Texture0 << texcoordIndex)),
					 "Material references texture coordinate index %d but SubModel doesn't have this channel", texcoordIndex);
		
		glBindBuffer(GL_ARRAY_BUFFER, m_buffers[3 + texcoordIndex]);
		glTexCoordPointer(2, GL_FLOAT, 0, 0);
	}
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_buffers[maxBuffers-1]);

	renderer->stateCache()->apply();

	glDrawElements(GL_TRIANGLES, m_triangleCount * 3, GL_UNSIGNED_SHORT, 0);
	//*/
	
	TT_CHECK_OPENGL_ERROR();
	
	if (p_renderContext.pass != RenderPass_ShadowVolumes)
	{
		renderer->setCullingEnabled(oldCullingSetting);
		renderer->setLighting(oldLightSetting);
	}
	
	// Restore States
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE, 1.0f);
	for (s32 i = 1; i < textureStageCount; ++i)
	{
		Texture::setActiveClientChannel(i);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	}
	Texture::setActiveClientChannel(0);
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


//--------------------------------------------------------------------------------------------------
// Private member functions

void SubModel::destroyBuffers()
{
	if (m_buffers[0] != 0)
	{
		glDeleteBuffers(maxBuffers, m_buffers);
		mem::zero32(m_buffers, maxBuffers * sizeof(GLuint));
	}
}

// Namespace end
}
}
}
