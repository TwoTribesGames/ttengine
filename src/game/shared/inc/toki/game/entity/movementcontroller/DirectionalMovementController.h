#if !defined(INC_TOKI_GAME_ENTITY_MOVEMENTCONTROLLER_DIRECTIONALMOVEMENTCONTROLLER_H)
#define INC_TOKI_GAME_ENTITY_MOVEMENTCONTROLLER_DIRECTIONALMOVEMENTCONTROLLER_H


#include <tt/code/fwd.h>
#include <tt/math/Rect.h>
#include <tt/math/Vector2.h>
#include <tt/pres/fwd.h>

#include <toki/game/entity/movementcontroller/PhysicsSettings.h>
#include <toki/game/entity/fwd.h>
#include <toki/game/movement/fwd.h>
#include <toki/game/movement/MovementSet.h>
#include <toki/game/movement/TileCollisionHelper.h>
#include <toki/pres/fwd.h>


namespace toki {
namespace game {
namespace entity {
namespace movementcontroller {

class DirectionalMovementController
{
public:
	struct CreationParams
	{
		inline CreationParams(const EntityHandle&             p_entityHandle,
		                      const movement::MovementSetPtr& p_movementSet)
		:
		entityHandle(p_entityHandle),
		movementSet (p_movementSet)
		{ }
		
		EntityHandle             entityHandle;
		movement::MovementSetPtr movementSet;
	};
	typedef const CreationParams& ConstructorParamType;
	
	
	DirectionalMovementController(const CreationParams&           p_creationParams,
	                              const MovementControllerHandle& p_ownHandle);
	~DirectionalMovementController();
	
	inline const EntityHandle& getEntityHandle() const { return m_entityHandle; }
	
	void setMovementSet(const std::string& p_name);
	void setParentEntity(const EntityHandle& p_handle,
	                     const tt::math::Vector2& p_offset = tt::math::Vector2::zero);
	inline const EntityHandle& getParentEntity() const { return m_parentEntity; }
	inline void setUseParentForward(bool p_useForward) { m_useParentForward = p_useForward; }
	inline bool hasUseParentForward() const            { return m_useParentForward;         }
	inline void setUseParentDown(bool p_useDown)       { m_useParentDown = p_useDown;       }
	inline bool hasUseParentDown() const               { return m_useParentDown;            }
	
	inline const movement::MoveBasePtr& getCurrentMove() const { return m_currentMove; }
	const std::string& getCurrentMoveName() const;
	void restartAllPresentationObjects(Entity& p_entity);
	void restartPresentationObject(toki::pres::PresentationObject& p_pres);
	
	static void updateChanges(DirectionalMovementController* p_first,
	                          s32                            p_count,
	                          real                           p_deltaTime,
	                          EntityMgr&                     p_entityMgr);
	
	static void update(DirectionalMovementController* p_first,
	                   s32                            p_count,
	                   real                           p_deltaTime,
	                   EntityMgr&                     p_entityMgr);
	
	void updateParentAndPushMovement(real p_deltaTime, Entity& p_entity);
	void updateChanges(real p_deltaTime, Entity& p_entity);
	void update(real p_deltaTime, Entity& p_entity);
	
	bool startNewMovement(movement::Direction p_direction, real p_endDistance);
	void stopMovement();
	
	/*! \brief Indicates whether this entity can be moved in the specified direction,
	           not taking its movement set into account (purely based on the level and entities layout). */
	bool canBeMovedInDirection(movement::Direction p_direction) const;
	
	// Physics Movement
	bool isDoingDirectionPhysicsMovement() const { return m_physicsMovementMode == PhysicsMovementMode_Direction; }
	void setMovementDirection(    const tt::math::Vector2& p_direction);
	void startMovementInDirection(const tt::math::Vector2& p_direction, const PhysicsSettings& p_settings);
	void startPathMovement(       const tt::math::Vector2& p_positionOrOffset,
	                              const PhysicsSettings&   p_settings,
	                              const EntityHandle&      p_optionalTarget);
	void startMovementToPosition( const tt::math::Vector2& p_position,  const PhysicsSettings& p_settings);
	void startMovementToEntity(const EntityHandle&      p_target,
	                           const tt::math::Vector2& p_offset,
	                           const PhysicsSettings&   p_settings);
	
