#if !defined(INC_TOKI_GAME_ENTITY_MOVEMENTCONTROLLER_MOVEMENTCONTROLLERMGR_H)
#define INC_TOKI_GAME_ENTITY_MOVEMENTCONTROLLER_MOVEMENTCONTROLLERMGR_H


#include <utility>
#include <vector>

#include <tt/code/HandleArrayMgr.h>

#include <toki/game/entity/movementcontroller/DirectionalMovementController.h>
#include <toki/game/entity/fwd.h>
#include <toki/game/movement/fwd.h>
#include <toki/serialization/SerializationMgr.h>


namespace toki {
namespace game {
namespace entity {
namespace movementcontroller {

class MovementControllerMgr
{
public:
	explicit MovementControllerMgr(s32 p_reserveCount);
	
	MovementControllerHandle createDirectionalController(const EntityHandle& p_entityHandle);
	
	void updateChanges(real p_deltaTime, EntityMgr& p_entityMgr);
	void update(real p_deltaTime, EntityMgr& p_entityMgr);
	
	DirectionalMovementController* getDirectionalController(const MovementControllerHandle& p_handle);
	
	void destroyController(const MovementControllerHandle& p_handle);
	
	void setCollisionParentEntity(const MovementControllerHandle& p_controller,
	                              const EntityHandle&             p_parent);
	
	void reset();
	
	/*! \brief Notifies all controllers that the PathMgr was reset:
	           any controllers actively using path finding need to re-acquire their resources. */
	void handlePathMgrReset();
	
	inline s32 getActiveControllerCount() const { return m_directionalControllers.getActiveCount(); }
	
	void serialize  (      toki::serialization::SerializationMgr& p_serializationMgr) const;
	void unserialize(const toki::serialization::SerializationMgr& p_serializationMgr);
	
private:
	typedef tt::code::HandleArrayMgr<DirectionalMovementController> DirectionalControllerArray;
	
	
	void updateParentChildRelationships();
	
	
	DirectionalControllerArray m_directionalControllers;
	
	/*
	
	update:
	- normal update
	- iterate afterupdatepoke, update parent movement (move based on parent move, not regular movement)
	
	vector<movementctrlhandle> AfterUpdatePoke;
	*/
	
	typedef std::vector<MovementControllerHandle> MovementControllers;
	MovementControllers m_scheduledControllerParentChanges;
};

// Namespace end
}
}
}
}


#endif  // !defined(INC_TOKI_GAME_ENTITY_MOVEMENTCONTROLLER_MOVEMENTCONTROLLERMGR_H)
