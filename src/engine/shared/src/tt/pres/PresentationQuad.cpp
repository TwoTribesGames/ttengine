#include <tt/pres/PresentationQuad.h>
#include <tt/engine/renderer/MatrixStack.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/engine/renderer/VertexBuffer.h>

namespace tt {
namespace pres {

//--------------------------------------------------------------------------------------------------
// Public member functions

PresentationQuad::PresentationQuad(const engine::renderer::TexturePtr& p_texture,
                                   const engine::renderer::ColorRGBA&  p_quadColor, 
                                   const math::Vector3&                p_translation)
:
m_width (0),
m_height(0),
m_radius(0),
m_translation(p_translation),
m_scale(1.0f, 1.0f),
m_rotation(0.0f),
m_quadScale(1.0f, 1.0f),
m_quad(engine::renderer::VertexBuffer::Property_Diffuse | engine::renderer::VertexBuffer::Property_Texture0,
       p_quadColor),
m_texture(),
m_lastSetColor(p_quadColor),
m_originalColor(p_quadColor),
m_blendMode(engine::renderer::BlendMode_Blend),
m_quadNeedsUpdate(true)
{
	setTexture(p_texture);
}


void PresentationQuad::render() const
{
	renderWithTexture(m_texture);
}


void PresentationQuad::renderWithTexture(const engine::renderer::TexturePtr& p_texture) const
{
	if (m_lastSetColor.a == 0)
	{
		// Early exit if quad is fully transparent
		return;
	}
	
	if(m_quadNeedsUpdate)
	{
		m_quad.update();
		m_quadNeedsUpdate = false;
	}
	
	engine::renderer::Renderer* renderer(engine::renderer::Renderer::getInstance());
	
	renderer->setTexture(p_texture);
	renderer->setBlendMode(m_blendMode);
	
	engine::renderer::MatrixStack* stack(engine::renderer::MatrixStack::getInstance());
	
	stack->setMode(engine::renderer::MatrixStack::Mode_Position);
	stack->push();
	stack->translate(m_translation);
	stack->rotateZ(m_rotation);
	stack->scale(math::Vector3(m_quadScale.x * m_scale.x, m_quadScale.y * m_scale.y, 1.0f));
	
	m_quad.render();
	
	stack->pop();
}


void PresentationQuad::setColor(const engine::renderer::ColorRGBA& p_color)
{
	if (p_color != m_lastSetColor)
	{
		engine::renderer::ColorRGBA newColor(p_color);
		
		if(m_texture != 0 && m_texture->isPremultiplied())
		{
			newColor.premultiply();
		}
		
		m_quad.setColor(newColor);
		m_quadNeedsUpdate = true;
		
		m_lastSetColor = p_color;
	}
}


void PresentationQuad::setWidth(real p_width)
{
	m_width = p_width;
	m_radius = math::sqrt(m_width*m_width + m_height*m_height) * 0.5f;
	m_quadScale.x = m_width  / (engine::renderer::Quad2D::quadSize * 2);
}


void PresentationQuad::setHeight(real p_height)
{
	m_height = p_height;
	m_radius = math::sqrt(m_width*m_width + m_height*m_height) * 0.5f;
	m_quadScale.y = m_height / (engine::renderer::Quad2D::quadSize * 2);
}


void PresentationQuad::setTexture(const engine::renderer::TexturePtr& p_texture)
{
	if (m_texture == p_texture) return;
	
	m_texture = p_texture;
	
	setWidth (static_cast<real>(p_texture->getWidth ()));
	setHeight(static_cast<real>(p_texture->getHeight()));
	
	m_quad.updateTexcoords(p_texture);
	
	if (m_texture->isPremultiplied())
	{
		// Set color
		engine::renderer::ColorRGBA newColor(m_originalColor);
		newColor.premultiply();
		m_quad.setColor(newColor);
	}
	
	m_quadNeedsUpdate = true;
}


void PresentationQuad::setTexcoords(const math::Vector2& p_topLeft, const math::Vector2& p_bottomRight)
{
	using tt::engine::renderer::Quad2D;
	m_quad.setTexcoord(Quad2D::Vertex_TopLeft,     p_topLeft);
	m_quad.setTexcoord(Quad2D::Vertex_TopRight,    math::Vector2(p_bottomRight.x, p_topLeft.y));
	m_quad.setTexcoord(Quad2D::Vertex_BottomLeft,  math::Vector2(p_topLeft.x,     p_bottomRight.y));
	m_quad.setTexcoord(Quad2D::Vertex_BottomRight, p_bottomRight);

	m_quadNeedsUpdate = true;
}


//namespace end
}
}