	inline void setPhysicsSettings(const PhysicsSettings& p_settings) { m_settings = p_settings; }
	inline const PhysicsSettings& getPhysicsSettings() const          { return m_settings;       }
	
	// Direction
	bool changeDirection(movement::Direction p_direction); // Needed for direct control.
	inline movement::Direction getDirection()          const { return m_direction;          }
	inline movement::Direction getRequestedDirection() const { return m_requestedDirection; }
	/*! \brief returns Direction based on m_speed, if zero speed it falls back on requestedDirection. */
	movement::Direction getActualMovementDirection() const;
	
	// Carry movement (collision parent/children):
	inline void                setCollisionParentEnabled(bool p_enabled) { m_collisionParentEnabled = p_enabled; }
	inline bool                isCollisionParentEnabled() const { return m_collisionParentEnabled; }
	bool                       setCollisionParentEntity(const EntityHandle& p_handle);
	inline const EntityHandle& getCollisionParentEntity() const { return m_collisionParentEntity; }
	
	void scheduleReevaluateCollisionParent()
	{ if (m_collisionParentEnabled) m_reevaluateCollisionParentScheduled = true; }
	
	inline void                setCollisionParentEntityScheduled(const EntityHandle& p_parent)
	{ if (m_collisionParentEnabled) m_collisionParentEntityScheduled = p_parent; }
	inline const EntityHandle& getCollisionParentEntityScheduled() const
	{ return m_collisionParentEntityScheduled; }
	
	void makeScheduledCollisionParentCurrent(bool p_unregisterFromCurrentParent = true);
	
	inline const EntityHandleSet& getCollisionChildren() const { return m_collisionChildren; }
	
	/*! \brief If this controller is a collision ancestor (i.e. has no collision parent),
	           sets itself as the ancestor for all its direct and indirect children. */
	void setCollisionAncestorForChildren();
	inline const EntityHandle& getCollisionAncestor() const { return m_collisionAncestor; }
	
	void makeCollisionChildrenReevaluateParent(const EntityHandle& p_caller) const;
	static void makeCollisionChildrenReevaluateParent(const EntityHandleSet& p_children,
	                                                  const EntityHandle&    p_caller);
	
	void calculateSortWeight();
	inline s32 getSortWeight() const { return m_sortWeight; }
	
	/*! \brief Notifies this controller that the PathMgr was reset:
	           any controllers actively using path finding need to re-acquire their resources. */
	void handlePathMgrReset();
	
	static DirectionalMovementController* getPointerFromHandle(const MovementControllerHandle& p_handle);
	
	/*! \brief Resets this controller: makes it invalid for use.
	    \note This is done so the temp object in swap (see below) doesn't do cleanup in dtor.*/
	void invalidateTempCopy();
	
	void                  serializeCreationParams  (tt::code::BufferWriteContext* p_context) const;
	static CreationParams unserializeCreationParams(tt::code::BufferReadContext*  p_context);
	
	void serialize  (tt::code::BufferWriteContext* p_context) const;
	void unserialize(tt::code::BufferReadContext*  p_context);
	
	inline const MovementControllerHandle& getHandle() const                          { return m_ownHandle; }
	inline const tt::math::Vector2&        getSpeed()  const                          { return m_speed;     }
	inline void                            setSpeed(const tt::math::Vector2& p_speed) { m_speed = p_speed;  }
	inline void                            setReferenceSpeedEntity(const EntityHandle& p_entity) { m_referenceSpeedEntity = p_entity;  }
	inline real                            getDistanceMoved() const                   { return m_distance;  }
	
	inline void scheduleSurveyUpdate() {                                           m_dirtyLevel = DirtyLevel_UpdateSurvey; }
	inline void scheduleReevalMove()   { if (m_dirtyLevel < DirtyLevel_ReevalMove) m_dirtyLevel = DirtyLevel_ReevalMove;   }
	inline void scheduleReselectMove() { m_requestBitMask.setFlag(RequestFlag_ReselectMove); }
	
	void updateLocalCollision(const tt::math::PointRect& p_tileRect, const entity::Entity& p_entity);
	movement::Directions getTouchingCollisionDirections(const Entity& p_entity) const;
	movement::Directions getTileCollisionDirections(const Entity& p_entity) const;
	
