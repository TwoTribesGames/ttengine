#include <tt/engine/renderer/ColorRGBA.h>
#include <tt/engine/renderer/MatrixStack.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/engine/renderer/Texture.h>
#include <tt/engine/renderer/TrianglestripBuffer.h>
#include <tt/fs/fs.h>
#include <tt/math/Vector3.h>
#include <tt/math/Vector2.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>


#include <toki/game/light/Polygon.h>


namespace toki {
namespace game {
namespace light {


//--------------------------------------------------------------------------------------------------
// Public member functions


Polygon::Polygon(const tt::math::Vector2& p_pos,
                 const tt::engine::renderer::ColorRGBA& p_color,
                 const Vertices& p_vertices,
                 bool p_renderLines)
:
m_pos(p_pos),
m_color(p_color),
m_vertices(p_vertices),
m_geometryReady(false)
#ifndef TT_BUILD_FINAL
,
m_debug(p_pos, 5.0f, tt::engine::renderer::ColorRGB(p_color.r, p_color.g, p_color.b))
#endif
{
	using namespace tt::engine::renderer;
	
	const s32 vtxCount = static_cast<s32>(m_vertices.size() + ((p_renderLines) ? 1 : 0));
	m_trianglestripBuffer.reset(new TrianglestripBuffer(vtxCount,
	                                                    1,
	                                                    TexturePtr(),
	                                                    BatchFlagTrianglestrip_UseVertexColor,
	                                                    (p_renderLines) ?
	                                                    TrianglestripBuffer::PrimitiveType_LineStrip :
	                                                    TrianglestripBuffer::PrimitiveType_TriangleFan));

	BufferVtxUV<1> defaultValue;
	defaultValue.setColor(ColorRGBA(0, 0, 0, 0));
	m_trianglestripBuffer->resize<1>(vtxCount, defaultValue);
	
	calculateVertices();
}


void Polygon::update(real /*p_elapsedTime*/)
{
}


void Polygon::render()
{
	if(m_geometryReady == false)
	{
		prepareGeometry();
	}
	
	m_trianglestripBuffer->render();
}


// -------------------------------------------------------------------------------------------------
// Private functions


void Polygon::calculateVertices()
{
	m_normals.resize(m_vertices.size());
	m_verticesWorldSpace = m_vertices;
	
	if (m_vertices.size() < 3)
	{
		TT_PANIC("Not enough vertices %d! Need >= 3!\n", m_vertices.size());
		return;
	}	
	
	tt::math::Vector2 previousVtx = m_vertices.back();
	
	tt::math::Vector2 min = (m_vertices.empty()) ? tt::math::Vector2::zero : m_vertices.front();
	tt::math::Vector2 max = (m_vertices.empty()) ? tt::math::Vector2::zero : m_vertices.front();
	
	s32 vtxIdx = 0;
	for (Vertices::iterator it = m_vertices.begin(); it != m_vertices.end(); ++it, ++vtxIdx)
	{
		tt::math::Vector2& vtxWS = m_verticesWorldSpace[vtxIdx];
		vtxWS.x += m_pos.x;
		vtxWS.y += m_pos.y;
		
		tt::math::Vector2& normal = m_normals[vtxIdx];
		normal.x =  ((*it).y - previousVtx.y);
		normal.y = -((*it).x - previousVtx.x);
		//normal.normalize();
		previousVtx =  (*it);
		
		if ((*it).x < min.x)
		{
			min.x = (*it).x;
		}
		if ((*it).x > max.x)
		{
			max.x = (*it).x;
		}
		if ((*it).y < min.y)
		{
			min.y = (*it).y;
		}
		if ((*it).y > max.y)
		{
			max.y = (*it).y;
		}
		
		/*
		TT_Printf("Polygon::calculateVertices - idx: %d, vtxWS x: %f, y: %f, normal x: %f, y: %f\n", 
		          vtxIdx, vtxWS.x, vtxWS.y, normal.x, normal.y);
		// */
	}
	
	m_boundingSphereMidPoint = (max + min) / 2.0f;
	m_boundingSphereRadius   = 0.0f;
	
	for (Vertices::iterator it = m_vertices.begin(); it != m_vertices.end(); ++it, ++vtxIdx)
	{
		tt::math::Vector2 diff(m_boundingSphereMidPoint - (*it));
		real lengthSquared = diff.lengthSquared();
		if (lengthSquared > m_boundingSphereRadius)
		{
			m_boundingSphereRadius = lengthSquared;
		}
	}
	m_boundingSphereRadius = tt::math::sqrt(m_boundingSphereRadius);
	
	/*
	TT_Printf("Polygon::calculateVertices - m_boundingSphereMidPoint x: %f, y: %f, radius: %f, min x: %f, y: %f, max x: %f, y: %f\n",
	          m_boundingSphereMidPoint.x, m_boundingSphereMidPoint.y, m_boundingSphereRadius, min.x, min.y, max.x, max.y);
	// */
}


void Polygon::prepareGeometry()
{
	TT_MIN_ASSERT(m_trianglestripBuffer->getTotalVerticesCount(), static_cast<s32>(m_vertices.size()));
	using namespace tt::engine::renderer;
	
	s32 vtxIdx = 0;
	for (Vertices::iterator it = m_verticesWorldSpace.begin(); it != m_verticesWorldSpace.end(); ++it, ++vtxIdx)
	{
		const tt::math::Vector2& vtxWS = m_verticesWorldSpace[vtxIdx];
		
		BufferVtxUV<1>& vtx = m_trianglestripBuffer->modifyVtx<1>(vtxIdx);
		vtx.setPosition(vtxWS.x, vtxWS.y, 0.0f);
		vtx.setColor(m_color);
		
		/*
		const tt::math::Vector3&               pos   = vtx.getPosition();
		const tt::engine::renderer::ColorRGBA& color = vtx.getColor();
		TT_Printf("Polygon::calculateVertices - idx: %d, pos x: %f, y: %f, z: %f, color r:%u, g:%u, b:%u, a:%u\n", 
		          vtxIdx, pos.x, pos.y, pos.z, color.r, color.g, color.b, color.a);
		// */
	}
	
	// Add extra vertex for LineStrip.
	if (m_verticesWorldSpace.empty() == false &&
	    m_trianglestripBuffer->getPrimitiveType() ==
	    tt::engine::renderer::TrianglestripBuffer::PrimitiveType_LineStrip)
	{
		BufferVtxUV<1>& vtx = m_trianglestripBuffer->modifyVtx<1>(vtxIdx);
		
		const tt::math::Vector2& vtxWS = m_verticesWorldSpace[0];
		vtx.setPosition(vtxWS.x, vtxWS.y, 0.0f);
		vtx.setColor(m_color);
	}
	
	m_trianglestripBuffer->applyChanges();
	m_geometryReady = true;
	
#ifndef TT_BUILD_FINAL
	m_debug.setCenterPos(getBoundingSphereMidPoint());
	m_debug.setRadius(m_boundingSphereRadius);
#endif
}

// Namespace end
}
}
}
