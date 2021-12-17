#include <tt/code/bufferutils.h>
#include <tt/code/HandleArrayMgr_utils.h>

#include <toki/game/entity/movementcontroller/MovementControllerMgr.h>
#include <toki/game/entity/Entity.h>
#include <toki/game/entity/EntityMgr.h>
#include <toki/game/Game.h>
#include <toki/AppGlobal.h>


namespace toki {
namespace game {
namespace entity {
namespace movementcontroller {

inline bool sortWeightLess(const DirectionalMovementController& p_a,
                           const DirectionalMovementController& p_b)
{
	return p_a.getSortWeight() <= p_b.getSortWeight();
}


//--------------------------------------------------------------------------------------------------
// Public member functions

MovementControllerMgr::MovementControllerMgr(s32 p_reserveCount)
:
m_directionalControllers(p_reserveCount),
m_scheduledControllerParentChanges()
{
}


MovementControllerHandle MovementControllerMgr::createDirectionalController(
		const EntityHandle& p_entityHandle)
{
	// Try to load the entity's movement set before creating a new controller
	// FIXME: This is too cumbersome to get a type from a handle, rewrite to helper?
	entity::Entity* entity = AppGlobal::getGame()->getEntityMgr().getEntity(p_entityHandle);
	if (entity == 0 || entity->getEntityScript() == 0)
	{
		TT_PANIC("Entity doesn't exist.");
		return MovementControllerHandle();
	}
	
	const std::string&               entityType = entity->getType();
	const level::entity::EntityInfo* entityInfo = AppGlobal::getEntityLibrary().getEntityInfo(entityType);
	if (entityInfo == 0)
	{
		TT_PANIC("Entity doesn't have EntityInfo for its type '%s'", entityType.c_str());
		return MovementControllerHandle();
	}
	
	const std::string movementSetFilename("movement/" + entityInfo->getMovementSetName() + ".ttms");
	game::movement::MovementSetPtr movementSet = game::movement::MovementSet::create(movementSetFilename);
	if (movementSet == 0)
	{
		TT_PANIC("No movementSet found for entity with type '%s'. Can't start Movement on this entity.",
		         entityType.c_str());
		return MovementControllerHandle();
	}
	
	// Now that the movement set has been loaded, continue with controller creation
	return m_directionalControllers.create(
			DirectionalMovementController::CreationParams(p_entityHandle, movementSet));
}


void MovementControllerMgr::updateChanges(real       p_deltaTime, EntityMgr& p_entityMgr)
{
	//TT_Printf("[%06u] MovementControllerMgr::updateChanges - START\n", AppGlobal::getUpdateFrameCount());
	
	DirectionalMovementController::updateChanges(m_directionalControllers.getFirst(),
	                                             m_directionalControllers.getActiveCount(),
	                                             p_deltaTime,
	                                             p_entityMgr);
	
	updateParentChildRelationships();
	
	{
		//TT_Printf("[%06u] MovementControllerMgr::update: Resorting controllers.\n",
		//          AppGlobal::getUpdateFrameCount());
		
		// Sort all active controllers
		// - Step one: have each controller update its sort weight
		DirectionalMovementController* ctrl = m_directionalControllers.getFirst();
		for (s32 i = m_directionalControllers.getActiveCount(); i > 0; --i, ++ctrl)
		{
			ctrl->calculateSortWeight();
		}
		
		// - Step two: sort using controller sort weight for comparison
		m_directionalControllers.sort(sortWeightLess);
		
		/*
		ctrl = m_directionalControllers.getFirst();
		const s32 activeCount = m_directionalControllers.getActiveCount();
		for (s32 i = 0; i < activeCount; ++i, ++ctrl)
		{
			TT_Printf("[%d] weight: %d Handle: 0X%X\n", i, ctrl->getSortWeight(), ctrl->getHandle().getValue());
		}
		// */
	}
	//TT_Printf("[%06u] MovementControllerMgr::updateChanges - END\n", AppGlobal::getUpdateFrameCount());
}


void MovementControllerMgr::update(real       p_deltaTime, EntityMgr& p_entityMgr)
{
	//TT_Printf("[%06u] MovementControllerMgr::update - START ----------------------------------------------\n", AppGlobal::getUpdateFrameCount());
	
	DirectionalMovementController::update(m_directionalControllers.getFirst(),
	                                      m_directionalControllers.getActiveCount(),
	                                      p_deltaTime,
	                                      p_entityMgr);
	//TT_Printf("[%06u] MovementControllerMgr::update - END\n", AppGlobal::getUpdateFrameCount());
}


DirectionalMovementController* MovementControllerMgr::getDirectionalController(const MovementControllerHandle& p_handle)
{
	return m_directionalControllers.get(p_handle);
}


void MovementControllerMgr::destroyController(const MovementControllerHandle& p_handle)
{
	m_directionalControllers.destroy(p_handle);
}


void MovementControllerMgr::setCollisionParentEntity(const MovementControllerHandle& p_controller,
                                                     const EntityHandle&             p_parent)
{
	DirectionalMovementController* ctrl = getDirectionalController(p_controller);
	TT_ASSERTMSG(ctrl != 0, "Trying to set collision parent for invalid movement controller.");
	TT_ASSERTMSG(ctrl == 0 || p_parent != ctrl->getEntityHandle(),
	             "Trying to set collision parent for entity to itself. This is not allowed.");
	TT_ASSERTMSG(ctrl == 0 || ctrl->isCollisionParentEnabled(), "Collision Parenting is not enabled");
	
	if (ctrl     != 0                       &&
	    p_parent != ctrl->getEntityHandle() &&
	    p_parent != ctrl->getCollisionParentEntityScheduled()) // Do nothing if the handle didn't change (prevent unnecessary work)
	{
		//TT_Printf("[%06u] Ctrl 0x%08X: parent 0x%08X\n",
		//          AppGlobal::getUpdateFrameCount(), p_controller.getValue(), p_parent.getValue());
		ctrl->setCollisionParentEntityScheduled(p_parent);
		m_scheduledControllerParentChanges.push_back(p_controller);
	}
}


void MovementControllerMgr::reset()
{
	m_directionalControllers.reset();
	m_scheduledControllerParentChanges.clear();
}


void MovementControllerMgr::handlePathMgrReset()
{
	DirectionalMovementController* ctrl = m_directionalControllers.getFirst();
	for (s32 i = m_directionalControllers.getActiveCount(); i > 0; --i, ++ctrl)
	{
		ctrl->handlePathMgrReset();
	}
}


void MovementControllerMgr::serialize(toki::serialization::SerializationMgr& p_serializationMgr) const
{
	const serialization::SerializerPtr& section = p_serializationMgr.getSection(serialization::Section_MovementControllerMgr);
	if (section == 0)
	{
		TT_PANIC("Serialization manager does not contain a section for the SensorMgr data.");
		return;
	}
	
	tt::code::BufferWriteContext context(section->getAppendContext());
	
	namespace bu = tt::code::bufferutils;
	
	tt::code::serializeHandleArrayMgr(m_directionalControllers, &context);
	
	bu::put(static_cast<u32>(m_scheduledControllerParentChanges.size()), &context);
	for (MovementControllers::const_iterator it = m_scheduledControllerParentChanges.begin();
	     it != m_scheduledControllerParentChanges.end(); ++it)
	{
		bu::putHandle(*it, &context);
	}
	
	// Done writing data: ensure the underlying buffer gets all the data that was written
	context.flush();
}


void MovementControllerMgr::unserialize(const toki::serialization::SerializationMgr& p_serializationMgr)
{
	const serialization::SerializerPtr& section = p_serializationMgr.getSection(serialization::Section_MovementControllerMgr);
	if (section == 0)
	{
		TT_PANIC("Serialization manager does not contain a section for the SensorMgr data.");
		return;
	}
	
	tt::code::BufferReadContext context(section->getReadContext());
	
	namespace bu = tt::code::bufferutils;
	
	tt::code::unserializeHandleArrayMgr(&m_directionalControllers, &context);
	
	const u32 scheduledChangesCount = bu::get<u32>(&context);
	m_scheduledControllerParentChanges.clear();
	m_scheduledControllerParentChanges.reserve(static_cast<MovementControllers::size_type>(scheduledChangesCount));
	for (u32 i = 0; i < scheduledChangesCount; ++i)
	{
		m_scheduledControllerParentChanges.push_back(bu::getHandle<DirectionalMovementController>(&context));
	}
}


//--------------------------------------------------------------------------------------------------
// Private member functions

void MovementControllerMgr::updateParentChildRelationships()
{
	if (m_scheduledControllerParentChanges.empty())
	{
		return;
	}
	
	//TT_Printf("[%06u] MovementControllerMgr::updateParentChildRelationships: Applying %u parent-child relationship updates.\n",
	//          AppGlobal::getUpdateFrameCount(), m_scheduledControllerParentChanges.size());
	
	MovementControllers changesToProcessNow;
	using std::swap;
	swap(changesToProcessNow, m_scheduledControllerParentChanges);
	
	for (MovementControllers::iterator it = changesToProcessNow.begin();
	     it != changesToProcessNow.end(); ++it)
	{
		DirectionalMovementController* ctrl = getDirectionalController(*it);
		if (ctrl != 0 && ctrl->getCollisionParentEntityScheduled().isEmpty() == false)
		{
			ctrl->makeScheduledCollisionParentCurrent();
		}
	}
	
	// Parent-child relationships changed
	
	// - Update each controller's collision ancestor
	DirectionalMovementController* ctrl = m_directionalControllers.getFirst();
	for (s32 i = m_directionalControllers.getActiveCount(); i > 0; --i, ++ctrl)
	{
		ctrl->setCollisionAncestorForChildren();
	}
}

// Namespace end
}
}
}
}
