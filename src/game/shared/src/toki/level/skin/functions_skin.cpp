#include <tt/engine/scene2d/shoebox/shoebox.h>
#include <tt/platform/tt_printf.h>
#include <tt/system/Time.h>

#include <toki/level/skin/BlobData.h>
#include <toki/level/skin/functions.h>
#include <toki/level/skin/GrowEdge.h>
#include <toki/level/skin/SkinConfig.h>
#include <toki/level/skin/SkinContext.h>
#include <toki/level/AttributeLayer.h>
#include <toki/level/helpers.h>
#include <toki/level/LevelData.h>
#include <toki/AppGlobal.h>


#if defined(TT_BUILD_DEV)
//#define ENABLE_DEBUG_VISUALIZATION

#if defined(ENABLE_DEBUG_VISUALIZATION)
// FIXME: Remove these includes once debug rendering is no longer needed.
#include <toki/game/DebugView.h>
#include <toki/game/Game.h>
#endif
#endif


namespace toki {
namespace level {
namespace skin {


#if defined(ENABLE_DEBUG_VISUALIZATION)
static void registerQuadToDebugView(const tt::math::Vector2& p_min,
                                    const tt::math::Vector2& p_max,
                                    const tt::engine::renderer::ColorRGBA& p_color)
{
	// Render Edges
	toki::game::DebugView& debugView = AppGlobal::getGame()->getDebugView();
	
	toki::game::DebugView::LineInfo lineInfo(p_min, tt::math::Vector2(p_max.x, p_min.y), p_color, 5.0f);
	debugView.registerLine(lineInfo);
	
	lineInfo.sourcePosition.setValues(p_max.x, p_max.y);
	debugView.registerLine(lineInfo);
	
	lineInfo.targetPosition.setValues(p_min.x, p_max.y);
	debugView.registerLine(lineInfo);
	
	lineInfo.sourcePosition = p_min;
	debugView.registerLine(lineInfo);
}
#endif


void generateSkinShoebox(const SkinContextPtr&                      p_context,
                         const LevelDataPtr&                        p_level,
                         ThemeType                                  p_defaultLevelTheme,
                         const ThemeTiles&                          p_overriddenThemeTiles,
                         tt::engine::scene2d::shoebox::ShoeboxData* p_shoebox_OUT)
{
	//TT_Printf("toki::level::skin::generateSkinShoebox...\n");
	
#if defined(TT_PLATFORM_WIN) && defined(TT_BUILD_DEV)
	TT_ASSERT(_CrtCheckMemory());
#endif
	
#if !defined(TT_BUILD_FINAL)
	const u64 startTime = tt::system::Time::getInstance()->getMilliSeconds();
#endif
	
	TT_NULL_ASSERT(p_context);
	
	const AttributeLayerPtr& layer(p_level->getAttributeLayer());
	
	if (layer == 0 || layer->getWidth() <= 0 || layer->getHeight() <= 0)
	{
		TT_PANIC("Can't generate skin shoebox! AttributeLayer null or has a dimension of 0.");
		return;
	}
	
	const SkinConfigPtr& config      (AppGlobal::getSkinConfig(SkinConfigType_Solid));
	
	config      ->resetRandomSeed();
	p_context->tileMaterial.update(layer, p_defaultLevelTheme);
	
	// Apply overridden theme tiles
	for (ThemeTiles::const_iterator it = p_overriddenThemeTiles.begin();
	     it != p_overriddenThemeTiles.end(); ++it)
	{
		TileMaterial& material = p_context->tileMaterial.getTileMaterial((*it).first);
		MaterialTheme originalTheme = material.getMaterialTheme();
		
		// FIXME: Perhaps move this to a separate method
		switch (originalTheme)
		{
		case MaterialTheme_Rocks:     p_defaultLevelTheme = ThemeType_Rocks;      break;
		case MaterialTheme_Sand :     p_defaultLevelTheme = ThemeType_Sand;       break;
		case MaterialTheme_Beach:     p_defaultLevelTheme = ThemeType_Beach;      break;
		case MaterialTheme_DarkRocks: p_defaultLevelTheme = ThemeType_DarkRocks;  break;
		case MaterialTheme_None :     p_defaultLevelTheme = ThemeType_DoNotTheme; break;
		default:
			break;
		}
		
		material.set(CollisionType_Solid, (*it).second, p_defaultLevelTheme);
	}
	
	config      ->setPlaneColorsFromLevelData(p_level, SkinConfigType_Solid);
	
	p_context->edgeCache.update(p_context->tileMaterial);
	
	impl::generateShoebox(
			*config,
			p_context->tileMaterial,
			p_context->edgeCache,
			p_context->scratchGrowEdge,
			p_context->scratchGrowEdgeSize,
			p_context->blobData,
			p_shoebox_OUT);
	
#if !defined(TT_BUILD_FINAL)
	const u64 duration = tt::system::Time::getInstance()->getMilliSeconds() - startTime;
	TT_Printf("generateSkinShoebox duration: %u ms\n", static_cast<u32>(duration));
#endif
	
#if defined(TT_PLATFORM_WIN) && defined(TT_BUILD_DEV)
	TT_ASSERT(_CrtCheckMemory());
#endif
	
	
#if defined(ENABLE_DEBUG_VISUALIZATION)
	// Render Edges
	toki::game::DebugView& debugView = AppGlobal::getGame()->getDebugView();
	toki::game::DebugView::LineInfo lineInfo(tt::math::Vector2::zero, tt::math::Vector2(1000.f, 1000.0f),
	                                         tt::engine::renderer::ColorRGB::white, 5.0f);
	
	for (tt::math::Point2 pos(0,0); pos.y < p_context->tileMaterial.getSize().y; ++pos.y)
	{
		for (pos.x = 0; pos.x < p_context->tileMaterial.getSize().x; ++pos.x)
		{
			lineInfo.sourcePosition = tt::math::Vector2(pos);
			
			EdgeType type = p_context->edgeCache.getBottomEdge(pos);
			if (type != EdgeType_None)
			{
				lineInfo.targetPosition = tt::math::Vector2(lineInfo.sourcePosition.x + 1, lineInfo.sourcePosition.y);
				lineInfo.color = ((type & EdgeType_SolidBit) != 0) ?
				                 tt::engine::renderer::ColorRGB::green :
				                 tt::engine::renderer::ColorRGB::blue;
				debugView.registerLine(lineInfo);
			}
			type = p_context->edgeCache.getLeftEdge(pos);
			if (type != EdgeType_None)
			{
				lineInfo.targetPosition = tt::math::Vector2(lineInfo.sourcePosition.x, lineInfo.sourcePosition.y + 1);
				lineInfo.color = ((type & EdgeType_SolidBit) != 0) ? 
				                 tt::engine::renderer::ColorRGB::green :
				                 tt::engine::renderer::ColorRGB::blue;
				debugView.registerLine(lineInfo);
			}
		}
	}
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// impl namespace ("private")

namespace impl
{


struct OutsideLevel
{
public:
	enum Side
	{
		Side_Up,
		Side_Down,
		Side_Left,
		Side_Right
	};
	
	inline OutsideLevel(Side p_side)
	:
	side(p_side),
	startPoint(-1),
	material()
	{}
	
	inline void grow(const tt::math::Point2& p_pos, const TileMaterial& p_material, 
	                 const SkinConfig& p_config, tt::engine::scene2d::shoebox::ShoeboxData* p_shoebox_OUT)
	{
		if (material.isSolid()) // Was already growing
		{
			if (material == p_material)
			{
				// No change, continue.
				return;
			}
			
			// We detected the change a tile later. So start by moving back 1 tile.
			// In the case of a non-solid material we moved outside of the solid tiles,
			// we want to ignore the edges of solid tiles so move an extra tile in that case.
			s32 endPoint = (p_material.isSolid() == false) ? -2 : -1;
			
			switch (side)
			{
			case Side_Up:   // Fall-through, no break.
			case Side_Down:  endPoint += p_pos.x;   break;
			case Side_Left: // Fall-through, no break.
			case Side_Right: endPoint += p_pos.y;   break;
			default: TT_PANIC("Unknown side: %d", side); break;
			}
			
			if (startPoint <= endPoint)
			{
				tt::math::Point2 min(0, 0);
				tt::math::Point2 max(0, 0);
				
				switch (side)
				{
				case Side_Up:
					min.setValues(startPoint, p_pos.y + 1);
					max.setValues(endPoint,   p_pos.y + Constants_LevelExtensionSize);
					break;
				case Side_Down:
					min.setValues(startPoint, p_pos.y - Constants_LevelExtensionSize);
					max.setValues(endPoint,   p_pos.y - 1);
					break;
				case Side_Left:
					min.setValues(p_pos.x - Constants_LevelExtensionSize, startPoint);
					max.setValues(p_pos.x - 1,                            endPoint);
					break;
				case Side_Right:
					min.setValues(p_pos.x + 1,                            startPoint);
					max.setValues(p_pos.x + Constants_LevelExtensionSize, endPoint);
					break;
				default:
					TT_PANIC("Unknown side: %d", side);
					break;
				}
				addShoeboxPlaneQuad(min, max, material, p_config, p_shoebox_OUT);
			}
			
			startPoint = endPoint + 1;
			// End solid material or continue with new material.
			material = p_material;
		}
		else if (p_material.isSolid())
		{
			// Start 1 tile later because we ignore tiles with an edge.
			switch (side)
			{
			case Side_Up:   // Fall-through, no break.
			case Side_Down:  startPoint = p_pos.x + 1;   break;
			case Side_Left: // Fall-through, no break.
			case Side_Right: startPoint = p_pos.y + 1;   break;
			default: TT_PANIC("Unknown side: %d", side); break;
			}
			material = p_material;
		}
	}
	
private:
	Side         side;
	s32          startPoint;
	TileMaterial material;
};


void generateShoebox(const SkinConfig&                          p_config,
                     const MaterialCache&                       p_tiles,
                     const EdgeCache&                           p_edges,
                     impl::GrowEdge*                            p_edgeScratch,
                     s32                                        p_edgeScratchSize,
                     BlobData&                                  p_blobData,
                     tt::engine::scene2d::shoebox::ShoeboxData* p_shoebox_OUT)
{
	const tt::math::Point2& levelSize = p_tiles.getSize();
	const TileMaterial* tilePtr       = p_tiles.getRawTiles();
	
	// Make sure our scratch data is cleared.
	for (s32 i = 0; i < p_edgeScratchSize; ++i)
	{
		p_edgeScratch[i].invalidate();
	}
	
	// FIXME: Have a different resize and reset function for BlobData. (This is reset, in the context it should use resize.)
	p_blobData.reset(p_edgeScratchSize);
	
	impl::GrowEdge   horizontalEdge;
	
	const tt::math::Point2 rightStep(1, 0);
	const tt::math::Point2 upStep(   0, 1);
	
	OutsideLevel left( OutsideLevel::Side_Left );
	OutsideLevel right(OutsideLevel::Side_Right);
	
	// Check outside border level border
	// TODO: Stairs need another copy of this code to be added (With solid stairs -> stairs material change.)
	{
		{
			const tt::math::Point2 pos(0, 0);
			const TileMaterial& material = p_tiles.getTileMaterial(pos);
			if (material.isSolid())
			{
				left.grow(pos - upStep, material, p_config, p_shoebox_OUT);
				
				addShoeboxPlaneQuad(tt::math::Point2(-Constants_LevelExtensionSize, -Constants_LevelExtensionSize),
				                    tt::math::Point2(-1, -1),
				                    material, p_config, p_shoebox_OUT);
			}
		}
		{
			const tt::math::Point2 pos(levelSize.x - 1, 0);
			const TileMaterial& material = p_tiles.getTileMaterial(pos);
			if (material.isSolid())
			{
				right.grow(pos - upStep, material, p_config, p_shoebox_OUT);
				
				addShoeboxPlaneQuad(tt::math::Point2(levelSize.x, -Constants_LevelExtensionSize),
				                    tt::math::Point2(levelSize.x + Constants_LevelExtensionSize, -1),
				                    material, p_config, p_shoebox_OUT);
			}
		}
		{
			const TileMaterial& material = p_tiles.getTileMaterial(tt::math::Point2(levelSize.x - 1, levelSize.y - 1));
			if (material.isSolid())
			{
				addShoeboxPlaneQuad(tt::math::Point2(levelSize.x, levelSize.y),
				                    tt::math::Point2(levelSize.x + Constants_LevelExtensionSize, levelSize.y + Constants_LevelExtensionSize),
				                    material, p_config, p_shoebox_OUT);
			}
		}
		{
			const TileMaterial& material = p_tiles.getTileMaterial(tt::math::Point2(0, levelSize.y - 1));
			if (material.isSolid())
			{
				addShoeboxPlaneQuad(tt::math::Point2(-Constants_LevelExtensionSize, levelSize.y),
				                    tt::math::Point2(-1, levelSize.y + Constants_LevelExtensionSize),
				                    material, p_config, p_shoebox_OUT);
			}
		}
	}
	
	{
		OutsideLevel down(OutsideLevel::Side_Down);
		
		tt::math::Point2 pos(0, 0);
		const TileMaterial* downTilePtr = p_tiles.getRawTiles(pos);
		down.grow(pos - rightStep, *downTilePtr, p_config, p_shoebox_OUT);
		for (; pos.x < levelSize.x; ++pos.x, ++downTilePtr)
		{
#if defined(TT_BUILD_DEV) // Asserts to make sure this usage for the raw data is correct.
			TT_ASSERT(downTilePtr == &p_tiles.getTileMaterial(pos));
#endif
			down.grow(pos, *downTilePtr, p_config, p_shoebox_OUT);
		}
		down.grow(pos + rightStep, TileMaterial(), p_config, p_shoebox_OUT); // flush final tile.
	}
	
	for (tt::math::Point2 pos(0, 0); pos.y < levelSize.y; ++pos.y)
	{
		horizontalEdge.invalidate();
		pos.x = 0;
		
		left.grow(pos, *tilePtr, p_config, p_shoebox_OUT);
		
		for (; pos.x < levelSize.x; ++pos.x, ++tilePtr)
		{
#if defined(TT_BUILD_DEV) // Asserts to make sure this usage for the raw data is correct.
			TT_ASSERT(tilePtr           == &p_tiles.getTileMaterial(pos));
#endif
			impl::GrowEdge& verticalEdge = p_edgeScratch[pos.x];
			
			if (tilePtr->isSolid())
			{
				u8 shapeBits = TileSideBit_None;
				if ((p_edges.getLeftEdge(pos)  & EdgeType_SolidBit) != 0)
				{
					shapeBits |= TileSideBit_Left;
					
					const tt::math::Point2 tileToLeft(pos.x - 1, pos.y);
					TT_ASSERT(p_tiles.getTileMaterial(tileToLeft).isSolid() == false);
					checkForInsideCornersRight(tt::math::Point2(tileToLeft), p_tiles,
					                           p_edges, p_config, p_shoebox_OUT);
				}
				if ((p_edges.getRightEdge(pos)  & EdgeType_SolidBit) != 0)
				{
					shapeBits |= TileSideBit_Right;
				}
				if ((p_edges.getBottomEdge(pos) & EdgeType_SolidBit) != 0)
				{
					shapeBits |= TileSideBit_Bottom;
				}
				if ((p_edges.getTopEdge(pos)    & EdgeType_SolidBit) != 0)
				{
					shapeBits |= TileSideBit_Top;
				}
				
				const Shape shape = static_cast<Shape>(shapeBits);
				
				addShoeboxPlane(p_config, shape, pos, *tilePtr, p_shoebox_OUT);
				
				// Only add center edge to tile without sides. (and split on theme.)
				doGrowBlobData(p_blobData, pos,
						(shapeBits != 0 && (tilePtr->getMaterialTheme() != MaterialTheme_Crystal)) ?
						0 : tilePtr->getRawValue());
				
				// Check the growing edges.
				
				// Horizontal grow edge
				doGrowEdge(shape, *tilePtr, TileSideBit_HorizontalMask,
				           horizontalEdge, pos, rightStep, p_config, p_shoebox_OUT);
				
				// Vertical grow edge
				doGrowEdge(shape, *tilePtr, TileSideBit_VerticalMask,
				           verticalEdge,   pos, upStep, p_config, p_shoebox_OUT);
			}
			// Outside collision
			else
			{
				// Only add quad when it's death.
				doGrowBlobData(p_blobData, pos, 0 );
				
				if ((p_edges.getLeftEdge(pos) & EdgeType_SolidBit) != 0)
				{
					TT_ASSERT(p_tiles.getTileMaterial(pos).isSolid() == false);
					checkForInsideCornersLeft(tt::math::Point2(pos), p_tiles,
					                          p_edges, p_config, p_shoebox_OUT);
				}
				
				horizontalEdge.stop(true, pos, rightStep, p_config, p_shoebox_OUT);
				verticalEdge.stop(  true, pos, upStep, p_config, p_shoebox_OUT);
			}
			
		}
		
		// Flush remaining grow edges at the (Right) edge of level.
		horizontalEdge.stopAtLevelBorder(      pos, rightStep, p_config, p_shoebox_OUT);
		
		const tt::math::Point2 rightInside(pos.x - 1, pos.y);
		right.grow(rightInside, p_tiles.getTileMaterial(rightInside), p_config, p_shoebox_OUT);
	}
	
	// Flush final tiles.
	left.grow( tt::math::Point2(0,               levelSize.y + 1), TileMaterial(), p_config, p_shoebox_OUT);
	right.grow(tt::math::Point2(levelSize.x - 1, levelSize.y + 1), TileMaterial(), p_config, p_shoebox_OUT);
	
	{
		OutsideLevel up(OutsideLevel::Side_Up);
		
		tt::math::Point2 pos(0, levelSize.y - 1);
		const TileMaterial* upTilePtr = p_tiles.getRawTiles(pos);
		up.grow(pos - rightStep, *upTilePtr, p_config, p_shoebox_OUT);
		for (; pos.x < levelSize.x; ++pos.x, ++upTilePtr)
		{
#if defined(TT_BUILD_DEV) // Asserts to make sure this usage for the raw data is correct.
			TT_ASSERT(upTilePtr == &p_tiles.getTileMaterial(pos));
#endif
			up.grow(pos, *upTilePtr, p_config, p_shoebox_OUT);
		}
		up.grow(pos + rightStep, TileMaterial(), p_config, p_shoebox_OUT); // flush final tile.
	}
	
	// Flush remaining grow edges at the (Top) edge of level.
	for (s32 i = 0; i < levelSize.x; ++i)
	{
		p_edgeScratch[i].stopAtLevelBorder(      tt::math::Point2(i, levelSize.y), upStep,
		                                         p_config, p_shoebox_OUT);
		
		doGrowBlobDataFlush(p_blobData,       i, levelSize.y);
	}
	
	// Check for Right Top tile.
	p_blobData.checkForQuadMerge(      levelSize.x - 1, levelSize.x - 1, -1);
	
	// --------------------------------------------------------------------------------------------
	// Add center planes based on quads.
	addShoeboxPlanesFromBlobData(p_blobData      , p_config,       p_shoebox_OUT);
}


void addShoeboxPlanesFromBlobData(const BlobData&                            p_blobData,
                                  const SkinConfig&                          p_config,
                                  tt::engine::scene2d::shoebox::ShoeboxData* p_shoebox_OUT)
{
	const BlobData& blobData = p_blobData;
	for (BlobData::Blobs::const_iterator blobIt = blobData.allBlobs.begin();
	     blobIt != blobData.allBlobs.end(); ++blobIt)
	{
		/*
		registerQuadToDebugView(tt::math::Vector2(blobIt->getMin().x + 0.1f,
		                                          blobIt->getMin().y + 0.1f), 
		                        tt::math::Vector2(blobIt->getMax().x + 0.9f,
		                                          blobIt->getMax().y + 0.9f),
		                        tt::engine::renderer::ColorRGB::blue);
		// */
		
		const SubQuads& quads = blobIt->getSubQuads();
		for (SubQuads::const_iterator it = quads.begin(); it != quads.end(); ++it)
		{
			const SubQuad& subQuad = (*it);
			const TileMaterial tileMaterial = TileMaterial::createFromRaw(subQuad.type);
			addShoeboxPlaneQuad(subQuad.min, subQuad.max, tileMaterial, p_config, p_shoebox_OUT);
		}
	}
}


void doGrowEdge(Shape p_shape, const TileMaterial& p_material,
                TileSideBit p_toCheckMask, GrowEdge& p_growEdge,
                const tt::math::Point2& p_pos, const tt::math::Point2& p_step,
                const SkinConfig&                          p_config,
                tt::engine::scene2d::shoebox::ShoeboxData* p_shoebox_OUT)
{
	if ((p_shape & p_toCheckMask) != 0)
	{
		// We have a edges
		
		// Use mask so corners become edges and tips become lines.
		const Shape edgeShape = static_cast<Shape>(p_shape & p_toCheckMask);
		
		// Check for changing (grow) edge.
		if (p_growEdge.isValid()) // We had an edge going.
		{
#if !TT_SINGLE_SIDE_EDGES
			// Check if we have a shape waiting to be added.
			if (p_growEdge.previousTileShape != Shape_None)
			{
				// Which sides continue from previous tile to this one.
				const Shape previousTileShapeValid = static_cast<Shape>(p_growEdge.previousTileShape & edgeShape);
				
				if (p_growEdge.shape == Shape_None) // previous tile is the first shape to be added.
				{
					// The valid part of previous tile becomes shape type.
					p_growEdge.shape             = previousTileShapeValid;
					p_growEdge.previousTileShape = Shape_None;
				}
				else if (p_growEdge.shape == previousTileShapeValid)
				{
					// Previous tile may be added to grow edge.
					p_growEdge.previousTileShape = Shape_None;
				}
				else // The grow edge shape differs from previousTileShapeValid.
				{
					p_growEdge.stopAndContinue(p_shape, p_material, p_pos, p_step, previousTileShapeValid,
					                           p_config, p_shoebox_OUT);
					return;
				}
			}
#endif
			
			if (p_growEdge.shape != edgeShape) // Did we change type?
			{
				// From edge to line or top/left became bottom/right.
				// In this case we want to glue at half tiles on previous tile and continue 
#if TT_SINGLE_SIDE_EDGES
				p_growEdge.stop(false, p_pos, p_step, p_config, p_shoebox_OUT);
				p_growEdge.start(edgeShape, p_material, false, p_pos);
#else
				++p_growEdge.distance;
				
				p_growEdge.previousTileShape = edgeShape;
				
				if (isEndingWithOutside(p_shape, p_toCheckMask))
				{
					p_growEdge.stop(true, p_pos + p_step, p_step, p_config, p_shoebox_OUT);
				}
#endif
				return;
			}
			else
			{
				++p_growEdge.distance;
				
				if (p_growEdge.material != p_material) // Did we change Material?
				{
					// Stop old vertical grow edge
					p_growEdge.endHalf  = false; // We didn't change same, glue full tile
					addShoeboxPlane(p_config, p_growEdge, p_shoebox_OUT);
					
					// Continue with new one.
					p_growEdge.shape     = edgeShape;
					p_growEdge.material  = p_material;
					p_growEdge.startHalf = false;
					p_growEdge.startTile = p_pos;
					p_growEdge.distance  = 0;
				}
			}
		}
		else // We didn't have an edge going.
		{
			// Start new one.
			if (p_pos.x - p_step.x < 0 ||
			    p_pos.y - p_step.y < 0) // If we take a step back we exit the level.
			{
				p_growEdge.startAtLevelBorder(edgeShape, p_material,       p_pos, p_step);
			}
			else
			{
				const bool startWithhalf = isStartingWithOutside(p_shape, p_toCheckMask);
#if TT_SINGLE_SIDE_EDGES
				// Not start an edge that's already ended. (Starts and Ends with half on same tile.)
				if (startWithhalf == false || isEndingWithOutside(p_shape, p_toCheckMask) == false)
#endif
				{
					p_growEdge.start(edgeShape, p_material, startWithhalf, p_pos);
				}
			}
		}
	}
	// No more edge, (but still inside collision)
	else if (p_growEdge.isValid())
	{
		// Stop old vertical grow edge
		p_growEdge.stop(false, p_pos, p_step, p_config, p_shoebox_OUT);
	}
}


void checkForInsideCornersRight(const tt::math::Point2& p_pos, const MaterialCache& p_tiles,
                                const EdgeCache& p_edges, const SkinConfig& p_config,
                                tt::engine::scene2d::shoebox::ShoeboxData* p_shoebox_OUT)
{
	TT_ASSERT((p_edges.getRightEdge(p_pos) & EdgeType_SolidBit) != 0);
	
	if ((p_edges.getTopEdge(p_pos) & EdgeType_SolidBit) != 0)
	{
		const tt::math::Point2 insidePos(p_pos.x + 1, p_pos.y + 1);
		const TileMaterial& insideMat = p_tiles.getTileMaterial(insidePos);
		if (insideMat.isSolid())
		{
			addShoeboxPlane(p_config, Shape_CornerInsideTopRight, insidePos, insideMat, p_shoebox_OUT);
		}
		else
		{
			TT_ASSERT((p_edges.getLeftEdge(  insidePos) & EdgeType_SolidBit) != 0);
			TT_ASSERT((p_edges.getBottomEdge(insidePos) & EdgeType_SolidBit) != 0);
			//TT_Printf("Diagonal connection bottom\n"); // FIXME: TODO
		}
	}
	if ((p_edges.getBottomEdge(p_pos) & EdgeType_SolidBit) != 0)
	{
		const tt::math::Point2 insidePos(p_pos.x + 1, p_pos.y - 1);
		const TileMaterial& insideMat = p_tiles.getTileMaterial(insidePos);
		if (insideMat.isSolid())
		{
			addShoeboxPlane(p_config, Shape_CornerInsideBottomRight, insidePos, insideMat, p_shoebox_OUT);
		}
		else
		{
			TT_ASSERT((p_edges.getLeftEdge(insidePos) & EdgeType_SolidBit) != 0);
			TT_ASSERT((p_edges.getTopEdge( insidePos) & EdgeType_SolidBit) != 0);
			//TT_Printf("Diagonal connection top\n"); // FIXME: TODO
		}
	}
}


void checkForInsideCornersLeft(const tt::math::Point2& p_pos, const MaterialCache& p_tiles,
                               const EdgeCache& p_edges, const SkinConfig& p_config,
                               tt::engine::scene2d::shoebox::ShoeboxData* p_shoebox_OUT)
{
	TT_ASSERT((p_edges.getLeftEdge(p_pos) & EdgeType_SolidBit) != 0);
	
	if ((p_edges.getTopEdge(p_pos) & EdgeType_SolidBit) != 0)
	{
		const tt::math::Point2 insidePos(p_pos.x - 1, p_pos.y + 1);
		const TileMaterial& insideMat = p_tiles.getTileMaterial(insidePos);
		if (insideMat.isSolid())
		{
			addShoeboxPlane(p_config, Shape_CornerInsideTopLeft, insidePos, insideMat, p_shoebox_OUT);
		}
	}
	if ((p_edges.getBottomEdge(p_pos) & EdgeType_SolidBit) != 0)
	{
		const tt::math::Point2 insidePos(p_pos.x - 1, p_pos.y - 1);
		const TileMaterial& insideMat = p_tiles.getTileMaterial(insidePos);
		if (insideMat.isSolid())
		{
			addShoeboxPlane(p_config, Shape_CornerInsideBottomLeft, insidePos, insideMat, p_shoebox_OUT);
		}
	}
}



void addShoeboxPlane(const SkinConfig&                          p_config,
                     Shape                                      p_shape,
                     const tt::math::Point2&                    p_tile,
                     const TileMaterial&                        p_material,
                     tt::engine::scene2d::shoebox::ShoeboxData* p_shoebox_OUT)
{
#if defined(TT_BUILD_DEV)
	/*
	if (p_shape != Shape_None           &&
	    p_shape != Shape_EdgeTop        &&
	    p_shape != Shape_EdgeBottom     &&
	    p_shape != Shape_EdgeLeft       &&
	    p_shape != Shape_EdgeRight      &&
	    p_shape != Shape_LineHorizontal &&
	    p_shape != Shape_LineVertical)
	{
		TT_Printf("addShoeboxPlane: (%d, %d) - shape: '%s'\n", p_tile.x, p_tile.y, getShapeName(p_shape));
	}
	// */
#endif
	
	static const tt::math::Vector2 halfTile(tileToWorld(tt::math::Point2::allOne) * 0.5f);
	
	const SkinConfig::PlaneSet& planeSet(p_config.getPlanes(p_material, p_shape));
	
	namespace sb = tt::engine::scene2d::shoebox;
	
	for (SkinConfig::Planes::const_iterator it = planeSet.planes.begin(); it != planeSet.planes.end(); ++it)
	{
		const SkinConfig::Plane& cfgPlane(*it);
		sb::PlaneData plane(cfgPlane.planeData);
		
		const tt::math::Vector2 worldPos(tileToWorld(p_tile) + halfTile);
		plane.position.x += worldPos.x;
		plane.position.y += worldPos.y;
		plane.position.y = -plane.position.y;
		
		p_shoebox_OUT->planes.push_back(plane);
	}
}


void addShoeboxPlane(const SkinConfig&                          p_config,
                     const EdgeShape&                           p_edge,
                     tt::engine::scene2d::shoebox::ShoeboxData* p_shoebox_OUT)
{
#if defined(TT_BUILD_DEV)
	/*
	TT_Printf("addShoeboxPlane: start (%d, %d) dist %d startH %d endH %d "
	          "mat type %d theme %d (%d) shape '%s'.\n",
	          p_edge.startTile.x, p_edge.startTile.y, p_edge.distance, p_edge.startHalf, p_edge.endHalf,
	          p_edge.material->getMaterialType(), p_edge.material->getMaterialTheme(), p_edge.material->isSolid(),
	          getShapeName(p_edge.shape));
	// */
#endif
	
	TT_ASSERT(p_edge.distance > 0);
	TT_ASSERT(p_edge.isValid());
	
#if defined(ENABLE_DEBUG_VISUALIZATION)
	
	// Render Edges
	tt::math::Vector2 startVec(p_edge.startTile);
	tt::math::Vector2 endVec(startVec); 
	
	// Make sure we don't have both axis. (one must be false.)
	TT_ASSERT((p_edge.shape & TileSideBit_HorizontalMask) == 0 ||
	          (p_edge.shape & TileSideBit_VerticalMask  ) == 0 );
	
	if ((p_edge.shape & TileSideBit_HorizontalMask) != 0)
	{
		// Horizontal shape
		if (p_edge.startHalf)
		{
			startVec.x += 0.5f;
		}
		endVec.x += p_edge.distance - 0.2f;
		if (p_edge.endHalf)
		{
			endVec.x -= 0.5f;
		}
	}
	else
	{
		// Vertical shape
		if (p_edge.startHalf)
		{
			startVec.y += 0.5f;
		}
		endVec.y += p_edge.distance - 0.2f;
		if (p_edge.endHalf)
		{
			endVec.y -= 0.5f;
		}
	}
	
	if ((p_edge.shape & TileSideBit_Bottom) != 0 ||
	    (p_edge.shape & TileSideBit_Left  ) != 0)
	{
		toki::game::DebugView::LineInfo lineInfo(startVec + tt::math::Vector2(0.1f, 0.1f), 
		                                         endVec   + tt::math::Vector2(0.1f, 0.1f), 
		                                         tt::engine::renderer::ColorRGB::red, 5.0f);
		AppGlobal::getGame()->getDebugView().registerLine(lineInfo);
	}
	if ((p_edge.shape & TileSideBit_Top  ) != 0 ||
	    (p_edge.shape & TileSideBit_Right) != 0)
	{
		const tt::math::Vector2 otherSideOffset( ((p_edge.shape & TileSideBit_Top  ) != 0) ? 0.0f : 0.8f,
		                                         ((p_edge.shape & TileSideBit_Top  ) != 0) ? 0.8f : 0.0f);
		
		toki::game::DebugView::LineInfo lineInfo(startVec + tt::math::Vector2(0.1f, 0.1f) + otherSideOffset, 
		                                         endVec   + tt::math::Vector2(0.1f, 0.1f) + otherSideOffset, 
		                                         tt::engine::renderer::ColorRGB::red, 5.0f);
		AppGlobal::getGame()->getDebugView().registerLine(lineInfo);
	}
#endif
	
	static const tt::math::Vector2 halfTile(tileToWorld(tt::math::Point2::allOne) * 0.5f);
	
	tt::math::Vector2 edgeMinPos(tileToWorld(p_edge.startTile));
	tt::math::Vector2 edgeMaxPos(edgeMinPos);
	
	bool horizontal = (p_edge.shape & TileSideBit_HorizontalMask) != 0;
	
#if defined(TT_BUILD_DEV)
	switch (p_edge.shape)
	{
	case Shape_LineHorizontal:
	case Shape_EdgeTop:
	case Shape_EdgeBottom:
		TT_ASSERT(horizontal);
		break;
		
	case Shape_LineVertical:
	case Shape_EdgeLeft:
	case Shape_EdgeRight:
		TT_ASSERT(horizontal == false);
		break;
		
	default:
		TT_PANIC("Unsupported edge shape: %d", p_edge.shape);
		return;
	}
#endif
	
	const SkinConfig::PlaneSet& planeSet(p_config.getEdgePlanes(p_edge.material, p_edge.shape));
	
	if (horizontal)
	{
		edgeMaxPos.y += tileToWorld(1);  // edges are 1 tile thick
		
		if (p_edge.startHalf)
		{
			edgeMinPos.x += planeSet.beginHalfTile;
		}
		
		edgeMaxPos.x += tileToWorld(p_edge.distance);
		if (p_edge.endHalf)
		{
			edgeMaxPos.x -= planeSet.endHalfTile;
		}
	}
	else
	{
		edgeMaxPos.x += tileToWorld(1);  // edges are 1 tile thick
		
		if (p_edge.startHalf)
		{
			edgeMinPos.y += planeSet.beginHalfTile;
		}
		
		edgeMaxPos.y += tileToWorld(p_edge.distance);
		if (p_edge.endHalf)
		{
			edgeMaxPos.y -= planeSet.endHalfTile;
		}
	}
	TT_ASSERT(edgeMinPos.x <= edgeMaxPos.x);
	TT_ASSERT(edgeMinPos.y <= edgeMaxPos.y);
	
	const real              edgeLength = horizontal ? (edgeMaxPos.x - edgeMinPos.x) : (edgeMaxPos.y - edgeMinPos.y);
	const tt::math::Vector2 planeCenterPos(edgeMinPos + ((edgeMaxPos - edgeMinPos) * 0.5f));
	const real              randomUOffset = (p_edge.startTile.x % planeSet.anchorCount) / static_cast<real>(planeSet.anchorCount);
	
	for (SkinConfig::Planes::const_iterator it = planeSet.planes.begin(); it != planeSet.planes.end(); ++it)
	{
		const SkinConfig::Plane& cfgPlane(*it);
		tt::engine::scene2d::shoebox::PlaneData plane(cfgPlane.planeData);
		
		const real texWorldWidth  = plane.width  / plane.texBottomRightU;
		const real texWorldHeight = plane.height / plane.texBottomRightV;
		
		bool rotateUV = (horizontal == false);
		
		if (horizontal)
		{
			plane.width = edgeLength;
		}
		else
		{
			plane.width  = plane.height; // HACK
			plane.height = edgeLength;
			plane.rotation = 0.0f; // HACK
		}
		
		plane.position.x += planeCenterPos.x;
		plane.position.y += planeCenterPos.y;
		
		const tt::math::Vector2 planeMinPos(plane.position.x - (plane.width  * 0.5f),
		                                    plane.position.y - (plane.height * 0.5f));
		const tt::math::Vector2 planeMaxPos(plane.position.x + (plane.width  * 0.5f),
		                                    plane.position.y + (plane.height * 0.5f));
		
		// Flip the plane position's Y axis so that the position is in shoebox space
		plane.position.y = -plane.position.y;
		
		// Calculate the UV coordinates for this plane
		real minU = plane.texTopLeftU;
		real minV = plane.texTopLeftV;
		
		real maxU = plane.texBottomRightU;
		real maxV = plane.texBottomRightV;
		
		// Fiddle U
		switch (cfgPlane.texCoordModeU)
		{
		case SkinConfig::Plane::TexCoordMode_DoNotModify:
			break;
			
		case SkinConfig::Plane::TexCoordMode_Edge:
			// Calculate the UV coordinates for this plane
			minU = 0.0f;
			maxU = edgeLength / texWorldWidth;
			
			// Snap the max U coordinate to an "anchor point" in the texture
			if (planeSet.snapToUVAnchor)
			{
				s32  integerPart = static_cast<s32>(maxU);
				real remainder   = maxU - integerPart;
				
				// FIXME: This should take planeSet.anchorCount into account
				if (integerPart > 0 && remainder < 0.13f)
				{
					remainder = 0.0f;
				}
				else if (remainder < 0.38f)
				{
					remainder = 0.25f;
				}
				else if (remainder < 0.63f)
				{
					remainder = 0.5f;
				}
				else if (remainder < 0.88f)
				{
					remainder = 0.75f;
				}
				else
				{
					remainder = 1.0f;
				}
				
				maxU = static_cast<real>(integerPart) + remainder;
			}
			
			if (planeSet.randomizeU)
			{
				minU += randomUOffset;
				maxU += randomUOffset;
			}
			break;
			
		case SkinConfig::Plane::TexCoordMode_UseWorldU:
		case SkinConfig::Plane::TexCoordMode_UseWorldU_RandomOffset:
			{
				TT_ASSERT(tt::math::realEqual(plane.texTopRightU, plane.texBottomRightU));
				
				const real uWidth = plane.width / texWorldWidth;
				
				minU = planeMinPos.x / texWorldWidth;
				if (cfgPlane.texCoordModeU == SkinConfig::Plane::TexCoordMode_UseWorldU_RandomOffset)
				{
					// FIXME: How to "randomize" this?
					minU += static_cast<real>(p_edge.startTile.y) / 5.0f;
				}
				maxU = minU + uWidth;
				
				rotateUV = false;
			}
			break;
			
		case SkinConfig::Plane::TexCoordMode_UseWorldV:
		case SkinConfig::Plane::TexCoordMode_UseWorldV_RandomOffset:
			{
				TT_ASSERT(tt::math::realEqual(plane.texBottomLeftV, plane.texBottomRightV));
				
				const real vWidth = plane.height / texWorldWidth;
				
				minU = planeMinPos.y / texWorldWidth;
				if (cfgPlane.texCoordModeU == SkinConfig::Plane::TexCoordMode_UseWorldV_RandomOffset)
				{
					// FIXME: How to "randomize" this?
					minU += randomUOffset; //static_cast<real>(p_edge.startTile.x) / 5.0f;
				}
				maxU = minU + vWidth;
			}
			break;
			
		default: break;
		}
		
		// Fiddle V
		switch (cfgPlane.texCoordModeV)
		{
		case SkinConfig::Plane::TexCoordMode_DoNotModify:
			break;
			
			/* FIXME: WRONG!
		case SkinConfig::Plane::TexCoordMode_UseWorldU:
		case SkinConfig::Plane::TexCoordMode_UseWorldU_RandomOffset:
			{
				TT_ASSERT(tt::math::realEqual(plane.texTopRightU, plane.texBottomRightU));
				
				const real uWidth = plane.width / texWorldWidth;
				
				minV = planeMinPos.x / texWorldWidth;
				maxV = minV + uWidth;
			}
			break;
			*/
			
		case SkinConfig::Plane::TexCoordMode_UseWorldV:
		case SkinConfig::Plane::TexCoordMode_UseWorldV_RandomOffset:
			{
				TT_ASSERT(tt::math::realEqual(plane.texBottomLeftV, plane.texBottomRightV));
				
				const real vHeight = plane.height / texWorldHeight;
				
				minV = -planeMaxPos.y / texWorldHeight;
				if (cfgPlane.texCoordModeV == SkinConfig::Plane::TexCoordMode_UseWorldV_RandomOffset)
				{
					// FIXME: How to "randomize" this?
					minV += static_cast<real>(p_edge.startTile.y) / 5.0f;
				}
				maxV = minV + vHeight;
				
				rotateUV = false;
			}
			break;
			
		default: break;
		}
		
		
		if (rotateUV)
		{
			plane.texTopLeftU     = maxU;
			plane.texTopLeftV     = minV;
			plane.texTopRightU    = maxU;
			plane.texTopRightV    = maxV;
			plane.texBottomRightU = minU;
			plane.texBottomRightV = maxV;
			plane.texBottomLeftU  = minU;
			plane.texBottomLeftV  = minV;
		}
		else
		{
			plane.texTopLeftU     = minU;
			plane.texTopLeftV     = minV;
			plane.texTopRightU    = maxU;
			plane.texTopRightV    = minV;
			plane.texBottomRightU = maxU;
			plane.texBottomRightV = maxV;
			plane.texBottomLeftU  = minU;
			plane.texBottomLeftV  = maxV;
		}
		
		p_shoebox_OUT->planes.push_back(plane);
	}
}


void addShoeboxPlaneQuad(const tt::math::Point2& p_min, const tt::math::Point2& p_max,
                         const TileMaterial& p_material, const SkinConfig& p_config,
                         tt::engine::scene2d::shoebox::ShoeboxData* p_shoebox_OUT)
{
	
#if defined(ENABLE_DEBUG_VISUALIZATION)
	MaterialTheme theme = p_material.getMaterialTheme();
	MaterialType  type  = p_material.getMaterialType();
	
	tt::engine::renderer::ColorRGB color = tt::engine::renderer::ColorRGB::magenta;
	
	if (theme == MaterialTheme_None || type  == MaterialType_None)
	{
		return;
	}
	
	switch (theme)
	{
	case MaterialTheme_Sand:
		color = (type == MaterialType_Solid) ? tt::engine::renderer::ColorRGB::red : tt::engine::renderer::ColorRGB::orange;
		break;
	case MaterialTheme_Rocks:
		color = (type == MaterialType_Solid) ? tt::engine::renderer::ColorRGB::blue : tt::engine::renderer::ColorRGB::green;
		break;
	case MaterialTheme_Beach:
		color = (type == MaterialType_Solid) ? tt::engine::renderer::ColorRGB(127, 0, 0) : tt::engine::renderer::ColorRGB(127, 63, 0);
		break;
	case MaterialTheme_DarkRocks:
		color = (type == MaterialType_Solid) ? tt::engine::renderer::ColorRGB(0, 0, 127) : tt::engine::renderer::ColorRGB(0, 127, 0);
		break;
	case MaterialTheme_Crystal:
		color = (type == MaterialType_Solid) ? tt::engine::renderer::ColorRGB::white : tt::engine::renderer::ColorRGB::yellow;
		break;
	default:
		TT_PANIC("Unknown theme: %d\n", theme);
		break;
	}
	
	if (type == MaterialType_Death)
	{
		// FIXME: Maybe we should support death for all themes?
		color = tt::engine::renderer::ColorRGB::cyan;
	}
	
	registerQuadToDebugView(tt::math::Vector2(p_min.x + 0.2f,
	                                          p_min.y + 0.2f), 
	                        tt::math::Vector2(p_max.x + 0.8f,
	                                          p_max.y + 0.8f),
	                        color);
#endif
	
	// Make planes overlap a tiny bit, to prevent 'lines' or 'holes' in the generated skin
	static const tt::math::Vector2 overlapOffset(0.01f, 0.01f);
	
	const tt::math::Vector2 minPos(tileToWorld(p_min)                            - overlapOffset);
	const tt::math::Vector2 maxPos(tileToWorld(p_max + tt::math::Point2::allOne) + overlapOffset);
	const tt::math::Vector2 planeTargetSize(maxPos - minPos);
	const tt::math::Vector2 planeCenterPos(minPos + (planeTargetSize * 0.5f));
	
	const SkinConfig::PlaneSet& planeSet(p_config.getCenterPlanes(p_material));
	
	for (SkinConfig::Planes::const_iterator it = planeSet.planes.begin(); it != planeSet.planes.end(); ++it)
	{
		const SkinConfig::Plane& cfgPlane(*it);
		tt::engine::scene2d::shoebox::PlaneData plane(cfgPlane.planeData);
		
		TT_ASSERT(tt::math::realEqual(plane.texTopRightU,   plane.texBottomRightU));
		TT_ASSERT(tt::math::realEqual(plane.texBottomLeftV, plane.texBottomRightV));
		const real texWorldWidth  = plane.width  / plane.texBottomRightU;
		const real texWorldHeight = plane.height / plane.texBottomRightV;
		
		const real uWidth  = planeTargetSize.x / texWorldWidth;
		const real vHeight = planeTargetSize.y / texWorldHeight;
		
		const real minU =  minPos.x / texWorldWidth;
		const real minV = -maxPos.y / texWorldHeight;
		const real maxU = minU + uWidth;
		const real maxV = minV + vHeight;
		
		plane.width       = planeTargetSize.x;
		plane.height      = planeTargetSize.y;
		plane.position.x += planeCenterPos.x;
		plane.position.y += planeCenterPos.y;
		plane.position.y = -plane.position.y;
		
		plane.texTopLeftU     = minU;
		plane.texTopLeftV     = minV;
		plane.texTopRightU    = maxU;
		plane.texTopRightV    = minV;
		plane.texBottomLeftU  = minU;
		plane.texBottomLeftV  = maxV;
		plane.texBottomRightU = maxU;
		plane.texBottomRightV = maxV;
		
		p_shoebox_OUT->planes.push_back(plane);
	}
}


// impl namespace end
}

// Namespace end
}
}
}
