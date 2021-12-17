#include <cstring>

#include <recastnavigation/Detour/DetourCommon.h>
#include <recastnavigation/Detour/DetourNavMeshBuilder.h>
#include <recastnavigation/DebugUtils/DetourDebugDraw.h>
#include <recastnavigation/DebugUtils/RecastDebugDraw.h>
#include <recastnavigation/DebugUtils/tt/RecastDebugDrawTT.h>
#include <recastnavigation/Recast/RecastAlloc.h>

#include <tt/engine/debug/DebugRenderer.h>
#include <tt/engine/renderer/ColorRGBA.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/math/math.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>
#include <tt/system/Time.h>

#include <toki/game/pathfinding/fwd.h>
#include <toki/game/pathfinding/TileRasterizationContext.h>
#include <toki/level/AttributeLayer.h>
#include <toki/level/helpers.h>
#include <toki/level/types.h>

#include <toki/serialization/SerializationMgr.h>
#include <toki/AppGlobal.h>


namespace toki {
namespace game {
namespace pathfinding {

//--------------------------------------------------------------------------------------------------
// Public member functions

TileRasterizationContext::TileRasterizationContext()
:
m_solid(0),
m_chf(0),
m_lset(0),
m_ntiles(0)
{
	memset(m_tiles, 0, sizeof(TileCacheData) * MAX_LAYERS);
}


TileRasterizationContext::~TileRasterizationContext()
{
	cleanup();
}


void TileRasterizationContext::cleanup()
{
	rcFreeHeightField(m_solid);
	m_solid = 0;
	rcFreeCompactHeightfield(m_chf);
	m_chf = 0;
	rcFreeHeightfieldLayerSet(m_lset);
	m_lset = 0;
	
	for (int i = 0; i < MAX_LAYERS; ++i)
	{
		dtFree(m_tiles[i].data);
		m_tiles[i].data     = 0;
		m_tiles[i].dataSize = 0;
	}
}


int TileRasterizationContext::rasterizeTileLayers(rcContext* ctx,
                                                  const level::AttributeLayerPtr& p_layer,
                                                  const int tx, const int ty,
                                                  const rcConfig& cfg,
                                                  TileCacheData* tiles,
                                                  const int maxTiles)
{
	if (p_layer == 0)
	{
		TT_NULL_ASSERT(p_layer);
		return 0;
	}
	
	(void) ctx;
	(void) tx;
	(void) ty;
	(void) cfg;
	(void) tiles;
	(void) maxTiles;
	
	cleanup();
	
	FastLZCompressor comp;
	
	/*
	const float* verts = geom->getMesh()->getVerts();
	const int nverts = geom->getMesh()->getVertCount();
	const rcChunkyTriMesh* chunkyMesh = geom->getChunkyMesh();
	*/
	
	// Tile bounds.
	const float tcs = cfg.tileSize * cfg.cs;
	
	rcConfig tcfg;
	memcpy(&tcfg, &cfg, sizeof(tcfg));

	tcfg.bmin[0] = cfg.bmin[0] + tx*tcs;
	tcfg.bmin[1] = cfg.bmin[1];
	tcfg.bmin[2] = cfg.bmin[2] + ty*tcs;
	tcfg.bmax[0] = cfg.bmin[0] + (tx+1)*tcs;
	tcfg.bmax[1] = cfg.bmax[1];
	tcfg.bmax[2] = cfg.bmin[2] + (ty+1)*tcs;
	tcfg.bmin[0] -= tcfg.borderSize * tcfg.cs;
	tcfg.bmin[2] -= tcfg.borderSize * tcfg.cs;
	tcfg.bmax[0] += tcfg.borderSize * tcfg.cs;
	tcfg.bmax[2] += tcfg.borderSize * tcfg.cs;
	
	// Allocate voxel heightfield where we rasterize our input data to.
	m_solid = rcAllocHeightfield();
	if (!m_solid)
	{
		ctx->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'solid'.");
		return 0;
	}
	if (!rcCreateHeightfield(ctx, *m_solid, tcfg.width, tcfg.height, tcfg.bmin, tcfg.bmax, tcfg.cs, tcfg.ch))
	{
		ctx->log(RC_LOG_ERROR, "buildNavigation: Could not create solid heightfield.");
		return 0;
	}
	
	//--------------------------------------------------------------------------------------------
	
	rasterizeBitmap(*m_solid, p_layer);
	
	//--------------------------------------------------------------------------------------------
	
#if 0 // We can propably skip the following filter functions.
	
	// Once all geometry is rasterized, we do initial pass of filtering to
	// remove unwanted overhangs caused by the conservative rasterization
	// as well as filter spans where the character cannot possibly stand.
	rcFilterLowHangingWalkableObstacles(ctx, tcfg.walkableClimb, *m_solid);
	rcFilterLedgeSpans(ctx, tcfg.walkableHeight, tcfg.walkableClimb, *m_solid);
	rcFilterWalkableLowHeightSpans(ctx, tcfg.walkableHeight, *m_solid);
	
#endif
	
	m_chf = rcAllocCompactHeightfield();
	if (!m_chf)
	{
		ctx->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'chf'.");
		return 0;
	}
	if (!rcBuildCompactHeightfield(ctx, tcfg.walkableHeight, tcfg.walkableClimb, *m_solid, *m_chf))
	{
		ctx->log(RC_LOG_ERROR, "buildNavigation: Could not build compact data.");
		return 0;
	}
	
	// Erode the walkable area by agent radius.
	if (!rcErodeWalkableAreaRect(ctx, tcfg.walkableRadius, *m_chf))
	{
		ctx->log(RC_LOG_ERROR, "buildNavigation: Could not erode.");
		return 0;
	}
	
	/*
	// (Optional) Mark areas.
	const ConvexVolume* vols = geom->getConvexVolumes();
	for (int i  = 0; i < geom->getConvexVolumeCount(); ++i)
	{
		rcMarkConvexPolyArea(ctx, vols[i].verts, vols[i].nverts,
							 vols[i].hmin, vols[i].hmax,
							 (unsigned char)vols[i].area, *m_chf);
	}
	*/
	
	m_lset = rcAllocHeightfieldLayerSet();
	if (!m_lset)
	{
		ctx->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'lset'.");
		return 0;
	}
	if (!rcBuildHeightfieldLayers(ctx, *m_chf, tcfg.borderSize, tcfg.walkableHeight, *m_lset))
	{
		ctx->log(RC_LOG_ERROR, "buildNavigation: Could not build heighfield layers.");
		return 0;
	}
	
	m_ntiles = 0;
	for (int i = 0; i < rcMin(m_lset->nlayers, MAX_LAYERS); ++i)
	{
		TileCacheData* tile = &m_tiles[m_ntiles++];
		const rcHeightfieldLayer* layer = &m_lset->layers[i];
		
		// Store header
		dtTileCacheLayerHeader header;
		header.magic = DT_TILECACHE_MAGIC;
		header.version = DT_TILECACHE_VERSION;
		
		// Tile layer location in the navmesh.
		header.tx = tx;
		header.ty = ty;
		header.tlayer = i;
		dtVcopy(header.bmin, layer->bmin);
		dtVcopy(header.bmax, layer->bmax);
		
		// Tile info.
		header.width = (unsigned char)layer->width;
		header.height = (unsigned char)layer->height;
		header.minx = (unsigned char)layer->minx;
		header.maxx = (unsigned char)layer->maxx;
		header.miny = (unsigned char)layer->miny;
		header.maxy = (unsigned char)layer->maxy;
		header.hmin = (unsigned short)layer->hmin;
		header.hmax = (unsigned short)layer->hmax;
		
		dtStatus status = dtBuildTileCacheLayer(&comp, &header, layer->heights, layer->areas, layer->cons,
		                                        &tile->data, &tile->dataSize);
		if (dtStatusFailed(status))
		{
			return 0;
		}
	}

	// Transfer ownsership of tile data from build context to the caller.
	int n = 0;
	for (int i = 0; i < rcMin(m_ntiles, maxTiles); ++i)
	{
		tiles[n++] = m_tiles[i];
		m_tiles[i].data = 0;
		m_tiles[i].dataSize = 0;
	}
	
	return n;
}


void TileRasterizationContext::render() const
{
	tt::engine::debug::DebugRendererPtr debug = 
		tt::engine::renderer::Renderer::getInstance()->getDebug();
	
	RecastDebugDrawTT ttDebugDraw;
	
	// Debug renders
	const DebugRenderMask& mask = AppGlobal::getDebugRenderMask();
	
	if (m_chf != 0 &&
	    mask.checkFlag(DebugRender_PathMgrCompactHeightfield))
	{
		duDebugDrawCompactHeightfieldRegions(&ttDebugDraw, *m_chf);
		//duDebugDrawCompactHeightfieldSolid(&ttDebugDraw, *m_chf);
	}
	
	if (m_solid != 0 &&
	    mask.checkFlag(DebugRender_PathMgrHeightfield))
	{
		const bool drawWalkable = true;
		if (drawWalkable)
		{
			duDebugDrawHeightfieldWalkable(&ttDebugDraw, *m_solid);
		}
		else
		{
			duDebugDrawHeightfieldSolid(&ttDebugDraw, *m_solid);
		}
	}
}


//--------------------------------------------------------------------------------------------------
// Private member functions

void TileRasterizationContext::rasterizeBitmap(rcHeightfield& solid, const level::AttributeLayerPtr& p_layer)
{
	if (solid.bmin[1] > 0 || solid.bmax[1] < 0) // "above" or "below" our layer.
	{
		return;
	}
	const int flagMergeThr = 1;
	
	unsigned short ismin = 0;
	unsigned short ismax = 1;
	
	TT_NULL_ASSERT(p_layer);
	
	tt::math::Point2 minPos(level::worldToTile(solid.bmin[0]), level::worldToTile(solid.bmin[2]));
	tt::math::Point2 maxPos(level::worldToTile(solid.bmax[0]), level::worldToTile(solid.bmax[2]));
	const tt::math::Vector2 offset(solid.bmin[0], solid.bmin[2]);
	
	//TT_Printf("TRContext::rasterizeBitmap - min: (%d, %d), max: (%d, %d), offset: (%f, %f).\n",
	//          minPos.x, minPos.y, maxPos.x, maxPos.y, offset.x, offset.y);
	
	const int layerTilesPerCell = (int)ceilf(1.0f / solid.cs);
	// We only support 1.0f, 0.5f, 0.25f, etc. as cellSize.
	TT_ASSERT(tt::math::realEqual(1.0f / layerTilesPerCell, solid.cs));
	
	TT_ASSERT(((maxPos.x - minPos.x) / solid.cs) - (layerTilesPerCell - 1) <= solid.width);
	TT_ASSERT(((maxPos.y - minPos.y) / solid.cs) - (layerTilesPerCell - 1) <= solid.height);
	
	//TT_Printf("TRContext::rasterizeBitmap - rect w: %d, h: %d, solid w: %d, h: %d.\n",
	//          (maxPos.x - minPos.x), (maxPos.y - minPos.y), solid.width, solid.height);
	
	const s32 w = p_layer->getWidth();
	const s32 h = p_layer->getHeight();
	
	// Clamp to level
	minPos.x = std::max(minPos.x, static_cast<s32>(0));
	minPos.y = std::max(minPos.y, static_cast<s32>(0));
	maxPos.x = std::min(maxPos.x, w - 1);
	maxPos.y = std::min(maxPos.y, h - 1);
	
	if (minPos.x >= w || maxPos.x < 0 ||
	    minPos.y >= h || maxPos.y < 0)
	{
		// Completely outside of level.
		return;
	}
	TT_ASSERT(minPos.x <= maxPos.x);
	TT_ASSERT(minPos.y <= maxPos.y);
	
	//TT_Printf("TRContext::rasterizeBitmap - min: (%d, %d), max: (%d, %d)\n",
	//          minPos.x, minPos.y, maxPos.x, maxPos.y);
	
	const u8* rawDataPtr = p_layer->getRawData();
	const u8* columnPtr  = &rawDataPtr[(minPos.y * w) + minPos.x];
	
	const tt::math::Point2 cellOffset((s32)tt::math::ceil(offset.x / solid.cs),
	                                  (s32)tt::math::ceil(offset.y / solid.cs));
	
	for (tt::math::Point2 pos(minPos); pos.y <= maxPos.y; ++pos.y, columnPtr += w)
	{
		const u8* tilePtr = columnPtr;
		for (pos.x = minPos.x; pos.x <= maxPos.x; ++pos.x, ++tilePtr)
		{
			const u8 value = *tilePtr;
			
			const level::CollisionType collisionType = level::getCollisionType(value);
			
#if defined(TT_BUILD_DEV)
			TT_ASSERT(collisionType == p_layer->getCollisionType(pos));
#endif
			
			// FIXME: Combine fluids with isSolidForPathfinding
			if (level::isSolidForPathfinding(collisionType) || level::isFluidStill(collisionType))
			{
				continue; // Can't move through collision nor fluids
			}
			
			unsigned char area = PolyAreas_Air_NormalCost;
			
			switch (collisionType)
			{
			case level::CollisionType_AirPrefer: area = PolyAreas_Air_LowCost;  break;
			case level::CollisionType_AirAvoid:  area = PolyAreas_Air_HighCost; break;
			default:                                                            break;
			}
			
			const int x = (pos.x * layerTilesPerCell) - cellOffset.x;
			const int y = (pos.y * layerTilesPerCell) - cellOffset.y;
			
			for (int cellX = 0; cellX < layerTilesPerCell; ++cellX)
			{
				for (int cellY = 0; cellY < layerTilesPerCell; ++cellY)
				{
					addSpan(solid, x + cellX, y + cellY, ismin, ismax, area, flagMergeThr);
				}
			}
		}
	}
}


void TileRasterizationContext::addSpan(rcHeightfield& hf, const int x, const int y,
                      const unsigned short smin, const unsigned short smax,
                      const unsigned char area, const int flagMergeThr)
{
	if (x < 0 || x >= hf.width ||
	    y < 0 || y >= hf.height)
	{
		return;
	}
	
	int idx = x + y*hf.width;
	
	rcSpan* s = allocSpan(hf);
	s->smin = smin;
	s->smax = smax;
	s->area = area;
	s->next = 0;
	
	// Empty cell, add he first span.
	if (!hf.spans[idx])
	{
		hf.spans[idx] = s;
		return;
	}
	rcSpan* prev = 0;
	rcSpan* cur = hf.spans[idx];
	
	// Insert and merge spans.
	while (cur)
	{
		if (cur->smin > s->smax)
		{
			// Current span is further than the new span, break.
			break;
		}
		else if (cur->smax < s->smin)
		{
			// Current span is before the new span advance.
			prev = cur;
			cur = cur->next;
		}
		else
		{
			// Merge spans.
			if (cur->smin < s->smin)
				s->smin = cur->smin;
			if (cur->smax > s->smax)
				s->smax = cur->smax;
			
			// Merge flags.
			if (rcAbs((int)s->smax - (int)cur->smax) <= flagMergeThr)
				s->area = rcMax(s->area, cur->area);
			
			// Remove current span.
			rcSpan* next = cur->next;
			freeSpan(hf, cur);
			if (prev)
				prev->next = next;
			else
				hf.spans[idx] = next;
			cur = next;
		}
	}
	
	// Insert new span.
	if (prev)
	{
		s->next = prev->next;
		prev->next = s;
	}
	else
	{
		s->next = hf.spans[idx];
		hf.spans[idx] = s;
	}
}


rcSpan* TileRasterizationContext::allocSpan(rcHeightfield& hf)
{
	// If running out of memory, allocate new page and update the freelist.
	if (!hf.freelist || !hf.freelist->next)
	{
		// Create new page.
		// Allocate memory for the new pool.
		rcSpanPool* pool = (rcSpanPool*)rcAlloc(sizeof(rcSpanPool), RC_ALLOC_PERM);
		if (!pool) return 0;
		pool->next = 0;
		// Add the pool into the list of pools.
		pool->next = hf.pools;
		hf.pools = pool;
		// Add new items to the free list.
		rcSpan* freelist = hf.freelist;
		rcSpan* head = &pool->items[0];
		rcSpan* it = &pool->items[RC_SPANS_PER_POOL];
		do
		{
			--it;
			it->next = freelist;
			freelist = it;
		}
		while (it != head);
		hf.freelist = it;
	}
	
	// Pop item from in front of the free list.
	rcSpan* it = hf.freelist;
	hf.freelist = hf.freelist->next;
	return it;
}


void TileRasterizationContext::freeSpan(rcHeightfield& hf, rcSpan* ptr)
{
	if (!ptr) return;
	// Add the node in front of the free list.
	ptr->next = hf.freelist;
	hf.freelist = ptr;
}

// Namespace end
}
}
}

