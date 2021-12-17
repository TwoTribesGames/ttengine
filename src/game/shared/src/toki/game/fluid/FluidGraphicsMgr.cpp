#include <tt/cfg/Handle.h>
#include <tt/engine/debug/DebugRenderer.h>
#include <tt/engine/renderer/MatrixStack.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/engine/renderer/Texture.h>
#include <tt/engine/renderer/QuadBuffer.h>
#include <tt/math/math.h>
#include <tt/platform/tt_printf.h>
#include <tt/str/toStr.h>

#include <toki/game/Camera.h>
#include <toki/game/Game.h>
#include <toki/game/fluid/FluidGraphicsMgr.h>
#include <toki/game/fluid/FluidParticlesMgr.h>
#include <toki/level/AttributeLayer.h>
#include <toki/level/TileRegistrationMgr.h>
#include <toki/level/helpers.h>
#include <toki/AppGlobal.h>
#include <toki/cfg.h>


namespace toki {
namespace game {
namespace fluid {


static const s32 maxFallQuads       = 1024;
static const s32 maxFlowMiddleQuads = 4096;
static const s32 maxStillQuads      = 2048;

static const real levelExtension = 0.25f;

// TODO
// * Fall borders in 1 texture / batch
// * Don't need lava fall back layer
// * Remove old buffers


//--------------------------------------------------------------------------------------------------
// Public member functions

FluidGraphicsMgr::FluidGraphicsMgr()
:
m_enabled(true),
m_debugRenderEnabled(false),
m_waveGenerator(),
m_drainLevel(0.0f),
m_animationTime(0.0f),
m_originalFogSetting(false),
m_graphicsNeedUpdate(true),
m_stillBufferNeedsUpdate(true)
{
	// Get all textures
	for (s32 i = 0; i < QuadType_Count; ++i)
	{
		QuadType quadType = static_cast<QuadType>(i);
		
		const bool texUNeedsWrap =
			quadType != QuadType_WaterFallEdgeLeft  &&
			quadType != QuadType_WaterFallEdgeRight &&
			quadType != QuadType_LavaFallEdgeLeft   &&
			quadType != QuadType_LavaFallEdgeRight;
		
		const bool texVNeedsWrap =
			quadType == QuadType_WaterFall               ||
			quadType == QuadType_Bg_WaterFall            ||
			quadType == QuadType_Bg_LavaFall             ||
			quadType == QuadType_WaterFallEdgeLeft       ||
			quadType == QuadType_WaterFallEdgeRight      ||
			quadType == QuadType_LavaFall                ||
			quadType == QuadType_LavaFallEdgeLeft        ||
			quadType == QuadType_LavaFallEdgeRight;
		
		using namespace tt::engine::renderer;
		m_fluidTextures[i] = TextureCache::get(getQuadTypeName(quadType), "textures.fluids", true);
		m_fluidTextures[i]->setAddressModeU(texUNeedsWrap ? AddressMode_Wrap : AddressMode_Clamp);
		m_fluidTextures[i]->setAddressModeV(texVNeedsWrap ? AddressMode_Wrap : AddressMode_Clamp);
	}

	// Create all vertex buffers
	for(s32 i = 0; i < FluidType_Count; ++i)
	{
		using namespace tt::engine::renderer;
		TexturePtr texture =
			(i == FluidType_Water) ? m_fluidTextures[QuadType_WaterFall] : m_fluidTextures[QuadType_LavaFall];

		m_fallBodyBuffer[i].reset(new QuadBuffer(maxFallQuads, texture, BatchFlagQuad_UseVertexColor));

		texture = (i == FluidType_Water) ?
			m_fluidTextures[QuadType_WaterFallEdgeLeft] : m_fluidTextures[QuadType_LavaFallEdgeLeft];

		m_fallLeftBorderBuffer [i].reset(new QuadBuffer(maxFallQuads, texture, BatchFlagQuad_UseVertexColor));

		texture = (i == FluidType_Water) ?
			m_fluidTextures[QuadType_WaterFallEdgeRight] : m_fluidTextures[QuadType_LavaFallEdgeRight];

		m_fallRightBorderBuffer[i].reset(new QuadBuffer(maxFallQuads, texture, BatchFlagQuad_UseVertexColor));


		texture = (i == FluidType_Water) ?
			m_fluidTextures[QuadType_WaterPoolLeft] : m_fluidTextures[QuadType_LavaPoolLeft];

		m_flowMiddleBuffer[i]     .reset(new QuadBuffer(maxFlowMiddleQuads, texture, BatchFlagQuad_UseVertexColor));
		m_flowStillMiddleBuffer[i].reset(new QuadBuffer(maxFlowMiddleQuads, texture, BatchFlagQuad_UseVertexColor));
		
		m_stillFluidsBuffer[i].reset(new QuadBuffer(maxStillQuads, texture, BatchFlagQuad_UseVertexColor));
	}

	// Prepare glow texture
	{
		using namespace tt::engine::renderer;
		TexturePtr texture = TextureCache::get("white_glow", "textures.backgrounds", true);
		m_lavaGlowBuffer.reset(new QuadBuffer(128, texture, BatchFlagQuad_UseVertexColor));

		const ColorRGBA glowColor(154, 58, 12, 255);

		m_lavaGlowQuad.topLeft.setColor    (glowColor);
		m_lavaGlowQuad.topRight.setColor   (glowColor);
		m_lavaGlowQuad.bottomLeft.setColor (glowColor);
		m_lavaGlowQuad.bottomRight.setColor(glowColor);

		m_lavaGlowQuad.topLeft.setTexCoord    (0,0);
		m_lavaGlowQuad.topRight.setTexCoord   (1,0);
		m_lavaGlowQuad.bottomLeft.setTexCoord (0,1);
		m_lavaGlowQuad.bottomRight.setTexCoord(1,1);
	}

	initFluidSettings();
}


void FluidGraphicsMgr::update(real p_delta, utils::FluidMgrSectionProfiler& p_sectionProfiler)
{
	m_animationTime += p_delta;
	updateWaves(p_sectionProfiler, p_delta);
	m_graphicsNeedUpdate = true;
}


void FluidGraphicsMgr::updateForRender(const level::AttributeLayerPtr& p_layer,
                                       FluidParticlesMgr&              p_particleMgr,
                                       utils::FluidMgrSectionProfiler& p_sectionProfiler,
                                       const tt::math::VectorRect&     p_visibilityRect,
                                       const LevelFalls&               p_falls,
                                       const LevelFlows&               p_flows)
{
	if(m_graphicsNeedUpdate == false) return;
	
	m_waveGenerator.updateForRender();

	using namespace utils;
	p_sectionProfiler.startFrameUpdateSection(FluidMgrSection_FluidGraphicsMgr);
	TT_NULL_ASSERT(p_layer);
	if (p_layer == 0)
	{
		return;
	}
	
	// DEBUG: Do nothing if not enabled
	if (m_enabled == false)
	{
		return;
	}

	if(m_stillBufferNeedsUpdate)
	{
		for(s32 i = 0; i < FluidType_Count; ++i)
		{
			m_stillFluidsBuffer[i]->applyChanges();
		}
		m_stillBufferNeedsUpdate = false;
	}

	m_lavaGlowBatch.clear();

	createFallGeometry(p_falls, p_visibilityRect, p_layer, p_particleMgr);
	createFlowGeometry(p_flows, p_visibilityRect, p_particleMgr);

	m_lavaGlowBuffer->fillBuffer(m_lavaGlowBatch.begin(), m_lavaGlowBatch.end());

	m_graphicsNeedUpdate = false;
}


void FluidGraphicsMgr::renderBack()
{
	// DEBUG: Do nothing if not enabled
	if (m_enabled == false)
	{
		return;
	}
	
	tt::engine::renderer::Renderer::getInstance()->setBlendMode(tt::engine::renderer::BlendMode_Blend);
	
	using tt::engine::renderer::MatrixStack;
	MatrixStack* stack = MatrixStack::getInstance();
	stack->setMode(MatrixStack::Mode_Texture);

	{
		// Render fluid falls

		for(s32 i = 0; i < FluidType_Count; ++i)
		{
			// Lava is not transparent so it makes no sense to render a back layer for it
			if(static_cast<FluidType>(i) == FluidType_Lava) continue;

			handleIgnoreFog(static_cast<FluidType>(i));

			tt::engine::renderer::TexturePtr texture = (i == FluidType_Water) ?
				m_fluidTextures[QuadType_Bg_WaterFall] : m_fluidTextures[QuadType_Bg_LavaFall];
			
			stack->push();
			stack->load44(tt::math::Matrix44::getTranslation(m_fallUVSpeedBack[i] * m_animationTime));
			stack->updateTextureMatrix();

			m_fallBodyBuffer[i]->setTexture(texture);
			m_fallBodyBuffer[i]->render();

			stack->pop();

			restoreIgnoreFog();
		}
	}

	{
		// Render fluid flows
		stack->updateTextureMatrix();

		for(s32 i = 0; i < FluidType_Count; ++i)
		{
			handleIgnoreFog(static_cast<FluidType>(i));
			stack->resetTextureMatrix();

			tt::engine::renderer::TexturePtr texture = (i == FluidType_Water) ?
				m_fluidTextures[QuadType_Bg_WaterPoolStill] : m_fluidTextures[QuadType_Bg_LavaPoolStill];
			
			m_flowMiddleBuffer[i]->setTexture(texture);
			m_flowMiddleBuffer[i]->render();
			
			m_flowStillMiddleBuffer[i]->setTexture(texture);
			m_flowStillMiddleBuffer[i]->render();
			
			texture = (i == FluidType_Water) ?
				m_fluidTextures[QuadType_Bg_WaterPoolLeft] : m_fluidTextures[QuadType_Bg_LavaPoolLeft];

			stack->push();
			stack->load44(tt::math::Matrix44::getTranslation(m_flowUVSpeed[i] * m_animationTime));
			stack->updateTextureMatrix();
			
			for(FlowEdgeBuffers::const_iterator it = m_flowEdges[i].begin(); it != m_flowEdges[i].end(); ++it)
			{
				(*it)->setTexture(texture);
				(*it)->render();
			}
			
			stack->pop();
			
			restoreIgnoreFog();
		}
	}
	
	// Restore defaults.
	stack->updateTextureMatrix();
	stack->setMode(MatrixStack::Mode_Position);
}


void FluidGraphicsMgr::renderFront()
{
	// DEBUG: Do nothing if not enabled
	if (m_enabled == false)
	{
		return;
	}
	
	tt::engine::renderer::Renderer::getInstance()->setBlendMode(tt::engine::renderer::BlendMode_Blend);
	
	using tt::engine::renderer::MatrixStack;
	MatrixStack* stack = MatrixStack::getInstance();
	stack->setMode(MatrixStack::Mode_Texture);

	{
		// Render fluid falls

		for(s32 i = 0; i < FluidType_Count; ++i)
		{
			handleIgnoreFog(static_cast<FluidType>(i));

			stack->push();
			stack->load44(tt::math::Matrix44::getTranslation(m_fallUVSpeedFront[i] * m_animationTime));
			stack->updateTextureMatrix();

			tt::engine::renderer::TexturePtr texture = (i == FluidType_Water) ?
				m_fluidTextures[QuadType_WaterFall] : m_fluidTextures[QuadType_LavaFall];

			m_fallBodyBuffer[i]->setTexture(texture);
			m_fallBodyBuffer[i]->render();
			m_fallLeftBorderBuffer[i]->render();
			m_fallRightBorderBuffer[i]->render();

			stack->pop();

			restoreIgnoreFog();
		}
	}

	{
		// Render fluid flows

		for(s32 i = 0; i < FluidType_Count; ++i)
		{
			handleIgnoreFog(static_cast<FluidType>(i));
			
			stack->push();
			stack->resetTextureMatrix();
			
			stack->load44(tt::math::Matrix44::getTranslation(m_flowUVSpeed[i] * m_animationTime));
			stack->updateTextureMatrix();
			
			tt::engine::renderer::TexturePtr texture = (i == FluidType_Water) ?
				m_fluidTextures[QuadType_WaterPoolLeft] : m_fluidTextures[QuadType_LavaPoolLeft];
			
			for(FlowEdgeBuffers::const_iterator it = m_flowEdges[i].begin(); it != m_flowEdges[i].end(); ++it)
			{
				(*it)->setTexture(texture);
				(*it)->render();
			}

			texture = (i == FluidType_Water) ?
				m_fluidTextures[QuadType_WaterPoolStill] : m_fluidTextures[QuadType_LavaPoolStill];

			stack->resetTextureMatrix();
			m_flowMiddleBuffer[i]->setTexture(texture);
			m_flowMiddleBuffer[i]->render();

			stack->pop();

			restoreIgnoreFog();
		}
	}

	stack->setMode(MatrixStack::Mode_Position);
}


void FluidGraphicsMgr::renderBackStillWater()
{
	// DEBUG: Do nothing if not enabled
	if (m_enabled == false)
	{
		return;
	}
	
	tt::engine::renderer::Renderer::getInstance()->setBlendMode(tt::engine::renderer::BlendMode_Blend);
	
	using tt::engine::renderer::MatrixStack;
	MatrixStack* stack = MatrixStack::getInstance();
	stack->setMode(MatrixStack::Mode_Texture);
	
	{
		// Render fluid flows

		for(s32 i = 0; i < FluidType_Count; ++i)
		{
			handleIgnoreFog(static_cast<FluidType>(i));
			
			stack->push();
			stack->resetTextureMatrix();

			tt::engine::renderer::TexturePtr texture = (i == FluidType_Water) ?
				m_fluidTextures[QuadType_Bg_WaterPoolStill] : m_fluidTextures[QuadType_Bg_LavaPoolStill];

			m_stillFluidsBuffer[i]->setTexture(texture);
			m_stillFluidsBuffer[i]->render();
			
			texture = (i == FluidType_Water) ?
				m_fluidTextures[QuadType_Bg_WaterPoolStill] : m_fluidTextures[QuadType_Bg_LavaPoolStill];

			stack->resetTextureMatrix();
			m_flowStillMiddleBuffer[i]->setTexture(texture);
			m_flowStillMiddleBuffer[i]->render();

			stack->pop();

			restoreIgnoreFog();
		}
	}

	stack->setMode(MatrixStack::Mode_Position);
}


void FluidGraphicsMgr::renderFrontStillWater()
{
	// DEBUG: Do nothing if not enabled
	if (m_enabled == false)
	{
		return;
	}
	
	tt::engine::renderer::Renderer::getInstance()->setBlendMode(tt::engine::renderer::BlendMode_Blend);
	
	using tt::engine::renderer::MatrixStack;
	MatrixStack* stack = MatrixStack::getInstance();
	stack->setMode(MatrixStack::Mode_Texture);
	
	{
		// Render fluid flows

		for(s32 i = 0; i < FluidType_Count; ++i)
		{
			handleIgnoreFog(static_cast<FluidType>(i));
			
			stack->push();
			stack->resetTextureMatrix();

			tt::engine::renderer::TexturePtr texture = (i == FluidType_Water) ?
				m_fluidTextures[QuadType_WaterPoolStill] : m_fluidTextures[QuadType_LavaPoolStill];

			m_stillFluidsBuffer[i]->setTexture(texture);
			m_stillFluidsBuffer[i]->render();
			
			texture = (i == FluidType_Water) ?
				m_fluidTextures[QuadType_WaterPoolStill] : m_fluidTextures[QuadType_LavaPoolStill];

			stack->resetTextureMatrix();
			m_flowStillMiddleBuffer[i]->setTexture(texture);
			m_flowStillMiddleBuffer[i]->render();

			stack->pop();

			restoreIgnoreFog();
		}
	}

	stack->setMode(MatrixStack::Mode_Position);
}


void FluidGraphicsMgr::renderLavaGlow()
{
	if (m_enabled == false)
	{
		return;
	}

	using namespace tt::engine::renderer;
	Renderer::getInstance()->setBlendMode(tt::engine::renderer::BlendMode_Add);
	handleIgnoreFog(FluidType_Lava);

	m_lavaGlowBuffer->render();

	restoreIgnoreFog();
	Renderer::getInstance()->setBlendMode(tt::engine::renderer::BlendMode_Blend);
}


void FluidGraphicsMgr::handleLevelResized()
{
	m_waveGenerator.handleLevelResized();
}


struct StillFluidBlob
{
	tt::math::Point2 min;
	tt::math::Point2 max;
	Point2Set tiles;
	FluidType fluidType;
	
