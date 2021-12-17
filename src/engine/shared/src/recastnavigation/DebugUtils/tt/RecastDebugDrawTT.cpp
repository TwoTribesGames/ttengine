#include <recastnavigation/DebugUtils/tt/RecastDebugDrawTT.h>
#include <tt/engine/renderer/Renderer.h>


// Use these for the whole file
using namespace tt::engine::renderer;


RecastDebugDrawTT::RecastDebugDrawTT()
:
// Save renderer state
m_zBufferWasEnabled(Renderer::getInstance()->isZBufferEnabled()),
m_cullingWasEnabled(Renderer::getInstance()->isCullingEnabled()),
m_vertexBuffer(TrianglestripBuffer::maxVtxBufSize, 1, TexturePtr(), BatchFlagTrianglestrip_UseVertexColor),
m_activePrimitiveType(DU_DRAW_TRIS),
m_vtxIdxInPrim(0)
{
	Renderer* renderer = Renderer::getInstance();
	renderer->setZBufferEnabled(true);
	renderer->setCullingEnabled(true);
	//renderer->setCullFrontOrder(CullFrontOrder_CounterClockWise);
}


RecastDebugDrawTT::~RecastDebugDrawTT()
{
	// Restore renderer state.
	Renderer* renderer = Renderer::getInstance();
	renderer->setZBufferEnabled(m_zBufferWasEnabled);
	renderer->setCullingEnabled(m_cullingWasEnabled);
	//renderer->setCullFrontOrder(CullFrontOrder_ClockWise);
}



void RecastDebugDrawTT::depthMask(bool state)
{
	Renderer* renderer = Renderer::getInstance();
	renderer->setZBufferEnabled(state);
	
	// OpenGL reference: glDepthMask(state ? GL_TRUE : GL_FALSE);
}


void RecastDebugDrawTT::texture(bool /*state*/)
{
	// TODO: Turn some texture on or off based on state. (In the recast demo it has a checkerboard pattern.)
}


void RecastDebugDrawTT::begin(duDebugDrawPrimitives prim, float /*size*/)
{
	m_activePrimitiveType = prim;
	m_vtxIdxInPrim = 0;
	
	switch (prim)
	{
		case DU_DRAW_POINTS:
			// FIXME: Implement this
			break;
		case DU_DRAW_LINES:
			m_vertexBuffer.setPrimitiveType(TrianglestripBuffer::PrimitiveType_Lines);
			break;
		case DU_DRAW_TRIS:
			m_vertexBuffer.setPrimitiveType(TrianglestripBuffer::PrimitiveType_Triangles);
			break;
		case DU_DRAW_QUADS:
			m_vertexBuffer.setPrimitiveType(TrianglestripBuffer::PrimitiveType_Triangles); // note: We change the incoming quad vtx to triangles.
			break;
		default:
			// Panic here?
			break;
	};
	
	/* // OpenGL reference:
	switch (prim)
	{
		case DU_DRAW_POINTS:
			glPointSize(size);
			glBegin(GL_POINTS);
			break;
		case DU_DRAW_LINES:
			glLineWidth(size);
			glBegin(GL_LINES);
			break;
		case DU_DRAW_TRIS:
			glBegin(GL_TRIANGLES);
			break;
		case DU_DRAW_QUADS:
			glBegin(GL_QUADS);
			break;
	};
	*/
}


void RecastDebugDrawTT::vertex(const float* pos, unsigned int color)
{
	vertex(pos[0], pos[1], pos[2], color);
	
	/* // OpenGL reference:
	glColor4ubv((GLubyte*)&color);
	glVertex3fv(pos);
	*/
}


void RecastDebugDrawTT::vertex(const float x, const float y, const float z, unsigned int color)
{
	if (m_activePrimitiveType == DU_DRAW_POINTS)
	{
		return;
	}
	
	const u8 r = ((color      ) & 0xFF);
	const u8 g = ((color >>  8) & 0xFF);
	const u8 b = ((color >> 16) & 0xFF);
	const u8 a = ((color >> 24) & 0xFF);
	
	m_vertexCollection.push_back(BufferVtx(tt::math::Vector3(x, z, y * 0.1f), // Translate position coords to TT (actually Toki Tori 2) space. (Also flatten z (y in recast) so our navmesh doesn't hover above tile layers.)
	                                                                          // note: (This might break the color for voxels, etc. Maybe I should keep it in recast space and mvoe the camera.)
	                                       ColorRGBA(r,g,b,a),                // Translate color
	                                       tt::math::Vector2::zero));         // Unused UV coords.
	
	if (m_activePrimitiveType == DU_DRAW_QUADS)
	{
		// We don't support quads with TrianglestripBuffer so add an extra triangle to make it a quad.
		++m_vtxIdxInPrim;
		if (m_vtxIdxInPrim == 4)
		{
			TT_ASSERT(m_vertexCollection.size() >= 4);
			TrianglestripVertices::size_type lastVtx = m_vertexCollection.size() - 1;
			m_vertexCollection.push_back(m_vertexCollection[lastVtx - 3]);
			m_vertexCollection.push_back(m_vertexCollection[lastVtx - 1]);
			
			m_vtxIdxInPrim = 0;
		}
	}
	
	/* // OpenGL reference:
	glColor4ubv((GLubyte*)&color);
	glVertex3f(x,y,z);
	*/
}


void RecastDebugDrawTT::vertex(const float* pos, unsigned int color, const float* /*uv*/)
{
	vertex(pos, color); // FIXME: Use uv when texture supported is added.
	
	/* // OpenGL reference:
	glColor4ubv((GLubyte*)&color);
	glTexCoord2fv(uv);
	glVertex3fv(pos);
	*/
}


void RecastDebugDrawTT::vertex(const float x, const float y, const float z, unsigned int color, const float /*u*/, const float /*v*/)
{
	vertex(x, y, z, color); // FIXME: Use uv when texture supported is added.
	
	/* // OpenGL reference:
	glColor4ubv((GLubyte*)&color);
	glTexCoord2f(u,v);
	glVertex3f(x,y,z);
	*/
}


void RecastDebugDrawTT::end()
{
	if (m_vertexCollection.empty() == false)
	{
		/* // Debug code to render only some triangles.
		tt::engine::renderer::TrianglestripVertices subsection;
		const s32 startTri = 0;
		const s32 triCount = 1;
		const s32 endTri = std::min(startTri + triCount, static_cast<s32>(8));
		for (s32 triangle = startTri; triangle < endTri; ++triangle)
		{
			const s32 vtxIndex = triangle * 3;
			subsection.push_back(m_vertexCollection[vtxIndex + 0]);
			subsection.push_back(m_vertexCollection[vtxIndex + 1]);
			subsection.push_back(m_vertexCollection[vtxIndex + 2]);
		}
		std::swap(subsection, m_vertexCollection);
		// */
		
		m_vertexBuffer.clear(); // Remove old vertex data before adding new collision.
		m_vertexBuffer.setCollection(m_vertexCollection);
		m_vertexCollection.clear();
		m_vertexBuffer.applyChanges();
		
		//Renderer::getInstance()->getActiveCamera();
		m_vertexBuffer.render();
	}
	
	/* // OpenGL reference:
	glEnd();
	glLineWidth(1.0f);
	glPointSize(1.0f);
	*/
}

