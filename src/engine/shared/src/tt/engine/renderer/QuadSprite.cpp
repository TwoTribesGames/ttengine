#include <tt/engine/renderer/QuadSprite.h>
#include <tt/engine/renderer/Texture.h>
#include <tt/engine/renderer/Material.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/engine/renderer/MatrixStack.h>
#include <tt/engine/renderer/VertexBuffer.h>
#include <tt/engine/scene/Camera.h>
#include <tt/platform/tt_error.h>
#include <tt/math/math.h>


namespace tt {
namespace engine {
namespace renderer {


QuadSprite::QuadSprite(real p_width, real p_height, u32 p_vertexType)
:
m_transform(),
m_position(math::Vector3::zero),
m_offset(math::Vector3::zero),
m_rotation(0.0f),
m_scale(1.0f),
m_width(p_width   / (Quad2D::quadSize * 2)),
m_height(p_height / (Quad2D::quadSize * 2)),
m_material(new Material(EngineID(0, 0), Material::Flag_DisableLighting)),
m_flags(Flag_Visible | Flag_NeedUpdate | Flag_NeedQuadUpdate),
m_maxAlpha(255),
m_fadeStart(0.0f),
m_fadeEnd(0.0f),
m_quad(p_vertexType)
{
}


QuadSprite::QuadSprite(const TexturePtr& p_texture, u32 p_vertexType, const ColorRGBA& p_color)
:
m_transform(),
m_position(math::Vector3::zero),
m_offset(math::Vector3::zero),
m_rotation(0.0f),
m_scale(1.0f),
m_width(0.0f),
m_height(0.0f),
m_material(new Material(EngineID(0, 0), Material::Flag_DisableLighting)),
m_flags(Flag_Visible | Flag_NeedUpdate | Flag_NeedQuadUpdate),
m_maxAlpha(255),
m_fadeStart(0.0f),
m_fadeEnd(0.0f),
m_color(p_color),
m_quad(p_vertexType, p_color)
{
	m_material->setTexture(p_texture);
	
	if(p_texture != 0)
	{
		m_width  = static_cast<real>(p_texture->getWidth())  / (Quad2D::quadSize * 2);
		m_height = static_cast<real>(p_texture->getHeight()) / (Quad2D::quadSize * 2);
		
		m_quad.updateTexcoords(p_texture);

		if(p_texture->isPremultiplied() && (p_vertexType & VertexBuffer::Property_Diffuse) != 0)
		{
			m_flags |= Flag_Premultiply;
			ColorRGBA premultipliedColor(m_color);
			premultipliedColor.premultiply();
			m_quad.setColor(premultipliedColor);
		}
	}
}


// FIXME: This explicit implementation is probably unnecessary (just use the compiler-generated one)
QuadSprite::QuadSprite(const QuadSprite& p_rhs)
:
m_transform(p_rhs.m_transform),
m_position(p_rhs.m_position),
m_offset(p_rhs.m_offset),
m_rotation(p_rhs.m_rotation),
m_scale(p_rhs.m_scale),
m_width(p_rhs.m_width),
m_height(p_rhs.m_height),
m_material(p_rhs.m_material),
m_flags(p_rhs.m_flags),
m_maxAlpha(p_rhs.m_maxAlpha),
m_fadeStart(p_rhs.m_fadeStart),
m_fadeEnd(p_rhs.m_fadeEnd),
m_color(p_rhs.m_color),
m_quad(p_rhs.m_quad)
{
}


void QuadSprite::setTexture(const TexturePtr& p_texture)
{
	m_material->setTexture(p_texture);

	if (p_texture != 0)
	{
		m_quad.updateTexcoords(p_texture);
		updateTextureCoordsForFlip();

		p_texture->isPremultiplied() ? setFlag(Flag_Premultiply) : resetFlag(Flag_Premultiply);
	}
}


const TexturePtr& QuadSprite::getTexture() const
{
	TT_NULL_ASSERT(m_material);
	return m_material->getTexture();
}


void QuadSprite::setColor(const ColorRGBA& p_color)
{
	if(checkFlag(Flag_Premultiply))
	{
		ColorRGBA color(p_color);
		color.premultiply();
		
		m_quad.setColor(color);
	}
	else
	{
		m_quad.setColor(p_color);
	}
	setFlag(Flag_NeedQuadUpdate);
	
	m_maxAlpha = p_color.a;
	m_color = p_color;
}


void QuadSprite::setColor(const ColorRGB& p_color)
{
	m_color = ColorRGBA(p_color, m_color.a);

	if(checkFlag(Flag_Premultiply))
	{
		ColorRGBA color(m_color);
		color.premultiply();
		m_quad.setColor(color);
	}
	else
	{
		m_quad.setColor(p_color);
	}
	
	setFlag(Flag_NeedQuadUpdate);
}


void QuadSprite::setOpacity(u8 p_opacity)
{
	m_color.a = p_opacity;

	if(checkFlag(Flag_Premultiply))
	{
		ColorRGBA color(m_color);
		color.premultiply();
		m_quad.setColor(color);
	}
	else
	{
		m_quad.setAlpha(p_opacity);
	}
	
	setFlag(Flag_NeedQuadUpdate);
	
	m_maxAlpha = p_opacity;
	
	if (p_opacity > 0)
	{
		setFlag(  Flag_Visible);
	}
	else
	{
		resetFlag(Flag_Visible);
	}
}


void QuadSprite::fadeIn(real p_time, u8 p_opacity)
{
	m_fadeStart = Renderer::getInstance()->getTime();
	m_fadeEnd   = m_fadeStart + p_time;
	m_maxAlpha  = p_opacity;
	
	resetFlag(Flag_FadingOut);
	setFlags(Flag_FadingIn|Flag_Visible);
}


void QuadSprite::fadeOut(real p_time)
{
	if (checkFlag(Flag_Visible) == false)
	{
		// Already invisible. No fading to be done.
		// Just make sure the FadingIn flag is off.
		resetFlag(Flag_FadingIn);
		return;
	}
	
	m_fadeStart = Renderer::getInstance()->getTime();
	m_fadeEnd   = m_fadeStart + p_time;
	
	resetFlag(Flag_FadingIn);
	setFlag(Flag_FadingOut);
}


void QuadSprite::setFlippedHorizontal(bool p_flip)
{
	if(isFlippedHorizontal() != p_flip)
	{
		if(p_flip)
		{
			m_flags |= Flag_FlipHorizontal;
		}
		else
		{
			m_flags &= ~Flag_FlipHorizontal;
		}
		
		updateTextureCoordsForFlip();
	}
}


void QuadSprite::setFlippedVertical(bool p_flip)
{
	if(isFlippedVertical() != p_flip)
	{
		if(p_flip)
		{
			m_flags |= Flag_FlipVertical;
		}
		else
		{
			m_flags &= ~Flag_FlipVertical;
		}
		
		updateTextureCoordsForFlip();
	}
}


bool QuadSprite::update()
{
	// Visibility check
	if (checkFlag(Flag_Visible) == false)
	{
		return false;
	}
	
	// Fading
	if(checkFlag(Flag_FadingIn) || checkFlag(Flag_FadingOut))
	{
		// Get engine time
		real time(Renderer::getInstance()->getTime());
		u8 alpha(0);
		
		// If still fading
		if(time < m_fadeEnd)
		{
			// Get position on fading the timeline
			real pos = (time - m_fadeStart) / (m_fadeEnd - m_fadeStart);
			
			// Compute alpha
			alpha = checkFlag(Flag_FadingIn) ?
				static_cast<u8>(pos * m_maxAlpha) :      // from 0 to 255
				static_cast<u8>((1 - pos) * m_maxAlpha); // from 255 to 0
		}
		else
		{
			// End of fade
			if(checkFlag(Flag_FadingIn))
			{
				alpha = m_maxAlpha;
				resetFlag(Flag_FadingIn);
			}
			else
			{
				alpha = 0;
				resetFlag(Flag_FadingOut);
				resetFlag(Flag_Visible);
			}
		}
		
		// Update quad colors
		if(checkFlag(Flag_Premultiply))
		{
			ColorRGBA color(m_color);
			color.a = alpha;
			color.premultiply();
			m_quad.setColor(color);
		}
		else
		{
			m_quad.setAlpha(alpha);
		}
		setFlag(Flag_NeedQuadUpdate);
	}
	
	// Only do this if needed
	if (checkFlag(Flag_NeedUpdate))
	{
		if (checkFlag(Flag_WorldSpace))
		{
			m_transform = math::Matrix44::getTranslation(m_position);
		}
		else
		{
			m_transform = math::Matrix44::getTranslation(m_position.x, -m_position.y, m_position.z);
		}
		m_transform.translate(m_offset);
		m_transform.rotateZ(m_rotation);
		m_transform.translate(-m_offset);
		m_transform.scale(m_scale * m_width, m_scale * m_height);
		
		resetFlag(Flag_NeedUpdate);
	}
	
	return true;
}


bool QuadSprite::render()
{
	// Visibility check
	if (checkFlag(Flag_Visible) == false || m_maxAlpha == 0)
	{
		return false;
	}

	if (checkFlag(Flag_NeedQuadUpdate))
	{
		m_quad.update();
		resetFlag(Flag_NeedQuadUpdate);
	}
	
	RenderContext rc;
	if(m_material != 0) m_material->select(rc);
	
	MatrixStack::getInstance()->push();
	MatrixStack::getInstance()->multiply44(m_transform);
	
	m_quad.render();
	
	MatrixStack::getInstance()->pop();
	
	return true;
}


void QuadSprite::setFrame(s32 p_frameWidth, s32 p_frameHeight)
{
	setWidth(static_cast<real>(p_frameWidth));
	setHeight(static_cast<real>(p_frameHeight));
	setFlag(Flag_NeedQuadUpdate);
	m_quad.updateTexcoords(m_material->getTexture(), p_frameWidth, p_frameHeight);
}


///////////////////////////////////
// Static creation functions

QuadSpritePtr QuadSprite::createQuad(real p_width, real p_height, u32 p_vertexType)
{
	return QuadSpritePtr(new QuadSprite(p_width, p_height, p_vertexType));
}


QuadSpritePtr QuadSprite::createQuad(const TexturePtr& p_texture, u32 p_vertexType)
{
	return QuadSpritePtr(new QuadSprite(p_texture, p_vertexType));
}


// FIXME: Need namespace
//QuadSpritePtr QuadSprite::createQuad(const std::string& p_filename, u32 p_vertexType)
//{
//	return QuadSpritePtr(new QuadSprite(TextureCache::get(p_filename), p_vertexType));
//}


QuadSpritePtr QuadSprite::createQuad(real p_width, real p_height, const ColorRGBA& p_color)
{
	QuadSpritePtr quad(new QuadSprite(p_width, p_height, VertexBuffer::Property_Diffuse));
	quad->setColor(p_color);
	
	return quad;
}


QuadSpritePtr QuadSprite::createQuad(const TexturePtr& p_texture, const ColorRGBA& p_color)
{
	return QuadSpritePtr(new QuadSprite(
		p_texture, (VertexBuffer::Property_Diffuse|VertexBuffer::Property_Texture0), p_color));
}


QuadSpritePtr QuadSprite::createQuad(const TexturePtr& p_texture)
{
	return QuadSpritePtr(new QuadSprite(p_texture, VertexBuffer::Property_Texture0));
}


void QuadSprite::updateTextureCoordsForFlip()
{
	real width = 1.0f;
	real height = 1.0f;
	
	using namespace tt::engine::renderer;
	
	real zero = 0.0f;
	m_quad.setTexcoord(Quad2D::Vertex_TopLeft, tt::math::Vector2(
		               isFlippedHorizontal() ? width : zero,
		               isFlippedVertical() ? height : zero));
	m_quad.setTexcoord(Quad2D::Vertex_TopRight, tt::math::Vector2(
		               isFlippedHorizontal() ? zero : width,
		               isFlippedVertical() ? height : zero));
	
	m_quad.setTexcoord(Quad2D::Vertex_BottomLeft, tt::math::Vector2(
		               isFlippedHorizontal() ? width : zero,
		               isFlippedVertical() ? zero : height));
	m_quad.setTexcoord(Quad2D::Vertex_BottomRight, tt::math::Vector2(
		               isFlippedHorizontal() ? zero : width,
		               isFlippedVertical() ? zero : height));
	
	setFlag(Flag_NeedQuadUpdate);
}

// Namespace end
}
}
}
