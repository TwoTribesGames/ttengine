#include <tt/engine/renderer/MatrixStack.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/engine/renderer/Texture.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>
#include <tt/profiler/PerformanceProfiler.h>

#include <toki/game/AttributeDebugView.h>
#include <toki/level/AttributeLayer.h>
#include <toki/level/helpers.h>
#include <toki/constants.h>


namespace toki {
namespace game {

//--------------------------------------------------------------------------------------------------
// Public member functions

AttributeDebugViewPtr AttributeDebugView::create(const level::AttributeLayerPtr& p_layerData,
                                                 ViewMode                        p_mode,
                                                 const std::string&              p_tilesetID,
                                                 const std::string&              p_tilesetNamespace)
{
	if (p_layerData == 0)
	{
		TT_PANIC("Must have valid attribute layer data pointer.");
		return AttributeDebugViewPtr();
	}
	
	// Extra check to support unit tests (which run without initializing any of the TTdev systems)
	if (tt::engine::renderer::Renderer::hasInstance() == false)
	{
		return AttributeDebugViewPtr();
	}
	
	// Load the tile set image
	tt::engine::renderer::TexturePtr tileSet(
			tt::engine::renderer::TextureCache::get(p_tilesetID, p_tilesetNamespace));
	if (tileSet == 0)
	{
		TT_PANIC("Loading tile set texture '%s' (namespace '%s') failed.",
		         p_tilesetID.c_str(), p_tilesetNamespace.c_str());
		return AttributeDebugViewPtr();
	}
	
	AttributeDebugViewPtr view(new AttributeDebugView(p_layerData, tileSet, p_mode));
	view->m_this = view;
	p_layerData->registerObserver(view);
	return view;
}


AttributeDebugView::~AttributeDebugView()
{
}


void AttributeDebugView::update()
{
	if (m_tilesAreDirty)
	{
		rebuildQuadBuffer();
	}
}


void AttributeDebugView::render()
{
	if (isVisible() == false) return;

	if(m_bufferNeedsUpdate)
	{
		m_tileQuads->applyChanges();
		m_bufferNeedsUpdate = false;
	}
	
	//PROFILE_PERFORMANCE("Rendering attribute layer.");
	
	tt::engine::renderer::MatrixStack* stack = tt::engine::renderer::MatrixStack::getInstance();
	stack->push();
	stack->translate(m_position);
	stack->updateWorldMatrix();
	
	m_tileQuads->render();
	
	stack->pop();
}


void AttributeDebugView::setAttributeLayer(const level::AttributeLayerPtr& p_attributes)
{
	TT_NULL_ASSERT(p_attributes);
	if (p_attributes == 0) return;
	
	if (p_attributes != m_attribs)
	{
		AttributeDebugViewPtr self(m_this.lock());
		TT_NULL_ASSERT(self);
		
		m_attribs->unregisterObserver(self);
		
		m_attribs = p_attributes;
		
		m_attribs->registerObserver(self);
		
		rebuildQuadBuffer();
	}
}


void AttributeDebugView::setVertexColor(const tt::engine::renderer::ColorRGBA& p_color)
{
	if (p_color != m_vertexColor)
	{
		m_vertexColor = p_color;
		// With the current rendering, the only way to change the color is to rebuild all quads...
		rebuildQuadBuffer();
	}
}


void AttributeDebugView::setTexture(const tt::engine::renderer::TexturePtr& p_texture)
{
	m_tileSet = p_texture;

	m_tileQuads->setTexture(p_texture);
}


void AttributeDebugView::onTileLayerDirty()
{
	m_tilesAreDirty = true;
}


//--------------------------------------------------------------------------------------------------
// Private member functions

AttributeDebugView::AttributeDebugView(const level::AttributeLayerPtr&         p_layerData,
                                       const tt::engine::renderer::TexturePtr& p_tileSet,
                                       ViewMode                                p_mode)
:
m_this(),
m_visible(true),
m_viewMode(p_mode),
m_tileIndexFunc(0),
m_position(tt::math::Vector3::zero),
m_tileSet(p_tileSet),
m_attribs(p_layerData),
m_tilesAreDirty(true),
m_tileSetTilesX(p_tileSet->getWidth()  / TexTile_Width),
m_tileSetTilesY(p_tileSet->getHeight() / TexTile_Height),
m_tileTexW(static_cast<real>(TexTile_Width)  / p_tileSet->getWidth()),
m_tileTexH(static_cast<real>(TexTile_Height) / p_tileSet->getHeight()),
m_bufferNeedsUpdate(false),
m_vertexColor(tt::engine::renderer::ColorRGB::white)
{
	switch (m_viewMode)
	{
	default:
		TT_PANIC("Unsupported attribute view mode: %d. Defaulting to CollisionType.", p_mode);
		// Intentional fall-through (use CollisionType in case of error)
		
	case ViewMode_CollisionType: m_tileIndexFunc = getTileIndexCollisionType; break;
	case ViewMode_ThemeType:     m_tileIndexFunc = getTileIndexThemeType;     break;
	case ViewMode_Fluids:        m_tileIndexFunc = getTileIndexFluids;        break;
	}
	TT_NULL_ASSERT(m_tileIndexFunc);
	
	m_tileSet->setAddressMode(tt::engine::renderer::AddressMode_Clamp, tt::engine::renderer::AddressMode_Clamp);

	{
		using namespace tt::engine::renderer;
		u32 quadsNeeded = std::min(QuadBuffer::maxBatchSize, p_layerData->getLength());
		m_tileQuads.reset(new QuadBuffer(quadsNeeded, p_tileSet, BatchFlagQuad_UseVertexColor));
	}
}


tt::engine::renderer::BatchQuad AttributeDebugView::createQuadForTile(s32 p_x, s32 p_y, u8 p_tile) const
{
	const real xPos     = level::tileToWorld(p_x);
	const real yPos     = level::tileToWorld(p_y);
	const real nextPosX = level::tileToWorld(p_x + 1);
	const real nextPosY = level::tileToWorld(p_y + 1);
	
	// Create quad
	tt::engine::renderer::BatchQuad quad;
	
	quad.topLeft.setColor    (m_vertexColor);
	quad.topRight.setColor   (m_vertexColor);
	quad.bottomLeft.setColor (m_vertexColor);
	quad.bottomRight.setColor(m_vertexColor);
	
	quad.topLeft.setPosition    (xPos,     nextPosY, 0.0f);
	quad.topRight.setPosition   (nextPosX, nextPosY, 0.0f);
	quad.bottomLeft.setPosition (xPos,     yPos,     0.0f);
	quad.bottomRight.setPosition(nextPosX, yPos,     0.0f);
	
	const real texStartX = (p_tile % m_tileSetTilesX) * m_tileTexW;
	const real texStartY = (p_tile / m_tileSetTilesX) * m_tileTexH;

	// Texel offset to prevent sampling from neighbour tile when filtering
	static const real texelOffsetX = 1.0f / m_tileSet->getWidth();
	static const real texelOffsetY = 1.0f / m_tileSet->getHeight();
	
	// Select the correct tile
	quad.topLeft.setTexCoord    (texStartX + texelOffsetX,              texStartY + texelOffsetY);
	quad.topRight.setTexCoord   (texStartX - texelOffsetX + m_tileTexW, texStartY + texelOffsetY);
	quad.bottomLeft.setTexCoord (texStartX + texelOffsetX,              texStartY - texelOffsetY + m_tileTexH);
	quad.bottomRight.setTexCoord(texStartX - texelOffsetX + m_tileTexW, texStartY - texelOffsetY + m_tileTexH);
	
	return quad;
}


void AttributeDebugView::rebuildQuadBuffer()
{
	// Rebuild the quad buffer for the current state of the attribute layer
	tt::engine::renderer::BatchQuadCollection staticTiles;
	
	const u8* tiles = m_attribs->getRawData();
	for (s32 y = 0; y < m_attribs->getHeight(); ++y)
	{
		for (s32 x = 0; x < m_attribs->getWidth(); ++x, ++tiles)
		{
			u8 tile = m_tileIndexFunc(*tiles);
			if (tile > 0) // Zero means no tile
			{
				--tile;  // in-texture tiles start at 0
				staticTiles.push_back(createQuadForTile(x, y, tile));
			}
		}
	}
	
	s32 tileCount = static_cast<s32>(staticTiles.size());

	if(tileCount > m_tileQuads->getCapacity())
	{
		using namespace tt::engine::renderer;
		if(tileCount > QuadBuffer::maxBatchSize)
		{
			TT_PANIC("Need space for %d quads, can only store %d in quad buffer.",
				tileCount, QuadBuffer::maxBatchSize);

			// This will throw away the quads that do not fit
			staticTiles.resize(QuadBuffer::maxBatchSize);
		}

		// Create more storage space
		m_tileQuads.reset(new QuadBuffer(tileCount, m_tileSet, BatchFlagQuad_UseVertexColor));
	}

	{
		m_tileQuads->clear();
		m_tileQuads->setCollection(staticTiles);
	}
	
	m_tilesAreDirty = false;
	m_bufferNeedsUpdate = true;
}


//--------------------------------------------------------------------------------------------------
// Attribute tile to texture tile index translators

enum AttribTile
{
	AttribTile_None,
	AttribTile_Solid,
	AttribTile_UnusedIndex2,
	AttribTile_UnusedIndex3,
	AttribTile_UnusedIndex4,
	AttribTile_UnusedIndex5,
	AttribTile_UnusedIndex6,
	AttribTile_UnusedIndex7,
	AttribTile_WarpTile,
	
