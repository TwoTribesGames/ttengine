#include <tt/engine/renderer/MatrixStack.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/engine/renderer/Texture.h>
#include <tt/math/Random.h>
#include <tt/math/Vector2.h>
#include <tt/math/Vector3.h>
#include <tt/str/str.h>

#include <toki/game/event/SoundGraphicsMgr.h>
#include <toki/level/helpers.h>



namespace toki {
namespace game {
namespace event {

//--------------------------------------------------------------------------------------------------
// Public member functions

SoundGraphicsMgr::SoundGraphicsMgr()
:
m_graphics(0),
m_graphicsNeedUpdate(false)
{
	tt::math::Random random(0xCAFEBABE);
	
	// Initialize batch settings
	using namespace tt::engine::renderer;
	for (s32 i = 0; i < numberOfBatchSettings; ++i)
	{
		const std::string assetID("wavefront" + tt::str::toStr(i+1));
		m_settings[i].texture     = TextureCache::get(assetID, "textures.soundgraphic", true);
		m_settings[i].scale       = 1.8f;
		m_settings[i].fadeInTime  = 0.1f;
		m_settings[i].fadeOutTime = 0.25f;
		m_settings[i].lifetime    = m_settings[i].fadeInTime + m_settings[i].fadeOutTime +
		                            (random.getNormalizedNext() * 0.1f) + 0.3f;
		m_settings[i].uvAnim      = tt::math::Vector3(0.0f, 0.8f, 0.0f);
	}
}


void SoundGraphicsMgr::registerSound(real p_radius, const helpers::SoundChecker::Locations& p_visitedSoundLocations)
{
	using namespace tt::engine::renderer;
	BatchQuadCollection quadBatch[numberOfBatchSettings];

	for (s32 i = 0; i < numberOfBatchSettings; ++i)
	{
		quadBatch[i].reserve(128);
	}
	
	for (helpers::SoundChecker::Locations::const_iterator it = p_visitedSoundLocations.begin();
	     it != p_visitedSoundLocations.end(); ++it)
	{
		// No sound graphics should be displayed inside collision. Early out.
		if ((*it).isInsideCollision)
		{
			continue;
		}
		
		const bool isWaveFront = (*it).distance > (p_radius - 1.0f);
		if (isWaveFront)
		{
			// Calculate angle (only if location is on wave front)
			const tt::math::Point2& direction((*it).direction);
			if (direction.x != 0 || direction.y != 0)
			{
				// Calculate angle
				const real angle = tt::math::atan2(static_cast<real>(direction.y), static_cast<real>(direction.x)) -
				                   tt::math::halfPi;
				
				// Choose a random index
				const u32 index = tt::math::Random::getStatic().getNext() % numberOfBatchSettings;
				
				// Add to batch
				batchSoundGraphic((*it).location, angle, m_settings[index], &quadBatch[index]);
			}
		}
	}
	
	// Add graphics
	for (s32 i = 0; i < numberOfBatchSettings; ++i)
	{
		if (quadBatch[i].empty() == false)
		{
			m_graphics.push_back(SoundGraphicsPtr(new SoundGraphics(quadBatch[i], m_settings[i])));
		}
	}
}


void SoundGraphicsMgr::update(real p_deltatime)
{
	for (SoundGraphicsCollection::iterator it = m_graphics.begin(); it != m_graphics.end();)
	{
		(*it)->update(p_deltatime);
		if ((*it)->isAlive() == false)
		{
			it = m_graphics.erase(it);
		}
		else
		{
			++it;
		}
	}
	m_graphicsNeedUpdate = true;
}


void SoundGraphicsMgr::updateForRender()
{
	if(m_graphicsNeedUpdate)
	{
		for (SoundGraphicsCollection::const_iterator it = m_graphics.begin(); it != m_graphics.end(); ++it)
		{
			(*it)->updateForRender();
		}
	}
	m_graphicsNeedUpdate = false;
}


void SoundGraphicsMgr::render(const tt::math::VectorRect& p_visibilityRect) const
{
	for (SoundGraphicsCollection::const_iterator it = m_graphics.begin(); it != m_graphics.end(); ++it)
	{
		if (p_visibilityRect.intersects((*it)->getBoundingRect()))
		{
			(*it)->render();
		}
	}
	
	tt::engine::renderer::MatrixStack::getInstance()->resetTextureMatrix();
}


void SoundGraphicsMgr::renderLightmask(const tt::math::VectorRect& p_visibilityRect) const
{
	using namespace tt::engine::renderer;
	Renderer* renderer(Renderer::getInstance());
	
	// Lights are only rendered to alpha channel.
	renderer->setColorMask(ColorMask_Alpha);
	renderer->setCustomBlendMode(BlendFactor_One, BlendFactor_One);
	renderer->resetCustomBlendModeAlpha();
	
	// NOTE: For now, the sound graphics light masks are the same as the normal graphics, so just call render()
	render(p_visibilityRect);
	
	// Restore the defaults for normal renders after this.
	renderer->setColorMask(ColorMask_All);
	renderer->setBlendMode(BlendMode_Blend); // Restore the default blend mode.
	renderer->setCustomBlendModeAlpha(BlendFactor_Zero, BlendFactor_InvSrcAlpha);
}


//----------------------------------------------------------------------------------------------------------------
// Public member functions of SoundGraphics

SoundGraphicsMgr::SoundGraphics::SoundGraphics(const tt::engine::renderer::BatchQuadCollection& p_quadBatch,
                                               const BatchSettings& p_settings)
:
m_settings(p_settings),
m_quadBuffer(new tt::engine::renderer::QuadBuffer(static_cast<s32>(p_quadBatch.size()), p_settings.texture,
                                                  tt::engine::renderer::BatchFlagQuad_UseVertexColor)),
m_quadBatch(p_quadBatch),
m_time(0.0f),
m_opacity(0)
{
	// Compute bounds of graphics
	tt::math::Vector2 minPosition( 1000000.0f,  1000000.0f);
	tt::math::Vector2 maxPosition(-1000000.0f, -1000000.0f);
	
	for (tt::engine::renderer::BatchQuadCollection::const_iterator it = m_quadBatch.begin();
		it != m_quadBatch.end(); ++it)
	{
		minPosition.x = std::min(minPosition.x, (*it).bottomLeft .getPosition().x);
		minPosition.y = std::min(minPosition.y, (*it).bottomLeft .getPosition().y);
		minPosition.x = std::min(minPosition.x, (*it).bottomRight.getPosition().x);
		minPosition.y = std::min(minPosition.y, (*it).bottomRight.getPosition().y);
		minPosition.x = std::min(minPosition.x, (*it).topLeft    .getPosition().x);
		minPosition.y = std::min(minPosition.y, (*it).topLeft    .getPosition().y);
		minPosition.x = std::min(minPosition.x, (*it).topRight   .getPosition().x);
		minPosition.y = std::min(minPosition.y, (*it).topRight   .getPosition().y);
		
		maxPosition.x = std::max(maxPosition.x, (*it).bottomLeft .getPosition().x);
		maxPosition.y = std::max(maxPosition.y, (*it).bottomLeft .getPosition().y);
		maxPosition.x = std::max(maxPosition.x, (*it).bottomRight.getPosition().x);
		maxPosition.y = std::max(maxPosition.y, (*it).bottomRight.getPosition().y);
		maxPosition.x = std::max(maxPosition.x, (*it).topLeft    .getPosition().x);
		maxPosition.y = std::max(maxPosition.y, (*it).topLeft    .getPosition().y);
		maxPosition.x = std::max(maxPosition.x, (*it).topRight   .getPosition().x);
		maxPosition.y = std::max(maxPosition.y, (*it).topRight   .getPosition().y);
	}

	m_boundingRect = tt::math::VectorRect(minPosition, maxPosition);
}


void SoundGraphicsMgr::SoundGraphics::update(real p_deltatime)
{
	m_time += p_deltatime;
}


void SoundGraphicsMgr::SoundGraphics::updateForRender()
{
	real opacity = 1.0f;
	
	using namespace tt::engine::renderer;
	if (m_time < m_settings.fadeInTime)
	{
		opacity = 1.0f - (m_settings.fadeInTime - m_time) / m_settings.fadeInTime;
	}
	else if (m_time > (m_settings.lifetime - m_settings.fadeOutTime))
	{
		opacity = (m_settings.lifetime - m_time) / m_settings.fadeOutTime;
	}
	
	const u8 val = static_cast<u8>(opacity * 255);
	
	if (m_opacity != val)
	{
		TT_NULL_ASSERT(m_quadBuffer->getTexture());
		const u8 cval = m_quadBuffer->getTexture()->isPremultiplied() ? val : 255;
		
		for (BatchQuadCollection::iterator it = m_quadBatch.begin(); it != m_quadBatch.end(); ++it)
		{
			(*it).topLeft.    setColor(cval, cval, cval, val);
			(*it).topRight.   setColor(cval, cval, cval, val);
			(*it).bottomLeft. setColor(cval, cval, cval, val);
			(*it).bottomRight.setColor(cval, cval, cval, val);
		}
		
		m_quadBuffer->fillBuffer(m_quadBatch.begin(), m_quadBatch.end());
		m_opacity = val;
	}
}


void SoundGraphicsMgr::SoundGraphics::render() const
{
	TT_NULL_ASSERT(m_quadBuffer);
	using tt::engine::renderer::MatrixStack;
	MatrixStack* stack = MatrixStack::getInstance();
	
	stack->setMode(MatrixStack::Mode_Texture);
	
	stack->push();
	stack->load44(tt::math::Matrix44::getTranslation(m_settings.uvAnim * m_time));
	stack->updateTextureMatrix();
	m_quadBuffer->render();
	stack->pop();
	
	stack->setMode(MatrixStack::Mode_Position);
}


//----------------------------------------------------------------------------------------------------------------
// Private member functions

void SoundGraphicsMgr::batchSoundGraphic(const tt::math::Point2& p_tilePosition, real p_angle,
                                         const SoundGraphicsMgr::BatchSettings& p_settings,
                                         tt::engine::renderer::BatchQuadCollection* p_quadBatch_OUT)
{
	const tt::math::Vector2 pos2D(level::tileToWorld(p_tilePosition));
	const tt::math::Vector3 pos3D(pos2D.x + 0.5f, pos2D.y + 0.5f, 0.0f);
	
	TT_NULL_ASSERT(p_quadBatch_OUT);
	tt::engine::renderer::BatchQuad quad;
	
	tt::math::Matrix44 transform = tt::math::Matrix44::getTranslation(pos3D);
	transform.scale(p_settings.scale, p_settings.scale, 0.0f);
	transform.rotateZ(p_angle);
	
	static const tt::math::Vector3 topLeft    (-0.5f,  0.5f, 0.0f);
	static const tt::math::Vector3 topRight   ( 0.5f,  0.5f, 0.0f);
	static const tt::math::Vector3 bottomLeft (-0.5f, -0.5f, 0.0f);
	static const tt::math::Vector3 bottomRight( 0.5f, -0.5f, 0.0f);
	
	quad.topLeft.setPosition    (topLeft     * transform);
	quad.topRight.setPosition   (topRight    * transform);
	quad.bottomLeft.setPosition (bottomLeft  * transform);
	quad.bottomRight.setPosition(bottomRight * transform);
	
	quad.topLeft.    setTexCoord(0.0f, 0.0f);
	quad.topRight.   setTexCoord(1.0f, 0.0f);
	quad.bottomLeft. setTexCoord(0.0f, 1.0f);
	quad.bottomRight.setTexCoord(1.0f, 1.0f);
	
	p_quadBatch_OUT->push_back(quad);
}

// Namespace end
}
}
}
