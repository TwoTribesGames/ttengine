#include <tt/code/bufferutils.h>
#include <tt/code/HandleArrayMgr_utils.h>
#include <tt/code/helpers.h>
#include <tt/engine/renderer/Renderer.h>

#include <toki/game/entity/graphics/PowerBeamGraphic.h>
#include <toki/game/entity/sensor/Sensor.h>
#include <toki/game/entity/EntityMgr.h>
#include <toki/game/fluid/FluidMgr.h>
#include <toki/game/light/LightMgr.h>
#include <toki/game/script/EntityBase.h>
#include <toki/game/script/EntityScriptMgr.h>
#include <toki/game/Game.h>
#include <toki/level/entity/EntityInstance.h>
#include <toki/level/LevelData.h>
#include <toki/serialization/SerializationMgr.h>
#include <toki/AppGlobal.h>


namespace toki {
namespace game {
namespace entity {

//--------------------------------------------------------------------------------------------------
// Public member functions

EntityMgr::EntityMgr(s32 p_reserveCount)
:
tt::code::HandleArrayMgr<Entity>(p_reserveCount),
m_movementControllerMgr(p_reserveCount),
m_deathRow(),
m_powerBeamGraphicMgr(p_reserveCount / 8),
m_textLabelMgr(p_reserveCount / 8),
m_sensorMgr(p_reserveCount),
m_tileSensorMgr(p_reserveCount),
m_effectRectMgr(p_reserveCount / 8),
m_idToHandleMapping(),
m_isCreatingEntities(false),
m_postCreateSpawn(),
m_entityCullingEnabled(true),
m_sectionProfiler("EntityMgr - update")
{
}


EntityMgr::~EntityMgr()
{
	resetAll();
}


void EntityMgr::createSpawnSections(const level::entity::EntityInstances& p_instances,
                                    const std::string&                    p_missionID,
                                    bool                                  p_gameReloaded)
{
	// Filter out non-mission specific entities
	level::entity::EntityInstances missionEntities;
	appendMissionSpecificEntities(p_instances, p_missionID, missionEntities);
	
	// First reset spawn section ids
	for (level::entity::EntityInstances::const_iterator it = missionEntities.begin();
	     it != missionEntities.end(); ++it)
	{
		(*it)->setSpawnSectionID(-1);
	}
	
	// Create sections
	for (level::entity::EntityInstances::const_iterator it = missionEntities.begin();
	     it != missionEntities.end(); ++it)
	{
		level::entity::EntityInstancePtr entityInfo(*it);
		if (entityInfo->getType() == "SpawnSection")
		{
			EntityHandle handle = createEntity(entityInfo->getType(), entityInfo->getID());
			if (handle.isEmpty())
			{
				// Failed to create this entity, try next
				continue;
			}
			Entity* entity = handle.getPtr();
			level::entity::EntityInstance::Properties properties = entityInfo->getProperties();
			entity->init(entityInfo->getPosition(), properties, p_gameReloaded);
			const script::EntityBasePtr& entityBase = entity->getEntityScript();
			if (entityBase != 0)
			{
				entityBase->onInit();
			}
			// No onSpawn needed
			entity->kill();
		}
	}
}


void EntityMgr::createEntities(const level::entity::EntityInstances& p_instances,
                               const std::string&                    p_missionID,
                               s32                                   p_overridePositionEntityID,
                               const tt::math::Vector2&              p_overridePosition,
                               bool                                  p_gameReloaded,
	                           s32                                   p_spawnSectionID)
{
	TT_ASSERT(m_isCreatingEntities == false);
	m_isCreatingEntities = true;
	
	AppGlobal::getGame()->clearEditorWarnings();
	
	typedef std::map<entity::EntityHandle, level::entity::EntityInstancePtr> EntityCreationInfo;
	EntityCreationInfo entityCreationInfo;
	
	// Filter out non-mission specific entities
	level::entity::EntityInstances missionEntities;
	appendMissionSpecificEntities(p_instances, p_missionID, missionEntities);
	
	// Filter out the delayed entities
	typedef std::set<s32> DelayedEntityIDs;
	DelayedEntityIDs delayedIDs;
	{
		using namespace level::entity;
		const entity::EntityLibrary& library    = AppGlobal::getEntityLibrary();
		
		for (level::entity::EntityInstances::const_iterator it = missionEntities.begin();
			 it != missionEntities.end(); ++it)
		{
			using namespace level::entity;
			const EntityInfo* entityInfo = library.getEntityInfo((*it)->getType());
		
			if (entityInfo == 0)
			{
				TT_PANIC("Unsupported entity type: '%s' (no type information is available for it).", (*it)->getType().c_str());
				continue;
			}
			const level::entity::EntityInstance::Properties& props = (*it)->getProperties();
			for (level::entity::EntityInstance::Properties::const_iterator propIt = props.begin();
			     propIt != props.end(); ++propIt)
			{
				const EntityProperty& prop = entityInfo->getProperty((*propIt).first);
				EntityProperty::Type type = prop.getType();
				if (type != EntityProperty::Type_DelayedEntityID &&
					type != EntityProperty::Type_DelayedEntityIDArray)
				{
					continue;
				}
				
				if (type == EntityProperty::Type_DelayedEntityID)
				{
					delayedIDs.insert(tt::str::parseS32((*propIt).second, 0));
				}
				else
				{
					tt::str::Strings elements = tt::str::explode((*propIt).second, ",");
					for (tt::str::Strings::const_iterator elemIt = elements.begin();
					     elemIt != elements.end(); ++elemIt)
					{
						delayedIDs.insert(tt::str::parseS32(*elemIt, 0));
					}
				}
			}
		}
	}
	
	// Sort the entities based on order
	std::stable_sort(missionEntities.begin(), missionEntities.end(), level::entity::EntityInstance::sortOrder);
	
	// Create entities
	for (level::entity::EntityInstances::const_iterator it = missionEntities.begin();
	     it != missionEntities.end(); ++it)
	{
		level::entity::EntityInstancePtr entityInfo(*it);
		TT_NULL_ASSERT(entityInfo);
		if (entityInfo->getType() == "SpawnSection" || entityInfo->getSpawnSectionID() != p_spawnSectionID ||
		    delayedIDs.find((*it)->getID()) != delayedIDs.end())
		{
			continue;
		}
		
		EntityHandle handle = createEntity(entityInfo->getType(), entityInfo->getID());
		if (handle.isEmpty())
		{
			// Failed to create this entity, try next
			continue;
		}
		
		entityCreationInfo.insert(std::make_pair(handle, entityInfo));
	}
	
	// Init entities
	for (EntityCreationInfo::const_iterator it = entityCreationInfo.begin();
	     it != entityCreationInfo.end(); ++it)
	{
		Entity* entity = get((*it).first);
		TT_NULL_ASSERT(entity);
		
		TT_ASSERT(entity->getEntityState() == Entity::State_Loaded);
		
		level::entity::EntityInstance::Properties properties = (*it).second->getProperties();
		
		const bool overridePositionForThisEntity =
				p_overridePositionEntityID >= 0 &&
				(*it).second->getID() == p_overridePositionEntityID;
		
		entity->init(
			overridePositionForThisEntity ? p_overridePosition : (*it).second->getPosition(),
			properties, p_gameReloaded);
		
		// It is possible to kill entities in the onInit callback; in that case the entitystate will be deinitialized after the init
		
		if (entity->isInitialized() && properties != (*it).second->getProperties())
		{
			(*it).second->setProperties(properties);
			(*it).second->setPropertiesUpdatedByScript(true);
		}
	}
	
	// Call onInit for all entities
	for (EntityCreationInfo::const_iterator it = entityCreationInfo.begin();
	     it != entityCreationInfo.end(); ++it)
	{
		Entity* entity = get((*it).first);
		TT_NULL_ASSERT(entity);
		
		if (entity->getEntityState() == Entity::State_Initialized)
		{
			const script::EntityBasePtr& entityBase = entity->getEntityScript();
			if (entityBase != 0)
			{
				entityBase->onInit();
			}
		}
		else
		{
			TT_ASSERT(entity->getEntityState() == Entity::State_Initialized);
		}
	}
	
	Game* game = AppGlobal::getGame();
	game->getLightMgr().updatePostInit();
	
	flushPostCreateSpawn();
	
	// Call onSpawn for all entities
	for (EntityCreationInfo::const_iterator it = entityCreationInfo.begin();
	     it != entityCreationInfo.end(); ++it)
	{
		Entity* entity = get((*it).first);
		TT_NULL_ASSERT(entity);
		if (entity->isInitialized() == false)
		{
			// Ignore entities that were somehow killed between onInit and onSpawn
			continue;
		}
		
		handleEntityPreSpawn(*entity);
		
		const script::EntityBasePtr& entityBase = entity->getEntityScript();
		if (entityBase != 0)
		{
			entityBase->onSpawn();
		}
	}
	
	flushPostCreateSpawn();
	
	callOnValidateScriptStateOnAllEntities();
	
	m_isCreatingEntities = false;
}


EntityHandle EntityMgr::createEntity(const std::string& p_type, s32 p_id)
{
	EntityHandle handle = create(Entity::CreationParams());
	if (handle.isEmpty())
	{
		// Failed to create a new handle. (Too many entities?)
		return handle;
	}
	Entity* newEntity = get(handle);
	TT_NULL_ASSERT(newEntity);
	
	if (newEntity->load(p_type, p_id) == false)
	{
		destroy(handle);
		return EntityHandle();
	}
	
	// Register ID with this handle (only when not -1 and when not existing yet)
	if (p_id >= 0 && m_idToHandleMapping.find(p_id) == m_idToHandleMapping.end())
	{
		m_idToHandleMapping[p_id] = handle;
	}
	
	return handle;
}


bool EntityMgr::initEntity(const EntityHandle& p_handle, const tt::math::Vector2& p_position,
	                       EntityProperties& p_properties)
{
	// FIXME: Code duplication with other initEntity
	Entity* entity = get(p_handle);
	if (entity == 0)
	{
		TT_PANIC("Can't initialize entity, because it does not exist. "
		         "Entity handle 0x%08X at position (%.2f, %.2f).",
		         p_handle.getValue(), p_position.x, p_position.y);
		return false;
	}
	TT_ASSERT(entity->getEntityState() == Entity::State_Loaded);
	
	entity->init(p_position, p_properties, false);
	
	const script::EntityBasePtr& entityBase = entity->getEntityScript();
	
	if (entityBase == 0)
	{
		return false;
	}
	
	entityBase->onInit();
	
	// It is possible to kill entities in the onInit callback; in that case the entitystate will be deinitialized after the init
	if (entity->isInitialized() == false)
	{
		return false;
	}
	
	if (m_isCreatingEntities)
	{
		m_postCreateSpawn.push_back(p_handle);
		return true;
	}
	
	handleEntityPreSpawn(*entity);
	
	entityBase->onSpawn();
	
	// It is possible to kill entities in the onSpawn callback as well; in that case the entitystate will be deinitialized at this point
	return entity->isInitialized();
}


bool EntityMgr::initEntity(const EntityHandle& p_handle, const tt::math::Vector2& p_position,
                           const HSQOBJECT& p_properties)
{
	// FIXME: Code duplication with other initEntity
	Entity* entity = get(p_handle);
	if (entity == 0)
	{
		TT_PANIC("Can't initialize entity, because it does not exist. "
		         "Entity handle 0x%08X at position (%.2f, %.2f).",
		         p_handle.getValue(), p_position.x, p_position.y);
		return false;
	}
	TT_ASSERT(entity->getEntityState() == Entity::State_Loaded);
	
	entity->init(p_position, p_properties);
	
	const script::EntityBasePtr& entityBase = entity->getEntityScript();
	
	if (entityBase == 0)
	{
		return false;
	}
	
	entityBase->onInit();
	
	// It is possible to kill entities in the onInit callback; in that case the entitystate will be deinitialized after the init
	if (entity->isInitialized() == false)
	{
		return false;
	}
	
	if (m_isCreatingEntities)
	{
		m_postCreateSpawn.push_back(p_handle);
		return true;
	}
	
	handleEntityPreSpawn(*entity);
	
	entityBase->onSpawn();
	
	// It is possible to kill entities in the onSpawn callback as well; in that case the entitystate will be deinitialized at this point
	return entity->isInitialized();
}


void EntityMgr::updateEntities(real p_deltaTime, const Camera& p_camera)
{
	using namespace toki::utils;
	
	m_sectionProfiler.startFrameUpdate();
	
	// Handle culling first
	m_sectionProfiler.startFrameUpdateSection(EntityMgrSection_Culling);
	updateCullingForAllEntities(p_camera);
	
	// Handle screen enter/exit
	m_sectionProfiler.startFrameUpdateSection(EntityMgrSection_ScreenEnterExit);
	updateIsOnScreenAllEntities(p_camera);
	
	m_sectionProfiler.startFrameUpdateSection(EntityMgrSection_MovementControllers);
	m_movementControllerMgr.update(p_deltaTime, *this);
	
	m_sectionProfiler.startFrameUpdateSection(EntityMgrSection_TileRegistrationMgr);
	AppGlobal::getGame()->getTileRegistrationMgr().update(p_deltaTime);
	
	m_sectionProfiler.startFrameUpdateSection(EntityMgrSection_Sensors);
	m_sensorMgr.update(AppGlobal::getGame()->getGameTimeInSeconds());
	m_sectionProfiler.startFrameUpdateSection(EntityMgrSection_TileSensors);
	m_tileSensorMgr.update();
	m_sectionProfiler.startFrameUpdateSection(EntityMgrSection_PowerBeamGraphicMgr);
	m_powerBeamGraphicMgr.update(p_deltaTime);
	m_sectionProfiler.startFrameUpdateSection(EntityMgrSection_TextLabelMgr);
	m_textLabelMgr.update(p_deltaTime);
	m_sectionProfiler.startFrameUpdateSection(EntityMgrSection_EffectRectMgr);
	m_effectRectMgr.update(p_deltaTime, p_camera);
	
	m_sectionProfiler.startFrameUpdateSection(EntityMgrSection_DeathRow);
	
	// Destroy the entities that are on deathrow
	//static const u32 maxEntitiesToDestroy = 3;
	u32 entitiesDestroyed(0);

	for (DeathRowEntries::iterator it = m_deathRow.begin(); it != m_deathRow.end();)
	{
		if ((*it).framesToLive <= 0)
		{
			//if(entitiesDestroyed < maxEntitiesToDestroy)
			{
				destroyEntity((*it).handle);
				it = tt::code::helpers::unorderedErase(m_deathRow, it);
				++entitiesDestroyed;
			}
			//else
			//{
			//	++it;
			//}
		}
		else
		{
			--(*it).framesToLive;
			++it;
		}
	}
	
	m_sectionProfiler.stopFrameUpdate();
}


void EntityMgr::updateEntitiesChanges(real p_deltaTime)
{
	m_sectionProfiler.startFrameUpdateSection(toki::utils::EntityMgrSection_MovementControllersChanges);
	m_movementControllerMgr.updateChanges(p_deltaTime, *this);
}


void EntityMgr::updateForRender(const tt::math::VectorRect& p_visibilityRect)
{
	m_powerBeamGraphicMgr.updateForRender(p_visibilityRect);
	m_textLabelMgr.updateForRender();
}


void EntityMgr::renderEntitiesDebug()
{
#if !defined(TT_BUILD_FINAL)
	const DebugRenderMask& mask = AppGlobal::getDebugRenderMask();
	if (mask.checkFlag(DebugRender_RenderScreenspace))
	{
		return;
	}
	
	Entity* entity = getFirst();
	for (s32 i = 0; i < getActiveCount(); ++i, ++entity)
	{
		if (entity->isInitialized()) // FIXME: Sort for state, no branch loop.
		{
			entity->renderDebug(false);
		}
	}
	
	if (mask.checkFlag(DebugRender_RenderEffectRects))
	{
		m_effectRectMgr.renderDebug();
	}
	
	m_tileSensorMgr.renderDebug();
	m_sensorMgr.renderDebug();
#endif
}


void EntityMgr::renderPowerBeamGraphics(const tt::math::VectorRect& p_visibilityRect) const
{
	m_powerBeamGraphicMgr.render(p_visibilityRect);
	
	m_powerBeamGraphicMgr.renderLightmask(p_visibilityRect);
}


void EntityMgr::renderTextLabels() const
{
	m_textLabelMgr.render();
}


EntityHandle EntityMgr::getEntityHandleByID(s32 p_id) const
{
	IDToHandleMapping::const_iterator it = m_idToHandleMapping.find(p_id);
	if (it != m_idToHandleMapping.end())
	{
		return (*it).second;
	}
	
	return EntityHandle();
}


s32 EntityMgr::getEntityIDByHandle(const EntityHandle& p_handle) const
{
	for (IDToHandleMapping::const_iterator it = m_idToHandleMapping.begin();
	     it != m_idToHandleMapping.end(); ++it)
	{
		if ((*it).second == p_handle)
		{
			return (*it).first;
		}
	}
	
	return -1;
}


void EntityMgr::removeEntityIDFromMapping(s32 p_id)
{
	IDToHandleMapping::iterator it = m_idToHandleMapping.find(p_id);
	if (it != m_idToHandleMapping.end())
	{
		m_idToHandleMapping.erase(it);
	}
}


#if !defined(TT_BUILD_FINAL)
void EntityMgr::setDirectionForAllEntities(const tt::math::Vector2& p_dir)
{
	Entity* entity = getFirst();
	for (s32 i = 0; i < getActiveCount(); ++i, ++entity)
	{
		if (entity->isInitialized()) // FIXME: Sort for state, no branch loop.
		{
			entity->startMovementInDirection(p_dir, movementcontroller::PhysicsSettings());
		}
	}
}


std::string EntityMgr::getDebugUnculledInfo() const
{
	typedef std::map<std::string, s32> EntityInfo;
	EntityInfo unculledEntities;
	
	const Entity* entity = getFirst();
	for (s32 i = 0; i < getActiveCount(); ++i, ++entity)
	{
		if (entity->isInitialized() && entity->isPositionCulled() == false) // FIXME: Sort for state, no branch loop.
		{
			const std::string type(entity->getType());
			EntityInfo::iterator it = unculledEntities.find(type);
			if (it == unculledEntities.end())
			{
				unculledEntities[type] = 1;
			}
			else
			{
				it->second++;
			}
		}
	}
	
	// Now sort by size
	typedef std::multimap<s32, std::string> EntityCountInfo;
	EntityCountInfo unculledInfo;
	s32 totalUnculled = 0;
	
	for (EntityInfo::const_iterator it = unculledEntities.begin(); it != unculledEntities.end(); ++it)
	{
		totalUnculled += it->second;
		unculledInfo.insert(std::make_pair(it->second, it->first));
	}
	
	char buf[256];
	sprintf(buf, "Active Entities (%d):\n", totalUnculled);
	std::string result(buf);
	for (EntityCountInfo::reverse_iterator it = unculledInfo.rbegin(); it != unculledInfo.rend(); ++it)
	{
		sprintf(buf, "%5d %s\n", it->first, it->second.c_str());
		result += std::string(buf);
	}
	
	return result;
}


std::string EntityMgr::getDebugCulledInfo() const
{
	typedef std::map<std::string, s32> EntityInfo;
	EntityInfo culledEntities;
	
	const Entity* entity = getFirst();
	for (s32 i = 0; i < getActiveCount(); ++i, ++entity)
	{
		if (entity->isInitialized() && entity->isPositionCulled()) // FIXME: Sort for state, no branch loop.
		{
			const std::string type(entity->getType());
			EntityInfo::iterator it = culledEntities.find(type);
			if (it == culledEntities.end())
			{
				culledEntities[type] = 1;
			}
			else
			{
				it->second++;
			}
		}
	}
	
	// Now sort by size
	typedef std::multimap<s32, std::string> EntityCountInfo;
	EntityCountInfo culledInfo;
	s32 totalCulled = 0;
	
	for (EntityInfo::const_iterator it = culledEntities.begin(); it != culledEntities.end(); ++it)
	{
		totalCulled += it->second;
		culledInfo.insert(std::make_pair(it->second, it->first));
	}
	
	char buf[256];
	sprintf(buf, "Culled Entities (%d):\n", totalCulled);
	std::string result(buf);
	for (EntityCountInfo::reverse_iterator it = culledInfo.rbegin(); it != culledInfo.rend(); ++it)
	{
		sprintf(buf, "%5d %s\n", it->first, it->second.c_str());
		result += std::string(buf);
	}
	
	return result;
}
#endif


void EntityMgr::serialize(toki::serialization::SerializationMgr& p_serializationMgr) const
{
	const serialization::SerializerPtr& section = p_serializationMgr.getSection(serialization::Section_EntityMgr);
	if (section == 0)
	{
		TT_PANIC("Serialization manager does not contain a section for the EntityMgr data.");
		return;
	}
	
	tt::code::BufferWriteContext context(section->getAppendContext());
	
	namespace bu = tt::code::bufferutils;
	
	tt::code::serializeHandleArrayMgr(*this, &context);
	
	// Serialize m_idToHandleMapping
	const u32 idToHandleMappingCount = static_cast<u32>(m_idToHandleMapping.size());
	bu::put(idToHandleMappingCount, &context);
	for (IDToHandleMapping::const_iterator it = m_idToHandleMapping.begin();
	     it != m_idToHandleMapping.end(); ++it)
	{
		bu::put((*it).first,        &context);
		bu::putHandle((*it).second, &context);
	}
	
	m_movementControllerMgr.serialize(p_serializationMgr);
	const u32 deathRowCount = static_cast<u32>(m_deathRow.size());
	bu::put(deathRowCount, &context);
	for (DeathRowEntries::const_iterator it = m_deathRow.begin(); it != m_deathRow.end(); ++it)
	{
		bu::put((*it).framesToLive, &context);
		bu::putHandle((*it).handle, &context);
	}
	
	m_powerBeamGraphicMgr.serialize(p_serializationMgr);
	m_textLabelMgr.serialize(p_serializationMgr);
	m_sensorMgr.serialize(p_serializationMgr);
	m_tileSensorMgr.serialize(p_serializationMgr);
	m_effectRectMgr.serialize(p_serializationMgr);
	
	TT_ASSERTMSG(m_isCreatingEntities == false && m_postCreateSpawn.empty(),
	             "Can't serialize while creating entities.");
	
	// Done writing data: ensure the underlying buffer gets all the data that was written
	context.flush();
}


void EntityMgr::unserialize(const toki::serialization::SerializationMgr& p_serializationMgr)
{
	const serialization::SerializerPtr& section = p_serializationMgr.getSection(serialization::Section_EntityMgr);
	if (section == 0)
	{
		TT_PANIC("Serialization manager does not contain a section for the EntityMgr data.");
		return;
	}
	
	tt::code::BufferReadContext context(section->getReadContext());
	
	namespace bu = tt::code::bufferutils;
	
	resetAll();
	
	// Load
	tt::code::unserializeHandleArrayMgr(this, &context);
	
	const u32 idToHandleMappingCount = bu::get<u32>(&context);
	for (u32 i = 0; i < idToHandleMappingCount; ++i)
	{
		const s32 id              = bu::get<s32>(&context);
		const EntityHandle handle = bu::getHandle<Entity>(&context);
		m_idToHandleMapping[id] = handle;
	}
	
	m_movementControllerMgr.unserialize(p_serializationMgr);
	const u32 deathRowCount = bu::get<u32>(&context);
	m_deathRow.reserve(deathRowCount);
	for (u32 i = 0; i < deathRowCount; ++i)
	{
		DeathRowEntry entry;
		entry.framesToLive = bu::get<s32>(&context);
		entry.handle = bu::getHandle<Entity>(&context);
		m_deathRow.push_back(entry);
	}
	
	m_powerBeamGraphicMgr.unserialize(p_serializationMgr);
	m_textLabelMgr.unserialize(p_serializationMgr);
	m_sensorMgr.unserialize(p_serializationMgr);
	m_tileSensorMgr.unserialize(p_serializationMgr);
	m_effectRectMgr.unserialize(p_serializationMgr);
	
	m_isCreatingEntities = false;
	m_postCreateSpawn.clear();
}


void EntityMgr::resetAll()
{
	m_movementControllerMgr.reset();
	m_deathRow.clear();
	m_powerBeamGraphicMgr.reset();
	m_textLabelMgr.reset();
	m_sensorMgr.reset();
	m_tileSensorMgr.reset();
	m_effectRectMgr.reset();
	m_idToHandleMapping.clear();
	m_isCreatingEntities = false;
	m_postCreateSpawn.clear();
	
	// Remove all callbacks before removing the entities to prevent callbacks that still reference entities
	Entity* entity = getFirst();
	for (s32 i = 0; i < getActiveCount(); ++i, ++entity)
	{
		script::EntityBasePtr script(entity->getEntityScript());
		TT_NULL_ASSERT(script);
		script->removeCallbacks();
	}
	
	// Remove all entites
	reset();
	
	script::EntityScriptMgr& mgr = AppGlobal::getEntityScriptMgr();
	mgr.collectGarbage();
}


void EntityMgr::callOnValidateScriptStateOnAllEntities() const
{
	if (AppGlobal::isInLevelEditorMode() == false &&
	    AppGlobal::isInDeveloperMode()   == false)
	{
		// Early out for non-editor, non-developer mode. (i.e. Normal game mode.)
		return;
	}
	
	const entity::Entity* entity = getFirstEntity();
	const s32 entityCount = getActiveEntitiesCount();
	for (s32 i = 0; i < entityCount; ++i, ++entity)
	{
		TT_ASSERT(entityCount == getActiveEntitiesCount());
		if (entity->isInitialized())
		{
			const script::EntityBasePtr& localPtr(entity->getEntityScript());
			TT_NULL_ASSERT(localPtr);
			localPtr->onValidateScriptState();
		}
	}
}


void EntityMgr::callFunctionOnAllEntities(const std::string& p_functionName) const
{
	const entity::Entity* entity = getFirstEntity();
	const s32 entityCount = getActiveEntitiesCount();
	for (s32 i = 0; i < entityCount; ++i, ++entity)
	{
		TT_ASSERTMSG(entityCount == getActiveEntitiesCount(),
		             "The number of entities changed from %d to %d while calling the script function '%s' on all entities."
		             "(The current implementation of EntityMgr::callFunctionOnAllEntities does not support this!)",
		             entityCount, getActiveEntitiesCount(), p_functionName.c_str());
		if (entity->isInitialized())
		{
			const script::EntityBasePtr& localPtr(entity->getEntityScript());
			TT_NULL_ASSERT(localPtr);
			localPtr->callSqFun(p_functionName);
		}
	}
}


void EntityMgr::callUpdateOnAllEntities(real p_deltaTime) const
{
	const bool isPaused = p_deltaTime <= 0.0f;
	
	// This functions allows the active entities to change while the function is being called.
	const entity::Entity* entity = getFirstEntity();
	const s32 entityCount = getActiveEntitiesCount();
	
	// Get all active entities
	static EntityHandles toCallEntities;
	toCallEntities.reserve(entityCount);
	for (s32 i = 0; i < entityCount; ++i, ++entity)
	{
		TT_ASSERT(entityCount == getActiveEntitiesCount());
		if (entity->isInitialized() && entity->isPositionCulled() == false &&
		   (isPaused == false || (isPaused && entity->canBePaused() == false)))
		{
			toCallEntities.push_back(entity->getHandle());
		}
	}
	
	// Now call all of the entities.
#if !defined(TT_BUILD_FINAL)
	++m_currentTimingFrame;
	if (m_currentTimingFrame >= maxTimingFrames)
	{
		m_currentTimingFrame = 0;
	}
	for (auto& it : m_timings)
	{
		it.second[m_currentTimingFrame] = 0;
	}
#endif
	for (EntityHandles::const_iterator it = toCallEntities.begin(); it != toCallEntities.end(); ++it)
	{
		entity = (*it).getPtr();
		if (entity != 0 && entity->isInitialized())
		{
			const script::EntityBasePtr& localPtr(entity->getEntityScript());
			TT_NULL_ASSERT(localPtr);
#if !defined(TT_BUILD_FINAL)
			const u64 startTime = tt::system::Time::getInstance()->getMicroSeconds();
#endif
			localPtr->callSqFun("update", p_deltaTime);
#if !defined(TT_BUILD_FINAL)
			const u64 duration = tt::system::Time::getInstance()->getMicroSeconds() - startTime;
			const std::string& type(localPtr->getType());
			auto timingsIt(m_timings.find(type));
			if (timingsIt == m_timings.end())
			{
				u64* vals(m_timings[type]);
				memset(vals, 0, sizeof(u64[maxTimingFrames]));
				m_timings[type][m_currentTimingFrame] = duration;
			}
			else
			{
				m_timings[type][m_currentTimingFrame] += duration;
			}
#endif

		}
	}
	toCallEntities.clear();
}


void EntityMgr::appendMissionSpecificEntities(const level::entity::EntityInstances& p_allEntities,
                                              const std::string& p_missionID,
                                              level::entity::EntityInstances& p_missionEntities_OUT)
{
	p_missionEntities_OUT.reserve(p_missionEntities_OUT.size() + p_allEntities.size());
	
	// filter out all non mission specific entities
	using namespace level::entity;
	if (p_missionID.empty() || p_missionID == "*")
	{
		// No specific mission; simply copy all and return
		p_missionEntities_OUT.insert(p_missionEntities_OUT.end(), p_allEntities.begin(), p_allEntities.end());
		return;
	}
	
	for (EntityInstances::const_iterator it = p_allEntities.begin(); it != p_allEntities.end(); ++it)
	{
		const EntityInstance::Properties& props(it->get()->getProperties());
		EntityInstance::Properties::const_iterator findIt = props.find("MISSION_ID");
		
		if (findIt != props.end())
		{
			const std::string& missionStr = findIt->second;
			tt::str::Strings missionIDs(tt::str::explode(missionStr, ";,"));
			
			for (tt::str::Strings::const_iterator missionIt = missionIDs.begin();
			     missionIt != missionIDs.end(); ++missionIt)
			{
				const std::string& missionID = tt::str::trim(*missionIt);
				if (tt::str::wildcardCompare(p_missionID, missionID))
				{
					p_missionEntities_OUT.push_back(*it);
					break;
				}
			}
		}
		else
		{
			p_missionEntities_OUT.push_back(*it);
		}
	}
}


void EntityMgr::initCullingForAllEntities(const Camera& p_camera)
{
	const tt::math::VectorRect& cullingRect(p_camera.getCurrentCullingRect());
	
	// First update parents
	Entity* entity = getFirst();
	for (s32 i = 0; i < getActiveCount(); ++i, ++entity)
	{
		if (entity->isInitialized() && entity->hasPositionCullingParent() == false &&
		    entity->isPositionCullingInitialized() == false)
		{
			entity->initPositionCulling(cullingRect);
		}
	}
	
	// Then update childs
	entity = getFirst();
	for (s32 i = 0; i < getActiveCount(); ++i, ++entity)
	{
		if (entity->isInitialized() && entity->hasPositionCullingParent() &&
		    entity->isPositionCullingInitialized() == false)
		{
			entity->initPositionCulling(cullingRect);
		}
	}
}


#if !defined(TT_BUILD_FINAL)
std::string EntityMgr::getDebugTimings() const
{
	using ReverseTimings = std::multimap<u64, std::string>;
	ReverseTimings reverseTimings;
	
	u64 totalTime = 0;
	
	for (auto& it : m_timings)
	{
		u64 time = 0;
		u64 maxTime = 0;
		u32 count = 0;
		for (int i = 0; i < maxTimingFrames; ++i)
		{
			u64 val(it.second[i]);
			if (val > 0)
			{
				time += val;
				maxTime = std::max(maxTime, val);
				++count;
			}
		}
		
		if (count > 0)
		{
			u64 avg(time / count);
			reverseTimings.insert(std::make_pair(avg, it.first + " (" + tt::str::toStr(maxTime) + ")"));
			totalTime += avg;
		}
	}
	
	char buf[256];
	sprintf(buf, "Script Timings (%5lld us):\n", totalTime);
	std::string result(buf);
	for (auto it(reverseTimings.rbegin()); it != reverseTimings.rend(); ++it)
	{
		sprintf(buf, "%5lld %s\n", it->first, it->second.c_str());
		result += std::string(buf);
	}
	return result;
}
#endif


//--------------------------------------------------------------------------------------------------
// Private member functions

void EntityMgr::handleEntityPreSpawn(Entity& p_entity)
{
	// Update possible changes to level by other entities in init.
	p_entity.updateSurvey(false);
	/*
	// Rewind doesn't use onLightEnter/Exit callbacks
	if (p_entity.isDetectableByLight() && AppGlobal::getGame()->getLightMgr().isEntityInLight(p_entity))
	{
		p_entity.onLightEnter();
	}
	*/
}


void EntityMgr::flushPostCreateSpawn()
{
	TT_ASSERT(m_isCreatingEntities);
	
	while (m_postCreateSpawn.empty() == false)
	{
		EntityHandle handle = m_postCreateSpawn.front();
		tt::code::helpers::unorderedErase(m_postCreateSpawn, m_postCreateSpawn.begin());
		
		Entity* entity = handle.getPtr();
		TT_NULL_ASSERT(entity);
		
		// Ignore entities that were somehow killed between onInit and onSpawn
		if (entity->isInitialized() == false)
		{
			continue;
		}
		
		handleEntityPreSpawn(*entity);
		
		const script::EntityBasePtr& entityBase = entity->getEntityScript();
		if (entityBase != 0)
		{
			entityBase->onSpawn();
		}
	}
}


void EntityMgr::deinitEntity(EntityHandle p_handle)
{
	Entity* ptr = getEntity(p_handle);
	if (ptr == 0)
	{
		TT_PANIC("Entity already destroyed");
		return;
	}
	ptr->deinit();
	
	// Remove from mapping
	for (IDToHandleMapping::iterator it = m_idToHandleMapping.begin();
	     it != m_idToHandleMapping.end(); ++it)
	{
		if ((*it).second == p_handle)
		{
			m_idToHandleMapping.erase(it);
			break;
		}
	}
	
#ifndef TT_BUILD_FINAL
	for (DeathRowEntries::const_iterator it = m_deathRow.begin(); it != m_deathRow.end(); ++it)
	{
		if ((*it).handle == p_handle)
		{
			TT_PANIC("Entity already part of deathrow, this shouldn't happen");
		}
	}
#endif
	
	// Add to deathrow
	DeathRowEntry entry;
	entry.handle = p_handle;
	entry.framesToLive = 5; // Needs to be more than the 2 sensor updates we do.
	m_deathRow.push_back(entry);
}


void EntityMgr::destroyEntity(EntityHandle p_handle)
{
	// At this point, Entity is not allowed to be initialized any more
	// (deinit() should have been called in a separate step)
	TT_ASSERT(getEntity(p_handle) == 0 || getEntity(p_handle)->isInitialized() == false);
	
	destroy(p_handle);
	
	TT_ASSERT(getEntity(p_handle) == 0);
}


void EntityMgr::updateCullingForAllEntities(const Camera& p_camera)
{
#if !defined(TT_BUILD_FINAL)
	if (m_entityCullingEnabled == false)
	{
		Entity* entity = getFirst();
		for (s32 i = 0; i < getActiveCount(); ++i, ++entity)
		{
			if (entity->isInitialized())
			{
				entity->setPositionCulled(false);
			}
		}
	}
	else
#endif 
	{
		const tt::math::VectorRect& cullingRect(p_camera.getCurrentCullingRect());
		const tt::math::VectorRect& uncullingRect(p_camera.getCurrentUncullingRect());
		
		// First update parents
		Entity* entity = getFirst();
		for (s32 i = 0; i < getActiveCount(); ++i, ++entity)
		{
			if (entity->isInitialized() && entity->hasPositionCullingParent() == false)
			{
				entity->updatePositionCulling(cullingRect, uncullingRect);
			}
		}
		
		// Then update childs
		entity = getFirst();
		for (s32 i = 0; i < getActiveCount(); ++i, ++entity)
		{
			if (entity->isInitialized() && entity->hasPositionCullingParent())
			{
				entity->updatePositionCulling(cullingRect, uncullingRect);
			}
		}
	}
}


void EntityMgr::updateIsOnScreenAllEntities(const Camera& p_camera)
{
	// Use the unculling rect for this purpose so that entities that are slightly off screen, are still
	// counted as being on screen.
	const tt::math::VectorRect& screenRect(p_camera.getCurrentUncullingRect());
	
	Entity* entity = getFirst();
	for (s32 i = 0; i < getActiveCount(); ++i, ++entity)
	{
		if (entity->isInitialized())
		{
			entity->updateIsOnScreen(screenRect);
		}
	}
}


// Namespace end
}
}
}
