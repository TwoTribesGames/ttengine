#include <cstring>
#include <cfloat>

#include <recastnavigation/Detour/DetourCommon.h>
#include <recastnavigation/Detour/DetourNavMeshBuilder.h>
#include <recastnavigation/DetourTileCache/DetourTileCache.h>
#include <recastnavigation/DebugUtils/DetourDebugDraw.h>
#include <recastnavigation/DebugUtils/RecastDebugDraw.h>
#include <recastnavigation/DebugUtils/tt/RecastDebugDrawTT.h>
#include <recastnavigation/Recast/RecastAlloc.h>

#include <tt/code/BufferReadContext.h>
#include <tt/code/bufferutils.h>
#include <tt/code/BufferWriteContext.h>
#include <tt/engine/debug/DebugRenderer.h>
#include <tt/engine/renderer/ColorRGBA.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/fs/utils/utils.h>
#include <tt/fs/File.h>
#include <tt/mem/util.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>
#include <tt/system/Time.h>

#include <toki/game/pathfinding/fwd.h>
#include <toki/game/pathfinding/TileCache.h>
#include <toki/game/pathfinding/TileRasterizationContext.h>
#include <toki/level/AttributeLayer.h>
#include <toki/level/helpers.h>
#include <toki/level/types.h>

#include <toki/serialization/SerializationMgr.h>
#include <toki/AppGlobal.h>

#if !defined(TT_BUILD_FINAL)
//#define DEBUG_PATHMGR
#endif

namespace toki {
namespace game {
namespace pathfinding {


//--------------------------------------------------------------------------------------------------
// Helpers

struct LinearAllocator : public dtTileCacheAlloc
{
	unsigned char* buffer;
	int capacity;
	int top;
	int high;
	
	LinearAllocator(const int cap) : buffer(0), capacity(0), top(0), high(0)
	{
		resize(cap);
	}
	
	~LinearAllocator()
	{
		dtFree(buffer);
	}

	void resize(const int cap)
	{
		if (buffer) dtFree(buffer);
		buffer = (unsigned char*)dtAlloc(cap, DT_ALLOC_PERM);
		capacity = cap;
	}
	
	virtual void reset()
	{
		high = dtMax(high, top);
		top = 0;
	}
	
	virtual void* alloc(const int size)
	{
		if (!buffer)
			return 0;
		if (top+size > capacity)
			return 0;
		unsigned char* mem = &buffer[top];
		top += size;
		return mem;
	}
	
	virtual void free(void* /*ptr*/)
	{
		// Empty
	}
};


struct MeshProcess : public dtTileCacheMeshProcess
{
	/*
	InputGeom* m_geom;

	inline MeshProcess() : m_geom(0)
	{
	}

	inline void init(InputGeom* geom)
	{
		m_geom = geom;
	}
	*/
	