	inline tt::pres::Tags getTags() const
	{
	    tt::pres::Tags result;
	    result.insert(tt::pres::Tag( m_rightFoot ? "right_foot" : "left_foot" ));
	    return result;
	}
	
	inline void setSpeedFactor(const tt::math::Vector2& p_factor) {        m_speedFactor = p_factor; }
	inline const tt::math::Vector2& getSpeedFactor() const        { return m_speedFactor;            }
	inline void setSpeedFactorX(real p_x) {        m_speedFactor.x = p_x; }
	inline void setSpeedFactorY(real p_y) {        m_speedFactor.y = p_y; }
	inline real getSpeedFactorX() const   { return m_speedFactor.x;       }
	inline real getSpeedFactorY() const   { return m_speedFactor.y;       }
	inline void setExternalForce(const tt::math::Vector2& p_force) {         m_externalForce = p_force; }
	inline const tt::math::Vector2& getExternalForce() const        { return m_externalForce;           }
	
	/*! \brief Indicates whether the current move can be interrupted. */
	bool isCurrentMoveInterruptible() const;
	
	// Please don't call this.
	inline bool checkCollision_HACK(Entity& p_entity, const tt::math::Vector2& p_deltaPos)
	{ return checkCollision(p_entity, p_deltaPos); }

private:
	// For 'physicsed' movement
	enum PhysicsMovementMode
	{
		PhysicsMovementMode_None,      // "Normal" step movement
		PhysicsMovementMode_Idle,      // Keep updating the physics
		PhysicsMovementMode_Direction, // Move in a direction
		PhysicsMovementMode_Point,     // Move to a point
		PhysicsMovementMode_Entity,    // Move to an entity
		PhysicsMovementMode_Path       // Follow path to point/entity*. (*entity not supported yet.)
	};
	
	enum DirtyLevel
	{
		DirtyLevel_Nothing,
		DirtyLevel_ReevalMove,
		DirtyLevel_UpdateSurvey
	};
	
	enum RequestFlag
	{
		RequestFlag_ReselectMove,   // Re-Select move based on current location and active dir. (Differs from reevaluate in that it can select another move while the current is valid.)
		RequestFlag_PresistentMove, // Presistent movement never fails. It will continue to try and move in the requested direction until explicitly stopped.
		//RequestFlag_Stop,         // Deferred stop command. (Remove this flag if unused/commented out.)
		
		RequestFlag_UpdateLocalCollision,
		
		RequestFlag_Count
	};
	typedef tt::code::BitMask<RequestFlag, RequestFlag_Count> RequestBitMask;
	
	
	inline void setMovementSetImpl(const std::string& p_name)
	{
		const std::string fileName("movement/" + p_name + ".ttms");
		m_moveSet = game::movement::MovementSet::create(fileName);
		TT_NULL_ASSERT(m_moveSet);
	}
	
	bool startNewMovementImpl(movement::Direction p_direction, real p_endDistance);
	void endMovement();
	
	void doAnimation(      real p_deltaTime, Entity& p_entity);
	void endAnimationMove(Entity& p_entity, const tt::math::PointRect& p_previousTiles);
	void doPhysicsMovement(real p_deltaTime, Entity& p_entity, const tt::math::PointRect& p_previousTiles);
	void doNormalMovement( real p_deltaTime, Entity& p_entity, const tt::math::PointRect& p_previousTiles);
	
	void pushOtherEntities                (const Entity& p_entity);
	void establishCollisionParentHierarchy(const Entity& p_entity);
	
	/*! \brief Sets a new ancestor for self and all direct and indirect children. */
	void overrideCollisionAncestorRecursive(const EntityHandle& p_ancestorToSet);
	
	void addCollisionChild   (const EntityHandle& p_childHandle);
	void removeCollisionChild(const EntityHandle& p_childHandle);
	
	bool checkCollision(Entity&                    p_entity,
	                    const tt::math::Vector2&   p_deltaPos);
	
	tt::math::PointRect calculateNewTileRect(const Entity&            p_entity,
	                                         const tt::math::Vector2& p_speed,
	                                         bool                     p_preMove = true,
	                                         bool                     p_doHackCheck = false) const;
	