	AttribTile_Water_Source,
	AttribTile_Water_Down,
	AttribTile_Water_Left,
	AttribTile_Water_Right,
	AttribTile_Water_Still,
	AttribTile_Water_Left2,
	AttribTile_Water_Right2,
	
	AttribTile_Solid_FluidKill,
	
	AttribTile_Water_LeftOverFlow,
	AttribTile_Water_RightOverFlow,
	
	AttribTile_UnusedIndex19,
	AttribTile_UnusedIndex20,
	AttribTile_UnusedIndex21,
	AttribTile_UnusedIndex22,
	AttribTile_UnusedIndex23,
	AttribTile_UnusedIndex24,
	
	AttribTile_Lava_Source,
	AttribTile_Lava_Down,
	AttribTile_Lava_Left,
	AttribTile_Lava_Right,
	AttribTile_Lava_Still,
	AttribTile_Lava_Left2,
	AttribTile_Lava_Right2,
	AttribTile_Lava_LeftOverFlow,
	AttribTile_Lava_RightOverFlow,
	
	AttribTile_UnusedIndex34,
	AttribTile_UnusedIndex35,
	AttribTile_UnusedIndex36,
	AttribTile_UnusedIndex37,
	AttribTile_UnusedIndex38,
	AttribTile_UnusedIndex39,
	AttribTile_UnusedIndex40,
	