	virtual void process(struct dtNavMeshCreateParams* params,
	                     unsigned char* polyAreas, unsigned short* polyFlags)
	{
		// Update poly flags from areas.
		for (int i = 0; i < params->polyCount; ++i)
		{
			if (polyAreas[i] == DT_TILECACHE_WALKABLE_AREA ||
			    polyAreas[i] == RC_WALKABLE_AREA)
			{
				polyAreas[i] = PolyAreas_Air_NormalCost;
			}
			
			if (polyAreas[i] == PolyAreas_Air_LowCost    ||
			    polyAreas[i] == PolyAreas_Air_NormalCost ||
			    polyAreas[i] == PolyAreas_Air_HighCost   )
			{
				polyFlags[i] = PolyFlags_Fly;
			}
			else if (polyAreas[i] == PolyAreas_Warp)
			{
				polyFlags[i] = PolyFlags_Fly | PolyFlags_Warp;
			}
		}
		
		// Pass in off-mesh connections.
		//if (m_geom)
		{
			params->offMeshConVerts  = 0; //m_geom->getOffMeshConnectionVerts();
			params->offMeshConRad    = 0; //m_geom->getOffMeshConnectionRads();
			params->offMeshConDir    = 0; //m_geom->getOffMeshConnectionDirs();
			params->offMeshConAreas  = 0; //m_geom->getOffMeshConnectionAreas();
			params->offMeshConFlags  = 0; //m_geom->getOffMeshConnectionFlags();
			params->offMeshConUserID = 0; //m_geom->getOffMeshConnectionId();
			params->offMeshConCount  = 0; //m_geom->getOffMeshConnectionCount();	
		}
	}
};


//--------------------------------------------------------------------------------------------------
// Public member functions

TileCache::TileCache(real p_agentRadius)
:
m_navMesh(0),
m_navQuery(0),
m_crowd(0),
m_agentRadius(p_agentRadius),
m_talloc(0),
m_tcomp(0),
m_tmproc(0),
m_tileCache(0),
m_targetRef(0)
{
	dtVset(m_targetPos, 0.0f, 0.0f, 0.0f);
	m_navQuery = dtAllocNavMeshQuery();
	
	m_talloc = new LinearAllocator(32000);
	m_tcomp  = new FastLZCompressor;
	m_tmproc = new MeshProcess;
	
	tt::mem::zero8(m_obstacles, sizeof(dtObstacleRef) * MaxObstacles);
}


TileCache::~TileCache()
{
	cleanup();
	
	delete m_talloc;
	delete m_tcomp;
	delete m_tmproc;
	
	dtFreeNavMeshQuery(m_navQuery);
	dtFreeNavMesh(     m_navMesh);
	dtFreeCrowd(       m_crowd);
}


void TileCache::cleanup()
{
	dtFreeNavMesh(m_navMesh);
	m_navMesh = 0;
	dtFreeTileCache(m_tileCache);
	m_tileCache = 0;
}


void TileCache::load(tt::code::BufferReadContext* p_context)
{
#if defined(DEBUG_PATHMGR)
	const u64 startTime = tt::system::Time::getInstance()->getMilliSeconds();
#endif
	
	namespace bu = tt::code::bufferutils;
	
	s32 levelWidth  = bu::get<s32>(p_context);
	s32 levelHeight = bu::get<s32>(p_context);
	s32 ntiles      = bu::get<s32>(p_context);
	
	s32 tileWidth  = 0;
	s32 tileHeight = 0;
	rcConfig          cfg;
	dtTileCacheParams tcParams;
	dtNavMeshParams   nmParams;
	
	initConfig(levelWidth, levelHeight, tileWidth, tileHeight, cfg, &tcParams, &nmParams);
	initMemory(cfg, tcParams, nmParams);
	
	for (s32 i = 0; i < ntiles; ++i)
	{
		TileCacheData tile;
		tile.dataSize = bu::get<u32>(p_context);
		
		tile.data = reinterpret_cast<unsigned char*>(dtAlloc(tile.dataSize, DT_ALLOC_PERM));
		
		bu::get(tile.data, tile.dataSize, p_context);
		
		dtStatus status = m_tileCache->addTile(tile.data, tile.dataSize, DT_COMPRESSEDTILE_FREE_DATA, 0);
		
		// Failed on wrong magic number; try to swap endianess
		if (dtStatusDetail(status, DT_WRONG_MAGIC))
		{
			dtTileCacheHeaderSwapEndian(tile.data, tile.dataSize);
			status = m_tileCache->addTile(tile.data, tile.dataSize, DT_COMPRESSEDTILE_FREE_DATA, 0);
		}
		
		if (dtStatusFailed(status))
		{
			TT_PANIC("Failed to add tile to tilecache. Status 0x%x", status);
			dtFree(tile.data);
			tile.data = 0;
			continue;
		}
	}
	
	// Post processing
	buildMeshes(tileWidth, tileHeight);
	initCrowd();
	
#if defined(DEBUG_PATHMGR)
	const u64 duration = tt::system::Time::getInstance()->getMilliSeconds() - startTime;
	TT_Printf("TileCache::load duration: %u ms\n", static_cast<u32>(duration));
#endif
}


void TileCache::save(const level::AttributeLayerPtr& p_layer, tt::code::BufferWriteContext* p_context) const
{
	const s32 levelWidth  = p_layer->getWidth();
	const s32 levelHeight = p_layer->getHeight();
	s32 tileWidth  = 0;
	s32 tileHeight = 0;
	rcConfig cfg;
	initConfig(levelWidth, levelHeight, tileWidth, tileHeight, cfg);
	
	// Get preprocessed tiles.
	TileCacheData* tileCacheData = 0;
	const s32 ntiles = preprocess(p_layer, tileWidth, tileHeight, cfg, tileCacheData);
	TT_NULL_ASSERT(tileCacheData);
	
	namespace bu = tt::code::bufferutils;
	
	// Store width/height
	bu::put(levelWidth, p_context);
	bu::put(levelHeight, p_context);
	
	// Save preprocessed tiles.
	bu::put(ntiles, p_context);
	
	for (s32 i = 0; i < ntiles; ++i)
	{
		TileCacheData* tile = &tileCacheData[i];
		TT_NULL_ASSERT(tile->data);
		
		// Save datasize tiles.
		u32 dataSize = static_cast<u32>(tile->dataSize);
		bu::put(dataSize, p_context);
		
		// Save data
		bu::put(tile->data, tile->dataSize, p_context);
		
		// Free the data
		dtFree(tile->data);
		tile->data = 0;
	}
	
	delete [] tileCacheData;
	tileCacheData = 0;
}


void TileCache::build(const level::AttributeLayerPtr& p_layer)
{
#if defined(TT_PLATFORM_WIN) && defined(TT_BUILD_DEV)
	TT_ASSERT(_CrtCheckMemory());
#endif
	
#if defined(DEBUG_PATHMGR)
	const u64 startTime = tt::system::Time::getInstance()->getMilliSeconds();
#endif
	
	s32 tileWidth  = 0;
	s32 tileHeight = 0;
	rcConfig          cfg;
	dtTileCacheParams tcParams;
	dtNavMeshParams   nmParams;
	initConfig(p_layer->getWidth(), p_layer->getHeight(), tileWidth, tileHeight, cfg, &tcParams, &nmParams);
	initMemory(cfg, tcParams, nmParams);
	
	// Get preprocessed tiles.
	TileCacheData* tileCacheData = 0;
	const s32 ntiles = preprocess(p_layer, tileWidth, tileHeight, cfg, tileCacheData);
	TT_NULL_ASSERT(tileCacheData);
	
	// Add preprocessed tiles.
	for (s32 i = 0; i < ntiles; ++i)
	{
		TileCacheData* tile = &tileCacheData[i];
		TT_NULL_ASSERT(tile->data);
		dtStatus status = m_tileCache->addTile(tile->data, tile->dataSize, DT_COMPRESSEDTILE_FREE_DATA, 0);
		if (dtStatusFailed(status))
		{
			TT_PANIC("Failed to add tile to tilecache. Status 0x%x", status);
			dtFree(tile->data);
			tile->data = 0;
			continue;
		}
	}
	
	delete [] tileCacheData;
	tileCacheData = 0;
	
	// Post processing
	buildMeshes(tileWidth, tileHeight);
	initCrowd();

#if defined(DEBUG_PATHMGR)
	const u64 duration = tt::system::Time::getInstance()->getMilliSeconds() - startTime;
	TT_Printf("TileCache::build duration: %u ms\n", static_cast<u32>(duration));
#endif
	
#if defined(TT_PLATFORM_WIN) && defined(TT_BUILD_DEV)
	TT_ASSERT(_CrtCheckMemory());
#endif
}


void TileCache::initCrowd()
{
	if (m_navMesh == 0)
	{
		return;
	}
	
	if (m_crowd == 0)
	{
		m_crowd = dtAllocCrowd();
	}
	
	TT_NULL_ASSERT(m_crowd);
	
	static const int MAX_AGENTS = 256;
	m_crowd->init(MAX_AGENTS, std::max(m_agentRadius, 1.0f), m_navMesh);
	
	// Make polygons with 'disabled' flag invalid.
	dtQueryFilter* filter = m_crowd->getEditableFilter();
	filter->setExcludeFlags(PolyFlags_Disabled);
	
	filter->setAreaCost(PolyAreas_Air_LowCost   ,  1.0f);
	filter->setAreaCost(PolyAreas_Air_NormalCost,  8.0f);
	filter->setAreaCost(PolyAreas_Air_HighCost  , 64.0f);
	
	// Setup local avoidance params to different qualities.
	dtObstacleAvoidanceParams params;
	// Use mostly default settings, copy from dtCrowd.
	memcpy(&params, m_crowd->getObstacleAvoidanceParams(0), sizeof(dtObstacleAvoidanceParams));
	
	// Low (11)
	params.velBias = 0.5f;
	params.adaptiveDivs = 5;
	params.adaptiveRings = 2;
	params.adaptiveDepth = 1;
	m_crowd->setObstacleAvoidanceParams(0, &params);
	
	// Medium (22)
	params.velBias = 0.5f;
	params.adaptiveDivs = 5; 
	params.adaptiveRings = 2;
	params.adaptiveDepth = 2;
	m_crowd->setObstacleAvoidanceParams(1, &params);
	
	// Good (45)
	params.velBias = 0.5f;
	params.adaptiveDivs = 7;
	params.adaptiveRings = 2;
	params.adaptiveDepth = 3;
	m_crowd->setObstacleAvoidanceParams(2, &params);
	
	// High (66)
	params.velBias = 0.5f;
	params.adaptiveDivs = 7;
	params.adaptiveRings = 3;
	params.adaptiveDepth = 3;
	
	m_crowd->setObstacleAvoidanceParams(3, &params);
}


bool TileCache::hasTileCacheUpdatesPending() const
{
	if (m_navMesh == 0 || m_tileCache == 0)
	{
		return false;
	}
	return m_tileCache->getPendingUpdateCount() > 0;
}


void TileCache::update(real p_deltaTime)
{
	if (m_navMesh != 0)
	{
		if (m_tileCache != 0)
		{
			m_tileCache->update(p_deltaTime, m_navMesh);
		}
		
		if (m_crowd != 0)
		{
			m_crowd->update(p_deltaTime, 0); //&m_agentDebug); // TODO: Add a dtCrowdAgentDebugInfo* here?
		}
	}
}


void TileCache::render() const
{
#if !defined(TT_BUILD_FINAL)
	const DebugRenderMask& mask = AppGlobal::getDebugRenderMask();
	
	// Early out to limit RecastDebugDrawTT creation.
	if (mask.checkFlag(DebugRender_PathMgrNavMesh)   == false &&
	    mask.checkFlag(DebugRender_PathMgrAgents)    == false &&
	    mask.checkFlag(DebugRender_PathMgrObstacles) == false)
	{
		return;
	}
	
	tt::engine::debug::DebugRendererPtr debug = 
		tt::engine::renderer::Renderer::getInstance()->getDebug();
	
	RecastDebugDrawTT ttDebugDraw; // RIIA for renderer states, but also has render buffers.
	//  FIXME: Split RecastDebugDrawTT so we can keep buffers as member and don't create the buffers each frame.
	
	// Debug renders
	
	if (m_navMesh != 0 &&
	    mask.checkFlag(DebugRender_PathMgrNavMesh))
	{
		duDebugDrawNavMesh(&ttDebugDraw, *m_navMesh, DU_DRAWNAVMESH_OFFMESHCONS);
	}
	
	if (m_crowd != 0 &&
	    mask.checkFlag(DebugRender_PathMgrAgents))
	{
		for (int i = 0; i < m_crowd->getAgentCount(); ++i)
		{
			const dtCrowdAgent* ag = m_crowd->getAgent(i);
			if (!ag->active) continue;
			
			const float radius = ag->params.radius;
			const float* pos   = ag->npos;
			const float* vel   = ag->vel;
			const float* dvel  = ag->dvel;
			
			tt::engine::renderer::ColorRGBA col(220,220,220,128);
			if (ag->targetState == DT_CROWDAGENT_TARGET_REQUESTING       ||
			    ag->targetState == DT_CROWDAGENT_TARGET_WAITING_FOR_QUEUE)
			{
				//col = duLerpCol(col, duRGBA(128,0,255,128), 32);
				col.setColor(32,0,100,128);
			}
			else if (ag->targetState == DT_CROWDAGENT_TARGET_WAITING_FOR_PATH)
			{
				//col = duLerpCol(col, duRGBA(128,0,255,128), 128);
				col.setColor(128,0,255,128);
			}
			else if (ag->targetState == DT_CROWDAGENT_TARGET_FAILED)
			{
				col.setColor(255,32,16,128);
			}
			else if (ag->targetState == DT_CROWDAGENT_TARGET_VELOCITY)
			{
				//col = duLerpCol(col, duRGBA(64,255,0,128), 128);
				col.setColor(64,255,0,128);
			}
			
			//duDebugDrawCylinder(&dd, pos[0]-radius, pos[1]+radius*0.1f, pos[2]-radius,
			//					pos[0]+radius, pos[1]+height, pos[2]+radius, col);
			
			const tt::math::Vector2 posVec(  pos[0] , pos[2]);
			const tt::math::Vector3 posVec3D(pos[0] , pos[2] , 0.0f);
			const tt::math::Vector3 dvelVec( dvel[0], dvel[2], 0.0f);
			const tt::math::Vector3 velVec(  vel[0] , vel[2] , 0.0f);
			debug->renderCircle(col, posVec, radius);
			debug->renderLine(tt::engine::renderer::ColorRGBA(0, 192, 255, 192), posVec3D, posVec3D + dvelVec);
			debug->renderLine(tt::engine::renderer::ColorRGBA(0, 0  , 0  , 160), posVec3D, posVec3D + dvelVec);
			
			// Render corner vectors.
			// The following is duplicate code from calcSmoothSteerDirection.
			if (ag->ncorners > 0)
			{
				const int ip0 = 0;
				const int ip1 = dtMin(1, ag->ncorners-1);
				const float* p0 = &ag->cornerVerts[ip0*3];
				const float* p1 = &ag->cornerVerts[ip1*3];
				
				float dir0[3], dir1[3];
				dtVsub(dir0, p0, ag->npos);
				dtVsub(dir1, p1, ag->npos);
				dir0[1] = 0;
				dir1[1] = 0;
				
				const float len0 = dtMin(10.0f, dtVlen(dir0));
				const float len1 = dtVlen(dir1);
				if (len1 > 0.001f)
					dtVscale(dir1,dir1,1.0f/len1);
				
				const float len = len0 * 0.25f;
				
				float dir[3]; // This is the parameter in calcSmoothSteerDirection.
				dir[0] = dir0[0] - dir1[0] * len;
				dir[1] = 0;
				dir[2] = dir0[2] - dir1[2] * len;
				
				const tt::math::Vector3 p0Vec3D(p0[0] , p0[2] , 0.0f);
				const tt::math::Vector3 p1Vec3D(p1[0] , p1[2] , 0.0f);
				const tt::math::Vector3 dirVec( dir[0], dir[2], 0.0f);
				
				debug->renderLine(tt::engine::renderer::ColorRGBA(255,   0,   0, 160), posVec3D, p0Vec3D);
				debug->renderLine(tt::engine::renderer::ColorRGBA(  0, 255,   0, 160), posVec3D, p1Vec3D);
				debug->renderLine(tt::engine::renderer::ColorRGBA(  0,   0, 255, 160), posVec3D, posVec3D + dirVec);
			}
		}
	}
	
	if (m_targetRef != 0 &&
	    mask.checkFlag(DebugRender_PathMgrAgents))
	{
		const tt::math::Vector3 targetPos(m_targetPos[0] ,m_targetPos[2] , 0.0f);
		const tt::engine::renderer::ColorRGBA col(255, 255, 255, 192);
		
		// bottom left -> top right line
		tt::math::Vector3 crossCorner(1.0f, 1.0f, 0.0f);
		debug->renderLine(col, targetPos - crossCorner, targetPos + crossCorner);
		// top left -> bottom right line
		crossCorner.y = -1.0f;
		debug->renderLine(col, targetPos - crossCorner, targetPos + crossCorner);
	}
	
	if (m_tileCache != 0 &&
	    mask.checkFlag(DebugRender_PathMgrObstacles))
	{
		// Draw obstacles
		for (int i = 0; i < m_tileCache->getObstacleCount(); ++i)
		{
			const dtTileCacheObstacle* ob = m_tileCache->getObstacle(i);
			if (ob->state == DT_OBSTACLE_EMPTY) continue;
			float bmin[3], bmax[3];
			if (ob->isBox)
			{
				dtVcopy(bmin, ob->pos);
				dtVcopy(bmax, ob->maxPos);
			}
			else
			{
				m_tileCache->getObstacleBounds(ob, bmin, bmax);
			}
			
			unsigned int col = 0;
			if (ob->state == DT_OBSTACLE_PROCESSING)
			{
				col = duRGBA(255,255,0,128);
			}
			else if (ob->state == DT_OBSTACLE_PROCESSED)
			{
				col = duRGBA(0,192,255,192);
			}
			else if (ob->state == DT_OBSTACLE_REMOVING)
			{
				col = duRGBA(220,0,0,128);
			}
			
			if (ob->isBox)
			{
				unsigned int colSides[6];
				for (s32 sideIndex = 0; sideIndex < 6; ++sideIndex)
				{
					colSides[sideIndex] = col;
				}
				
				duDebugDrawBox    (&ttDebugDraw, bmin[0], bmin[1], bmin[2], bmax[0], bmax[1], bmax[2], colSides);
				duDebugDrawBoxWire(&ttDebugDraw, bmin[0], bmin[1], bmin[2], bmax[0], bmax[1], bmax[2], duDarkenCol(col), 2.0f);
			}
			else
			{
				duDebugDrawCylinder    (&ttDebugDraw, bmin[0],bmin[1],bmin[2], bmax[0],bmax[1],bmax[2], col);
				duDebugDrawCylinderWire(&ttDebugDraw, bmin[0],bmin[1],bmin[2], bmax[0],bmax[1],bmax[2], duDarkenCol(col), 2.0f);
			}
		}
	}
#endif
}


s32 TileCache::addAgent(const tt::math::Vector2& p_pos, const tt::math::Vector2* p_targetPos, bool p_separation)
{
	if (m_crowd == 0) return -1;
	dtCrowd* crowd = m_crowd;
	
	dtCrowdAgentParams ap;
	memset(&ap, 0, sizeof(ap));
	ap.radius = m_agentRadius;
	ap.height = 1.0f;
	ap.maxAcceleration = 8.0f;
	ap.maxSpeed = 3.5f;
	ap.collisionQueryRange = ap.radius * 12.0f;
	ap.pathOptimizationRange = ap.radius * 30.0f;
	ap.updateFlags = 0; 
	//if (m_toolParams.m_anticipateTurns)
		ap.updateFlags |= DT_CROWD_ANTICIPATE_TURNS;
	//if (m_toolParams.m_optimizeVis)
	//	ap.updateFlags |= DT_CROWD_OPTIMIZE_VIS;
	//if (m_toolParams.m_optimizeTopo)
		ap.updateFlags |= DT_CROWD_OPTIMIZE_TOPO;
	//if (m_toolParams.m_obstacleAvoidance)
		ap.updateFlags |= DT_CROWD_OBSTACLE_AVOIDANCE; // FIXME: Turning this off will increase performance.
	if (p_separation)
		ap.updateFlags |= DT_CROWD_SEPARATION;
	ap.obstacleAvoidanceType = 3;//(unsigned char)m_toolParams.m_obstacleAvoidanceType;
	ap.separationWeight = 2.0f; //m_toolParams.m_separationWeight;
	
	const float pos[3] = { p_pos.x, 0.0f, p_pos.y };
	
	int idx = crowd->addAgent(pos, &ap);
	if (idx != -1)
	{
		if (p_targetPos != 0)
		{
			const float trgPos[3] = { p_targetPos->x, 0.0f, p_targetPos->y };
			float closestPos[3]   = { 0.0f          , 0.0f, 0.0f           };
			
			const dtQueryFilter* filter = m_crowd->getFilter();
			const float* ext = m_crowd->getQueryExtents();
			dtPolyRef targetRef;
			
			dtStatus state = m_navQuery->findNearestPoly(trgPos, ext, filter, &targetRef, closestPos);
			//TT_Printf("TileCache::addAgent target pos %f, %f -> %f, %f (PolyRef:% d)\n", p_targetPos->x, p_targetPos->y, closestPos[0], closestPos[2], targetRef);
			TT_ASSERT(state == DT_SUCCESS);
			crowd->requestMoveTarget(idx, targetRef, closestPos);
		}
	}
	else
	{
		TT_PANIC("Couldn't add agent! (Max agents reached?)");
	}
	
	return static_cast<s32>(idx);
}


void TileCache::removeAgent(const s32 p_idx)
{
	if (m_crowd == 0) return;
	
	m_crowd->removeAgent(static_cast<int>(p_idx));
	
	/*
	if (p_idx == m_agentDebug.idx)
	{
		m_agentDebug.idx = -1;
	}
	*/
}


void TileCache::removeAllAgents()
{
	if (m_crowd == 0) return;
	
	const int agentCount = m_crowd->getAgentCount();
	for (int i = 0; i < agentCount; ++i)
	{
		m_crowd->removeAgent(i);
	}
}


void TileCache::setMoveTarget(const tt::math::Vector2& p_pos)
{
	if (m_navQuery == 0 || m_crowd == 0)
	{
		return;
	}
	
	// Find nearest point on navmesh and set move request to that location.
	const dtQueryFilter* filter = m_crowd->getFilter();
	const float* ext = m_crowd->getQueryExtents();
	
	const float pos[3] = { p_pos.x, 0.0f, p_pos.y};
	m_navQuery->findNearestPoly(pos, ext, filter, &m_targetRef, m_targetPos);
	
	//TT_Printf("TileCache::setMoveTarget target pos %f, %f -> %f, %f\n", p_pos.x, p_pos.y, m_targetPos[0], m_targetPos[2]);
	/*
	if (m_agentDebug.idx != -1)
	{
		const dtCrowdAgent* ag = crowd->getAgent(m_agentDebug.idx);
		if (ag && ag->active)
			crowd->requestMoveTarget(m_agentDebug.idx, m_targetRef, m_targetPos);
	}
	else
	*/
	{
		for (int i = 0; i < m_crowd->getAgentCount(); ++i)
		{
			const dtCrowdAgent* ag = m_crowd->getAgent(i);
			if (!ag->active) continue;
			m_crowd->requestMoveTarget(i, m_targetRef, m_targetPos);
		}
	}
}


s32 TileCache::hitTestAgents(const tt::math::Vector2& p_pos) const
{
	if (m_crowd == 0) return -1;
	
	int   isel = -1;
	float tsel = FLT_MAX;
	
	// Find the agents closest to point.
	for (int i = 0; i < m_crowd->getAgentCount(); ++i)
	{
		const dtCrowdAgent* ag = m_crowd->getAgent(i);
		if (!ag->active) continue;
		
		const tt::math::Vector2 aPos(ag->npos[0], ag->npos[2]);
		
		const tt::math::Vector2 diff(p_pos - aPos);
		const real len = diff.lengthSquared();
		if (len > 0.0f && len < tsel)
		{
			isel = i;
			tsel = len;
		}
	}
	
	const dtCrowdAgent* ag = m_crowd->getAgent(isel);
	TT_ASSERT(ag->active);
	if (tsel < (ag->params.radius * ag->params.radius))
	{
		// The closest agent is close enough to point.
		return static_cast<s32>(isel);
	}
	
	return -1;
}


AgentState TileCache::getAgentState(s32 p_idx) const
{
	if (p_idx < 0)
	{
		return AgentState_Invalid;
	}
	
	TT_NULL_ASSERT(m_crowd);
	
	const dtCrowdAgent* ag = m_crowd->getAgent(p_idx);
	TT_ASSERT(ag->active);
	
	const dtPolyRef targetRef = ag->corridor.getLastPoly();
	
	switch (ag->state)
	{
	case DT_CROWDAGENT_STATE_INVALID:
		return AgentState_PathFailed;
	case DT_CROWDAGENT_STATE_WALKING:
	case DT_CROWDAGENT_STATE_OFFMESH:
		break; // BREAK (check targetState)
	}
	
	switch (ag->targetState)
	{
	case DT_CROWDAGENT_TARGET_NONE:              return AgentState_PathFailed; // AgentState_None;
	case DT_CROWDAGENT_TARGET_FAILED:            return AgentState_PathFailed;
	case DT_CROWDAGENT_TARGET_VALID:             return (targetRef != ag->targetRef) ?
	                                                    AgentState_PathFailed : AgentState_PathSucceded;
	case DT_CROWDAGENT_TARGET_REQUESTING:        return AgentState_Busy;
	case DT_CROWDAGENT_TARGET_WAITING_FOR_QUEUE: return AgentState_Busy;
	case DT_CROWDAGENT_TARGET_WAITING_FOR_PATH:  return AgentState_Busy;
	case DT_CROWDAGENT_TARGET_VELOCITY:          return AgentState_Invalid;
	default:
		TT_PANIC("Unknown crowdagent state: %d", ag->targetState);
		break;
	}
	
	return AgentState_Invalid;
}


tt::math::Vector2 TileCache::getAgentEndPoint(s32 p_idx) const
{
	if (p_idx < 0)
	{
		TT_PANIC("Invalid index");
		return tt::math::Vector2::zero;
	}
	
	TT_NULL_ASSERT(m_crowd);
	
	const dtCrowdAgent* ag = m_crowd->getAgent(p_idx);
	TT_ASSERT(ag->active);
	
	const float* target = ag->corridor.getTarget();
	return tt::math::Vector2(target[0], target[2]);
}


tt::math::Vector2 TileCache::getAgentVelocity(s32 p_idx) const
{
	if (p_idx < 0)
	{
		TT_PANIC("Invalid index");
		return tt::math::Vector2::zero;
	}
	
	TT_NULL_ASSERT(m_crowd);
	
	const dtCrowdAgent* ag = m_crowd->getAgent(p_idx);
	TT_ASSERT(ag->active);
	
	return tt::math::Vector2(ag->dvel[0], ag->dvel[2]);
}


void TileCache::updateAgentPosition(s32 p_idx, const tt::math::Vector2& p_pos)
{
	if (p_idx < 0)
	{
		TT_PANIC("Invalid index");
		return;
	}
	
	TT_NULL_ASSERT(m_crowd);
	
	dtCrowdAgent* ag = m_crowd->getAgent_HACK(p_idx);
	TT_ASSERT(ag->active);
	ag->npos[0] = p_pos.x;
	ag->npos[2] = p_pos.y;
}


void TileCache::updateTargetPosition(s32 p_idx, const tt::math::Vector2& p_targetPos)
{
	if (p_idx < 0)
	{
		TT_PANIC("Invalid index");
		return;
	}
	
	TT_NULL_ASSERT(m_crowd);
	
	const float trgPos[3] = { p_targetPos.x, 0.0f, p_targetPos.y };
	float closestPos[3]   = { 0.0f         , 0.0f, 0.0f          };
	
	const dtQueryFilter* filter = m_crowd->getFilter();
	const float* ext = m_crowd->getQueryExtents();
	dtPolyRef targetRef;
	
	m_navQuery->findNearestPoly(trgPos, ext, filter, &targetRef, closestPos);
	//TT_Printf("TileCache::updateTargetPosition target pos %f, %f -> %f, %f\n", p_targetPos.x, p_targetPos.y, closestPos[0], closestPos[2]);
	
	m_crowd->requestMoveTarget(p_idx, targetRef, closestPos);
}


/*
void CrowdToolState::updateAgentParams()
{
	if (!m_sample) return;
	dtCrowd* crowd = m_sample->getCrowd();
	if (!crowd) return;
	
	unsigned char updateFlags = 0;
	unsigned char obstacleAvoidanceType = 0;
	
	if (m_toolParams.m_anticipateTurns)
		updateFlags |= DT_CROWD_ANTICIPATE_TURNS;
	if (m_toolParams.m_optimizeVis)
		updateFlags |= DT_CROWD_OPTIMIZE_VIS;
	if (m_toolParams.m_optimizeTopo)
		updateFlags |= DT_CROWD_OPTIMIZE_TOPO;
	if (m_toolParams.m_obstacleAvoidance)
		updateFlags |= DT_CROWD_OBSTACLE_AVOIDANCE;
	if (m_toolParams.m_obstacleAvoidance)
		updateFlags |= DT_CROWD_OBSTACLE_AVOIDANCE;
	if (m_toolParams.m_separation)
		updateFlags |= DT_CROWD_SEPARATION;
	
	obstacleAvoidanceType = (unsigned char)m_toolParams.m_obstacleAvoidanceType;
	
	dtCrowdAgentParams params;
	
	for (int i = 0; i < crowd->getAgentCount(); ++i)
	{
		const dtCrowdAgent* ag = crowd->getAgent(i);
		if (!ag->active) continue;
		memcpy(&params, &ag->params, sizeof(dtCrowdAgentParams));
		params.updateFlags = updateFlags;
		params.obstacleAvoidanceType = obstacleAvoidanceType;
		params.separationWeight = m_toolParams.m_separationWeight;
		crowd->updateAgentParameters(i, &params);
	}	
}
*/


ObstacleIndex TileCache::addTempObstacle(const tt::math::VectorRect& p_rectangle)
{
	if (m_tileCache == 0)
	{
		return g_invalidObstacleIndex;
	}
	
	// Find a free spot for a new obstacle
	ObstacleIndex index = g_invalidObstacleIndex;
	for (s32 i = 0; i < MaxObstacles; ++i)
	{
		if (m_obstacles[i] == 0)
		{
			index = i;
			break;
		}
	}
	
	if (index == g_invalidObstacleIndex)
	{
		TT_PANIC("Cannot add another obstacle to tile cache for agent radius %f: "
		        "reached obstacle limit. MaxObstacles: %d.", m_agentRadius, MaxObstacles);
		return g_invalidObstacleIndex;
	}
	
	// Add a new obstacle to the Detour tile cache
	const float minPos[3] = { p_rectangle.getMin().x     - m_agentRadius, -0.5f, p_rectangle.getMin().y     - m_agentRadius };
	const float maxPos[3] = { p_rectangle.getMaxEdge().x + m_agentRadius,  1.5f, p_rectangle.getMaxEdge().y + m_agentRadius };
	//dtVcopy(p, pos);
	//p[1] -= 0.5f;
	dtObstacleRef obstacleRef = 0;
	dtStatus      status      = m_tileCache->addObstacle(minPos, maxPos, &obstacleRef);
	if (status != DT_SUCCESS)
	{
		TT_PANIC("Adding obstacle with rectangle at pos (%f, %f), size %f x %f."
		         "to tile cache for agent radius %f failed. Error code: '%x'",
		         p_rectangle.getMin().x, p_rectangle.getMin().y,
		         p_rectangle.getWidth(), p_rectangle.getHeight(),
		         m_agentRadius, status);
		return g_invalidObstacleIndex;
	}
	
	// Update bookkeeping
	//TT_Printf("TileCache::addTempObstacle: [%f] Index %d = obstacle 0x%08X\n",
	//          m_agentRadius, index, obstacleRef);
	m_obstacles[index] = obstacleRef;
	return index;
}


void TileCache::removeTempObstacle(ObstacleIndex p_index)
{
	if (m_tileCache != 0 && p_index >= 0)
	{
		TT_ASSERTMSG(p_index < MaxObstacles,
		             "Invalid obstacle index: %d. Can be at most %d.", p_index, MaxObstacles);
		if (p_index < MaxObstacles)
		{
			dtObstacleRef ref = m_obstacles[p_index];
			//TT_Printf("TileCache::removeTempObstacle: [%f] Removing index %d (obstacle 0x%08X)\n",
			//          m_agentRadius, p_index, ref);
			m_obstacles[p_index] = 0;
			dtStatus status = m_tileCache->removeObstacle(ref);
			TT_ASSERTMSG(status == DT_SUCCESS, "Removing path finding obstacle failed. Error code: '%x'", status);
		}
	}
}


void TileCache::clearAllTempObstacles()
{
	if (m_tileCache == 0)
	{
		return;
	}
	
	for (s32 i = 0; i < MaxObstacles; ++i)
	{
		if (m_obstacles[i] != 0)
		{
			m_tileCache->removeObstacle(m_obstacles[i]);
			m_obstacles[i] = 0;
		}
	}
}


ObstacleIndex TileCache::hitTestObstacle(const tt::math::Vector2& p_pos)
{
	if (m_tileCache == 0) return g_invalidObstacleIndex;
	
	float tmin = FLT_MAX;
	const dtTileCacheObstacle* obmin = 0;
	for (int i = 0; i < m_tileCache->getObstacleCount(); ++i)
	{
		const dtTileCacheObstacle* ob = m_tileCache->getObstacle(i);
		if (ob->state == DT_OBSTACLE_EMPTY)
		{
			continue;
		}
		
		const tt::math::Vector2 aPos(ob->pos[0], ob->pos[2]);
		
		const tt::math::Vector2 diff(p_pos - aPos);
		const real len = diff.lengthSquared();
		if (len > 0.0f && len < tmin)
		{
			tmin = len;
			obmin = ob;
		}
	}
	
	if (obmin != 0 &&
	    tmin < (obmin->radius * obmin->radius))
	{
		// The closest agent is close enough to point.
		dtObstacleRef ref = m_tileCache->getObstacleRef(obmin);
		TT_ASSERT(ref != 0);
		for (s32 i = 0; i < MaxObstacles; ++i)
		{
			if (ref == m_obstacles[i])
			{
				return i;
			}
		}
		TT_PANIC("Tile cache obstacle bookkeeping error: obstacle ref 0x%08X "
		         "is not present in the obstacle<->index mapping.", ref);
	}
	
	return g_invalidObstacleIndex;
}


void TileCache::serialize(toki::serialization::SerializationMgr& p_serializationMgr) const
{
	// FIXME: SERIALIZE HERE!!!!!!!!
	(void) p_serializationMgr;
}


void TileCache::unserialize(const toki::serialization::SerializationMgr& p_serializationMgr)
{
	(void) p_serializationMgr;
}


//--------------------------------------------------------------------------------------------------
// Private member functions

void TileCache::initConfig(s32 p_levelWidth, s32 p_levelHeight,
                           s32& p_tileWidth_OUT, s32& p_tileHeight_OUT, rcConfig& p_config_OUT,
                           dtTileCacheParams* p_tileCacheParams_OUT,
                           dtNavMeshParams* p_navMeshParams_OUT) const
{
	TT_ASSERT(p_levelWidth > 0);
	TT_ASSERT(p_levelHeight > 0);
	
	//
	// NOTE: IF ANY OF THESE VALUES ARE CHANGED, UPDATE THE VERSION NUMBER g_chunkVersionPathfinding IN LevelData
	//
	
	///////////////////////////////////////////////////////////////////////////
	// Init build configuration from GUI
	std::memset(&p_config_OUT, 0, sizeof(rcConfig));
	p_config_OUT.cs = 1.0f/3.0f; //0.25f; // Cell Size
	p_config_OUT.ch = 1.0f; // Cell Height
	p_config_OUT.walkableSlopeAngle = 45.0f;
	p_config_OUT.walkableHeight = (int)ceilf(/*m_agentHeight*/ 3.0f / p_config_OUT.ch);
	p_config_OUT.walkableClimb  = 0; //(int)floorf(m_agentMaxClimb / m_p_config_OUT.ch);
	p_config_OUT.walkableRadius = (int)ceilf(m_agentRadius / p_config_OUT.cs);
	p_config_OUT.maxEdgeLen     = 0; //(int)(m_edgeMaxLen / m_cellSize);
	p_config_OUT.maxSimplificationError = 0; //m_edgeMaxError;
	p_config_OUT.minRegionArea    = 0; //(int)rcSqr(m_regionMinSize);		// Note: area = size*size
	p_config_OUT.mergeRegionArea  = 0; //(int)rcSqr(m_regionMergeSize);	// Note: area = size*size
	p_config_OUT.maxVertsPerPoly  = 6; //(int)m_vertsPerPoly;
	
	// Newly added for TileCache
	p_config_OUT.tileSize   = 48;
	p_config_OUT.borderSize = p_config_OUT.walkableRadius + 4; // Reserve enough padding.
	p_config_OUT.width      = p_config_OUT.tileSize + p_config_OUT.borderSize * 2;
	p_config_OUT.height     = p_config_OUT.tileSize + p_config_OUT.borderSize * 2;
	// ---- end of newly added for TileCache.
	
	p_config_OUT.detailSampleDist = 0; //m_detailSampleDist < 0.9f ? 0 : m_cellSize * m_detailSampleDist;
	p_config_OUT.detailSampleMaxError = 0.1f; //m_cellHeight * m_detailSampleMaxError;
	
	// Set the area where the navigation will be build.
	// Here the bounds of the input mesh are used, but the
	// area could be specified by an user defined box, etc.
	//rcVcopy(p_config_OUT.bmin, bmin);
	p_config_OUT.bmin[0] = 0.0f;
	p_config_OUT.bmin[1] = 0.0f;
	p_config_OUT.bmin[2] = 0.0f;
	//rcVcopy(p_config_OUT.bmax, bmax);
	
	p_config_OUT.bmax[0] = level::tileToWorld(p_levelWidth);
	p_config_OUT.bmax[1] = 1.0f;
	p_config_OUT.bmax[2] = level::tileToWorld(p_levelHeight);
	
	// Calculate tilewidth and height
	int gw = 0, gh = 0;
	rcCalcGridSize(p_config_OUT.bmin, p_config_OUT.bmax, p_config_OUT.cs, &gw, &gh);
	
	const int ts = p_config_OUT.tileSize;
	p_tileWidth_OUT = (gw + ts-1) / ts;
	p_tileHeight_OUT = (gh + ts-1) / ts;
	
	// This value specifies how many layers (or "floors") each navmesh tile is expected to have.
	static const int EXPECTED_LAYERS_PER_TILE = 1;
	
	///////////////////////////////////////////////////////////////////////////
	// Init tilecache params
	if (p_tileCacheParams_OUT != 0)
	{
		memset(p_tileCacheParams_OUT, 0, sizeof(dtTileCacheParams));
		rcVcopy(p_tileCacheParams_OUT->orig, p_config_OUT.bmin);
		p_tileCacheParams_OUT->cs                     = p_config_OUT.cs; // m_cellSize;
		p_tileCacheParams_OUT->ch                     = p_config_OUT.ch; // m_cellHeight;
		p_tileCacheParams_OUT->width                  = p_config_OUT.tileSize;
		p_tileCacheParams_OUT->height                 = p_config_OUT.tileSize;
		p_tileCacheParams_OUT->walkableHeight         = 0.0f; // p_config_OUT.walkableHeight; // m_agentHeight;
		p_tileCacheParams_OUT->walkableRadius         = m_agentRadius;
		p_tileCacheParams_OUT->walkableClimb          = 45.0f; // p_config_OUT.walkableClimb;  // m_agentMaxClimb;
		p_tileCacheParams_OUT->maxSimplificationError = p_config_OUT.maxSimplificationError; // m_edgeMaxError;
		p_tileCacheParams_OUT->maxTiles               = p_tileWidth_OUT * p_tileHeight_OUT * EXPECTED_LAYERS_PER_TILE;
		p_tileCacheParams_OUT->maxObstacles           = MaxObstacles;
	}
	
	///////////////////////////////////////////////////////////////////////////
	// Init navmesh params
	if (p_navMeshParams_OUT != 0)
	{
		// Max tiles and max polys affect how the tile IDs are caculated.
		// There are 22 bits available for identifying a tile and a polygon.
		int tileBits = rcMin((int)dtIlog2(dtNextPow2(p_tileWidth_OUT*p_tileHeight_OUT*EXPECTED_LAYERS_PER_TILE)), 14);
		if (tileBits > 14) tileBits = 14;
		const int polyBits = 22 - tileBits;
		
		///////////////////////////////////////////////////////////////////////////
		// Init navmesh params
		memset(p_navMeshParams_OUT, 0, sizeof(dtNavMeshParams));
		rcVcopy(p_navMeshParams_OUT->orig, p_config_OUT.bmin); //m_geom->getMeshBoundsMin());
		p_navMeshParams_OUT->tileWidth  = p_config_OUT.tileSize * p_config_OUT.cs; // m_cellSize;
		p_navMeshParams_OUT->tileHeight = p_config_OUT.tileSize * p_config_OUT.cs; // m_cellSize;
		p_navMeshParams_OUT->maxTiles   = 1 << tileBits;
		p_navMeshParams_OUT->maxPolys   = 1 << polyBits;
	}
}


bool TileCache::initMemory(const rcConfig& /*p_config*/, const dtTileCacheParams& p_tileCacheParams,
                           const dtNavMeshParams& p_navMeshParams)
{
	cleanup();
	
	///////////////////////////////////////////////////////////////////////////
	// Init memory structures
	TT_ASSERT(m_tileCache == 0);
	m_tileCache = dtAllocTileCache();
	if (m_tileCache == 0)
	{
		TT_PANIC("dtAllocTileCache failed");
		return false;
	}
	
	dtStatus status = m_tileCache->init(&p_tileCacheParams, m_talloc, m_tcomp, m_tmproc);
	if (dtStatusFailed(status))
	{
		TT_PANIC("m_tileCache->init failed");
		return false;
	}
	
	TT_ASSERT(m_navMesh == 0);
	m_navMesh = dtAllocNavMesh();
	if (m_navMesh == 0)
	{
		TT_PANIC("dtAllocNavMesh failed");
		return false;
	}
	
	status = m_navMesh->init(&p_navMeshParams);
	if (dtStatusFailed(status))
	{
		TT_PANIC("m_navMesh->init failed");
		return false;
	}
	
	status = m_navQuery->init(m_navMesh, 2048);
	if (dtStatusFailed(status))
	{
		TT_PANIC("m_navQuery->init failed");
		return false;
	}
	
	return true;
}


s32 TileCache::preprocess(const level::AttributeLayerPtr& p_layer,
                          s32 p_tileWidth, s32 p_tileHeight, const rcConfig& p_cfg,
                          TileCacheData*& p_tileCache_OUT) const
{
	TT_ASSERT(p_tileCache_OUT == 0);
	
	// Make sure output buffer is large enough
	TileCacheData* tileCache = new TileCacheData[p_tileWidth * p_tileHeight * MAX_LAYERS];
	
	s32 totalTiles = 0;
	
	// Preprocess tiles.
	rcContext tmpContext(false);
	for (s32 y = 0; y < p_tileHeight; ++y)
	{
		for (s32 x = 0; x < p_tileWidth; ++x)
		{
			TileCacheData tiles[MAX_LAYERS];
			memset(tiles, 0, sizeof(tiles));
			TileRasterizationContext trcontext;
			const s32 ntiles = trcontext.rasterizeTileLayers(&tmpContext, p_layer, x, y, p_cfg, tiles, MAX_LAYERS);
			
			// Copy tiles to output buffer
			for (s32 i = 0; i < ntiles; ++i, ++totalTiles)
			{
				TT_NULL_ASSERT(tiles[i].data);
				tileCache[totalTiles] = tiles[i];
			}
		}
	}
	
	p_tileCache_OUT = tileCache;
	
	return totalTiles;
}


void TileCache::buildMeshes(s32 p_tileWidth, s32 p_tileHeight)
{
	TT_NULL_ASSERT(m_tileCache);
	TT_NULL_ASSERT(m_navMesh);
	
	// Build initial meshes
	for (s32 y = 0; y < p_tileHeight; ++y)
	{
		for (s32 x = 0; x < p_tileWidth; ++x)
		{
			m_tileCache->buildNavMeshTilesAt(x, y, m_navMesh);
		}
	}
}

// Namespace end
}
}
}