	tt::math::PointRect onNewTilePosition(Entity&                    p_entity,
	                                      const tt::math::PointRect& p_previousTiles,
	                                      bool&                      p_newMoveSucceeded_OUT);
	
	/*! \brief Handles bookkeeping for reaching a new tile position,
	           without handling any movement logic (updating move, selecting new move). */
	void onNewTilePositionWithoutMoving(Entity&                    p_entity,
	                                    const tt::math::PointRect& p_previousTiles);
	
	void doMoveStartLogic (Entity& p_entity);
	void doMoveFailedLogic(Entity& p_entity,
	                       movement::Direction p_dir,
	                       const movement::MoveBasePtr& p_failedMove);
	
	inline tt::math::Vector2 applySpeedFactor(const tt::math::Vector2& p_speed) const
	{
		return tt::math::Vector2(p_speed.x * m_speedFactor.x,
		                         p_speed.y * m_speedFactor.y);
	}
	
	MovementControllerHandle m_ownHandle;
	
	tt::math::Vector2            m_speedFactor;          // Scale factor set by script
	tt::math::Vector2            m_speed;                // Local speed (relative to the reference speed entity)
	tt::math::Vector2            m_externalForce;        // External force (such as gravity)
	entity::EntityHandle         m_referenceSpeedEntity;
	movement::MoveBasePtr        m_currentMove;
	movement::MoveBasePtr        m_previousMove;
	movement::ConstTransitionPtr m_transition;
	movement::MovementSetPtr     m_moveSet;
	
	RequestBitMask           m_requestBitMask;
	movement::Direction      m_requestedDirection;
	movement::Direction      m_direction;   // The direction we're currently moving in.
	real                     m_distance;    // The distance that was moved in m_direction.
	real                     m_endDistance; // At which m_distance this movement should stop.
	
	DirtyLevel               m_dirtyLevel;
	
	// Animation movement
	real                     m_currentTime;
	tt::math::Vector2        m_startPosition;
	bool                     m_animationMoveCanceled;
	bool                     m_rightFoot; // Toggled each animation move to set left_foot en right_foot tags.
	
	// 'Physics' movement
	PhysicsMovementMode      m_physicsMovementMode;
	tt::math::Vector2        m_directionControl;  // Normalized input for the physics
	tt::math::Vector2        m_targetPosition;
	entity::EntityHandle     m_targetEntity;
	tt::math::Vector2        m_targetEntityOffset; // also used for offset when parent entity is set.
	PhysicsSettings          m_settings;
	s32                      m_pathAgentID; // FIXME: This needs to be serialized. (Once PathMgr is serialized.)
	bool                     m_hasSolidCollision;
	tt::math::Vector2        m_lastSolidNormal;
	
	// Bitmask indicating which sides/corners have collision; based on registered tiles
	movement::TileCollisionHelper::CollisionResultMask m_localCollision;
	movement::TileCollisionHelper::CollisionResultMask m_localCollisionInclOtherEntities;
	tt::math::PointRect                                m_localCollisionTileRect;
	
	EntityHandle             m_parentEntity;
	bool                     m_useParentForward;
	bool                     m_useParentDown;
	EntityHandle             m_entityHandle;  // To which entity this controller belongs
	
	tt::math::Vector2 m_externalPush;  // accumulated position changes done by outside code (not by this movement controller itself)
	
	// Carry movement
	bool                     m_collisionParentEnabled;
	EntityHandle             m_collisionParentEntity;
	EntityHandle             m_collisionParentEntityScheduled;  // scheduled to be set as new parent
	tt::math::Vector2        m_collisionParentPosition;
	tt::math::Vector2        m_collisionParentSpeed;
	EntityHandleSet          m_collisionChildren;
	EntityHandle             m_collisionAncestor;  // root of the collision parent hierarchy
	s32                      m_sortWeight;
	bool                     m_reevaluateCollisionParentScheduled;
};

// Namespace end
}
}
}
}


#endif  // !defined(INC_TOKI_GAME_ENTITY_MOVEMENTCONTROLLER_DIRECTIONALMOVEMENTCONTROLLER_H)