	AttribTile_Crystal_Solid,
	AttribTile_Solid_Allow_Pathfinding,
	
	AttribTile_FluidKill,
	
	AttribTile_UnusedIndex44,
	AttribTile_UnusedIndex45,
	AttribTile_UnusedIndex46,
	AttribTile_UnusedIndex47,
	AttribTile_UnusedIndex48,
	
	AttribTile_Theme_DoNotTheme,
	AttribTile_Theme_Sand,
	AttribTile_Theme_Rocks,
	AttribTile_Theme_Beach,
	AttribTile_Theme_DarkRocks,
	
	AttribTile_UnusedIndex54,
	AttribTile_UnusedIndex55,
	AttribTile_UnusedIndex56,
	
	AttribTile_AirPrefer,
	AttribTile_AirAvoid
};


u8 AttributeDebugView::getTileIndexCollisionType(u8 p_attrib)
{
	switch (level::getCollisionType(p_attrib))
	{
	case level::CollisionType_Air:                      return AttribTile_None;
	case level::CollisionType_Solid:                    return AttribTile_Solid;
	case level::CollisionType_Water_Source:             return AttribTile_Water_Source;
	case level::CollisionType_Water_Still:              return AttribTile_Water_Still;
	case level::CollisionType_Lava_Source:              return AttribTile_Lava_Source;
	case level::CollisionType_Lava_Still:               return AttribTile_Lava_Still;
	case level::CollisionType_Solid_FluidKill:          return AttribTile_Solid_FluidKill;
	case level::CollisionType_FluidKill:                return AttribTile_FluidKill;
	case level::CollisionType_Crystal_Solid:            return AttribTile_Crystal_Solid;
	case level::CollisionType_AirPrefer:                return AttribTile_AirPrefer;
	case level::CollisionType_AirAvoid:                 return AttribTile_AirAvoid;
	case level::CollisionType_Solid_Allow_Pathfinding:  return AttribTile_Solid_Allow_Pathfinding;
	
	default: return AttribTile_None;
	}
}


u8 AttributeDebugView::getTileIndexThemeType(u8 p_attrib)
{
	switch (level::getThemeType(p_attrib))
	{
	// NOTE: level::ThemeType_UseLevelDefault is like Air: do not render this tile
	case level::ThemeType_DoNotTheme:      return AttribTile_Theme_DoNotTheme;
	case level::ThemeType_Sand:            return AttribTile_Theme_Sand;
	case level::ThemeType_Rocks:           return AttribTile_Theme_Rocks;
	case level::ThemeType_Beach:           return AttribTile_Theme_Beach;
	case level::ThemeType_DarkRocks:       return AttribTile_Theme_DarkRocks;
		
	default: return AttribTile_None;
	}
}


u8 AttributeDebugView::getTileIndexFluids(u8 p_attrib)
{
	switch (fluid::getFluidType(p_attrib))
	{
	case fluid::FluidType_Water:
		switch (fluid::getFluidFlowType(p_attrib))
		{
		case fluid::FluidFlowType_StillUnderFall:
		case fluid::FluidFlowType_Still:         return AttribTile_Water_Still;
		case fluid::FluidFlowType_Fall:          return AttribTile_Water_Down;
		case fluid::FluidFlowType_LeftLvl2:      return AttribTile_Water_Left2;
		case fluid::FluidFlowType_Left:          return AttribTile_Water_Left;
		case fluid::FluidFlowType_LeftOverFlow:  return AttribTile_Water_LeftOverFlow;
		case fluid::FluidFlowType_RightLvl2:     return AttribTile_Water_Right2;
		case fluid::FluidFlowType_Right:         return AttribTile_Water_Right;
		case fluid::FluidFlowType_RightOverFlow: return AttribTile_Water_RightOverFlow;
		case fluid::FluidFlowType_None:
		default: break;
		}
	
	case fluid::FluidType_Lava:
		switch (fluid::getFluidFlowType(p_attrib))
		{
		case fluid::FluidFlowType_StillUnderFall:
		case fluid::FluidFlowType_Still:         return AttribTile_Lava_Still;
		case fluid::FluidFlowType_Fall:          return AttribTile_Lava_Down;
		case fluid::FluidFlowType_LeftLvl2:      return AttribTile_Lava_Left2;
		case fluid::FluidFlowType_Left:          return AttribTile_Lava_Left;
		case fluid::FluidFlowType_LeftOverFlow:  return AttribTile_Lava_LeftOverFlow;
		case fluid::FluidFlowType_RightLvl2:     return AttribTile_Lava_Right2;
		case fluid::FluidFlowType_Right:         return AttribTile_Lava_Right;
		case fluid::FluidFlowType_RightOverFlow: return AttribTile_Lava_RightOverFlow;
		case fluid::FluidFlowType_None:
		default: break;
		}
		
	default: break;
	}
	
	if (fluid::isWarpTile(p_attrib)) return AttribTile_WarpTile;
	
	return AttribTile_None;
}

// Namespace end
}
}
