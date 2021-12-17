#include <tt/code/BufferReadContext.h>
#include <tt/code/bufferutils.h>
#include <tt/code/BufferWriteContext.h>
#include <tt/engine/renderer/QuadSprite.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/engine/renderer/Texture.h>

#include <toki/game/light/Glow.h>


namespace toki {
namespace game {
namespace light {

//--------------------------------------------------------------------------------------------------
// Public member functions
	
GlowPtr Glow::create(const CreationParams& p_creationParams)
{
	if (p_creationParams.imageName.empty())
	{
		TT_PANIC("Glow cannot be created with an empty image name");
		return GlowPtr();
	}
	
	if (p_creationParams.maxRadius < p_creationParams.minRadius)
	{
		TT_PANIC("Glow max radius %f < min radius %f", 
		         p_creationParams.maxRadius, p_creationParams.minRadius);
		return GlowPtr();
	}
	
	if (p_creationParams.minRadius < 0.0f || p_creationParams.maxRadius < 0.0f)
	{
		TT_PANIC("Glow max radius %f or min radius %f are < 0.0", 
		         p_creationParams.maxRadius, p_creationParams.minRadius);
		return GlowPtr();
	}
	
	if (p_creationParams.fadeRadius < 0.0f)
	{
		TT_PANIC("Glow fade radius %f is < 0.0", 
		         p_creationParams.fadeRadius);
		return GlowPtr();
	}
	
	using namespace tt::engine::renderer;
	QuadSpritePtr quadPtr = QuadSprite::createQuad(
		tt::engine::renderer::TextureCache::get(p_creationParams.imageName, "textures.lights"),
		tt::engine::renderer::ColorRGB::white);
	
	if (quadPtr != 0)
	{
		return GlowPtr(new Glow(p_creationParams, quadPtr));
	}
	
	return GlowPtr();
}


void Glow::update(const Light& p_light)
{
	if (m_glowQuad != 0)
	{
		real radius = p_light.getCurrentRadius();
		
		// Check for alpha fade
		u8 opacity = 255;
		if (p_light.getCurrentRadius() < m_creationParams.fadeRadius)
		{
			opacity = static_cast<u8>((radius / m_creationParams.fadeRadius) * 255.0);
		}
		
		if (opacity != m_opacity)
		{
			m_glowQuad->setOpacity(opacity);
			m_opacity = opacity;
		}
		
		radius *= m_creationParams.scale;
		if (radius < m_creationParams.minRadius)
		{
			radius = m_creationParams.minRadius;
		}
		else if (radius > m_creationParams.maxRadius)
		{
			radius = m_creationParams.maxRadius;
		}
		const real diameter = (radius * 2.0f);
		m_glowQuad->setWidth(diameter);
		m_glowQuad->setHeight(diameter);
		
		const tt::math::Vector2& position(p_light.getWorldPosition());
		m_glowQuad->setPosition(position.x, -position.y, 0.0f);
	}
}


void Glow::render() const
{
	if (m_glowQuad != 0)
	{
		m_glowQuad->update();
		m_glowQuad->render();
	}
}


void Glow::serialize(tt::code::BufferWriteContext* p_context) const
{
	namespace bu = tt::code::bufferutils;
	
	bu::put(m_creationParams.imageName,  p_context);
	bu::put(m_creationParams.scale,      p_context);
	bu::put(m_creationParams.minRadius,  p_context);
	bu::put(m_creationParams.maxRadius,  p_context);
	bu::put(m_creationParams.fadeRadius, p_context);
}


GlowPtr Glow::unserialize(tt::code::BufferReadContext* p_context)
{
	namespace bu = tt::code::bufferutils;
	
	std::string name  = bu::get<std::string>(p_context);
	real scale        = bu::get<real       >(p_context);
	real minRadius    = bu::get<real       >(p_context);
	real maxRadius    = bu::get<real       >(p_context);
	real fadeRadius   = bu::get<real       >(p_context);
	
	return create(CreationParams(name, scale, minRadius, maxRadius, fadeRadius));
}


//----------------------------------------------------------------------------------------------------------------
// Private member functions

Glow::Glow(const CreationParams& p_creationParams,
           const tt::engine::renderer::QuadSpritePtr p_glowQuad)
:
m_creationParams(p_creationParams),
m_glowQuad(p_glowQuad),
m_opacity(255)
{
}


// Namespace end
}
}
}
