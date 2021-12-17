
#include <tt/code/bufferutils.h>
#include <tt/engine/renderer/Texture.h>
#include <tt/engine/renderer/QuadBuffer.h>
#include <tt/pres/PresentationMgr.h>

#include <toki/game/Minimap.h>
#include <toki/level/helpers.h>
#include <toki/serialization/SerializationMgr.h>


// FIXME: Remove these three includes and fix getLevelWidth() (see below)
#include <toki/game/Game.h>
#include <toki/level/LevelData.h>
#include <toki/AppGlobal.h>


namespace toki {
namespace game {

static s32 getLevelWidth()
{
	// FIXME: I rather not use AppGlobal here,
	//        but I want to make sure we always have the correct and current level width.
	
	if (AppGlobal::hasGame() == false)
	{
		return 1;
	}
	return AppGlobal::getGame()->getLevelData()->getLevelWidth();
}


//--------------------------------------------------------------------------------------------------
// Public member functions

Minimap::Minimap()
:
m_enabled(false),
m_hidden(false),
m_sidesBorder(0.5f),
m_yOffset(0.01f),
m_biggestLevelWidth(300),
m_graphicsSize(0.1f),
m_extraWidth(0.1f),
m_sideFadeWidth(0.05f),
m_needsUpdate(false)
{
}


tt::math::Vector2 Minimap::getPositionFromWorld(const tt::math::Vector2& p_pos) const
{
	real x = p_pos.x;
	
	const s32 biggestLevelWidth = getBiggestLevelWidth();
	const s32 levelWidth        = getLevelWidth();

	// Move range from 0.0 - level width to -halfLevelWidth - halfLevelWidth
	x -= (levelWidth * 0.5f);
	
	// Scale to -1.0 - 1.0 range based on biggest level width
	x = x / (biggestLevelWidth * 0.5f);
	
	// Resize based on border size.
	x *= (1.0f - getSidesBorder());
	
	return tt::math::Vector2(x, getYOffset()); // No Y for minimap.
}


void Minimap::serialize(tt::code::BufferWriteContext* p_context) const
{
	TT_NULL_ASSERT(p_context);
	
	namespace bu = tt::code::bufferutils;
	
	bu::put(m_enabled,           p_context);
	bu::put(m_hidden,            p_context);
	bu::put(m_sidesBorder,       p_context);
	bu::put(m_yOffset,           p_context);
	bu::put(m_biggestLevelWidth, p_context);
	bu::put(m_graphicsSize,      p_context);
	bu::put(m_extraWidth,        p_context);
	bu::put(m_sideFadeWidth,     p_context);
}


void Minimap::unserialize(tt::code::BufferReadContext*  p_context)
{
	TT_NULL_ASSERT(p_context);
	
	namespace bu = tt::code::bufferutils;
	
	m_enabled           = bu::get<bool>(p_context);
	m_hidden            = bu::get<bool>(p_context);
	m_sidesBorder       = bu::get<real>(p_context);
	m_yOffset           = bu::get<real>(p_context);
	m_biggestLevelWidth = bu::get<s32> (p_context);
	m_graphicsSize      = bu::get<real>(p_context);
	m_extraWidth        = bu::get<real>(p_context);
	m_sideFadeWidth     = bu::get<real>(p_context);
	updateGraphics();
}


void Minimap::render(const tt::pres::PresentationMgrPtr& p_presMgr)
{
	if (m_enabled == false || m_hidden)
	{
		return;
	}
	
	TT_NULL_ASSERT(m_graphics);
	if (m_graphics == 0)
	{
		return;
	}
	if(m_needsUpdate)
	{
		m_graphics->applyChanges();
		m_needsUpdate = false;
	}
	
	m_graphics->render();
	
	if (p_presMgr != 0)
	{
		p_presMgr->render();
	}
}


//--------------------------------------------------------------------------------------------------
// Private member functions


void Minimap::updateGraphics()
{
	if (m_enabled == false)
	{
		return;
	}
	
	if (m_graphics == 0)
	{
		using namespace tt::engine::renderer;
		TexturePtr texture = TextureCache::get("minimap_graphics", "textures.hud", true);
		TT_NULL_ASSERT(texture);
		texture->setAddressMode(tt::engine::renderer::AddressMode_Wrap,
		                        tt::engine::renderer::AddressMode_Clamp);
		m_graphics.reset(new QuadBuffer(5, texture, BatchFlagQuad_UseVertexColor));
	}
	TT_NULL_ASSERT(m_graphics);
	
	m_graphics->clear();
	
	// Note: We're working in screenspace.
	tt::math::Vector2 leftSide  = getPositionFromWorld(tt::math::Vector2::zero);
	const tt::math::Vector2 rightLevelEdge(level::tileToWorld(getLevelWidth()), 0.0f);
	tt::math::Vector2 rightSide = getPositionFromWorld(rightLevelEdge);
	leftSide.x  -= m_extraWidth;
	rightSide.x += m_extraWidth;
	const real graphicSize = m_graphicsSize;
	
	tt::engine::renderer::BatchQuadCollection quadBatch;
	
	const real halfGraphicSize = graphicSize * 0.5f;
	TT_ASSERT(leftSide.y == rightSide.y);
	const real top    = leftSide.y + halfGraphicSize;
	const real bottom = leftSide.y - halfGraphicSize;
	
	// Fix size for very small levels.
	if (leftSide.x > -graphicSize)
	{
		leftSide.x = -graphicSize;
	}
	if (rightSide.x < graphicSize)
	{
		rightSide.x = graphicSize;
	}
	
	// Left
	{
		tt::engine::renderer::BatchQuad quad;
		const real left  = leftSide.x;
		const real right = leftSide.x + graphicSize;
		quad.topLeft.    setPosition(left , top   , 0);
		quad.topRight.   setPosition(right, top   , 0);
		quad.bottomLeft. setPosition(left , bottom, 0);
		quad.bottomRight.setPosition(right, bottom, 0);
		
		quad.topLeft.    setTexCoord(0.0f, 0.5f);
		quad.topRight.   setTexCoord(0.5f, 0.5f);
		quad.bottomLeft. setTexCoord(0.0f, 1.0f);
		quad.bottomRight.setTexCoord(0.5f, 1.0f);
		
		quadBatch.push_back(quad);
	}
	
	// Right
	{
		tt::engine::renderer::BatchQuad quad;
		const real left  = rightSide.x - graphicSize;
		const real right = rightSide.x;
		quad.topLeft.    setPosition(left , top   , 0);
		quad.topRight.   setPosition(right, top   , 0);
		quad.bottomLeft. setPosition(left , bottom, 0);
		quad.bottomRight.setPosition(right, bottom, 0);
		
		quad.topLeft.    setTexCoord(0.5f, 0.5f);
		quad.topRight.   setTexCoord(1.0f, 0.5f);
		quad.bottomLeft. setTexCoord(0.5f, 1.0f);
		quad.bottomRight.setTexCoord(1.0f, 1.0f);
		
		quadBatch.push_back(quad);
	}
	
	// Center, left fade, and right fade
	if (leftSide.x < rightSide.x)
	{
		const real left  = leftSide.x  + graphicSize;
		const real right = rightSide.x - graphicSize;
		
		const tt::engine::renderer::ColorRGBA transparent(
			m_graphics->getTexture()->isPremultiplied() ?
				tt::engine::renderer::ColorRGB::black :  // premultiplied color with alpha 0: color 0 as well
				tt::engine::renderer::ColorRGB::white,
			0);
		
		// Left Fade
		if (m_sideFadeWidth > 0.0f)
		{
			tt::engine::renderer::BatchQuad quad;
			const real fadeLeft  = left - m_sideFadeWidth;
			const real fadeRigth = left;
			quad.topLeft.    setPosition(fadeLeft , top   , 0);
			quad.topRight.   setPosition(fadeRigth, top   , 0);
			quad.bottomLeft. setPosition(fadeLeft , bottom, 0);
			quad.bottomRight.setPosition(fadeRigth, bottom, 0);
			
			TT_ASSERT(fadeRigth >= fadeLeft);
			const real width = fadeRigth - fadeLeft;
			const real V = (width / graphicSize) * 0.5f;
			
			quad.topLeft.    setTexCoord(-V  , 0.0f);
			quad.topRight.   setTexCoord(0.0f, 0.0f);
			quad.bottomLeft. setTexCoord(-V  , 0.5f);
			quad.bottomRight.setTexCoord(0.0f, 0.5f);
			
			// Left side should fade out.
			quad.topLeft.    setColor(transparent);
			quad.bottomLeft. setColor(transparent);
			
			quadBatch.push_back(quad);
		}
		
		real centerRightV = 0.0f;
		
		// Center
		{
			tt::engine::renderer::BatchQuad quad;
			quad.topLeft.    setPosition(left , top   , 0);
			quad.topRight.   setPosition(right, top   , 0);
			quad.bottomLeft. setPosition(left , bottom, 0);
			quad.bottomRight.setPosition(right, bottom, 0);
			
			TT_ASSERT(right >= left);
			const real width = right - left;
			centerRightV = (width / graphicSize) * 0.5f;
			
			quad.topLeft.    setTexCoord(0.0f        , 0.0f);
			quad.topRight.   setTexCoord(centerRightV, 0.0f);
			quad.bottomLeft. setTexCoord(0.0f        , 0.5f);
			quad.bottomRight.setTexCoord(centerRightV, 0.5f);
			
			quadBatch.push_back(quad);
		}
		
		// Right Fade
		if (m_sideFadeWidth > 0.0f)
		{
			tt::engine::renderer::BatchQuad quad;
			const real fadeLeft  = right;
			const real fadeRigth = right + m_sideFadeWidth;
			quad.topLeft.    setPosition(fadeLeft , top   , 0);
			quad.topRight.   setPosition(fadeRigth, top   , 0);
			quad.bottomLeft. setPosition(fadeLeft , bottom, 0);
			quad.bottomRight.setPosition(fadeRigth, bottom, 0);
			
			TT_ASSERT(fadeRigth >= fadeLeft);
			const real width = fadeRigth - fadeLeft;
			const real V = (width / graphicSize) * 0.5f;
			
			quad.topLeft.    setTexCoord(centerRightV    , 0.0f);
			quad.topRight.   setTexCoord(centerRightV + V, 0.0f);
			quad.bottomLeft. setTexCoord(centerRightV    , 0.5f);
			quad.bottomRight.setTexCoord(centerRightV + V, 0.5f);
			
			// Left side should fade out.
			quad.topRight.    setColor(transparent);
			quad.bottomRight. setColor(transparent);
			
			quadBatch.push_back(quad);
		}
	}
	
	
	m_graphics->setCollection(quadBatch);
	m_needsUpdate = true;
}


// Namespace end
}
}
