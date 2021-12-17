#include <tt/app/Application.h>
#include <tt/engine/debug/DebugRenderer.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/engine/renderer/Texture.h>
#include <tt/engine/renderer/TrianglestripBuffer.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>

#include <toki/game/DebugView.h>
#include <toki/game/Game.h>
#include <toki/game/entity/Entity.h>
#include <toki/level/helpers.h>
#include <toki/AppGlobal.h>
#include <toki/constants.h>


namespace toki {
namespace game {


//--------------------------------------------------------------------------------------------------
// Public member functions

DebugViewPtr DebugView::create(const std::string& p_tileTextureName)
{
	// Load the tile set image
	tt::engine::renderer::TexturePtr tileSet(
		tt::engine::renderer::TextureCache::get(p_tileTextureName, "textures"));
	if (tileSet == 0)
	{
		TT_PANIC("Loading tileset texture '%s' failed.", p_tileTextureName.c_str());
		return DebugViewPtr();
	}
	
	return DebugViewPtr(new DebugView(tileSet));
}


DebugView::~DebugView()
{
}


void DebugView::update(real p_deltaTime)
{
	// update lifetime texts
	for (TextCollection::iterator it = m_texts.begin(); it != m_texts.end();)
	{
		if ((*it).lifetime < 0)
		{
			it = m_texts.erase(it);
		}
		else
		{
			(*it).lifetime -= p_deltaTime;
			++it;
		}
	}
	
	// update lifetime circles
	for (CircleCollection::iterator it = m_circles.begin(); it != m_circles.end();)
	{
		if ((*it).lifetime < 0)
		{
			it = m_circles.erase(it);
		}
		else
		{
			(*it).lifetime -= p_deltaTime;
			++it;
		}
	}
	
	// update lifetime circleparts
	for (CirclePartCollection::iterator it = m_circleParts.begin(); it != m_circleParts.end();)
	{
		if ((*it).lifetime < 0)
		{
			it = m_circleParts.erase(it);
		}
		else
		{
			(*it).lifetime -= p_deltaTime;
			++it;
		}
	}
	
	// update lifetime lines
	for (LineCollection::iterator it = m_lines.begin(); it != m_lines.end();)
	{
		if ((*it).lifetime < 0)
		{
			it = m_lines.erase(it);
		}
		else
		{
			(*it).lifetime -= p_deltaTime;
			++it;
		}
	}

	// update lifetime entity lines
	for (EntityLineCollection::iterator it = m_entityLines.begin(); it != m_entityLines.end();)
	{
		if ((*it).lifetime < 0)
		{
			it = m_entityLines.erase(it);
		}
		else
		{
			(*it).lifetime -= p_deltaTime;
			++it;
		}
	}
	
	// update lifetime tiles
	for (TileCollection::iterator it = m_tiles.begin(); it != m_tiles.end();)
	{
		if ((*it).lifetime < 0)
		{
			it = m_tiles.erase(it);
			m_tilesDirty = true;
		}
		else
		{
			(*it).lifetime -= p_deltaTime;
			++it;
		}
	}
	
	if (m_tilesDirty)
	{
		tt::engine::renderer::BatchQuadCollection staticTiles;
		
		for (TileCollection::const_iterator it = m_tiles.begin(); it != m_tiles.end(); ++it)
		{
			for (Point2s::const_iterator posIt = (*it).tilePositions.begin();
				posIt != (*it).tilePositions.end(); ++posIt)
			{
				staticTiles.push_back(createQuad((*posIt).x, (*posIt).y, (*it).tileIndex));
			}
		}
		
		m_tileQuads->clear();
		
		if (staticTiles.empty() == false)
		{
			m_tileQuads->setCollection(staticTiles);
			m_tileQuads->applyChanges();
		}
		
		m_tilesDirty = false;
	}
}


void DebugView::render() const
{
	if (isVisible() == false) return;
	
	m_tileQuads->render();

	const tt::engine::debug::DebugRendererPtr dbg = 
		tt::engine::renderer::Renderer::getInstance()->getDebug();
	
	for (CircleCollection::const_iterator it = m_circles.begin(); it != m_circles.end(); ++it)
	{
		if ((*it).isSolid)
		{
			dbg->renderSolidCircle((*it).color, (*it).position, (*it).radius);
		}
		else
		{
			dbg->renderCircle((*it).color, (*it).position, (*it).radius);
		}
	}
	
	for (CirclePartCollection::const_iterator it = m_circleParts.begin(); it != m_circleParts.end(); ++it)
	{
		renderCirclePart((*it).color, (*it).position, (*it).radius, (*it).startAngle, (*it).endAngle);
	}
	
	for (LineCollection::const_iterator it = m_lines.begin(); it != m_lines.end(); ++it)
	{
		dbg->renderLine((*it).color,
			tt::math::Vector3((*it).sourcePosition.x, (*it).sourcePosition.y, 0.0f), 
			tt::math::Vector3((*it).targetPosition.x, (*it).targetPosition.y, 0.0f));
	}

	for (EntityLineCollection::const_iterator it = m_entityLines.begin(); it != m_entityLines.end(); ++it)
	{
		entity::Entity* from = it->fromEntity.getPtr();
		entity::Entity* to   = it->toEntity.getPtr();

		tt::math::Vector2 fromPos(it->fromOffset);
		tt::math::Vector2 toPos  (it->toOffset);

		if(from != 0) fromPos += from->getPosition();
		if(to   != 0) toPos   += to->getPosition();

		dbg->renderLine(it->color,
			tt::math::Vector3(fromPos.x, fromPos.y, 0),
			tt::math::Vector3(toPos.x,   toPos.y,   0));
	}
	
	for (TextCollection::const_iterator it = m_texts.begin(); it != m_texts.end(); ++it)
	{
		tt::math::Point2 pos(it->position);

		if(it->worldSpace)
		{
			pos = AppGlobal::getGame()->getCamera().worldToScreen(it->position);
		}

		if ((*it).renderShadow)
		{
			dbg->renderText((*it).text, pos.x + 1, pos.y + 1, tt::engine::renderer::ColorRGB::black);
		}
		dbg->renderText((*it).text, pos.x, pos.y, (*it).color);
	}
	dbg->flush();
}


void DebugView::registerText(const TextInfo& p_textInfo)
{
	m_texts.push_back(p_textInfo);
}


void DebugView::registerCircle(const CircleInfo& p_circleInfo)
{
	m_circles.push_back(p_circleInfo);
}


void DebugView::registerCirclePart(const CirclePartInfo& p_circlePartInfo)
{
	m_circleParts.push_back(p_circlePartInfo);
}


void DebugView::registerLine(const LineInfo& p_lineInfo)
{
	m_lines.push_back(p_lineInfo);
}


void DebugView::registerEntityLine(const EntityLineInfo& p_lineInfo)
{
	m_entityLines.push_back(p_lineInfo);
}


void DebugView::registerTiles(const TileInfo& p_tileInfo)
{
	m_tilesDirty = true;
	m_tiles.push_back(p_tileInfo);
}


void DebugView::clear()
{
	m_texts.clear();
	m_circles.clear();
	m_circleParts.clear();
	m_lines.clear();
	m_entityLines.clear();
	m_tiles.clear();
}


void DebugView::renderLine(const tt::engine::renderer::ColorRGBA& p_color,
                           const tt::math::Vector2& p_start, const tt::math::Vector2& p_end) const
{
	if (isVisible() == false) return;
	const tt::engine::debug::DebugRendererPtr dbg = 
		tt::engine::renderer::Renderer::getInstance()->getDebug();
	
	dbg->renderLine(p_color, tt::math::Vector3(p_start.x, p_start.y, 0.0f), 
	                tt::math::Vector3(p_end.x, p_end.y, 0.0f));
}


void DebugView::renderCircle(const tt::engine::renderer::ColorRGBA& p_color,
                             const tt::math::Vector2& p_pos, real p_radius) const
{
	if (isVisible() == false) return;
	const tt::engine::debug::DebugRendererPtr dbg = 
		tt::engine::renderer::Renderer::getInstance()->getDebug();
	
	dbg->renderCircle(p_color, p_pos, p_radius);
}


void DebugView::renderSolidCircle(const tt::engine::renderer::ColorRGBA& p_centerColor, 
                                  const tt::engine::renderer::ColorRGBA& p_edgeColor,
                                  const tt::math::Vector2& p_pos, real p_radius) const
{
	if (isVisible() == false) return;
	const tt::engine::debug::DebugRendererPtr dbg = 
		tt::engine::renderer::Renderer::getInstance()->getDebug();
	
	dbg->renderSolidCircle(p_centerColor, p_edgeColor, p_pos, p_radius);

}


void DebugView::renderCirclePart(const tt::engine::renderer::ColorRGBA& p_color,
                                 const tt::math::Vector2& p_pos, real p_radius, real p_startAngle, real p_endAngle) const
{
	real offset = p_startAngle;
	
	TT_ASSERT(p_endAngle >= p_startAngle);
	real spread = p_endAngle - p_startAngle;
	
	const real subdivisionAngle = spread / (circlePrimitiveCount - 1);
	
	for (s32 i = 0; i < (circlePrimitiveCount - 1); ++i)
	{
		tt::engine::renderer::Renderer::getInstance()->getDebug()->renderLine(
			p_color,
			tt::math::Vector3(p_radius * tt::math::sin(offset) + p_pos.x,
		                      p_radius * tt::math::cos(offset) + p_pos.y,
		                      0.0f),
			tt::math::Vector3(p_radius * tt::math::sin(offset + subdivisionAngle) + p_pos.x,
		                      p_radius * tt::math::cos(offset + subdivisionAngle) + p_pos.y,
		                      0.0f));

		offset += subdivisionAngle;
	}
}


//--------------------------------------------------------------------------------------------------
// Private member functions

DebugView::DebugView(const tt::engine::renderer::TexturePtr& p_tileSet)
:
m_visible(true),
m_tileSet(p_tileSet),
m_tileSetTilesX(p_tileSet->getWidth()  / TexTile_Width),
m_tileSetTilesY(p_tileSet->getHeight() / TexTile_Height),
m_tileTexW(static_cast<real>(TexTile_Width)  / p_tileSet->getWidth()),
m_tileTexH(static_cast<real>(TexTile_Height) / p_tileSet->getHeight()),
m_tileQuads(new tt::engine::renderer::QuadBuffer(tt::engine::renderer::QuadBuffer::maxBatchSize, p_tileSet,
                                                 tt::engine::renderer::BatchFlagQuad_None)),
m_tilesDirty(true)
{
	m_tileSet->setAddressMode(tt::engine::renderer::AddressMode_Clamp, tt::engine::renderer::AddressMode_Clamp);
}


tt::engine::renderer::BatchQuad DebugView::createQuad(s32 p_x, s32 p_y, s32 p_tileIndex) const
{
	const real xPos     = level::tileToWorld(p_x);
	const real yPos     = level::tileToWorld(p_y);
	const real nextPosX = level::tileToWorld(p_x + 1);
	const real nextPosY = level::tileToWorld(p_y + 1);
	
	// Create quad
	tt::engine::renderer::BatchQuad quad;
	quad.topLeft.setPosition    (xPos,     nextPosY, 0.0f);
	quad.topRight.setPosition   (nextPosX, nextPosY, 0.0f);
	quad.bottomLeft.setPosition (xPos,     yPos,     0.0f);
	quad.bottomRight.setPosition(nextPosX, yPos,     0.0f);
	
	const real texStartX = (p_tileIndex % m_tileSetTilesX) * m_tileTexW;
	const real texStartY = (p_tileIndex / m_tileSetTilesX) * m_tileTexH;
	
	// Select the correct tile
	quad.topLeft.setTexCoord    (texStartX,              texStartY);
	quad.topRight.setTexCoord   (texStartX + m_tileTexW, texStartY);
	quad.bottomLeft.setTexCoord (texStartX,              texStartY + m_tileTexH);
	quad.bottomRight.setTexCoord(texStartX + m_tileTexW, texStartY + m_tileTexH);
	
	// Set alpha
	u8 alpha = 128;
	quad.topLeft.setColor(255, 255, 255, alpha);
	quad.topRight.setColor(255, 255, 255, alpha);
	quad.bottomLeft.setColor(255, 255, 255, alpha);
	quad.bottomRight.setColor(255, 255, 255, alpha);
	
	return quad;
}

// Namespace end
}
}

