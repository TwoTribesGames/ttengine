#include <tt/engine/renderer/ColorRGBA.h>
#include <tt/engine/renderer/MatrixStack.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/engine/renderer/Texture.h>
#include <tt/engine/renderer/TrianglestripBuffer.h>
#include <tt/fs/fs.h>
#include <tt/math/Vector3.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>


#include <toki/game/light/Circle.h>

namespace toki {
namespace game {
namespace light {

	
//--------------------------------------------------------------------------------------------------
// Public member functions


Circle::Circle(const tt::math::Vector2& p_centerPos,
               real p_radius, 
               const tt::engine::renderer::ColorRGB& p_color)
:
m_centerPos(p_centerPos),
m_radius(p_radius),
m_color(p_color),
m_geometryReady(false)
{
	using namespace tt::engine::renderer;
	
	m_trianglestripBuffer.reset(new TrianglestripBuffer(primitiveCount,
														1,
														TexturePtr(),
														BatchFlagTrianglestrip_UseVertexColor,
														tt::engine::renderer::TrianglestripBuffer::PrimitiveType_TriangleFan));
	
	BufferVtxUV<1> defaultValue;
	defaultValue.setColor(m_color);
	m_trianglestripBuffer->resize<1>(primitiveCount, defaultValue);
}


void Circle::update(real /*p_elapsedTime*/)
{
	
}


void Circle::render()
{
	if(m_geometryReady == false)
	{
		calculateCircle();
	}
	m_trianglestripBuffer->render();
}


// -------------------------------------------------------------------------------------------------
// Private functions


void Circle::calculateCircle()
{
	using namespace tt::engine::renderer;
	
	{
		BufferVtxUV<1>& vtx = m_trianglestripBuffer->modifyVtx<1>(0);
		vtx.setPosition(m_centerPos.x, m_centerPos.y, 0.0f);
		vtx.setColor(ColorRGBA(m_color, 48));
		
		/*
		const tt::math::Vector3&               pos   = vtx.getPosition();
		const tt::engine::renderer::ColorRGBA& color = vtx.getColor();
		TT_Printf("Circle::calculateCircle - center pos x: %f, y: %f, z: %f, color r:%u, g:%u, b:%u, a:%u\n", 
		          pos.x, pos.y, pos.z, color.r, color.g, color.b, color.a);
		// */
	}
	
	const real subdivisionAngle = tt::math::twoPi / segments;
	
	for (s32 i = 1; i < primitiveCount; ++i)
	{
		const real angle = subdivisionAngle * (i - 1); // Replace with addition in loop
		BufferVtxUV<1>& vtx = m_trianglestripBuffer->modifyVtx<1>(i);
		vtx.setPosition(m_radius * tt::math::cos(angle) + m_centerPos.x,
		                m_radius * tt::math::sin(angle) + m_centerPos.y,
		                0.0f);
		vtx.setColor(ColorRGBA(m_color, 32));
		
		/*
		const tt::math::Vector3&               pos   = vtx.getPosition();
		const tt::engine::renderer::ColorRGBA& color = vtx.getColor();
		TT_Printf("Circle::calculateCircle - pos x: %f, y: %f, z: %f, color r:%u, g:%u, b:%u, a:%u\n", 
		          pos.x, pos.y, pos.z, color.r, color.g, color.b, color.a);
		// */
	}
	
	m_trianglestripBuffer->applyChanges();
	m_geometryReady = true;
}


// Namespace end
}
}
}
