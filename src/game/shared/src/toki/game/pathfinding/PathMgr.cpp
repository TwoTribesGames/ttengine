#include <set>

#include <tt/code/BufferReadContext.h>
#include <tt/code/bufferutils.h>
#include <tt/code/BufferWriteContext.h>
#include <tt/code/helpers.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>
#include <tt/system/Time.h>

#include <toki/game/pathfinding/PathMgr.h>
#include <toki/game/pathfinding/TileCache.h>
#include <toki/level/LevelData.h>
#include <toki/serialization/SerializationMgr.h>
#include <toki/AppGlobal.h>


#if !defined(TT_BUILD_FINAL)
//#define DEBUG_PATHMGR
#endif

namespace toki {
namespace game {
namespace pathfinding {


// Declared in pathfinding/fwd.h:
const ObstacleIndex g_invalidObstacleIndex = -1;


//--------------------------------------------------------------------------------------------------
// Public member functions

PathMgr::PathMgr()
:
m_tileCaches()
{
}


PathMgr::~PathMgr()
{
	cleanup();
	
	tt::code::helpers::freePairSecondContainer(m_tileCaches);
}


void PathMgr::cleanup()
{
	for (TileCaches::iterator it = m_tileCaches.begin(); it != m_tileCaches.end(); ++it)
	{
		(*it).second->cleanup();
	}
}


void PathMgr::buildTileCaches(const level::AttributeLayerPtr& p_layer)
{
#if defined(DEBUG_PATHMGR)
	//TT_Printf("toki::game::pathfinding::PathMgr::build generating NavMesh...\n");
	const u64 startTime = tt::system::Time::getInstance()->getMilliSeconds();
#endif
	
	for (TileCaches::iterator it = m_tileCaches.begin(); it != m_tileCaches.end(); ++it)
	{
		(*it).second->build(p_layer);
	}
	
#if defined(DEBUG_PATHMGR)
	const u64 duration = tt::system::Time::getInstance()->getMilliSeconds() - startTime;
	TT_Printf("PathMgr::build duration: %u ms\n", static_cast<u32>(duration));
#endif
}


void PathMgr::reset()
{
	removeAllAgents();
	clearAllTempObstacles();
	
	// Flush removal requests and process object's pending list so it can be actually removed.
	// This might take a couple of updates.
	for (TileCaches::iterator it = m_tileCaches.begin(); it != m_tileCaches.end(); ++it)
	{
		TileCache* tileCache = (*it).second;
		s32 updateCount = 0;
		// Do at least one update before calling hasTileCacheUpdatesPending.
		// The first update is to flush the requests.
		do
		{
			tileCache->update(0.0f);
			++updateCount;
		}
		while (tileCache->hasTileCacheUpdatesPending() && updateCount <= 64);
	}
}


void PathMgr::recreateTileCaches(const AgentRadii& p_agentRadii)
{
	tt::code::helpers::freePairSecondContainer(m_tileCaches);
	
	// Create a TileCache for each agent radius
	for (AgentRadii::const_iterator it = p_agentRadii.begin(); it != p_agentRadii.end(); ++it)
	{
		m_tileCaches[*it] = new TileCache(*it);
	}
	
#if defined(DEBUG_PATHMGR)
	TT_Printf("PathMgr::recreateTileCaches: Created %u tile caches.\n", m_tileCaches.size());
	
	TT_WARNING(m_tileCaches.empty() == false,
	           "There are no entity types with a path finding agent radius (pathFindAgentRadius property).\n"
	           "No TileCaches have been created, and consequently no path finding is available.");
#endif
}


void PathMgr::recreateTileCaches(const level::LevelDataPtr& p_levelData)
{
	recreateTileCaches(getUniqueAgentRadiiForLevel(p_levelData));
}


void PathMgr::addAndBuildMissingTileCaches(const level::LevelDataPtr& p_levelData)
{
	// Add new caches based on leveldata
	// Don't bother to remove the non-used anymore
	const AgentRadii radii = getUniqueAgentRadiiForLevel(p_levelData);
	for (AgentRadii::const_iterator it = radii.begin(); it != radii.end(); ++it)
	{
		if (m_tileCaches.find(*it) == m_tileCaches.end())
		{
			m_tileCaches[*it] = new TileCache(*it);
			m_tileCaches[*it]->build(p_levelData->getAttributeLayer());
		}
	}
}


void PathMgr::loadTileCachesFromLevelData(const level::LevelDataPtr& p_levelData)
{
	tt::code::AutoGrowBufferPtr buffer = p_levelData->getPathfindingData();
	tt::code::BufferReadContext context = buffer->getReadContext();
	loadTileCaches(&context);
}


void PathMgr::saveTileCachesToLevelData(const level::LevelDataPtr& p_levelData)
{
	tt::code::AutoGrowBufferPtr buffer = p_levelData->createPathfindingData();
	tt::code::BufferWriteContext context = buffer->getAppendContext();
	saveTileCaches(p_levelData->getAttributeLayer(), &context);
	context.flush();
}


void PathMgr::removeAllAgents()
{
	for (TileCaches::iterator it = m_tileCaches.begin(); it != m_tileCaches.end(); ++it)
	{
		(*it).second->removeAllAgents();
	}
}


PathMgr::AgentRadii PathMgr::getUniqueAgentRadiiForLevel(const level::LevelDataPtr& p_levelData) const
{
	AgentRadii result;
	
	const level::entity::EntityInstances& entities(p_levelData->getAllEntities());
	
	const entity::EntityLibrary& entityLib(AppGlobal::getEntityLibrary());
	for (level::entity::EntityInstances::const_iterator it = entities.begin(); it != entities.end(); ++it)
	{
		const level::entity::EntityInfo* info = entityLib.getEntityInfo((*it)->getType());
		if (info != 0)
		{
			const real agentRadius = info->getPathFindAgentRadius();
			if (agentRadius > 0.0f)
			{
				result.insert(agentRadius);
			}
		}
	}
	
	return result;
}

TileCache* PathMgr::getTileCacheForAgentRadius(real p_agentRadius)
{
	TileCaches::iterator it = m_tileCaches.find(p_agentRadius);
	TT_ASSERTMSG(it != m_tileCaches.end(), "PathMgr has no TileCache for agent radius %f", p_agentRadius);
	return (it != m_tileCaches.end()) ? (*it).second : 0;
}


void PathMgr::update(real p_deltaTime)
{
	for (TileCaches::iterator it = m_tileCaches.begin(); it != m_tileCaches.end(); ++it)
	{
		(*it).second->update(p_deltaTime);
	}
}


void PathMgr::render() const
{
	for (TileCaches::const_iterator it = m_tileCaches.begin(); it != m_tileCaches.end(); ++it)
	{
		(*it).second->render();
	}
}


ObstacleIndex PathMgr::addTempObstacle(const tt::math::VectorRect& p_rectangle)
{
	ObstacleIndex indexToReturn = g_invalidObstacleIndex;
	
	for (TileCaches::iterator it = m_tileCaches.begin(); it != m_tileCaches.end(); ++it)
	{
		ObstacleIndex obstacle = (*it).second->addTempObstacle(p_rectangle);
		if (it == m_tileCaches.begin()) indexToReturn = obstacle;
		TT_ASSERTMSG(obstacle >= 0,
		             "Failed to add temp object to tile cache for agent radius %f. "
		             "Rectangle (x, y, w, h) (%.2f, %.2f, %.2f, %.2f)", 
		             (*it).first,
		             p_rectangle.getMin().x, p_rectangle.getMin().y,
		             p_rectangle.getWidth(), p_rectangle.getHeight());
		TT_ASSERTMSG(obstacle == indexToReturn,
		             "Got different result (%d) while adding obstacle to tile cache "
		             "for agent radius %f than result from previous tile caches (%d).",
		             obstacle, (*it).first, indexToReturn);
	}
	
	return indexToReturn;
}


void PathMgr::removeTempObstacle(ObstacleIndex p_index)
{
	for (TileCaches::iterator it = m_tileCaches.begin(); it != m_tileCaches.end(); ++it)
	{
		(*it).second->removeTempObstacle(p_index);
	}
}


void PathMgr::clearAllTempObstacles()
{
	for (TileCaches::iterator it = m_tileCaches.begin(); it != m_tileCaches.end(); ++it)
	{
		(*it).second->clearAllTempObstacles();
	}
}


void PathMgr::serialize(toki::serialization::SerializationMgr& p_serializationMgr) const
{
	// FIXME: SERIALIZE HERE!!!!!!!!
	(void) p_serializationMgr;
}


void PathMgr::unserialize(const toki::serialization::SerializationMgr& p_serializationMgr)
{
	(void) p_serializationMgr;
}


//--------------------------------------------------------------------------------------------------
// Private member functions

void PathMgr::loadTileCaches(tt::code::BufferReadContext* p_context)
{
#if defined(DEBUG_PATHMGR)
	const u64 startTime = tt::system::Time::getInstance()->getMilliSeconds();
#endif
	
	namespace bu = tt::code::bufferutils;
	
	tt::code::helpers::freePairSecondContainer(m_tileCaches);
	
	AgentRadii radii;
	u32 numberOfCaches = bu::get<u32>(p_context);
	
	for (u32 i = 0; i < numberOfCaches; ++i)
	{
		real radius = bu::get<real>(p_context);
		radii.insert(radius);
		
		m_tileCaches[radius] = new TileCache(radius);
		m_tileCaches[radius]->load(p_context);
	}
	
#if defined(DEBUG_PATHMGR)
	const u64 duration = tt::system::Time::getInstance()->getMilliSeconds() - startTime;
	TT_Printf("PathMgr::load duration: %u ms; %d tile caches loaded\n", static_cast<u32>(duration), numberOfCaches);
#endif
}


void PathMgr::saveTileCaches(const level::AttributeLayerPtr& p_layer,
                             tt::code::BufferWriteContext* p_context) const
{
	namespace bu = tt::code::bufferutils;
	
	// Number of tilecaches
	u32 numberOfCaches = static_cast<u32>(m_tileCaches.size());
	bu::put(numberOfCaches, p_context);
	
	// Store tilecaches
	for (TileCaches::const_iterator it = m_tileCaches.begin(); it != m_tileCaches.end(); ++it)
	{
		// Store radius
		bu::put((*it).first, p_context);
		
		(*it).second->save(p_layer, p_context);
	}
}

// Namespace end
}
}
}
