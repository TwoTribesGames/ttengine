#include <tt/engine/renderer/ColorRGBA.h>
#include <tt/engine/renderer/MatrixStack.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/engine/renderer/Texture.h>
#include <tt/engine/renderer/TrianglestripBuffer.h>
#include <tt/fs/fs.h>
#include <tt/math/Vector3.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>


#include <toki/game/light/LightTriangle.h>
#include <toki/game/light/LightShape.h>
#include <toki/game/light/Polygon.h>


namespace toki {
namespace game {
namespace light {

	
//--------------------------------------------------------------------------------------------------
// Public member functions


LightTriangle::LightTriangle(const LightShape& p_parent,
							 const tt::math::Vector2& p_left,
							 const tt::math::Vector2& p_right)
:
m_parent(p_parent),
m_left(p_left),
m_right(p_right)
{
	using namespace tt::engine::renderer;
	
	m_trianglestripBuffer.reset(new TrianglestripBuffer(3,
														1,
														TexturePtr(),
														BatchFlagTrianglestrip_UseVertexColor,
														tt::engine::renderer::TrianglestripBuffer::PrimitiveType_TriangleFan));
	
	BufferVtxUV<1> defaultValue;
	defaultValue.setColor(tt::engine::renderer::ColorRGBA(0, 0, 0, 0));
	m_trianglestripBuffer->resize<1>(3, defaultValue);
	
	calculateCircle();
}


void LightTriangle::update(real /*p_elapsedTime*/, const Polygons& /*p_occluders*/)
{
	/*
	BufferVtxUV<1> graphicsVtx;
	typedef std::vector<BufferVtxUV<1> > Graphics;
	Graphics graphcis;
	graphicsVtx.setColor(m_color);
	graphicsVtx.setPosition(m_centerPos.x, m_centerPos.y, 0.0f);
	graphics.push_back(graphicsVtx);
	
	for (Polygons::const_iterator occluderIt = p_occluders.begin(); occluderIt != p_occluders.end(); ++occluderIt)
	{
		const Vertices& vertices = (*occluderIt)->getVertices();
		const Vertices& normals  = (*occluderIt)->getNormals();
		TT_ASSERT(vertices.size() == normals.size());
		
		{
			Vertices::const_iterator nmlIt = normals.begin();
			for (Vertices::const_iterator vtxIt = vertices.begin();
				 vtxIt != vertices.end() && nmlIt != normals.end(); ++vtxIt, ++nmlIt)
			{
				
				(*vtxIt);
				(*nmlIt);
			}
		}
	}
	*/
}


void LightTriangle::render()
{
	m_trianglestripBuffer->render();
}


//--------------------------------------------------------------------------------------------------
// Private functions


void LightTriangle::calculateCircle()
{
	using namespace tt::engine::renderer;
	
	const tt::math::Vector2& centerPos         = m_parent.getCenterPos();
	const tt::engine::renderer::ColorRGB color = m_parent.getColor(); 
	
	{
		BufferVtxUV<1>& vtx = m_trianglestripBuffer->modifyVtx<1>(0);
		vtx.setPosition(centerPos.x, centerPos.y, 0.0f);
		vtx.setColor(tt::engine::renderer::ColorRGBA(color,255));
	}
	
	tt::math::Vector2 left  = m_left.getNormalized()  * m_parent.getRadius();
	tt::math::Vector2 right = m_right.getNormalized() * m_parent.getRadius();
	
	{
		BufferVtxUV<1>& vtx = m_trianglestripBuffer->modifyVtx<1>(1);
		vtx.setPosition(centerPos.x + left.x, centerPos.y + left.y, 0.0f);
		vtx.setColor(tt::engine::renderer::ColorRGBA(color, 0));
	}
	
	{
		BufferVtxUV<1>& vtx = m_trianglestripBuffer->modifyVtx<1>(2);
		vtx.setPosition(centerPos.x + right.x, centerPos.y + right.y, 0.0f);
		vtx.setColor(tt::engine::renderer::ColorRGBA(color, 0));
	}
	
	m_trianglestripBuffer->applyChanges();
}


// Namespace end
}
}
}