	StillFluidBlob()
	:
	min(-1, -1),
	max(-1, -1),
	tiles(),
	fluidType(FluidType_Invalid)
	{}
	
	StillFluidBlob(const tt::math::Point2& p_initialTile, FluidType p_fluidType)
	:
	min(p_initialTile),
	max(p_initialTile),
	tiles(),
	fluidType(p_fluidType)
	{
		tiles.insert(p_initialTile);
	}
	
	void addTile(const tt::math::Point2& p_tile)
	{
		min.setValues(std::min(min.x, p_tile.x), std::min(min.y, p_tile.y));
		max.setValues(std::max(max.x, p_tile.x), std::max(max.y, p_tile.y));
		tiles.insert(p_tile);
	}
	
	void addBlob(const StillFluidBlob& p_blob)
	{
		min.setValues(std::min(min.x, p_blob.min.x), std::min(min.y, p_blob.min.y));
		max.setValues(std::max(max.x, p_blob.max.x), std::max(max.y, p_blob.max.y));
		tiles.insert(p_blob.tiles.begin(), p_blob.tiles.end());
	}
	
	void reset()
	{
		tiles.clear();
		fluidType = FluidType_Invalid;
		min.setValues(-1, -1);
		max.setValues(-1, -1);
	}
};


void FluidGraphicsMgr::createStillFluidsQuads(const Point2Set&                p_tiles,
                                              const level::AttributeLayerPtr& p_layer,
                                              Point2Set*                      p_surfaceTiles_OUT)
{
	TT_NULL_ASSERT(p_layer);
	
	// Clear old graphics
	for(s32 i = 0; i < FluidType_Count; ++i)
	{
		m_stillFluidsBuffer[i]->clear();
	}
	m_stillFlows.clear();
	
	if (p_tiles.empty())
	{
		return;
	}
	
	// --------------------------------------------------------------------------------------------
	// Step one. Group tiles in connected blobs.
	typedef std::vector<StillFluidBlob> StillBlobs;
	StillBlobs blobs; // We only add to this while looking for blobs.
	                  // This means I can use the index reference a blob.
	
	S32s blobIndexColumn;
	const s32 height = p_layer->getHeight();
	blobIndexColumn.resize(height, -1);
	
	s32 prevX = -1;
	s32 prevY = -1;
	// We use the fact that the tiles are ordered.
	// They are expect to go from 'bottom left most'-tile.
	// Then go up in that row till the last still fluid tile
	// Then to the bottom most still tile of the first row with still tiles.
	for (Point2Set::const_iterator it = p_tiles.begin(); it != p_tiles.end(); ++it)
	{
		FluidType fluidType = getFluidType(p_layer->getCollisionType(*it));
		const tt::math::Point2& tilePos = (*it);
		
		if (prevX != tilePos.x) // New Row
		{
			// Flush the remainder of the previous row.
			// Did we move a single row, then only rest the top part, otherwise reset whole row.
			const s32 startY = (prevX + 1 == tilePos.x) ? prevY + 1 : tilePos.y;
			for (s32 i = startY; i < height; ++i)
			{
				blobIndexColumn[i] = -1;
			}
			prevY = -1;
		}
		
		// Flush the tiles below except the prevY
		for (s32 i = prevY + 1; i < tilePos.y; ++i)
		{
			blobIndexColumn[i] = -1;
		}
		prevY = tilePos.y;
		prevX = tilePos.x;
		
		
		// Check connection with exiting blobs.
		const s32 tileBelow = tilePos.y - 1;
		if (tileBelow >= 0         &&
		    blobIndexColumn[tileBelow] != -1)
		{
			// Found something below
			const s32 belowBlobIndex = blobIndexColumn[tileBelow];
			TT_ASSERT(belowBlobIndex >= 0 && belowBlobIndex < static_cast<s32>(blobs.size()));
			StillFluidBlob& belowBlob = blobs[belowBlobIndex];
			if (belowBlob.fluidType == fluidType)
			{
				// Insert this tile.
				belowBlob.addTile(tilePos);
				
				// Match left with below.
				const s32 blobIndex = blobIndexColumn[tilePos.y];
				if (blobIndex == -1) // Left had no blob yet
				{
					// Add below blob
					blobIndexColumn[tilePos.y] = belowBlobIndex;
					continue;
				}
				if (blobIndex == belowBlobIndex)
				{
					// Already the same blob.
					continue;
				}
				TT_ASSERT(blobIndex >= 0 && blobIndex < static_cast<s32>(blobs.size()));
				StillFluidBlob& leftBlob = blobs[blobIndex];
				if (leftBlob.fluidType == belowBlob.fluidType)
				{
					// Found linkage. Merge the two.
					belowBlob.addBlob(leftBlob);
					
					// Reset leftBlob
					leftBlob.reset();
					
					// Replace all reference to the left blob index to that below.
					for (s32 i = 0; i < height; ++i)
					{
						if (blobIndexColumn[i] == blobIndex)
						{
							blobIndexColumn[i] = belowBlobIndex;
						}
					}
				}
				else // Left didn't match type
				{
					// Overwrite with below index.
					blobIndexColumn[tilePos.y] = belowBlobIndex;
				}
				// Below was used. done
				continue;
			}
		}
		
		// Check left
		const s32 blobIndex = blobIndexColumn[tilePos.y];
		if (blobIndex != -1)
		{
			TT_ASSERT(blobIndex >= 0 && blobIndex < static_cast<s32>(blobs.size()));
			StillFluidBlob& blob = blobs[blobIndex];
			if (blob.fluidType == fluidType)
			{
				// Insert this tile.
				blob.addTile(tilePos);
				
				// Done
				continue;
			}
		}
		
		// No match found (below or left)
		// Create new blob.
		blobIndexColumn[tilePos.y] = static_cast<s32>(blobs.size());
		blobs.push_back(StillFluidBlob(tilePos, fluidType));
	}
	
	// --------------------------------------------------------------------------------------------
	// Step two. Go over the blobs to create quads.
	
#define TT_USE_NEW_QUAD_CODE 1
	
	tt::engine::renderer::BatchQuadCollection stillFluidBatch[FluidType_Count];
	
	VecQuads vecQuads;
	vecQuads.reserve(32);
	
#if TT_USE_NEW_QUAD_CODE
	TileQuads tileQuads;
	
	S32s& vecQuadIndexColumn = blobIndexColumn; // Reuse blobIndexColumn buffer as quadRow (column with vecQuad index.)
	vecQuadIndexColumn.clear();
	vecQuadIndexColumn.resize(height, -1);
	
	enum IndexTileQuad
	{
		IndexTileQuad_Ping,
		IndexTileQuad_Pong,
		
		IndexTileQuad_Count
	};
	S32s tileQuadsIndex[IndexTileQuad_Count]; // Column with tileQuad index.
	IndexTileQuad curIndex  = IndexTileQuad_Ping;
	IndexTileQuad prevIndex = IndexTileQuad_Pong;
	
	// Scratch index buffers.
	S32s prevTileRightQuad;
	S32s currentTileLeftQuad;
#endif
	
	for (StillBlobs::const_iterator blobIt = blobs.begin(); blobIt != blobs.end(); ++blobIt)
	{
		const StillFluidBlob& blob = (*blobIt);
		if (blob.fluidType == FluidType_Invalid)
		{
			continue;
		}
		
		prevX = -1;
		
		TileQuad tileQuad;
		
		s32 startStillFlowX = -1;
		s32 endStillFlowX   = -1;
		
		vecQuads.clear();
		
#if TT_USE_NEW_QUAD_CODE
		tileQuads.clear();
		
		TT_ASSERT(blob.min.y <= blob.max.y);
		const s32 blobHeight = blob.max.y - blob.min.y;
		
		for (s32 i = 0; i < IndexTileQuad_Count; ++i)
		{
			tileQuadsIndex[i].clear();
			tileQuadsIndex[i].resize(blobHeight, -1);
		}
		
		// Resize scratch buffers.
		prevTileRightQuad.clear();
		prevTileRightQuad.resize(blobHeight, -1);
		currentTileLeftQuad.clear();
		currentTileLeftQuad.resize(blobHeight, -1);
#endif
		
		// Again we use the fact that the tiles are ordered.
		for (Point2Set::const_iterator it = blob.tiles.begin(); it != blob.tiles.end(); ++it)
		{
			const tt::math::Point2& tilePos = (*it);
			TT_ASSERT(blob.fluidType == getFluidType(p_layer->getCollisionType(tilePos)));
			
			const bool onNewColumn   = tilePos.x          != prevX;
			const bool foundGap      = tileQuad.endY + 1  != tilePos.y;
			const bool onSurfaceTile = tilePos.y          == blob.max.y;
			
			// Do we need to start new tileQuad?
			if (onNewColumn || foundGap)
			{
				// We already started something which isn't the surface layer
				if (tileQuad.isValid())
				{
					TT_ASSERT(tileQuad.startY != blob.max.y);
#if TT_USE_NEW_QUAD_CODE
					tileQuads.push_back(tileQuad);
#endif
					vecQuads.push_back(VecQuad(tt::math::Vector2(static_cast<real>(tileQuad.xPos),
					                                             static_cast<real>(tileQuad.startY      - levelExtension)),
					                           tt::math::Vector2(static_cast<real>(tileQuad.xPos) + 1.0f,
					                                             static_cast<real>(tileQuad.endY) + 1.0f + ((tileQuad.flow) ? -levelExtension : levelExtension))));
				}
				
				if (onSurfaceTile == false)
				{
					// Create/start new tileQuad.
					tileQuad = TileQuad(tilePos);
				}
				else
				{
					// Reset
					tileQuad = TileQuad();
				}
			}
			
#if TT_USE_NEW_QUAD_CODE
			// Check the previous and current column (vertical edge detection.)
			if (onNewColumn)
			{
				TT_ASSERT(prevX == -1 || prevX + 1 == tilePos.x); // Make sure we didn't skip a column.
				
				addStillFluidEdges(tileQuads, tileQuadsIndex[prevIndex], tileQuadsIndex[curIndex],
				                   blobHeight, vecQuads, prevTileRightQuad, currentTileLeftQuad,
				                   tilePos.x - 1, blob.min.y);
				
				// Update everyhing for the new column.
				const IndexTileQuad tempSwapIndex = curIndex; // Swap cur and prev.
				curIndex  = prevIndex;
				prevIndex = tempSwapIndex;
				tileQuadsIndex[curIndex].clear();
				tileQuadsIndex[curIndex].resize(blobHeight, -1);
			}
#endif
			
			prevX = tilePos.x;
			
			// The surface tiles need to be created as stillFlow and should not be added as quad.
			if (onSurfaceTile)
			{
				// Did we start a new flow?
				if (startStillFlowX == -1)
				{
					startStillFlowX = tilePos.x;
					endStillFlowX   = tilePos.x;
				}
				// Can we continue an existing flow?
				else if (endStillFlowX + 1 == tilePos.x)
				{
					endStillFlowX = tilePos.x;
				}
				// The previous flow can not be continued.
				else
				{
					if (p_surfaceTiles_OUT != 0)
					{
						for (tt::math::Point2 surfaceTile(startStillFlowX, blob.max.y);
						     surfaceTile.x <= endStillFlowX; ++surfaceTile.x)
						{
							p_surfaceTiles_OUT->insert(surfaceTile);
						}
					}
					// Create previous flow.
					createFlowForStill(tt::math::Point2(startStillFlowX, blob.max.y),
					                   tt::math::Point2(endStillFlowX  , blob.max.y),
					                   blob.fluidType);
					
					// Start new flow.
					startStillFlowX = tilePos.x;
					endStillFlowX   = tilePos.x;
				}
				
				tileQuad.flow = true;
			}
			else
			{
				tileQuad.endY = tilePos.y;
#if TT_USE_NEW_QUAD_CODE
				tileQuadsIndex[curIndex][tilePos.y - blob.min.y] = static_cast<s32>(tileQuads.size());
#endif
			}
			
			// Next tile
		}
		
		if (tileQuad.isValid()) // flush final quad.
		{
			TT_ASSERT(tileQuad.startY != blob.max.y);
#if TT_USE_NEW_QUAD_CODE
			tileQuads.push_back(tileQuad);
#endif
			vecQuads.push_back(VecQuad(tt::math::Vector2(static_cast<real>(tileQuad.xPos),
														 static_cast<real>(tileQuad.startY      - levelExtension)),
									   tt::math::Vector2(static_cast<real>(tileQuad.xPos) + 1.0f,
														 static_cast<real>(tileQuad.endY) + 1.0f + ((tileQuad.flow) ? -levelExtension : levelExtension))));
		}
		
		if (startStillFlowX != -1)
		{
			if (p_surfaceTiles_OUT != 0)
			{
				for (tt::math::Point2 surfaceTile(startStillFlowX, blob.max.y);
				     surfaceTile.x <= endStillFlowX; ++surfaceTile.x)
				{
					p_surfaceTiles_OUT->insert(surfaceTile);
				}
			}
			
			createFlowForStill(tt::math::Point2(startStillFlowX, blob.max.y),
			                   tt::math::Point2(endStillFlowX  , blob.max.y),
			                   blob.fluidType);
		}
		
		// Do final edges
		addStillFluidEdges(tileQuads, tileQuadsIndex[prevIndex], tileQuadsIndex[curIndex],
		                   blobHeight, vecQuads, prevTileRightQuad, currentTileLeftQuad,
		                   blob.max.x, blob.min.y);
		
		// Update everyhing for the new column.
		const IndexTileQuad tempSwapIndex = curIndex; // Swap cur and prev.
		curIndex  = prevIndex;
		prevIndex = tempSwapIndex;
		tileQuadsIndex[curIndex].clear();
		tileQuadsIndex[curIndex].resize(blobHeight, -1);
		
		addStillFluidEdges(tileQuads, tileQuadsIndex[prevIndex], tileQuadsIndex[curIndex],
		                   blobHeight, vecQuads, prevTileRightQuad, currentTileLeftQuad,
		                   blob.max.x + 1, blob.min.y);
		
		for (VecQuads::const_iterator it = vecQuads.begin(); it != vecQuads.end(); ++it)
		{
			const VecQuad& vecQuad = (*it);
			
			tt::engine::renderer::BatchQuad quad;
			
			const real left   = vecQuad.min.x;
			const real right  = vecQuad.max.x;
			const real top    = vecQuad.max.y;
			const real bottom = vecQuad.min.y;
			
			quad.topLeft.    setPosition(left,  top   , 0);
			quad.topRight.   setPosition(right, top   , 0);
			quad.bottomLeft. setPosition(left,  bottom, 0);
			quad.bottomRight.setPosition(right, bottom, 0);
			
			const real uOffset = m_stillUVOffset[blob.fluidType].x;
			const real vOffset = m_stillUVOffset[blob.fluidType].y;
			const real surface = static_cast<real>(blob.max.y) + 1.0f;
			const real topV    = (surface - top   ) * vOffset;
			const real bottomV = (surface - bottom) * vOffset;
			
			quad.topLeft.    setTexCoord(left  * uOffset, topV);
			quad.topRight.   setTexCoord(right * uOffset, topV);
			quad.bottomLeft. setTexCoord(left  * uOffset, bottomV);
			quad.bottomRight.setTexCoord(right * uOffset, bottomV);
			
			TT_ASSERT(isValidFluidType(blob.fluidType));
			stillFluidBatch[blob.fluidType].push_back(quad);
		}
		vecQuads.clear();
		
		// Next blob
	}
	
	for(s32 i = 0; i < FluidType_Count; ++i)
	{
		m_stillFluidsBuffer[i]->setCollection(stillFluidBatch[i]);
	}
	m_stillBufferNeedsUpdate = true;
}


//--------------------------------------------------------------------------------------------------
// Private member functions


void FluidGraphicsMgr::addStillFluidEdges(const TileQuads& p_tileQuads, S32s& p_prevTiles, S32s& p_curTiles,
                                          s32 p_blobHeight, VecQuads& p_vecQuads,
                                          S32s& p_prevTileRightQuadScratch, S32s& p_currentTileLeftQuadScratch,
                                          s32 p_xPos, s32 p_blobYPos)
{
	if (p_blobHeight <= 0)
	{
		return;
	}
	S32s&            prevTiles  = p_prevTiles;
	S32s&            curTiles   = p_curTiles;
	const s32        blobHeight = p_blobHeight;
	const TileQuads& tileQuads  = p_tileQuads;
	VecQuads&        vecQuads   = p_vecQuads;
	S32s& prevTileRightQuad     = p_prevTileRightQuadScratch;
	S32s& currentTileLeftQuad   = p_currentTileLeftQuadScratch;
	const s32        xPos       = p_xPos;
	
	TT_ASSERT(blobHeight > 0);
	TT_ASSERT(prevTileRightQuad  .size() >= static_cast<S32s::size_type>(blobHeight));
	TT_ASSERT(currentTileLeftQuad.size() >= static_cast<S32s::size_type>(blobHeight));
	
	// Compare prev with cur to find out where we need to add an edge.
	for (s32 blobY = 0; blobY < blobHeight; ++blobY)
	{
		s32 prevTileIndex = prevTiles[blobY];
		s32 curTileIndex  = curTiles [blobY];
		
		bool prevValid = prevTileIndex != -1;
		bool curValid  = curTileIndex  != -1;
		
		if (prevValid)
		{
			const TileQuad& prevQuad = tileQuads[prevTileIndex];
			if (prevQuad.isValid() == false)
			{
				prevValid        = false;
				prevTiles[blobY] = -1;
			}
		}
		if (curValid)
		{
			const TileQuad& curQuad = tileQuads[curTileIndex];
			if (curQuad.isValid() == false)
			{
				curValid        = false;
				curTiles[blobY] = -1;
			}
		}
		
		prevTileRightQuad    [blobY] = -1;
		currentTileLeftQuad  [blobY] = -1;
		
		if (prevValid != curValid)
		{
			prevTileRightQuad    [blobY] = (curValid ) ? curTileIndex : -1;
			currentTileLeftQuad  [blobY] = (prevValid) ? prevTileIndex : -1;
		}
	}
	
	// Already set the x values because those don't change, but make y invalid.
	VecQuad mergeQuad(tt::math::Vector2(static_cast<real>(xPos) - levelExtension, -1.0f),
	                  tt::math::Vector2(static_cast<real>(xPos)                 , -1.0f));
	
	bool prevTileValid = false;
	
	//TT_Printf("prevTileRightQuad -----------------------------------------------------\n");
	for (s32 blobY = 0; blobY < blobHeight; ++blobY)
	{
		const s32 tileQuadIndex     = prevTileRightQuad[blobY];
		const bool wholeTileIsValid = prevTiles[blobY] >= 0;
		
		//TT_Printf("checking xPos:%d BlobY: %d - found tileQuadIndex: %d - wholeTileIsValid: %d\n",
		//          xPos, blobY, tileQuadIndex, wholeTileIsValid);
		
		if (tileQuadIndex != -1)
		{
			if (mergeQuad.min.y == -1)
			{
				// No quad yet.
				mergeQuad.min.y = static_cast<real>(blobY) + p_blobYPos + (prevTileValid ? levelExtension : -levelExtension);
				mergeQuad.max.y = static_cast<real>(blobY) + p_blobYPos + 1.0f + levelExtension;
				
				// When on the bottom of the level we can get -levelExtension as min.y, fix this.
				if (mergeQuad.min.y < 0)
				{
					mergeQuad.min.y = 0;
				}
				//TT_Printf("New mergeQuad min.y: %f, max.y: %f\n", mergeQuad.min.y, mergeQuad.max.y);
			}
			else
			{
				mergeQuad.max.y = static_cast<real>(blobY) + p_blobYPos + 1.0f + levelExtension;
				//TT_Printf("Continue mergeQuad min.y: %f, max.y: %f\n", mergeQuad.min.y, mergeQuad.max.y);
			}
		}
		else
		{
			if (mergeQuad.min.y >= 0)
			{
				if (wholeTileIsValid)
				{
					mergeQuad.max.y -= (levelExtension + levelExtension);
					//TT_Printf("Limit mergeQuad min.y: %f, max.y: %f\n", mergeQuad.min.y, mergeQuad.max.y);
				}
				
				//TT_Printf("Adding VecQuad min %f, %f. max %f, %f\n",
				//          mergeQuad.min.x, mergeQuad.min.y, mergeQuad.max.x, mergeQuad.max.y);
				vecQuads.push_back(mergeQuad);
				mergeQuad.min.y = -1.0f;
			}
		}
		prevTileValid = wholeTileIsValid;
	}
	if (mergeQuad.min.y >= 0)
	{
		mergeQuad.max.y -= (levelExtension + levelExtension);
		
		//TT_Printf("Adding VecQuad min %f, %f. max %f, %f\n",
		//          mergeQuad.min.x, mergeQuad.min.y, mergeQuad.max.x, mergeQuad.max.y);
		vecQuads.push_back(mergeQuad);
		mergeQuad.min.y = -1.0f;
	}
	
	//TT_Printf("currentTileLeftQuad -----------------------------------------------------\n");
	
	// Already set the x values because those don't change, but make y invalid.
	mergeQuad.min.setValues(static_cast<real>(xPos)                 , -1.0f);
	mergeQuad.max.setValues(static_cast<real>(xPos) + levelExtension, -1.0f);
	
	prevTileValid = false;
	
	for (s32 blobY = 0; blobY < blobHeight; ++blobY)
	{
		const s32 tileQuadIndex     = currentTileLeftQuad[blobY];
		const bool wholeTileIsValid = curTiles[blobY] >= 0;
		
		//TT_Printf("checking xPos:%d BlobY: %d - found tileQuadIndex: %d - wholeTileIsValid: %d\n",
		//          xPos, blobY, tileQuadIndex, wholeTileIsValid);
		
		if (tileQuadIndex != -1)
		{
			if (mergeQuad.min.y == -1)
			{
				// No quad yet.
				mergeQuad.min.y = static_cast<real>(blobY) + p_blobYPos + (prevTileValid ? levelExtension : -levelExtension);
				mergeQuad.max.y = static_cast<real>(blobY) + p_blobYPos + 1.0f + levelExtension;
				
				// When on the bottom of the level we can get -levelExtension as min.y, fix this.
				if (mergeQuad.min.y < 0)
				{
					mergeQuad.min.y = 0;
				}
				//TT_Printf("New mergeQuad min.y: %f, max.y: %f\n", mergeQuad.min.y, mergeQuad.max.y);
			}
			else
			{
				mergeQuad.max.y = static_cast<real>(blobY) + p_blobYPos + 1.0f + levelExtension;
				//TT_Printf("Continue mergeQuad min.y: %f, max.y: %f\n", mergeQuad.min.y, mergeQuad.max.y);
			}
		}
		else
		{
			if (mergeQuad.min.y >= 0)
			{
				if (wholeTileIsValid)
				{
					mergeQuad.max.y -= (levelExtension + levelExtension);
					//TT_Printf("Limit mergeQuad min.y: %f, max.y: %f\n", mergeQuad.min.y, mergeQuad.max.y);
				}
				
				//TT_Printf("Adding VecQuad min %f, %f. max %f, %f\n",
				//          mergeQuad.min.x, mergeQuad.min.y, mergeQuad.max.x, mergeQuad.max.y);
				vecQuads.push_back(mergeQuad);
				mergeQuad.min.y = -1.0f;
			}
		}
		prevTileValid = wholeTileIsValid;
	}
	if (mergeQuad.min.y >= 0)
	{
		mergeQuad.max.y -= (levelExtension + levelExtension);
		
		//TT_Printf("Adding VecQuad min %f, %f. max %f, %f\n",
		//          mergeQuad.min.x, mergeQuad.min.y, mergeQuad.max.x, mergeQuad.max.y);
		vecQuads.push_back(mergeQuad);
		mergeQuad.min.y = -1.0f;
	}
}


//---------------------------------------------------------------

static const real waveQuadHeight = 1.0f;
static const s32  quadsPerTile(4);
static const real waveWidth = 1.0f / quadsPerTile; // 4 vertices per tile
static const u8 fadeMin = 5;


void FluidGraphicsMgr::initFluidSettings()
{
	for(s32 i = 0; i < FluidType_Count; ++i)
	{
		std::string configStr("toki.fluids.");
		configStr += (i == FluidType_Water) ? "water." : "lava.";

		// NOTE: Same offsets for both fluids
		m_fallUVOffset[i]  = tt::math::Vector2(1 / 4.0f, 1 / -8.0f);
		m_flowUVOffset[i]  = tt::math::Vector2(1 / 4.0f, 1 /  2.0f);
		m_stillUVOffset[i] = tt::math::Vector2(1 / 4.0f, 1 /  8.0f);
		
		m_fallBorderUVOffset[i] = tt::math::Vector2(1 / 1.0f, 1 / -8.0f);

		using tt::math::Vector3;
		std::string texSpeedCfg(configStr + "texture_speed.");
		m_fallUVSpeedFront[i] =
			Vector3(0, cfg()->getRealDirect(texSpeedCfg + "fall") * m_fallUVOffset[i].y, 0);
		m_fallUVSpeedBack[i] =
			Vector3(0, cfg()->getRealDirect(texSpeedCfg + "fall_back") * m_fallUVOffset[i].y, 0);

		m_flowUVSpeed[i] =
			Vector3(cfg()->getRealDirect(texSpeedCfg + "sideways") * m_flowUVOffset[i].x, 0, 0);

		configStr = "toki.fluids.waves.";
		configStr += (i == FluidType_Water) ? "water." : "lava.";

		m_waveInterval[i] = cfg()->getRealDirect(configStr + "wave_trigger_interval");
		m_waveHeight  [i] = cfg()->getRealDirect(configStr + "wave_trigger_height");
	}
}


void FluidGraphicsMgr::updateWaves(utils::FluidMgrSectionProfiler& /*p_sectionProfiler*/, real p_delta)
{
	m_waveGenerator.update(p_delta);
}


void FluidGraphicsMgr::createFallGeometry(const LevelFalls& p_falls, const tt::math::VectorRect& p_visibilityRect,
	const level::AttributeLayerPtr& p_layer, FluidParticlesMgr& p_particleMgr)
{
	for(s32 i = 0; i < FluidType_Count; ++i)
	{
		m_fallBodyQuadCount[i] = 0;

		m_fallLeftBorderBatch[i].clear();
		m_fallLeftBorderBuffer[i]->clear();

		m_fallRightBorderBatch[i].clear();
		m_fallRightBorderBuffer[i]->clear();
	}

	// Determine visibility
	const tt::math::PointRect visibleTiles(level::worldToTile(p_visibilityRect));

	s32 firstColumn = std::max(visibleTiles.getLeft()  - 2, s32(0));
	s32 lastColumn  = std::min(visibleTiles.getRight() + 2, static_cast<s32>(p_falls.size() - 1));
	s32 firstRow    = visibleTiles.getBottom() + 2;
	s32 lastRow     = visibleTiles.getTop()    - 2;

	Borders prevRightBorders[FluidType_Count];
	Borders leftBorders     [FluidType_Count];
	Borders rightBorders    [FluidType_Count];

	for(s32 i = firstColumn; i <= lastColumn; ++i)
	{
		const FluidFalls& falls = p_falls[i];

		for(FluidFalls::const_iterator it = falls.begin(); it != falls.end(); ++it)
		{
			if(it->startTile.y >= lastRow && it->growTile.y <= firstRow)
			{
				// Skip 1 tile falls (used for spawning flows etc.)
				if(it->startTile.y == it->growTile.y + 1 && it->state == FlowState_BlockedBySolid) continue;

				createQuadsForFall(*it);
				generateFallParticles(*it, p_layer, p_particleMgr);

				real borderStart = it->area.getBottom();
				real borderEnd = it->area.getTop();

				if(it->fallType != FallType_FromSource && it->fallType != FallType_FromWarp)
				{
					borderStart -= 1.0f;
				}

				leftBorders [it->type].push_back(tt::math::Vector2(borderStart, borderEnd));
				rightBorders[it->type].push_back(tt::math::Vector2(borderStart, borderEnd));
			}
		}

		for(u32 j = 0; j < FluidType_Count; ++j)
		{
			// Merge overlapping segments
			for(Borders::iterator it = leftBorders[j].begin(); it != leftBorders[j].end();)
			{
				Borders::iterator nextIt(it);
				++nextIt;

				if(nextIt != leftBorders[j].end() && it->y >= nextIt->x)
				{
					it->y = nextIt->y;
					it = leftBorders[j].erase(nextIt);
				}
				else
				{
					 ++it;
				}
			}
			for(Borders::iterator it = rightBorders[j].begin(); it != rightBorders[j].end();)
			{
				Borders::iterator nextIt(it);
				++nextIt;

				if(nextIt != rightBorders[j].end() && it->y >= nextIt->x)
				{
					it->y = nextIt->y;
					it = rightBorders[j].erase(nextIt);
				}
				else
				{
					 ++it;
				}
			}

			if(prevRightBorders[j].empty() == false)
			{
				// Remove overlapping areas between left borders & prev right borders
				for(Borders::iterator it = leftBorders[j].begin(); it != leftBorders[j].end(); ++it)
				{
					for(Borders::iterator itRight = prevRightBorders[j].begin(); itRight != prevRightBorders[j].end(); ++itRight)
					{
						if(it->x > itRight->y && it->y < itRight->x)
						{
							// intersection
							const real intersectX = std::min(it->x, itRight->x);
							const real intersectY = std::max(it->y, itRight->y);

							if(it->x <= itRight->x)
							{
								it->x = intersectY;

								if(it->y <= itRight->y)
								{
									itRight->y = intersectX;
								}
								else
								{
									// split itRight
									prevRightBorders[j].push_back(tt::math::Vector2(intersectY, itRight->y));
									itRight->y = intersectX;
								}
							}
							else
							{
								itRight->x = intersectY;

								if(it->y > itRight->y)
								{
									it->y = intersectX;
								}
								else
								{
									// split it
									leftBorders[j].push_back(tt::math::Vector2(intersectY, it->y));
									it->y = intersectX;
								}
							}
						}
					}
				}
			}

			// Process geometry for left and right borders
			createQuadsForFallBorders(leftBorders[j],      i, m_fallLeftBorderBatch[j]);
			createQuadsForFallBorders(prevRightBorders[j], i, m_fallRightBorderBatch[j]);

			prevRightBorders[j].assign(rightBorders[j].begin(), rightBorders[j].end());
			leftBorders [j].clear();
			rightBorders[j].clear();
		}
	}

	for(u32 j = 0; j < FluidType_Count; ++j)
	{
		// Render right most borders
		createQuadsForFallBorders(prevRightBorders[j], lastColumn + 1, m_fallRightBorderBatch[j]);
	}

	for(s32 i = 0; i < FluidType_Count; ++i)
	{
		m_fallBodyBuffer[i]->fillBuffer(m_fallBodyBatch[i].begin(), m_fallBodyBatch[i].begin() + m_fallBodyQuadCount[i]);

		m_fallLeftBorderBuffer[i]->setCollection(m_fallLeftBorderBatch[i]);
		m_fallLeftBorderBuffer[i]->applyChanges();

		m_fallRightBorderBuffer[i]->setCollection(m_fallRightBorderBatch[i]);
		m_fallRightBorderBuffer[i]->applyChanges();
	}
}


void FluidGraphicsMgr::createQuadsForFall(const Fall& p_fall)
{
	const bool fromSource = 
		(p_fall.fallType == FallType_FromSource || p_fall.fallType == FallType_FromWarp);

	using tt::engine::renderer::BatchQuad;

	const real left   = p_fall.area.getLeft();
	const real right  = p_fall.area.getRight();
	const real top    = p_fall.area.getBottom(); // NOTE: Y is flipped
	const real bottom = p_fall.area.getTop();

	real upperFade = p_fall.type == FluidType_Water ? 0.9f : 0.2f;
	static const real lowerFade = 0.9f;

	real length = top - bottom;
	u8 alpha(255);
	real startY = top;

	if(p_fall.state == FlowState_Drain)
	{
		upperFade = 0.25f;

		if(fromSource)
		{
			length += 0.5f;
			startY += 0.5f;
		}
	}

	if(p_fall.state == FlowState_BlockedByLava)
	{
		// Make waterfall stick into lava
		length += 1.0f;
	}

	// Upper fade in quad
	if(fromSource == false || p_fall.state == FlowState_Drain)
	{
		// Offset 1 tile (below flow that spawned it)
		startY -= 0.7f;
		length -= 0.7f;

		real quadHeight = std::min(length, upperFade);

		if(length < upperFade)
		{
			alpha = static_cast<u8>(alpha * (length / upperFade));
		}

		BatchQuad& quad = getNextFallQuad(p_fall.type);
		fillQuad(quad, left, right, startY, startY - quadHeight, 0, alpha, m_fallUVOffset[p_fall.type]);

		// Create a flow connection quad of the same size that fades out flow texture


		length -= upperFade;
		startY -= upperFade;
	}

	if(length <= 0.0f) return;

	// Middle quad
	if(length > lowerFade)
	{
		real quadHeight = (length - lowerFade);
		BatchQuad& quad = getNextFallQuad(p_fall.type);
		fillQuad(quad, left, right, startY, startY - quadHeight, 255, 255, m_fallUVOffset[p_fall.type]);

		length -= quadHeight;
		startY -= quadHeight;
	}

	// Lower fade quad
	{
		BatchQuad& quad = getNextFallQuad(p_fall.type);
		fillQuad(quad, left, right, startY, startY - length, alpha, 0, m_fallUVOffset[p_fall.type]);
	}

	if(p_fall.type == FluidType_Lava)
	{
		const real sidewaysExtend = 1 + p_fall.area.getHeight() * 0.2f;
		const real left2   = p_fall.area.getLeft()   - 2;
		const real right2  = p_fall.area.getRight()  + 2;
		const real top2    = p_fall.area.getBottom() + sidewaysExtend;
		const real bottom2 = p_fall.area.getTop()    - sidewaysExtend;

		m_lavaGlowQuad.topLeft.setPosition    (left2,  top2,    0);
		m_lavaGlowQuad.topRight.setPosition   (right2, top2,    0);
		m_lavaGlowQuad.bottomLeft.setPosition (left2,  bottom2, 0);
		m_lavaGlowQuad.bottomRight.setPosition(right2, bottom2, 0);

		m_lavaGlowBatch.push_back(m_lavaGlowQuad);
	}
}


void FluidGraphicsMgr::createQuadsForFallBorders(
	const Borders& p_borders, s32 p_column, tt::engine::renderer::BatchQuadCollection& p_batch)
{
	// FIXME: Assuming that these are the same for all fluid types
	const tt::math::Vector2 uvOffset = m_fallBorderUVOffset[0];

	for(Borders::const_iterator it = p_borders.begin(); it != p_borders.end(); ++it)
	{
		if(it->y >= it->x) continue;

		real length = it->x - it->y;
		real startX = it->x;

		static const real fadeInLength = 0.5f;

		u8 alpha(255);

		if(length < fadeInLength)
		{
			alpha = static_cast<u8>((length/fadeInLength) * alpha);
		}

		real endX = startX - std::min(length, fadeInLength);

		tt::engine::renderer::BatchQuad quad;

		fillQuad(quad, p_column - 0.5f, p_column + 0.5f, startX, endX, 0, alpha, uvOffset);

		quad.topLeft.    setTexCoord(0.0f, startX * uvOffset.y);
		quad.topRight.   setTexCoord(1.0f, startX * uvOffset.y);
		quad.bottomLeft. setTexCoord(0.0f, endX   * uvOffset.y);
		quad.bottomRight.setTexCoord(1.0f, endX   * uvOffset.y);

		p_batch.push_back(quad);

		length -= fadeInLength;
		startX -= fadeInLength;

		if(length <= 0) continue;

		static const real fadeOutLength = 0.5f;

		real fadeLength = std::min(fadeOutLength, length);
		startX = it->y + fadeLength;
		endX = it->y;

		fillQuad(quad, p_column - 0.5f, p_column + 0.5f, startX, endX, alpha, 0, uvOffset);

		quad.topLeft.    setTexCoord(0.0f, startX * uvOffset.y);
		quad.topRight.   setTexCoord(1.0f, startX * uvOffset.y);
		quad.bottomLeft. setTexCoord(0.0f, endX   * uvOffset.y);
		quad.bottomRight.setTexCoord(1.0f, endX   * uvOffset.y);

		p_batch.push_back(quad);

		length -= fadeLength;

		if(length <= 0.0f) continue;

		startX = it->x - fadeInLength;
		endX   = it->y + fadeOutLength;
		fillQuad(quad, p_column - 0.5f, p_column + 0.5f, startX, endX, 255, 255, uvOffset);
			
		quad.topLeft.    setTexCoord(0.0f, startX * uvOffset.y);
		quad.topRight.   setTexCoord(1.0f, startX * uvOffset.y);
		quad.bottomLeft. setTexCoord(0.0f, endX   * uvOffset.y);
		quad.bottomRight.setTexCoord(1.0f, endX   * uvOffset.y);

		p_batch.push_back(quad);
	}
}


void FluidGraphicsMgr::fillQuad(tt::engine::renderer::BatchQuad& p_quad,
	real p_left, real p_right, real p_top, real p_bottom, u8 p_topAlpha, u8 p_bottomAlpha,
	const tt::math::Vector2& p_uvOffset)
{
	p_quad.topLeft.    setPosition(p_left,  p_top,    0);
	p_quad.topRight.   setPosition(p_right, p_top,    0);
	p_quad.bottomLeft. setPosition(p_left,  p_bottom, 0);
	p_quad.bottomRight.setPosition(p_right, p_bottom, 0);
	
	p_quad.topLeft.    setTexCoord(p_left  * p_uvOffset.x, p_top    * p_uvOffset.y);
	p_quad.topRight.   setTexCoord(p_right * p_uvOffset.x, p_top    * p_uvOffset.y);
	p_quad.bottomLeft. setTexCoord(p_left  * p_uvOffset.x, p_bottom * p_uvOffset.y);
	p_quad.bottomRight.setTexCoord(p_right * p_uvOffset.x, p_bottom * p_uvOffset.y);
	
	p_quad.topLeft.    setColor(255,255,255,p_topAlpha);
	p_quad.topRight.   setColor(255,255,255,p_topAlpha);
	p_quad.bottomLeft. setColor(255,255,255,p_bottomAlpha);
	p_quad.bottomRight.setColor(255,255,255,p_bottomAlpha);
}


void FluidGraphicsMgr::generateFallParticles(const Fall& p_fall,
	const level::AttributeLayerPtr& p_layer, FluidParticlesMgr& p_particleMgr)
{
	if(p_fall.state == FlowState_BlockedByLava)
	{
		p_particleMgr.triggerParticle(p_fall.growTile,
			                          FluidParticlesMgr::TileOrientation_TopCenter,
				                      FluidParticlesMgr::ParticleType_WaterLavaCollision, 
				                      FluidType_Water);
	}
	else if(p_fall.state == FlowState_BlockedBySolid || p_fall.state == FlowState_BlockedByStill)
	{
		const tt::math::Point2 fallToLeft  (p_fall.growTile.x - 1, p_fall.growTile.y + 2);
		const tt::math::Point2 fallToRight (p_fall.growTile.x + 1, p_fall.growTile.y + 2);
		const tt::math::Point2 particleTile(p_fall.growTile.x    , p_fall.growTile.y + 2);

		const bool hasFallLeft = fallToLeft.x >= 0 &&
			p_layer->getFluidFlowType(fallToLeft) == FluidFlowType_Fall &&
			p_layer->getFluidType    (fallToLeft) == p_fall.type;

		const bool hasFallRight = fallToRight.x < p_layer->getWidth() &&
			p_layer->getFluidFlowType(fallToRight) == FluidFlowType_Fall &&
			p_layer->getFluidType    (fallToRight) == p_fall.type;

		if(hasFallLeft == false && hasFallRight == false)
		{
			// Single fall
			p_particleMgr.triggerParticle(particleTile,
			                           FluidParticlesMgr::TileOrientation_BottomLeft,
									   FluidParticlesMgr::ParticleType_SingleFall, 
									   p_fall.type);

			p_particleMgr.triggerParticle(particleTile,
			                           FluidParticlesMgr::TileOrientation_BottomRight,
									   FluidParticlesMgr::ParticleType_SingleFall, 
									   p_fall.type,
									   true);
		}
		else
		{
			// Multi fall
			FluidParticlesMgr::ParticleType particleType;
			FluidParticlesMgr::TileOrientation orientation;
			
			bool flipX = false;
			if(hasFallLeft && hasFallRight)
			{
				particleType = FluidParticlesMgr::ParticleType_DoubleFall_Center;
				orientation  = FluidParticlesMgr::TileOrientation_BottomCenter;
			}
			else if(hasFallLeft)
			{
				particleType = FluidParticlesMgr::ParticleType_DoubleFall;
				orientation  = FluidParticlesMgr::TileOrientation_BottomRight;
				flipX        = true;
			}
			else
			{
				particleType = FluidParticlesMgr::ParticleType_DoubleFall;
				orientation  = FluidParticlesMgr::TileOrientation_BottomLeft;
			}
			
			p_particleMgr.triggerParticle(particleTile, orientation, particleType, p_fall.type, flipX);
		}
	}

	if(p_fall.state == FlowState_Expand)
	{
		p_particleMgr.updateExpansion(&p_fall, FluidParticlesMgr::ParticleType_ExpandFall);
	}
}


void FluidGraphicsMgr::addWavePosition(tt::engine::renderer::TrianglestripVertices& p_vertices,
	real p_x, real p_y, real p_height, u8 p_alpha, bool p_upper, real p_speed, bool p_fadeBottom)
{
	using tt::engine::renderer::BufferVtx;

	const real waveHeight = waveQuadHeight * m_drainLevel;
	const real topV    = p_upper ? 0 : waveHeight * m_flowUVOffset[0].y;
	const real bottomV = p_upper ? waveHeight * m_flowUVOffset[0].y :
	                               (p_height + waveHeight) * m_flowUVOffset[0].y;

	BufferVtx down;
	down.setPosition(p_x, p_y, 0);
	down.setTexCoord(p_x * m_flowUVOffset[0].x * p_speed, bottomV);
	down.setColor(255,255,255, p_fadeBottom ? fadeMin : p_alpha);

	BufferVtx up;
	up.setPosition(p_x, p_y + p_height, 0);
	up.setTexCoord(p_x * m_flowUVOffset[0].x * p_speed, topV);
	up.setColor(255,255,255,p_alpha);

	p_vertices.push_back(down);
	p_vertices.push_back(up);
}


void FluidGraphicsMgr::addFlowCorner(tt::engine::renderer::TrianglestripVertices& p_vertices,
	                                 real p_pivotX, real p_pivotY, real p_startAngle, real p_width,
	                                 real p_height, real p_speed)
{
	using tt::engine::renderer::BufferVtx;
	BufferVtx pivot;
	pivot.setPosition(p_pivotX, p_pivotY, 0);
	pivot.setTexCoord(p_pivotX * m_flowUVOffset[0].x * p_speed, p_height * m_flowUVOffset[0].y); // HACK: Using same UV offset for both fluid types

	static const s32 subdivisionSteps = 10;

	real totalCornerAngle = tt::math::pi * 0.5f;
	real step = totalCornerAngle / subdivisionSteps;

	real angle = p_startAngle;

	for(s32 i = 0; i <= subdivisionSteps; ++i)
	{
		BufferVtx corner;
		const real x = p_pivotX - tt::math::cos(angle) * p_width;
		const real y = p_pivotY + tt::math::sin(angle) * p_height;

		corner.setPosition(x, y, 0);
		corner.setTexCoord(x * m_flowUVOffset[0].x * p_speed, 0);

		p_vertices.push_back(pivot);
		p_vertices.push_back(corner);

		angle += step;
	}
}


void FluidGraphicsMgr::getCornerSettings(FlowState p_state, real& p_offsetOUT, real& p_widthOUT, real p_percentage)
{
	p_widthOUT  = 1.0f;
	p_offsetOUT = 0.5f;

	switch(p_state)
	{
	case FlowState_Expand:
		break;

	case FlowState_OverflowSingle:
	case FlowState_OverflowDoubleFirstTile:
	case FlowState_EnterWarp:
		p_offsetOUT = 0.5f + p_percentage * 0.3f;
		break;

	case FlowState_OverflowDoubleSecondTile:
		p_offsetOUT = 0.8f;
		p_widthOUT  = 1.0f;
		break;

	case FlowState_ExitWarp:
	case FlowState_Done:
		break;

	case FlowState_BlockedBySolid:
	case FlowState_BlockedByKill:
	case FlowState_BlockedByLava:
		p_widthOUT = 0.4f;
		p_offsetOUT = 0.1f;
		break;

	case FlowState_BlockedByStill:
		p_widthOUT  = 0.0f;
		p_offsetOUT = 0.0f;
		break;

	case FlowState_DoneOverflowSingle:
	case FlowState_DoneEnterWarp:
		p_offsetOUT = 0.75f;
		p_widthOUT  = 1.0f;
		break;

	case FlowState_DoneOverflowDouble:
		p_offsetOUT = 0.75f;
		p_widthOUT  = 1.0f;
		break;

	case FlowState_Drain:
		break;

	default:
		break;
	}
}


void FluidGraphicsMgr::createWaveQuads(FluidType p_type, bool p_isStill, real p_x, real p_y, real p_heightLeft, real p_heightRight, u8 p_leftAlpha, u8 p_rightAlpha)
{
	const real waveHeight = waveQuadHeight * m_drainLevel;

	const real topWaveV    = 0;
	const real bottomWaveV = (waveHeight + levelExtension) * m_stillUVOffset[p_type].y;
	const real betweenV    =  waveHeight                   * m_stillUVOffset[p_type].y;

	const real left  = p_x;
	const real right = p_x + waveWidth;

	const real leftU  = left  * m_stillUVOffset[p_type].x;
	const real rightU = right * m_stillUVOffset[p_type].x;

	using tt::engine::renderer::BatchQuad;
	BatchQuad& upperQuad = getNextFlowQuad(p_type, p_isStill);

	const real heightLeft  = std::max(p_heightLeft,  waveHeight - levelExtension);
	const real heightRight = std::max(p_heightRight, waveHeight - levelExtension);

	upperQuad.topLeft.    setPosition(left,  p_y + heightLeft,  0);
	upperQuad.topRight.   setPosition(right, p_y + heightRight, 0);
	upperQuad.bottomLeft. setPosition(left,  p_y + heightLeft  - waveHeight, 0);
	upperQuad.bottomRight.setPosition(right, p_y + heightRight - waveHeight, 0);

	upperQuad.topLeft.    setTexCoord(leftU,  topWaveV);
	upperQuad.topRight.   setTexCoord(rightU, topWaveV);
	upperQuad.bottomLeft. setTexCoord(leftU , betweenV);
	upperQuad.bottomRight.setTexCoord(rightU, betweenV);

	upperQuad.topLeft.    setColor(255,255,255,p_leftAlpha );
	upperQuad.topRight.   setColor(255,255,255,p_rightAlpha);
	upperQuad.bottomLeft. setColor(255,255,255,p_leftAlpha );
	upperQuad.bottomRight.setColor(255,255,255,p_rightAlpha);

	BatchQuad& lowerQuad = getNextFlowQuad(p_type, p_isStill);

	lowerQuad.topLeft.    setPosition(left,  p_y + heightLeft  - waveHeight, 0);
	lowerQuad.topRight.   setPosition(right, p_y + heightRight - waveHeight, 0);
	lowerQuad.bottomLeft. setPosition(left,  p_y - levelExtension, 0);
	lowerQuad.bottomRight.setPosition(right, p_y - levelExtension, 0);
	
	lowerQuad.topLeft.    setTexCoord(leftU,  betweenV);
	lowerQuad.topRight.   setTexCoord(rightU, betweenV);
	lowerQuad.bottomLeft. setTexCoord(leftU , bottomWaveV);
	lowerQuad.bottomRight.setTexCoord(rightU, bottomWaveV);

	lowerQuad.topLeft.    setColor(255,255,255,p_leftAlpha );
	lowerQuad.topRight.   setColor(255,255,255,p_rightAlpha);
	lowerQuad.bottomLeft. setColor(255,255,255,p_leftAlpha );
	lowerQuad.bottomRight.setColor(255,255,255,p_rightAlpha);
}


void FluidGraphicsMgr::createMiddlePart(const Flow& p_flow, bool p_hasLeftPart, bool p_hasRightPart, bool p_isStill)
{
	using tt::engine::renderer::BatchQuad;

	real middlePartStartX = static_cast<real>(p_flow.currentLeft);
	real middlePartEndX   = static_cast<real>(p_flow.currentRight);

	if(middlePartStartX >= middlePartEndX) return;

	for(tt::math::Point2 tilePos(p_flow.currentLeft, p_flow.growLeft.y);
		tilePos.x < p_flow.currentRight; ++tilePos.x)
	{
		m_waveGenerator.addSimulationPosition(tilePos, WaveGenerator::EdgeOption_Default, WaveGenerator::PoolSurfaceType_Still, p_flow.type);

		for(s32 i = 0; i < quadsPerTile; ++i)
		{
			const u8 fadeMidlle = 255;
			u8 leftAlpha (255);
			u8 rightAlpha(255);

			// Create cross fade
			if(p_hasLeftPart && tilePos.x == p_flow.currentLeft)
			{
				if(i == 0)
				{
					leftAlpha  = fadeMin;
					rightAlpha = fadeMidlle;
				}
				else if(i == 1)
				{
					leftAlpha  = fadeMidlle;
					rightAlpha = 255;
				}
			}
			if(p_hasRightPart && tilePos.x == (p_flow.currentRight - 1))
			{
				if((i + 1) == quadsPerTile)
				{
					rightAlpha = fadeMin;
					leftAlpha = fadeMidlle;
				}
				else if ((i + 2) == quadsPerTile)
				{
					rightAlpha = fadeMidlle;
					leftAlpha = 255;
				}
			}

			const real x = tilePos.x + i * waveWidth;
			const real y = p_flow.area.getTop();

			const real waveHeight = m_drainLevel * waveQuadHeight;
			const real waveHeightLeft  = waveHeight + m_waveGenerator.getHeight(tilePos, i);
			const real waveHeightRight = waveHeight + m_waveGenerator.getHeight(tilePos, i+1);

			createWaveQuads(p_flow.type, p_isStill, x, y, waveHeightLeft, waveHeightRight, leftAlpha, rightAlpha);
		}
	}

	// Create level connections

	const real y = p_flow.area.getTop();
	if(p_hasLeftPart == false && p_flow.stateLeft == FlowState_BlockedBySolid)
	{
		tt::math::Point2 startPos(p_flow.currentLeft, p_flow.growLeft.y);
		const real waveHeight = (m_drainLevel * waveQuadHeight) + m_waveGenerator.getHeight(startPos, 0);

		createWaveQuads(p_flow.type, p_isStill, middlePartStartX - waveWidth, y, waveHeight - 0.1f, waveHeight, 255, 255);
	}

	if(p_hasRightPart == false && p_flow.stateRight == FlowState_BlockedBySolid)
	{
		tt::math::Point2 endPos(p_flow.currentRight, p_flow.growRight.y);
		const real waveHeight = (m_drainLevel * waveQuadHeight) + m_waveGenerator.getHeight(endPos, 0);

		createWaveQuads(p_flow.type, p_isStill, middlePartEndX, y, waveHeight, waveHeight - 0.1f, 255, 255);
	}
}


void FluidGraphicsMgr::createLeftPart(const Flow& p_flow, real p_cornerOffset, real p_cornerWidth)
{
	const real leftPartStartX = p_flow.area.getLeft() + p_cornerOffset;
	const real leftPartEndX   = static_cast<real>(p_flow.currentLeft);
	const real bottom         = p_flow.area.getTop();
	const real waveHeight     = m_drainLevel * waveQuadHeight;

	tt::engine::renderer::TrianglestripBufferPtr buffer = getEdgeBuffer();
	tt::engine::renderer::TrianglestripBufferPtr levelConnectBuffer = getEdgeBuffer();

	m_flowBodyVertices[p_flow.type].clear();
	m_flowLowerVertices[p_flow.type].clear();

	tt::engine::renderer::TrianglestripVertices& upperVertices = m_flowBodyVertices[p_flow.type];
	tt::engine::renderer::TrianglestripVertices& lowerVertices = m_flowLowerVertices[p_flow.type];

	bool hasCorner(p_flow.stateLeft != FlowState_BlockedByKill  &&
				   p_flow.stateLeft != FlowState_BlockedBySolid &&
				   p_flow.stateLeft != FlowState_BlockedByStill&&
				   p_flow.stateLeft != FlowState_BlockedByLava);

	if(hasCorner)
	{
		// Fall connection

		const real width = p_cornerWidth;
		real left = leftPartStartX - p_cornerWidth;

		static const real fallConnectionHeight = 0.5f;

		const real bottomVtx = bottom - fallConnectionHeight;
		const real leftVtx   = left;
		const real rightVtx  = left + width;

		const real connectU = (leftPartStartX - p_cornerWidth - fallConnectionHeight) * m_flowUVOffset[0].x;

		using tt::engine::renderer::BufferVtx;
		BufferVtx vtx0;
		vtx0.setPosition(rightVtx, bottomVtx, 0);
		vtx0.setColor(255,255,255,fadeMin);
		vtx0.setTexCoord(connectU, waveHeight * m_flowUVOffset[0].y);

		BufferVtx vtx1;
		vtx1.setPosition(leftVtx, bottomVtx, 0);
		vtx1.setColor(255,255,255,fadeMin);
		vtx1.setTexCoord(connectU, 0);

		upperVertices.push_back(vtx0);
		upperVertices.push_back(vtx1);

		// Start with corner
		addFlowCorner(upperVertices, leftPartStartX, bottom, 0, p_cornerWidth, waveHeight, 1.0f);
	}
	else if(p_flow.stateLeft == FlowState_BlockedBySolid)
	{
		addWavePosition(upperVertices, leftPartStartX - levelExtension, bottom - 0.1f, waveHeight, 255, true, 1.0f);
		addWavePosition(lowerVertices, leftPartStartX - levelExtension, bottom - levelExtension, levelExtension - 0.1f, 255, false, 1.0f);
	}

	// Wave starting point
	if(leftPartStartX < leftPartEndX)
	{
		const real levelExtend = hasCorner ? (2 * levelExtension) : levelExtension;
		addWavePosition(m_flowBodyVertices[p_flow.type] , leftPartStartX, bottom, waveHeight, 255, true, 1.0f);
		addWavePosition(m_flowLowerVertices[p_flow.type], leftPartStartX, bottom - levelExtend, levelExtend, 255, false, 1.0f, hasCorner);
	}
	else
	{
		const real levelExtend = hasCorner ? (2 * levelExtension) : levelExtension;
		real height = m_waveGenerator.getHeight(tt::math::Point2(p_flow.currentLeft, p_flow.growLeft.y), 0);
		addWavePosition(m_flowBodyVertices[p_flow.type] , leftPartEndX, bottom + height, waveHeight, 255, true, 1.0f);
		addWavePosition(m_flowLowerVertices[p_flow.type], leftPartEndX, bottom - levelExtend, height + levelExtend, 255, false, 1.0f, hasCorner);
	}

	real waveFadeIn  (0.0f);
	real waveFadeStep(0.1f);

	s32 fadeExtension(0);
	if(p_flow.stateLeft == FlowState_DoneOverflowDouble)
	{
		fadeExtension = 6;
	}
	else if(p_flow.stateLeft == FlowState_DoneOverflowSingle)
	{
		fadeExtension = 2;
	}
	
	for(tt::math::Point2 tilePos(static_cast<s32>(leftPartStartX), p_flow.growLeft.y);
		tilePos.x <= p_flow.currentLeft; ++tilePos.x)
	{
		m_waveGenerator.addSimulationPosition(tilePos, WaveGenerator::EdgeOption_Default, WaveGenerator::PoolSurfaceType_Left, p_flow.type);

		for(s32 i = 0; i < quadsPerTile; ++i)
		{
			const real x = tilePos.x + i * waveWidth;
			const real height = (m_waveGenerator.getHeight(tilePos, i) * std::min(1.0f, waveFadeIn));
			waveFadeIn += waveFadeStep;

			if (x > leftPartStartX && x <= leftPartEndX)
			{
				real levelExtend(levelExtension);
				bool fade(false);

				if(fadeExtension > 0)
				{
					levelExtend *= 2;
					fade = true;
				}
				--fadeExtension;

				addWavePosition(m_flowBodyVertices[p_flow.type],  x, bottom + height, waveHeight, 255, true, 1.0f);
				addWavePosition(m_flowLowerVertices[p_flow.type], x, bottom - levelExtend, height + levelExtend, 255, false, 1.0f, fade);
			}
		}
	}

	// Half tile overlay fade
	tt::math::Point2 tilePos(p_flow.currentLeft, p_flow.growLeft.y);
	real height0 = m_waveGenerator.getHeight(tilePos, 1);
	real height1 = m_waveGenerator.getHeight(tilePos, 2);

	const u8 fadeStep1(p_flow.type == FluidType_Water ? 255 : 255);
	const u8 fadeStep2(p_flow.type == FluidType_Water ?   0 : 255);

	addWavePosition(m_flowBodyVertices[p_flow.type], leftPartEndX + waveWidth,     bottom + height0, waveHeight, fadeStep1, true, 1.0f);
	addWavePosition(m_flowBodyVertices[p_flow.type], leftPartEndX + waveWidth * 2, bottom + height1, waveHeight, fadeStep2, true, 1.0f);

	addWavePosition(m_flowLowerVertices[p_flow.type], leftPartEndX + waveWidth,     bottom - levelExtension, height0 + levelExtension, fadeStep1, false, 1.0f);
	addWavePosition(m_flowLowerVertices[p_flow.type], leftPartEndX + waveWidth * 2, bottom - levelExtension, height1 + levelExtension, fadeStep2, false, 1.0f);

	if(m_flowBodyVertices[p_flow.type].empty() == false)
	{
		buffer->setCollection(m_flowBodyVertices[p_flow.type]);
		buffer->applyChanges();
		m_flowEdges[p_flow.type].push_back(buffer);
	}

	if(m_flowLowerVertices[p_flow.type].empty() == false)
	{
		levelConnectBuffer->setCollection(m_flowLowerVertices[p_flow.type]);
		levelConnectBuffer->applyChanges();
		m_flowEdges[p_flow.type].push_back(levelConnectBuffer);
	}
}


void FluidGraphicsMgr::createRightPart(const Flow& p_flow, real p_cornerOffset, real p_cornerWidth)
{
	const real rightPartStartX = static_cast<real>(p_flow.currentRight);
	const real rightPartEndX   = p_flow.area.getRight() - p_cornerOffset;
	const real bottom          = p_flow.area.getTop();
	const real waveHeight      = m_drainLevel * waveQuadHeight;
	
	tt::engine::renderer::TrianglestripBufferPtr buffer = getEdgeBuffer();
	tt::engine::renderer::TrianglestripBufferPtr levelConnectBuffer = getEdgeBuffer();

	m_flowBodyVertices [p_flow.type].clear();
	m_flowLowerVertices[p_flow.type].clear();

	// Half tile overlay fade
	tt::math::Point2 tilePos(p_flow.currentRight - 1, p_flow.growLeft.y);
	
	real height0 = m_waveGenerator.getHeight(tilePos, 2);
	real height1 = m_waveGenerator.getHeight(tilePos, 3);

	const u8 fadeStep1(p_flow.type == FluidType_Water ?   0 : 255);
	const u8 fadeStep2(p_flow.type == FluidType_Water ? 255 : 255);

	addWavePosition(m_flowBodyVertices[p_flow.type], rightPartStartX - waveWidth * 2, bottom + height0, waveHeight, fadeStep1, true, -1.0f);
	addWavePosition(m_flowBodyVertices[p_flow.type], rightPartStartX - waveWidth,     bottom + height1, waveHeight, fadeStep2, true, -1.0f);

	addWavePosition(m_flowLowerVertices[p_flow.type], rightPartStartX - waveWidth * 2, bottom - levelExtension, height0 + levelExtension, fadeStep1, false, -1.0f);
	addWavePosition(m_flowLowerVertices[p_flow.type], rightPartStartX - waveWidth,     bottom - levelExtension, height1 + levelExtension, fadeStep2, false, -1.0f);

	// Continue until grow point / overflow
	real waveFadeOut (1.0f);
	real waveFadeStep(0.1f);

	real fallExtension(rightPartEndX);
	if(p_flow.stateRight == FlowState_DoneOverflowDouble)
	{
		fallExtension -= 1.25f;
	}
	else if(p_flow.stateRight == FlowState_DoneOverflowSingle)
	{
		fallExtension -= 0.25f;
	}

	tilePos.x++;
	for( ;tilePos.x < rightPartEndX; ++tilePos.x)
	{
		m_waveGenerator.addSimulationPosition(tilePos, WaveGenerator::EdgeOption_Default, WaveGenerator::PoolSurfaceType_Right, p_flow.type);

		for(s32 i = 0; i < quadsPerTile; ++i)
		{
			const real x = tilePos.x + i * waveWidth;
			const real height = (m_waveGenerator.getHeight(tilePos, i) * std::max(0.0f, waveFadeOut));

			if(x > (rightPartEndX - 2.0f))
			{
				waveFadeOut -= waveFadeStep;
			}

			if (x < rightPartEndX)
			{
				real levelExtend(levelExtension);
				bool fade(false);

				if(x >= fallExtension)
				{
					levelExtend *= 2;
					fade = true;
				}
				addWavePosition(m_flowBodyVertices[p_flow.type],  x, bottom + height, waveQuadHeight * m_drainLevel, 255, true, -1.0f);
				addWavePosition(m_flowLowerVertices[p_flow.type], x, bottom - levelExtend, height + levelExtend, 255, false, -1.0f, fade);
			}
		}
	}

	bool hasCorner(p_flow.stateRight != FlowState_BlockedByKill  &&
				   p_flow.stateRight != FlowState_BlockedBySolid &&
				   p_flow.stateRight != FlowState_BlockedByStill &&
				   p_flow.stateRight != FlowState_BlockedByLava);

	// Wave end point
	const real levelExtend = hasCorner ? (2 * levelExtension) : levelExtension;
	if(rightPartEndX > rightPartStartX)
	{
		addWavePosition(m_flowBodyVertices[p_flow.type] , rightPartEndX, bottom, waveHeight, 255, true, -1.0f);
		addWavePosition(m_flowLowerVertices[p_flow.type], rightPartEndX, bottom - levelExtend, levelExtend, 255, false, -1.0f, hasCorner);
	}
	else
	{
		real height = m_waveGenerator.getHeight(tt::math::Point2(p_flow.currentRight, p_flow.growRight.y), 0);
		addWavePosition(m_flowBodyVertices[p_flow.type] , rightPartStartX, bottom + height, waveHeight, 255, true, -1.0f);
		addWavePosition(m_flowLowerVertices[p_flow.type], rightPartStartX, bottom - levelExtend, height + levelExtend, 255, false, -1.0f, hasCorner);
	}

	// End in corner

	if (hasCorner)
	{
		addFlowCorner(m_flowBodyVertices[p_flow.type], rightPartEndX, bottom, tt::math::halfPi, p_cornerWidth, waveHeight, -1.0f);

		const real width = p_cornerWidth;
		real right = rightPartEndX + p_cornerWidth;

		static const real fallConnectionHeight = 0.5f;

		const real bottomVtx = bottom - fallConnectionHeight;
		const real leftVtx   = right - width;
		const real rightVtx  = right;

		using tt::engine::renderer::BufferVtx;

		BufferVtx vtx2;
		vtx2.setPosition(leftVtx, bottomVtx, 0);
		vtx2.setColor(255,255,255,fadeMin);
		vtx2.setTexCoord((right + fallConnectionHeight) * -m_flowUVOffset[0].x, waveHeight * m_flowUVOffset[0].y);

		BufferVtx vtx3;
		vtx3.setPosition(rightVtx, bottomVtx, 0);
		vtx3.setColor(255,255,255,fadeMin);
		vtx3.setTexCoord((right + fallConnectionHeight) * -m_flowUVOffset[0].x,0);

		m_flowBodyVertices[p_flow.type].push_back(vtx2);
		m_flowBodyVertices[p_flow.type].push_back(vtx3);
	}
	else if(p_flow.stateRight == FlowState_BlockedBySolid)
	{
		// Level connect
		addWavePosition(m_flowBodyVertices[p_flow.type] , rightPartEndX + levelExtension, bottom - 0.1f, waveHeight, 255, true, -1.0f);
		addWavePosition(m_flowLowerVertices[p_flow.type], rightPartEndX + levelExtension, bottom - levelExtension, levelExtension - 0.1f, 255, false, -1.0f);
	}

	// Process geometry
	if(m_flowBodyVertices[p_flow.type].empty() == false)
	{
		buffer->setCollection(m_flowBodyVertices[p_flow.type]);
		buffer->applyChanges();
		m_flowEdges[p_flow.type].push_back(buffer);
	}

	if(m_flowLowerVertices[p_flow.type].empty() == false)
	{
		levelConnectBuffer->setCollection(m_flowLowerVertices[p_flow.type]);
		levelConnectBuffer->applyChanges();
		m_flowEdges[p_flow.type].push_back(levelConnectBuffer);
	}
}


void FluidGraphicsMgr::createFlowGeometry(const LevelFlows& p_flows, const tt::math::VectorRect& p_visibilityRect, FluidParticlesMgr& p_particleMgr)
{
	for(s32 i = 0; i < FluidType_Count; ++i)
	{
		m_flowQuadCount     [i] = 0;
		m_flowStillQuadCount[i] = 0;
		
		m_flowEdges[i].clear();
	}
	m_edgeBuffersUsed = 0;

	// Determine visibility
	const tt::math::PointRect visibleTiles(level::worldToTile(p_visibilityRect));

	s32 firstColumn = visibleTiles.getLeft()  - 2;
	s32 lastColumn  = visibleTiles.getRight() + 2;
	s32 firstRow    = std::max(visibleTiles.getTop() - 2, s32(0));
	s32 lastRow     = std::min(visibleTiles.getBottom() + 2, static_cast<s32>(p_flows.size() - 1));

	// Create geometry from top of screen to bottom
	for(s32 i = lastRow; i >= firstRow; --i)
	{
		const FluidFlows& flows = p_flows[i];

		// Dynamic fluids
		for(FluidFlows::const_iterator it = flows.begin(); it != flows.end(); ++it)
		{
			if(it->growLeft.x >= it->growRight.x) continue;

			if(it->growRight.x >= firstColumn && it->growLeft.x <= lastColumn)
			{
				real leftCornerOffset (0.0f);
				real leftCornerWidth  (0.0f);
				real rightCornerOffset(0.0f);
				real rightCornerWidth (0.0f);

				real leftPercentage  = 1.0f - (it->area.getLeft() - it->growLeft.x);
				real rightPercentage = it->area.getRight() - it->growRight.x;

				getCornerSettings(it->stateLeft,  leftCornerOffset,  leftCornerWidth,  leftPercentage);
				getCornerSettings(it->stateRight, rightCornerOffset, rightCornerWidth, rightPercentage);

				bool hasLeftPart(true);
				if (it->stateLeft == FlowState_BlockedByKill ||
					it->stateLeft == FlowState_BlockedByStill ||
					it->stateLeft == FlowState_BlockedBySolid ||
					it->stateLeft == FlowState_BlockedByLava)
				{
					if((it->growLeft.x + 1) == it->currentLeft)
					{
						hasLeftPart = false;
					}
				}

				bool hasRightPart(true);
				if (it->stateRight == FlowState_BlockedByKill ||
					it->stateRight == FlowState_BlockedByStill ||
					it->stateRight == FlowState_BlockedBySolid ||
					it->stateRight == FlowState_BlockedByLava)
				{
					if(it->growRight.x == it->currentRight)
					{
						hasRightPart = false;
					}
				}

				m_drainLevel = 1.0f;
				if(it->stateLeft == FlowState_Drain)
				{
					m_drainLevel = it->area.getHeight();
				}

				createMiddlePart(*it, hasLeftPart, hasRightPart, false);
				if (hasLeftPart)  createLeftPart  (*it, leftCornerOffset,  leftCornerWidth);
				if (hasRightPart) createRightPart (*it, rightCornerOffset, rightCornerWidth);

				if(it->type == FluidType_Lava)
				{
					const real sidewaysExtend = 1 + it->area.getWidth() * 0.2f;
					const real left   = it->area.getLeft()   - sidewaysExtend;
					const real right  = it->area.getRight()  + sidewaysExtend;
					const real top    = it->area.getBottom() + 2;
					const real bottom = it->area.getTop()    - 2;

					m_lavaGlowQuad.topLeft.setPosition    (left,  top,    0);
					m_lavaGlowQuad.topRight.setPosition   (right, top,    0);
					m_lavaGlowQuad.bottomLeft.setPosition (left,  bottom, 0);
					m_lavaGlowQuad.bottomRight.setPosition(right, bottom, 0);

					m_lavaGlowBatch.push_back(m_lavaGlowQuad);
				}

				if(it->stateLeft == FlowState_BlockedByLava)
				{
					p_particleMgr.triggerParticle(it->growLeft, FluidParticlesMgr::TileOrientation_BottomRight,
													FluidParticlesMgr::ParticleType_WaterLavaCollision, 
													FluidType_Water);
				}
				if(it->stateRight == FlowState_BlockedByLava)
				{
					p_particleMgr.triggerParticle(it->growRight, FluidParticlesMgr::TileOrientation_BottomLeft,
													FluidParticlesMgr::ParticleType_WaterLavaCollision, 
													FluidType_Water);
				}
				if(it->stateLeft < FlowState_Done)
				{
					p_particleMgr.updateExpansion(&(*it), FluidParticlesMgr::ParticleType_ExpandFlowLeft);
				}
				if(it->stateRight < FlowState_Done)
				{
					p_particleMgr.updateExpansion(&(*it), FluidParticlesMgr::ParticleType_ExpandFlowRight);
				}
			}
		}
	}

	// Still fluids
	m_drainLevel = 1.0f;
	for(FluidFlows::const_iterator it = m_stillFlows.begin(); it != m_stillFlows.end(); ++it)
	{
		if (it->growLeft.y  >= firstRow    && it->growLeft.y <= lastRow &&
			it->growRight.x >= firstColumn && it->growLeft.x <= lastColumn)
		{
			createMiddlePart(*it, false, false, true);
		}
	}

	for(s32 i = 0; i < FluidType_Count; ++i)
	{
		m_flowMiddleBuffer[i]     ->fillBuffer(     m_flowMiddleBatch[i].begin(),      m_flowMiddleBatch[i].begin() +      m_flowQuadCount[i]);
		m_flowStillMiddleBuffer[i]->fillBuffer(m_flowStillMiddleBatch[i].begin(), m_flowStillMiddleBatch[i].begin() + m_flowStillQuadCount[i]);
	}
}


void FluidGraphicsMgr::fillFlowQuad(tt::engine::renderer::BatchQuad& p_quad,
	real p_left, real p_right, real p_top, real p_bottom, u8 p_topAlpha, u8 p_bottomAlpha,
	const tt::math::Vector2& p_uvOffset, real p_topV, real p_bottomV)
{
	p_quad.topLeft.    setPosition(p_left,  p_top,    0);
	p_quad.topRight.   setPosition(p_right, p_top,    0);
	p_quad.bottomLeft. setPosition(p_left,  p_bottom, 0);
	p_quad.bottomRight.setPosition(p_right, p_bottom, 0);
	
	p_quad.topLeft.    setTexCoord(p_left  * p_uvOffset.x, p_topV);
	p_quad.topRight.   setTexCoord(p_right * p_uvOffset.x, p_topV);
	p_quad.bottomLeft. setTexCoord(p_left  * p_uvOffset.x, p_bottomV);
	p_quad.bottomRight.setTexCoord(p_right * p_uvOffset.x, p_bottomV);
	
	p_quad.topLeft.    setColor(255,255,255,p_topAlpha);
	p_quad.topRight.   setColor(255,255,255,p_topAlpha);
	p_quad.bottomLeft. setColor(255,255,255,p_bottomAlpha);
	p_quad.bottomRight.setColor(255,255,255,p_bottomAlpha);
}


tt::engine::renderer::BatchQuad& FluidGraphicsMgr::getNextFallQuad(FluidType p_type)
{
	u32 index = m_fallBodyQuadCount[p_type];

	// If the current index is outside the buffer range, add another quad to the batch
	if(index == m_fallBodyBatch[p_type].size())
	{
		m_fallBodyBatch[p_type].push_back(tt::engine::renderer::BatchQuad());
	}

	++m_fallBodyQuadCount[p_type];

	return m_fallBodyBatch[p_type][index];
}


tt::engine::renderer::BatchQuad& FluidGraphicsMgr::getNextFlowQuad(FluidType p_type, bool p_isStill)
{
	tt::engine::renderer::BatchQuadCollection& batch     = (p_isStill) ? m_flowStillMiddleBatch[p_type] : m_flowMiddleBatch[p_type];
	u32&                                       quadCount = (p_isStill) ? m_flowStillQuadCount[  p_type] : m_flowQuadCount[  p_type];
	
	const u32 index = quadCount;
	
	// If the current index is outside the buffer range, add another quad to the batch
	if(index == batch.size())
	{
		batch.push_back(tt::engine::renderer::BatchQuad());
	}
	
	++quadCount;
	
	return batch[index];
}


tt::engine::renderer::TrianglestripBufferPtr FluidGraphicsMgr::getEdgeBuffer()
{
	u32 index = m_edgeBuffersUsed;

	if(index == m_edgeBuffers.size())
	{
		using namespace tt::engine::renderer;
		m_edgeBuffers.push_back(TrianglestripBufferPtr(new TrianglestripBuffer(1024,1,TexturePtr(), BatchFlagTrianglestrip_UseVertexColor)));
	}

	++m_edgeBuffersUsed;

	return m_edgeBuffers[index];
}


void FluidGraphicsMgr::createFlowForStill(const tt::math::Point2& p_startTile,
	                                      const tt::math::Point2& p_endTile,
	                                      FluidType p_type)
{
	tt::math::VectorRect area(
		tt::math::Vector2(p_startTile),
		static_cast<real>(p_endTile.x - p_startTile.x),
		1.0f);

	Flow stillFlow(area, p_type);
	stillFlow.currentLeft  = p_startTile.x;
	stillFlow.currentRight = p_endTile.x + 1;
	stillFlow.stateLeft = stillFlow.stateRight = FlowState_BlockedByStill;
	stillFlow.growLeft  = p_startTile + tt::math::Point2::left;
	stillFlow.growRight = p_endTile   + tt::math::Point2::right;

	if(AppGlobal::getGame()->getTileRegistrationMgr().isSolid(p_startTile + tt::math::Point2::left))
	{
		stillFlow.stateLeft = FlowState_BlockedBySolid;
	}
	if(AppGlobal::getGame()->getTileRegistrationMgr().isSolid(p_endTile + tt::math::Point2::right))
	{
		stillFlow.stateRight = FlowState_BlockedBySolid;
	}

	m_stillFlows.push_back(stillFlow);
}


void FluidGraphicsMgr::handleIgnoreFog(FluidType p_type)
{
	tt::engine::renderer::Renderer* renderer(tt::engine::renderer::Renderer::getInstance());
	m_originalFogSetting = renderer->isFogEnabled();

	if(p_type == FluidType_Lava)
	{
		renderer->setFogEnabled(false);
	}
}


void FluidGraphicsMgr::restoreIgnoreFog()
{
	tt::engine::renderer::Renderer::getInstance()->setFogEnabled(m_originalFogSetting);
}

// Namespace end
}
}
}
