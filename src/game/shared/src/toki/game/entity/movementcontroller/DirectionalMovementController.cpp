#include <vector>

#include <tt/code/bufferutils.h>
#include <tt/platform/tt_printf.h>

#include <toki/game/entity/movementcontroller/DirectionalMovementController.h>
#include <toki/game/entity/movementcontroller/physics_integration.h>
#include <toki/game/entity/movementcontroller/PhysicsSettings.h>
#include <toki/game/entity/Entity.h>
#include <toki/game/entity/EntityMgr.h>
#include <toki/game/entity/EntityTiles.h>
#include <toki/game/movement/TileCollisionHelper.h>
#include <toki/game/movement/SurroundingsSurvey.h>
#include <toki/game/movement/MoveAnimation.h>
#include <toki/game/movement/MoveVector.h>
#include <toki/game/movement/MovementSet.h>
#include <toki/game/movement/Transition.h>
#include <toki/game/movement/Validator.h>
#include <toki/game/movement/fwd.h>
#include <toki/game/pathfinding/TileCache.h>
#include <toki/game/script/EntityBase.h>
#include <toki/game/Game.h>
#include <toki/level/helpers.h>
#include <toki/level/TileRegistrationMgr.h>
#include <toki/pres/PresentationObject.h>
#include <toki/serialization/utils.h>
#include <toki/AppGlobal.h>
#include <toki/constants.h>


namespace toki {
namespace game {
namespace entity {
namespace movementcontroller {

//--------------------------------------------------------------------------------------------------
// Public member functions

DirectionalMovementController::DirectionalMovementController(
		const CreationParams&           p_creationParams,
		const MovementControllerHandle& p_ownHandle)
:
m_ownHandle(p_ownHandle),
m_speedFactor(tt::math::Vector2::allOne),
m_speed(tt::math::Vector2::zero),
m_externalForce(tt::math::Vector2::zero),
m_referenceSpeedEntity(entity::EntityHandle()),
m_currentMove(),
m_previousMove(),
m_transition(),
m_moveSet(p_creationParams.movementSet),
m_requestedDirection(movement::Direction_None),
m_direction(movement::Direction_None),
m_distance(0.0f),
m_endDistance(0.0f),
m_dirtyLevel(DirtyLevel_Nothing),
m_currentTime(0.0f),
m_startPosition(tt::math::Vector2::zero),
m_animationMoveCanceled(false),
m_rightFoot(true),
m_physicsMovementMode(PhysicsMovementMode_None),
m_directionControl(tt::math::Vector2::zero),
m_targetPosition(tt::math::Vector2::zero),
m_targetEntity(),
m_targetEntityOffset(tt::math::Vector2::zero),
m_settings(),
m_pathAgentID(-1),
m_hasSolidCollision(false),
m_lastSolidNormal(tt::math::Vector2::zero),
m_localCollision(),
m_localCollisionTileRect(),
m_parentEntity(),
m_useParentForward(false),
m_useParentDown(false),
m_entityHandle(p_creationParams.entityHandle),
m_externalPush(tt::math::Vector2::zero),
m_collisionParentEnabled(false),
m_collisionParentEntity(),
m_collisionParentEntityScheduled(),
m_collisionParentPosition(tt::math::Vector2::zero),
m_collisionParentSpeed(tt::math::Vector2::zero),
m_collisionChildren(),
m_collisionAncestor(p_creationParams.entityHandle),
m_sortWeight(-1),
m_reevaluateCollisionParentScheduled(false)
{
	TT_NULL_ASSERT(m_moveSet);
	
	Entity* entity = m_entityHandle.getPtr();
	TT_NULL_ASSERT(entity);
	if (entity != 0)
	{
		updateLocalCollision(entity->getRegisteredTileRect(), *entity);
	}
}


DirectionalMovementController::~DirectionalMovementController()
{
	entity::Entity* collisionParent = m_collisionParentEntity.getPtr();
	if (collisionParent != 0)
	{
		DirectionalMovementController* currentParentCtrl = collisionParent->getMovementControllerHandle().getPtr();
		TT_ASSERTMSG(currentParentCtrl != 0,
		             "A collision parent entity must have a MovementController "
		             "(current parent 0x%p doesn't).", collisionParent);
		if (currentParentCtrl != 0)
		{
			currentParentCtrl->removeCollisionChild(getEntityHandle());
		}
	}
	
	for (EntityHandleSet::iterator it = m_collisionChildren.begin(); it != m_collisionChildren.end(); ++it)
	{
		Entity* child = (*it).getPtr();
		if (child != 0)
		{
			DirectionalMovementController* childCtrl = child->getMovementControllerHandle().getPtr();
			TT_NULL_ASSERT(childCtrl);
			if (childCtrl != 0)
			{
				childCtrl->setCollisionParentEntityScheduled(EntityHandle());
				// Only unregister current parent if that's not this object.
				const bool unregisterFromCurrentParent = (childCtrl->getCollisionParentEntity() != m_entityHandle);
				childCtrl->makeScheduledCollisionParentCurrent(unregisterFromCurrentParent);
			}
		}
	}
	
	
	if (m_pathAgentID >= 0 && AppGlobal::hasGame())
	{
		entity::Entity* ownEntity = m_entityHandle.getPtr();
		if (ownEntity != 0)
		{
			pathfinding::PathMgr& pathMgr(AppGlobal::getGame()->getPathMgr());
			pathfinding::TileCache* tileCache = pathMgr.getTileCacheForAgentRadius(
					ownEntity->getPathAgentRadius());
			if (tileCache != 0)
			{
				tileCache->removeAgent(m_pathAgentID);
			}
		}
		
		m_pathAgentID = -1;
	}
}


void DirectionalMovementController::setMovementSet(const std::string& p_name)
{
	setMovementSetImpl(p_name);
	endMovement(); //resetMovement();
	m_currentMove.reset();
	m_previousMove.reset();
	m_transition.reset();
	
	// Select a new move. (Don't use normal functions because those early out for suspended entities.)
	if (m_physicsMovementMode == PhysicsMovementMode_None)
	{
		entity::Entity* entity = m_entityHandle.getPtr();
		TT_NULL_ASSERT(entity);
		m_currentMove = m_moveSet->getMove(m_direction, m_currentMove, *entity);
		doMoveStartLogic(*entity);
		TT_ASSERTMSG(m_currentMove != 0,
		             "Failed to find new move after switching to new movementset: '%s'\n",
		             p_name.c_str());
	}
}


void DirectionalMovementController::setParentEntity(const EntityHandle&      p_handle,
                                                    const tt::math::Vector2& p_offset)
{
	if (p_handle == getEntityHandle())
	{
		TT_PANIC("Trying to set parent for entity to itself. This is not allowed.");
		return;
	}
	
	const bool hadAParent = (m_parentEntity.isEmpty() == false);
	
	m_parentEntity          = p_handle;
	m_targetEntityOffset    = p_offset;
	m_animationMoveCanceled = true;
	
	// Cannot have a collision parent if we have a 'normal' parent
	if (m_parentEntity.isEmpty()                      == false &&
	    getCollisionParentEntityScheduled().isEmpty() == false)
	{
		setCollisionParentEntity(EntityHandle());
	}
	
	if (m_parentEntity.isEmpty()) // No new parent
	{
		if (hadAParent && m_currentMove != 0)
		{
			// Restore speed.
			m_speed = applySpeedFactor(m_currentMove->getSpeed());
			scheduleReselectMove();
		}
	}
	else // New parent
	{
		m_speed.setValues(0.0f, 0.0f);
		
		Entity* parent = m_parentEntity.getPtr();
		if (parent == 0)
		{
			TT_PANIC("Calling setParentEntity with a non-empty handle %d but parent is null!", p_handle.getValue());
			setParentEntity(EntityHandle()); // Remove parent and continue with normal movement.
			return;
		}
		
		Entity* entity = getEntityHandle().getPtr();
		TT_NULL_ASSERT(entity);
		
		if (m_useParentForward &&
		    entity->isOrientationForwardLeft() != parent->isOrientationForwardLeft())
		{
			entity->setOrientationForwardAsLeft(parent->isOrientationForwardLeft());
		}
		
		if (m_useParentDown &&
		    entity->getOrientationDown() != parent->getOrientationDown())
		{
			entity->setOrientationDown(parent->getOrientationDown());
		}
		
		// Set same position as parent.
		entity->setPositionForced(parent->getPosition() + 
		                          parent->applyOrientationToVector2(m_targetEntityOffset),
		                          false);
	}
}


const std::string& DirectionalMovementController::getCurrentMoveName() const
{
	static const std::string empty;
	return (m_currentMove != 0) ? m_currentMove->getName() : empty;
}


void DirectionalMovementController::restartAllPresentationObjects(Entity& p_entity)
{
	TT_ASSERT(p_entity.getHandle() == m_entityHandle);
	if (m_physicsMovementMode != PhysicsMovementMode_None     && // Doing physics
		m_settings.presentationAnimationName.empty() == false)   // Have a custom name
	{
		// Start the physics name
		p_entity.startAllPresentationObjectsForMovement(m_settings.presentationAnimationName, tt::pres::Tags());
	}
	else if (m_currentMove != 0)
	{
		// Do default animation based on current move.
		m_currentMove->startAllPresentationObjectsImpl(p_entity, *this, false);
	}
}


void DirectionalMovementController::restartPresentationObject(toki::pres::PresentationObject& p_pres)
{
	if (p_pres.isAffectedByMovement())
	{
		if (m_physicsMovementMode != PhysicsMovementMode_None     && // Doing physics
		    m_settings.presentationAnimationName.empty() == false)   // Have a custom name
		{
			// Start the physics name
			p_pres.start(m_settings.presentationAnimationName, tt::pres::Tags(), false);
		}
		else if (m_transition != 0)
		{
			entity::Entity* entity = getEntityHandle().getPtr();
			TT_NULL_ASSERT(entity);
			const script::EntityBasePtr& script = entity->getEntityScript();
			if (m_transition->isTurn())
			{
				script->queueSqFun("onTurnEnded");
			}
			if (m_transition->hasEndCallback())
			{
				script->queueSqFun(m_transition->getEndCallback());
			}
			m_transition.reset();
			
			if (m_currentMove != 0)
			{
				m_speed = applySpeedFactor(m_currentMove->getSpeed());
				m_currentMove->startAllPresentationObjects(*entity, *this);
				//m_currentMove->startPresentationObjectImpl(p_pres, false);
			}
			else
			{
				TT_PANIC("Was playing move transition without a current move!");
			}
		}
		else if (m_currentMove != 0)
		{
			// Do default animation based on current move.
			m_currentMove->startPresentationObjectImpl(p_pres, false);
		}
		else if (p_pres.getPriority() <= 0)
		{
			// Need to start an animation at this point because there is no animation started from script (that's blocking).
			
			TT_NONFATAL_PANIC("Can't restartPresentationObject because there is no current move! (Entity type: '%s'.)",
			                  p_pres.getParentType().c_str());
		}
	}
}



void DirectionalMovementController::updateChanges(DirectionalMovementController* p_first,
                                                  s32                            p_count,
                                                  real                           p_deltaTime,
                                                  EntityMgr&                     p_entityMgr)
{
	DirectionalMovementController* controller = p_first;
	for (s32 i = 0; i < p_count; ++i, ++controller)
	{
		Entity* entity = p_entityMgr.getEntity(controller->getEntityHandle());
		
		TT_NULL_ASSERT(entity);
		TT_ASSERT(entity->isInitialized());
		if (entity != nullptr)
		{
			// FIXME: By getting the entity (and using it) we will pollute the cache.
			controller->updateChanges(p_deltaTime, *entity);
		}
	}
}


void DirectionalMovementController::update(DirectionalMovementController* p_first,
                                           s32                            p_count,
                                           real                           p_deltaTime,
                                           EntityMgr&                     p_entityMgr)
{
	DirectionalMovementController* controller = p_first;
	for (s32 i = 0; i < p_count; ++i, ++controller)
	{
		Entity* entity = p_entityMgr.getEntity(controller->getEntityHandle());
		TT_NULL_ASSERT(entity);
		TT_ASSERT(entity->isInitialized());
		if (entity != nullptr)
		{
			// FIXME: By getting the entity (and using it) we will pollute the cache.
			controller->update(p_deltaTime, *entity);
		}
	}
}


void DirectionalMovementController::updateParentAndPushMovement(real p_deltaTime, Entity& p_entity)
{
	(void)p_deltaTime;
	if (m_externalPush != tt::math::Vector2::zero &&
	    m_currentMove != 0 && m_currentMove->isAnimation())
	{
		// Got pushed, cancel animation move.
		m_animationMoveCanceled = true;
	}
	
	m_collisionParentEnabled = p_entity.canBeCarried();
	
	// Collision Parent movement
	if (getCollisionParentEntity().isEmpty() == false)
	{
		Entity* parent = getCollisionParentEntity().getPtr();
		if (parent == 0)
		{
			// Collision Parent is gone!
			// What to do here?
			setCollisionParentEntity(EntityHandle());
		}
		else
		{
			const tt::math::Vector2 parentDeltaPos(parent->getPosition() - m_collisionParentPosition);
			m_collisionParentSpeed    = parentDeltaPos;
			m_collisionParentPosition = parent->getPosition();
			
			m_externalPush += m_collisionParentSpeed;
			
			/* // DEBUG
			TT_Printf("[%06u] DMC::updateAndPushParentMovement: [0x%08X] E:[0x%08X] P:[0x%08X] parentDeltaPos: (%f, %f), ownPos: (%f, %f), parentPos: (%f, %f)\n",
			          AppGlobal::getUpdateFrameCount(), m_ownHandle.getValue(),
			          p_entity.getHandle().getValue(), parent->getHandle().getValue(),
			          parentDeltaPos.x,            parentDeltaPos.y,
			          p_entity.getPosition().x,    p_entity.getPosition().y,
			          m_collisionParentPosition.x, m_collisionParentPosition.y);
			
			const movement::Direction downDir = (p_entity.getOrientationDown() != movement::Direction_None) ?
			                                     p_entity.getOrientationDown() :  movement::Direction_Down;
			
			const bool standingOnSolid = getTouchingCollisionDirections(p_entity).checkFlag(downDir);
			TT_Printf("DMC::updateAndPushParentMovement - SurveyResult_StandOnSolid: %d\n", standingOnSolid );
			//getTouchingCollisionDirections(p_entity);
			// */
		}
	}
	
	if (m_externalPush != tt::math::Vector2::zero)
	{
		if (m_dirtyLevel == DirtyLevel_UpdateSurvey ||
		    m_requestBitMask.checkFlag(RequestFlag_UpdateLocalCollision))
		{
			m_requestBitMask.resetFlag(RequestFlag_UpdateLocalCollision);
			updateLocalCollision(p_entity.getRegisteredTileRect(), p_entity);
		}
		
		p_entity.modifyPosition() += m_externalPush;
		
		// NOTE: The checkCollision call has side effects, so this if cannot be commented out completely
		if (checkCollision(p_entity, m_externalPush))
		{
			if (m_currentMove != 0 && m_currentMove->isAnimation())
			{
				// We collided, cancel animation.
				m_animationMoveCanceled = true;
			}
			/*
			const tt::math::VectorRect newRect   = p_entity.calcWorldRect();
			const tt::math::PointRect  newPtRect = p_entity.calcRegisteredTileRect();
			
			// Movement was changed.
			TT_Printf("[%06u] DMC::update: [0x%08X] ('%s') COLLISION!\n"
			          "\tpos - x: %f, y: %f\n"
			          "\tnewRect Min x: %f, y: %f - Max x: %f, y: %f\n"
			          "\tnewPtRect Min x: %d, y: %d - Max x: %d, y: %d - w: %d, h: %d\n",
			          AppGlobal::getUpdateFrameCount(),
			          p_entity.getHandle().getValue(), p_entity.getType().c_str(),
			          p_entity.getPosition().x, p_entity.getPosition().y,
			          newRect.getMin().x,         newRect.getMin().y,
			          newRect.getMaxInside().x,   newRect.getMaxInside().y,
			          newPtRect.getMin().x,       newPtRect.getMin().y,
			          newPtRect.getMaxInside().x, newPtRect.getMaxInside().y,
			          newPtRect.getWidth(),       newPtRect.getHeight());
			// */
			
			/* NOTE: In order to reduce non-tile aligned carry movement, entities with their own tiles
			//       should detach from their parent as soon as they collide. However, this currently
			//       breaks stacks with adjacent hermits.
			if (p_entity.hasCollisionTiles())
			{
				setCollisionParentEntity(EntityHandle());
			}
			// */
		}
		
		m_externalPush = tt::math::Vector2::zero;
	}
}


void DirectionalMovementController::updateChanges(real p_deltaTime, Entity& p_entity)
{
	if (p_entity.isPositionCulled())
	{
		return;
	}
	
	if (m_reevaluateCollisionParentScheduled)
	{
		m_reevaluateCollisionParentScheduled = false;
		p_entity.reevaluateCollisionParent(EntityHandle());
	}
	
	updateParentAndPushMovement(p_deltaTime, p_entity);
	
	// If we will detach from our parent right after this update,
	// snap to a tile position immediately (if the current move has snapping enabled)
	if (m_collisionParentEntity.isEmpty() == false &&
	    m_collisionParentEntityScheduled.isEmpty())
	{
		// Flush the scheduled removal of collision parent so we get the proper snapped tile pos.
		makeScheduledCollisionParentCurrent();
		
		scheduleSurveyUpdate(); // Parent is gone. Make sure to get the correct 'stand on solid' survey result.
		
		if (m_currentMove != 0)
		{
			// FIXME: Should snap to the *new* tile position, not in any random direction
			const tt::math::Vector2 snappedPos = p_entity.getSnappedToTilePos();
			
			// Only snap if not too far off a tile boundary to start with
			static const real maxDistanceForSnap = 0.2f;
			
			if (m_currentMove->getFlags().checkFlag(movement::MoveBase::Flag_StartNeedsXPositionSnap))
			{
				if (tt::math::fabs(snappedPos.x - p_entity.getPosition().x) <= maxDistanceForSnap)
				{
					p_entity.modifyPosition().x = snappedPos.x;
				}
			}
			if (m_currentMove->getFlags().checkFlag(movement::MoveBase::Flag_StartNeedsYPositionSnap))
			{
				if (tt::math::fabs(snappedPos.y - p_entity.getPosition().y) <= maxDistanceForSnap)
				{
					p_entity.modifyPosition().y = snappedPos.y;
				}
			}
		}
	}
	
	// Dirty flush
	{
		const DirtyLevel dirtyLevel = m_dirtyLevel;
		m_dirtyLevel = DirtyLevel_Nothing;
		switch (dirtyLevel)
		{
		case DirtyLevel_UpdateSurvey:
			{
				updateLocalCollision(p_entity.getRegisteredTileRect(), p_entity);
				p_entity.updateSurvey(true);
				
				const bool canOverrideCurrentMove = isCurrentMoveInterruptible();
				if (canOverrideCurrentMove &&  m_physicsMovementMode == PhysicsMovementMode_None)
				{
					// Select a new move based on current survey.
					// If move is the as current do nothing.
					// If move differs then start the new move.
					const movement::Direction moveDir = (m_requestBitMask.checkFlag(RequestFlag_PresistentMove) == false) ?m_direction : m_requestedDirection;
					
					TT_NULL_ASSERT(m_moveSet);
					movement::MoveBasePtr move = m_moveSet->getMove(moveDir, m_currentMove, p_entity);
					
					if (move != m_currentMove)
					{
						const tt::math::Vector2 prevSpeed(m_speed); // Remember speed so we can restore it.
						if (move != 0)
						{
							m_speed = applySpeedFactor(move->getSpeed()); // Change m_speed so determineCollisionParent of potential child doesn't reject us because we're not moving.
						}
						establishCollisionParentHierarchy(p_entity);
						scheduleReselectMove();
						m_speed = prevSpeed;
					}
					
					p_entity.scheduleReevaluateCollisionParent();
				}
			}
			break;
			
		case DirtyLevel_ReevalMove:
			if ((m_direction == m_requestedDirection || m_requestedDirection == movement::Direction_None) &&
			    m_requestBitMask.checkFlag(RequestFlag_ReselectMove) == false && 
			    m_physicsMovementMode == PhysicsMovementMode_None && // 'Physics' movement is always valid.
			    p_entity.isSuspended() == false &&
			    isCurrentMoveInterruptible()) 
			{
				if (m_currentMove == 0 ||
				    m_currentMove->validate(p_entity.getSurvey(),
				                            m_currentMove->getLocalDir(),
				                            p_entity.getOrientationDown(),
				                            p_entity.isOrientationForwardLeft()) == false)
				{
					scheduleReselectMove();
				}
			}
			break;
			
		case DirtyLevel_Nothing:
			break;
			
		default:
			TT_PANIC("Unknown dirty level: %d\n", dirtyLevel);
			break;
		}
	}
	
	if (( m_direction != m_requestedDirection && m_requestedDirection != movement::Direction_None) ||
	    m_requestBitMask.checkFlag(RequestFlag_ReselectMove)                                       ||
	    m_currentMove == 0)
	{
		const bool canOverrideCurrentMove = isCurrentMoveInterruptible();
		
		if (canOverrideCurrentMove &&
		    m_physicsMovementMode == PhysicsMovementMode_None)
		{
			//#error Need to know requested distance here.
			//       Could be stared in normal distance because once a new move is requested
			//       the old one should not be stopped because of distance end.
			m_requestBitMask.resetFlag(RequestFlag_ReselectMove);
			const real previousDistance = m_distance;
			startNewMovementImpl(m_requestedDirection, m_endDistance);
			m_distance = previousDistance;
		}
		else if (m_requestBitMask.checkFlag(RequestFlag_ReselectMove))
		{
			m_animationMoveCanceled = true;
		}
	}
	
	/* // DEBUG
	TT_Printf("[%06u] DMC::updateChanges: [0x%08X] E:[0x%08X] ownPos: (%f, %f)\n",
	          AppGlobal::getUpdateFrameCount(), m_ownHandle.getValue(),
	          p_entity.getHandle().getValue(), 
	          p_entity.getPosition().x,    p_entity.getPosition().y);
	// */
}


void DirectionalMovementController::update(real p_deltaTime, Entity& p_entity)
{
	if (p_entity.isPositionCulled())
	{
		return;
	}
	
	updateParentAndPushMovement(p_deltaTime, p_entity);
	
	// Check parent movement. (Will override all other movement.)
	if (getParentEntity().isEmpty() == false)
	{
		Entity* parent = getParentEntity().getPtr();
		if (parent == 0)
		{
			// Parent is gone!
			setParentEntity(EntityHandle()); // Remove parent and continue with normal movement.
		}
		else
		{
			if (m_useParentForward &&
			    p_entity.isOrientationForwardLeft() != parent->isOrientationForwardLeft())
			{
				p_entity.setOrientationForwardAsLeft(parent->isOrientationForwardLeft());
			}
			
			if (m_useParentDown &&
			    p_entity.getOrientationDown() != parent->getOrientationDown())
			{
				p_entity.setOrientationDown(parent->getOrientationDown());
			}
			
			// Set same position as parent.
			p_entity.modifyPosition() = parent->getPosition() + 
			                            parent->applyOrientationToVector2(m_targetEntityOffset);
			
			// FIXME: No need to copy this once we have two GameStates. (previous and current.)
			const tt::math::PointRect previousTiles = p_entity.getRegisteredTileRect();
			
			// Check if we're at a new Tile position.
			if (previousTiles != p_entity.calcRegisteredTileRect())
			{
				onNewTilePositionWithoutMoving(p_entity, previousTiles);
			}
			else
			{
				p_entity.updateRects();
			}
			
			return;
		}
	}
	
	if (p_entity.isSuspended())
	{
		const tt::math::PointRect previousTiles = p_entity.getRegisteredTileRect();
		
		// Check if we're at a new Tile position.
		if (previousTiles != p_entity.calcRegisteredTileRect())
		{
			// Make sure to update tile registration if we changed position.
			onNewTilePositionWithoutMoving(p_entity, previousTiles);
		}
		else
		{
			p_entity.updateRects();
		}
		return;
	}
	else if (m_physicsMovementMode != PhysicsMovementMode_None)
	{
		// FIXME: Remove the p_entity.getRegisteredTileRect() parameter, can do that within the function.
		//        We already pass p_entity, so why not?
		doPhysicsMovement(p_deltaTime, p_entity, p_entity.getRegisteredTileRect());
	}
	else if (m_currentMove != 0 && m_currentMove->isAnimation())
	{
		// Do animation stuff here. (Keep it in this if block so it can be moved in the future.)
		doAnimation(p_deltaTime, p_entity);
	}
	else
	{
		// FIXME: Same as above. Remove p_entity.getRegisteredTileRect() parameter.
		doNormalMovement(p_deltaTime, p_entity, p_entity.getRegisteredTileRect());
	}
	
	/* // DEBUG
	TT_Printf("[%06u] DMC::update: [0x%08X] E:[0x%08X] ownPos: (%f, %f)\n",
	          AppGlobal::getUpdateFrameCount(), m_ownHandle.getValue(),
	          p_entity.getHandle().getValue(), 
	          p_entity.getPosition().x,    p_entity.getPosition().y);
	// */
}


bool DirectionalMovementController::startNewMovement(movement::Direction p_direction,
                                                     real                p_endDistance)
{
	entity::Entity* entity = m_entityHandle.getPtr();
	TT_NULL_ASSERT(entity);
	
	/* DEBUG: Test the canBeMovedInDirection function.
	if (p_direction != movement::Direction_None)
	{
		const bool canBeMoved = canBeMovedInDirection(p_direction);
		TT_Printf("[%06u] DMC::startNewMovement: [0x%08X] Can be moved in dir %d ('%s'): %s\n",
		          AppGlobal::getUpdateFrameCount(), m_entityHandle.getValue(),
		          p_direction, movement::getDirectionName(p_direction), canBeMoved ? "yes" : "no");
	}
	// END DEBUG */
	
	TT_NULL_ASSERT(m_moveSet);
	movement::MoveBasePtr move = m_moveSet->getMove(p_direction, m_currentMove, *entity);
	
	m_requestBitMask.resetFlag(RequestFlag_PresistentMove);
	m_distance = 0.0f;
	m_endDistance = p_endDistance;
	
	// Was this a valid move?
	if (p_direction != movement::Direction_None &&
	        ( move == 0 ||
	          move->getDirections().checkFlag(movement::Direction_None) ||
	          move->getLocalDir() == entity::LocalDir_None) )
	{
		// No valid move found.
		doMoveFailedLogic(*entity, p_direction, m_currentMove);
		m_requestedDirection = (m_requestBitMask.checkFlag(RequestFlag_PresistentMove)) ? p_direction : movement::Direction_None;
		return false;
	}
	else
	{
		// We found a move for this direction.
		// Do the other checks.
		
		// Before checking if anything blocks this entity's move,
		// ensure that all collision parent relationships are set
		m_requestedDirection = p_direction;
		
		if (p_direction != movement::Direction_None) // FIXME: Should I remove the if?
		{
			establishCollisionParentHierarchy(*entity);
		}
		
		return true;
	}
}


void DirectionalMovementController::stopMovement()
{
	m_requestedDirection  = movement::Direction_None;
	m_direction           = movement::Direction_None;
	
	m_requestBitMask.resetFlag(RequestFlag_PresistentMove);
	
	if (m_physicsMovementMode == PhysicsMovementMode_None)
	{
		entity::Entity* entity = m_entityHandle.getPtr();
		TT_NULL_ASSERT(entity);
		if (m_currentMove == m_moveSet->getMove(m_direction, m_currentMove, *entity))
		{
			// Same move, nothing todo.
			return;
		}
	}
	
	// Martijn: stopMovement should really stop all movement. So also physics mode should be none.
	m_physicsMovementMode = PhysicsMovementMode_None;
	
	endMovement();
	startNewMovementImpl(m_requestedDirection, 0.0f);
}


bool DirectionalMovementController::canBeMovedInDirection(movement::Direction p_direction) const
{
	// FIXME: Do I need to check m_physicsMovementMode as well?
	
	typedef movement::TileCollisionHelper TCH;
	TCH::CollisionResult flagToCheck = TCH::CollisionResult_Count;
	switch (p_direction)
	{
	case movement::Direction_Left:  flagToCheck = TCH::CollisionResult_Left;   break;
	case movement::Direction_Right: flagToCheck = TCH::CollisionResult_Right;  break;
	case movement::Direction_Up:    flagToCheck = TCH::CollisionResult_Top;    break;
	case movement::Direction_Down:  flagToCheck = TCH::CollisionResult_Bottom; break;
		
	default:
		TT_PANIC("Cannot check whether entity can move in direction %d ('%s'): "
		         "not a supported direction.", p_direction, movement::getDirectionName(p_direction));
		return false;
	}
	
	// FIXME: Is this the right thing to check? Could local collision be out of date at this point?
	if (m_localCollision.checkFlag(flagToCheck) == false)
	{
		// No collision in this direction: can be moved there
		return true;
	}
	
	// Something is in the way in the specified direction.
	// Find all entities on the neighboring tiles in that direction and check if they can move.
	EntityMgr& entityMgr(AppGlobal::getGame()->getEntityMgr());
	
	const Entity* entity = entityMgr.getEntity(m_entityHandle);
	if (entity == 0)
	{
		return false;
	}
	
	const tt::math::Point2& regdMin(entity->getRegisteredTileRect().getMin());
	const tt::math::Point2  regdMax(entity->getRegisteredTileRect().getMaxInside());
	const s32               regdWidth  = entity->getRegisteredTileRect().getWidth();
	const s32               regdHeight = entity->getRegisteredTileRect().getHeight();
	
	tt::math::PointRect tilesToCheck;
	switch (p_direction)
	{
	case movement::Direction_Left:
		tilesToCheck = tt::math::PointRect(tt::math::Point2(regdMin.x - 1, regdMin.y), 1, regdHeight);
		break;
		
	case movement::Direction_Right:
		tilesToCheck = tt::math::PointRect(tt::math::Point2(regdMax.x + 1, regdMin.y), 1, regdHeight);
		break;
		
	case movement::Direction_Up:
		tilesToCheck = tt::math::PointRect(tt::math::Point2(regdMin.x, regdMax.y + 1), regdWidth, 1);
		break;
		
	case movement::Direction_Down:
		tilesToCheck = tt::math::PointRect(tt::math::Point2(regdMin.x, regdMin.y - 1), regdWidth, 1);
		break;
		
	default: break;  // NOTE: This error situation was already handled by the switch above
	}
	
	
	EntityHandleSet entities;
	AppGlobal::getGame()->getTileRegistrationMgr().findRegisteredEntityHandles(tilesToCheck, entities);
	
	if (entities.empty())
	{
		// There are no entities in the way (level collision only), so no way to move in this direction
		return false;
	}
	
	for (EntityHandleSet::iterator it = entities.begin(); it != entities.end(); ++it)
	{
		if (*it == m_entityHandle)
		{
			continue;
		}
		
		const Entity* otherEntity = entityMgr.getEntity(*it);
		if (otherEntity == 0)
		{
			continue;
		}
		
		// FIXME: This function is not correct! If an entity is in the way, only report
		// that we can move anyway if that entity would select this entity as parent
		// (only then will it automatically be moved out of the way)
		
		if (otherEntity->canBeMovedInDirection(p_direction) == false)
		{
			// At least one entity that is in the way cannot be moved:
			// as a result, this entity can't go there either
			return false;
		}
	}
	
	// All entities that are in the way can be moved
	return true;
}


void DirectionalMovementController::setMovementDirection(const tt::math::Vector2& p_direction)
{
	if (m_physicsMovementMode == PhysicsMovementMode_Direction)
	{
		m_directionControl = p_direction;
	}
	else
	{
		TT_PANIC("Can only set (physics) movement direction when in Direction mode.");
	}
}


void DirectionalMovementController::startMovementInDirection(const tt::math::Vector2& p_direction,
                                                             const PhysicsSettings&   p_settings)
{
	m_directionControl    = p_direction;
	m_physicsMovementMode = PhysicsMovementMode_Direction;
	m_settings            = p_settings;
	
	Entity* entity = getEntityHandle().getPtr();
	TT_NULL_ASSERT(entity);
	restartAllPresentationObjects(*entity);
}


void DirectionalMovementController::startPathMovement(const tt::math::Vector2& p_positionOrOffset,
                                                      const PhysicsSettings&   p_settings,
                                                      const EntityHandle&      p_optionalTarget)
{
	Entity* targetEntity = 0;
	Game* game = AppGlobal::getGame();
	
	m_targetEntity = p_optionalTarget;
	
	if (m_targetEntity.isEmpty())
	{
		startMovementToPosition(p_positionOrOffset, p_settings);
	}
	else
	{
		targetEntity = game->getEntityMgr().getEntity(m_targetEntity);
		startMovementToEntity(m_targetEntity, p_positionOrOffset, p_settings);
	}
	
	m_physicsMovementMode = PhysicsMovementMode_Path;
	
	Entity* ownEntity = getEntityHandle().getPtr();
	TT_NULL_ASSERT(ownEntity);
	
	pathfinding::TileCache* tileCache =
			game->getPathMgr().getTileCacheForAgentRadius(ownEntity->getPathAgentRadius());
	if (tileCache == 0)
	{
		TT_PANIC("Path finding is not enabled for entity '%s' (Might not have 'pathFindAgentRadius' class property).\n"
		         "No TileCache found for agent radius: %f!",
		         ownEntity->getType().c_str(), ownEntity->getPathAgentRadius());
		return;
	}
	
	if (targetEntity != 0)
	{
		m_targetPosition += targetEntity->applyOrientationToVector2(m_targetEntityOffset) +
		                       ownEntity->applyOrientationToVector2(m_settings.pathOffset);
	}
	
	if (m_pathAgentID < 0) // No (valid) agent yet.
	{
		// Create agent
		m_pathAgentID = tileCache->addAgent(ownEntity->getCenterPosition(), m_targetPosition, ownEntity->hasPathCrowdSeparation());
		TT_ASSERTMSG(m_pathAgentID >= 0, "Path agent creation failed!");
	}
	else
	{
		tileCache->updateTargetPosition(m_pathAgentID, m_targetPosition);
	}
}


void DirectionalMovementController::startMovementToPosition(const tt::math::Vector2& p_position,
                                                            const PhysicsSettings&   p_settings)
{
	Entity* entity = getEntityHandle().getPtr();
	TT_NULL_ASSERT(entity);
	
	m_targetPosition      = p_position + entity->applyOrientationToVector2(p_settings.pathOffset);
	m_directionControl    = (m_targetPosition - entity->getPosition());
	m_physicsMovementMode = PhysicsMovementMode_Point;
	m_settings            = p_settings;
	restartAllPresentationObjects(*entity);
}


void DirectionalMovementController::startMovementToEntity(const EntityHandle&      p_target,
                                                          const tt::math::Vector2& p_offset,
                                                          const PhysicsSettings&   p_settings)
{
	m_targetEntity       = p_target;
	// FIXME: Why an assert if later the statement "Allow targetEntity to be null." is made?
	//TT_ASSERT(m_targetEntity.isEmpty() == false);
	
	m_targetEntityOffset = p_offset;
	
	Entity* entity       = AppGlobal::getGame()->getEntityMgr().getEntity(getEntityHandle());
	Entity* targetEntity = AppGlobal::getGame()->getEntityMgr().getEntity(m_targetEntity);
	
	// Allow targetEntity to be null. (will trigger script callback in update.)
	startMovementToPosition( (targetEntity != 0) ? targetEntity->getPosition() : entity->getPosition(), p_settings );
	m_physicsMovementMode = PhysicsMovementMode_Entity;
}


bool DirectionalMovementController::changeDirection(movement::Direction p_direction)
{
	m_requestBitMask.resetFlag(RequestFlag_PresistentMove);
	
	if (m_direction != p_direction)
	{
		if (p_direction == movement::Direction_None)
		{
			// changing dir to None will only do the last move.
			// If we were to start a new move it might snap or look weird.
			// (This was very visible when controlling Toki and stopping.)
			m_requestedDirection = movement::Direction_None;
		}
		else
		{
			// Try to get a move in this direction.
			// If a valid move was found.
			//    set m_requestedDirection to p_direction.
			// return if a valid move was found.
			
			TT_NULL_ASSERT(m_moveSet);
			
			if (isCurrentMoveInterruptible() == false)
			{
				// We can move in this direction.
				m_requestedDirection = p_direction;
				
				m_endDistance = -1.0f; // FIXME: Remove this hardcoded distance needed to make toki walk a single tile.
				
				return true;
			}
			const entity::Entity* entity = m_entityHandle.getPtr();
			
			movement::MoveBasePtr move = m_moveSet->getMove(p_direction, m_currentMove, *entity);
			
			// Was this a valid move?
			if (p_direction != movement::Direction_None &&
			        ( move == 0 ||
			          move->getDirections().checkFlag(movement::Direction_None) ||
			          move->getLocalDir() == entity::LocalDir_None) )
			{
				// No valid move found.
				return false;
			}
			else
			{
				// We can move in this direction.
				m_requestedDirection = p_direction;
				
				m_endDistance = -1.0f; // FIXME: Remove this hardcoded distance needed to make toki walk a single tile.
				m_distance    = 0.0f;
				
				return true;
			}
		}
	}
	return true;
}


movement::Direction DirectionalMovementController::getActualMovementDirection() const
{
	// Try to use actual movement speed to determine the direction, other wise fallback to requested dir.
	
	// Up has highest priority, and down is above left or right.
	if (m_speed.y > 0.0f)
	{
		return movement::Direction_Up;
	}
	else if (m_speed.y < 0.0f)
	{
		return movement::Direction_Down;
	}
	else if (m_speed.x > 0.0f)
	{
		return movement::Direction_Right;
	}
	else if (m_speed.x < 0.0f)
	{
		return movement::Direction_Left;
	}
	
	return m_requestedDirection;
}


bool DirectionalMovementController::setCollisionParentEntity(const EntityHandle& p_handle)
{
	if (isCollisionParentEnabled() == false)
	{
#if !defined(TT_BUILD_FINAL)
		/*
		// Construct useful error message
		const entity::Entity* collisionParent = p_handle.getPtr();
		const std::string collisionParentType(collisionParent == 0 ? "<unknown>" : collisionParent->getType());
		const s32 collisionParentID(AppGlobal::getGame()->getEntityMgr().getEntityIDByHandle(p_handle));
		
		TT_PANIC("isCollisionParentEnabled() failed. Cannot set collision parent entity '%s' (ID %d).",
			collisionParentType.c_str(), collisionParentID);
		*/
		return false;
#endif
	}
	
	AppGlobal::getGame()->getEntityMgr().getMovementControllerMgr().setCollisionParentEntity(
			getHandle(), p_handle);
	return true;
}


void DirectionalMovementController::makeScheduledCollisionParentCurrent(bool p_unregisterFromCurrentParent)
{
	TT_ASSERT(isCollisionParentEnabled());
	
	// FIXME: These two sanity checks should not be necessary: should already have been taken care of by MovementControllerMgr
	if (m_collisionParentEntityScheduled == getEntityHandle())
	{
		TT_PANIC("Trying to set collision parent for entity to itself. This is not allowed.");
		return;
	}
	
	// Do nothing if the handle didn't change (prevent unnecessary work)
	if (m_collisionParentEntityScheduled == m_collisionParentEntity)
	{
		return;
	}
	
	EntityMgr& entityMgr(AppGlobal::getGame()->getEntityMgr());
	
	entity::Entity* ourEntity     = entityMgr.getEntity(getEntityHandle());
	entity::Entity* currentParent = entityMgr.getEntity(m_collisionParentEntity);
	entity::Entity* newParent     = entityMgr.getEntity(m_collisionParentEntityScheduled);
	
	//const tt::math::Vector2 currentParentSpeed(m_collisionParentSpeed);
	
	m_collisionParentEntity   = m_collisionParentEntityScheduled;
	m_collisionParentPosition = (newParent != 0) ? newParent->getPosition() : tt::math::Vector2::zero;
	m_collisionParentSpeed    = tt::math::Vector2::zero;
	
	/* FIXME: Do we really still need this snapping code, or do our new changes solve the same problem (whatever the problem was)?
	if (currentParent != 0 && ourEntity != 0)
	{
		// If previous parent was pushing or pulling this entity, and we now decouple from this parent,
		// ensure we end up on a snapped tile position in the direction we were moving
		const tt::math::Vector2 localSpaceSpeed = entity::removeOrientationFromVector2(
			m_collisionParentSpeed, ourEntity->getOrientationDown(), false);
		
		if (localSpaceSpeed.y != 0.0f)
		{
			switch (ourEntity->getOrientationDown())
			{
			case movement::Direction_Left:
			case movement::Direction_Right:
				TT_ASSERT(ourEntity->getPosition().x == ourEntity->getSnappedToTilePos().x);
				ourEntity->modifyPosition().x = ourEntity->getSnappedToTilePos().x;
				break;
				
			case movement::Direction_Up:
			case movement::Direction_Down:
			case movement::Direction_None:
				TT_ASSERT(ourEntity->getPosition().y == ourEntity->getSnappedToTilePos().y);
				ourEntity->modifyPosition().y = ourEntity->getSnappedToTilePos().y;
				break;
				
			default:
				TT_PANIC("Unsupported entity orientation for snapping: %d",
				         ourEntity->getOrientationDown());
				break;
			}
		}
	}
	// */
	
	// Unregister this controller's entity from the previous parent
	if (currentParent != 0 && p_unregisterFromCurrentParent)
	{
		DirectionalMovementController* currentParentCtrl = currentParent->getMovementControllerHandle().getPtr();
		TT_ASSERTMSG(currentParentCtrl != 0,
		             "A collision parent entity must have a MovementController "
		             "(current parent 0x%p doesn't).", currentParent);
		if (currentParentCtrl != 0)
		{
			currentParentCtrl->removeCollisionChild(getEntityHandle());
		}
	}
	
	//TT_Printf("DirectionalMovementController::setCollisionParentEntity: [0x%08X] New parent is 0x%08X (old: 0x%08X).\n",
	//          ourEntity, newParent, currentParent);
	
	// Early out if it turns out that our own Entity has disappeared
	// (the movement controller no longer makes any sense in that case)
	if (ourEntity == 0)
	{
		TT_PANIC("Cannot set new collision parent:\n"
		         "Entity 0x%08X associated with movement controller 0x%08X no longer exists.",
		         getEntityHandle().getValue(), getHandle().getValue());
		return;
	}
	
	// Register this controller's entity with the new parent
	if (newParent != 0)
	{
		DirectionalMovementController* newParentCtrl = newParent->getMovementControllerHandle().getPtr();
		TT_ASSERTMSG(newParentCtrl != 0,
		             "A collision parent entity must have a MovementController "
		             "(new parent 0x%p doesn't).", newParent);
		if (newParentCtrl != 0)
		{
			newParentCtrl->addCollisionChild(getEntityHandle());
		}
		
		// Make sure we're standing on our new parent.
		ourEntity->snapToStandOnSolid();
	}
	else
	{
		// No longer have a parent: this means that this entity and all children have this entity as ancestor
		overrideCollisionAncestorRecursive(m_entityHandle);
	}
	
	if (newParent == 0 && m_direction == movement::Direction_None)
	{
		if (ourEntity != 0 && ourEntity->hasCollisionTiles())
		{
			// Detached from a collision parent: all children need to re-evaluate their parent
			//TT_Printf("[%06u] DMC::makeScheduledCollisionParentCurrent: [0x%08X] Detached from parent. Making children re-evaluate parent.\n",
			//          AppGlobal::getUpdateFrameCount(), getHandle().getValue());
			const EntityHandleSet& children(getCollisionChildren());
			for (EntityHandleSet::const_iterator it = children.begin(); it != children.end(); ++it)
			{
				entity::Entity* child = entityMgr.getEntity(*it);
				if (child != 0 && child->isInitialized())
				{
					child->reevaluateCollisionParent(getEntityHandle());
				}
			}
		}
		
		scheduleReselectMove();
	}
	
	// Also notify script about being carried
	if (currentParent == 0 && newParent != 0)
	{
		ourEntity->onCarryBegin(newParent->getHandle());
	}
	else if (currentParent != 0 && newParent == 0)
	{
		ourEntity->onCarryEnd();
	}
}


void DirectionalMovementController::setCollisionAncestorForChildren()
{
	if (m_collisionParentEntityScheduled.isEmpty() == false)
	{
		// This controller is not an ancestor: do not modify own state
		return;
	}
	
	// Recursively set the new ancestor handle (also sets our handle as our ancestor, which is what we want)
	overrideCollisionAncestorRecursive(getEntityHandle());
}


void DirectionalMovementController::makeCollisionChildrenReevaluateParent(const EntityHandle& p_caller) const
{
	TT_ASSERT(isCollisionParentEnabled());
	const EntityHandleSet copyOfChildren(m_collisionChildren);
	makeCollisionChildrenReevaluateParent(copyOfChildren, p_caller);
}


void DirectionalMovementController::makeCollisionChildrenReevaluateParent(
		const EntityHandleSet& p_children,
		const EntityHandle&    p_caller)
{
	EntityMgr& entityMgr(AppGlobal::getGame()->getEntityMgr());
	for (EntityHandleSet::const_iterator it = p_children.begin(); it != p_children.end(); ++it)
	{
		Entity* child = entityMgr.getEntity(*it);
		if (child != 0 && child->canBeCarried())
		{
			child->reevaluateCollisionParent(p_caller);
		}
	}
}


void DirectionalMovementController::calculateSortWeight()
{
	m_sortWeight = 0;
	
	const Entity* ourEntity = getEntityHandle().getPtr();
	if (ourEntity != 0)
	{
		// Increase the sort weight by 1 for each parent of this entity's collision parent hierarchy
		// (so that this object is sorted after all its direct and indirect parents)
		m_sortWeight += ourEntity->getCollisionAncestryDepth(true);
		
		if (ourEntity->hasCollisionTiles() &&
		    ourEntity->getCollisionTiles()->isSolid())
		{
			// Entities with collision tiles need to be triggered sooner
			m_sortWeight -= 1;
		}
		
		// Check the non-collision parent
		if (getParentEntity().isEmpty() == false)
		{
			//TT_ASSERT(m_sortWeight == 0); // We don't expect entites with collision tiles to be moved by a (non-collision) parent.
			m_sortWeight += 1;            // Make sure our parent is update before us.
		}
	}
}


void DirectionalMovementController::handlePathMgrReset()
{
	m_pathAgentID = -1;
	
	if (m_physicsMovementMode != PhysicsMovementMode_Path)
	{
		// Was not using path finding: nothing to do
		return;
	}
	
	Entity* ownEntity = getEntityHandle().getPtr();
	TT_NULL_ASSERT(ownEntity);
	if (ownEntity == 0)
	{
		return;
	}
	
	if (ownEntity->getPathAgentRadius() < 0.0f)
	{
		TT_PANIC("Path finding is not enabled for entity '%s' (it has no 'pathFindAgentRadius' class property).",
		         ownEntity->getType().c_str());
		return;
	}
	
	// Create agent
	pathfinding::TileCache* tileCache =
			AppGlobal::getGame()->getPathMgr().getTileCacheForAgentRadius(ownEntity->getPathAgentRadius());
	if (tileCache != 0)
	{
		m_pathAgentID = tileCache->addAgent(ownEntity->getCenterPosition(), m_targetPosition, ownEntity->hasPathCrowdSeparation());
	}
}


DirectionalMovementController* DirectionalMovementController::getPointerFromHandle(
		const MovementControllerHandle& p_handle)
{
	if (AppGlobal::hasGame()                 == false ||
	    AppGlobal::getGame()->hasEntityMgr() == false)
	{
		return 0;
	}
	
	return AppGlobal::getGame()->getEntityMgr().getMovementControllerMgr().getDirectionalController(p_handle);
}


void DirectionalMovementController::invalidateTempCopy()
{
	// Ensure the destructor does not try to unregister with a parent
	m_collisionParentEntity.invalidate();
	m_collisionParentEntityScheduled.invalidate();
	m_collisionChildren.clear();
	
	m_pathAgentID = -1; // Invalid agent so it doesn't get remove from mgr it in dtor.
}


void DirectionalMovementController::serializeCreationParams  (tt::code::BufferWriteContext* p_context) const
{
	TT_NULL_ASSERT(p_context);
	namespace bu = tt::code::bufferutils;
	
	bu::putHandle(m_entityHandle, p_context);
	
	const bool hasMoveSet = (m_moveSet != 0);
	bu::put(hasMoveSet, p_context);
	if (hasMoveSet)
	{
		bu::put(m_moveSet->getFilename(), p_context);
	}
}


DirectionalMovementController::CreationParams DirectionalMovementController::unserializeCreationParams(tt::code::BufferReadContext*  p_context)
{
	TT_NULL_ASSERT(p_context);
	
	namespace bu = tt::code::bufferutils;
	
	const EntityHandle             entityHandle     = bu::getHandle<Entity>(p_context);
	const bool hasMoveSet = bu::get<bool>(p_context);
	
	movement::MovementSetPtr movementSet;
	if (hasMoveSet)
	{
		const std::string filename = bu::get<std::string>(p_context);
		movementSet = game::movement::MovementSet::create(filename);
		TT_ASSERTMSG(movementSet != 0,
		             "Failed to load movementset: '%s'. Needed for unserialization.\n",
		             filename.c_str());
	}
	
	return CreationParams(entityHandle, movementSet);
}


void DirectionalMovementController::serialize(tt::code::BufferWriteContext* p_context) const
{
	TT_NULL_ASSERT(p_context);
	
	namespace bu = tt::code::bufferutils;
	
	bu::put(m_speedFactor             , p_context); // tt::math::Vector2
	bu::put(m_speed                   , p_context); // tt::math::Vector2
	bu::put(m_externalForce           , p_context); // tt::math::Vector2
	bu::putHandle(m_referenceSpeedEntity, p_context); // entity::EntityHandle
	serialization::serializeMove(m_currentMove,  m_moveSet, p_context);
	serialization::serializeMove(m_previousMove, m_moveSet, p_context);
	movement::Transition::serialize(m_transition, p_context);
	bu::putBitMask (m_requestBitMask    , p_context); // RequestBitMask
	bu::putEnum<u8>(m_requestedDirection, p_context); // movement::Direction
	bu::putEnum<u8>(m_direction       , p_context); // movement::Direction
	bu::put(m_distance                , p_context); // real
	bu::put(m_endDistance             , p_context); // real
	bu::putEnum<u8>(m_dirtyLevel      , p_context); // DirtyLevel
	bu::put(m_currentTime             , p_context); // real
	bu::put(m_startPosition           , p_context); // tt::math::Vector2
	bu::put(m_animationMoveCanceled   , p_context); // bool
	bu::put(m_rightFoot               , p_context); // bool
	bu::putEnum<u8>(m_physicsMovementMode, p_context); // PhysicsMovementMode
	bu::put(m_directionControl        , p_context); // tt::math::Vector2
	bu::put(m_targetPosition          , p_context); // tt::math::Vector2
	bu::putHandle(m_targetEntity      , p_context); // entity::EntityHandle
	bu::put(m_targetEntityOffset      , p_context); // tt::math::Vector2
	m_settings.serialize(p_context); // PhysicsSettings
	bu::put(m_hasSolidCollision       , p_context); // bool
	bu::put(m_lastSolidNormal         , p_context); // tt::math::Vector2
	bu::putBitMask(m_localCollision,                 p_context); // movement::TileCollisionHelper::CollisionResultMask
	bu::put(m_localCollisionTileRect, p_context);
	bu::putHandle(m_parentEntity, p_context); // EntityHandle
	bu::put(m_useParentForward        , p_context); // bool
	bu::put(m_useParentDown           , p_context); // bool
	bu::put(m_externalPush            , p_context); // tt::math::Vector2
	bu::putHandle(m_collisionParentEntity, p_context); // EntityHandle
	bu::putHandle(m_collisionParentEntityScheduled, p_context); // EntityHandle
	bu::put(m_collisionParentPosition , p_context); // tt::math::Vector2
	bu::put(m_collisionParentSpeed    , p_context); // tt::math::Vector2
	
	bu::put(static_cast<u32>(m_collisionChildren.size()), p_context);
	
	for (EntityHandleSet::const_iterator it = m_collisionChildren.begin();
	     it != m_collisionChildren.end(); ++it)
	{
		bu::putHandle(*it, p_context);
	}
	
	bu::putHandle(m_collisionAncestor, p_context); // EntityHandle
	// No need to serialize m_sortWeight will be recalculated.
	bu::put(m_reevaluateCollisionParentScheduled, p_context);
}


void DirectionalMovementController::unserialize(tt::code::BufferReadContext*  p_context)
{
	TT_NULL_ASSERT(p_context);
	
	namespace bu = tt::code::bufferutils;
	
	m_speedFactor             = bu::get<tt::math::Vector2          >(p_context);
	m_speed                   = bu::get<tt::math::Vector2          >(p_context);
	m_externalForce           = bu::get<tt::math::Vector2          >(p_context);
	m_referenceSpeedEntity    = bu::getHandle<Entity               >(p_context);
	
	m_currentMove             = serialization::unserializeMove(m_moveSet, p_context);
	m_previousMove            = serialization::unserializeMove(m_moveSet, p_context);
	m_transition              = movement::Transition::unserialize(p_context);
	m_requestBitMask          = bu::getBitMask<RequestBitMask::Flag,
	                                           RequestBitMask::FlagCount>(p_context);
	m_requestedDirection      = bu::getEnum<u8, movement::Direction>(p_context);
	m_direction               = bu::getEnum<u8, movement::Direction>(p_context);
	m_distance                = bu::get<real                       >(p_context);
	m_endDistance             = bu::get<real                       >(p_context);
	m_dirtyLevel              = bu::getEnum<u8, DirtyLevel         >(p_context);
	m_currentTime             = bu::get<real                       >(p_context);
	m_startPosition           = bu::get<tt::math::Vector2          >(p_context);
	m_animationMoveCanceled   = bu::get<bool                       >(p_context);
	m_rightFoot               = bu::get<bool                       >(p_context);
	m_physicsMovementMode     = bu::getEnum<u8, PhysicsMovementMode>(p_context);
	m_directionControl        = bu::get<tt::math::Vector2          >(p_context);
	m_targetPosition          = bu::get<tt::math::Vector2          >(p_context);
	m_targetEntity            = bu::getHandle<Entity               >(p_context);
	m_targetEntityOffset      = bu::get<tt::math::Vector2          >(p_context);
	m_settings                = PhysicsSettings::unserialize(p_context);
	m_hasSolidCollision       = bu::get<bool                       >(p_context);
	m_lastSolidNormal         = bu::get<tt::math::Vector2          >(p_context);
	m_localCollision          = bu::getBitMask<movement::TileCollisionHelper::CollisionResultMask::Flag,
	                                           movement::TileCollisionHelper::CollisionResultMask::FlagCount>(p_context);
	m_localCollisionTileRect  = bu::get<tt::math::PointRect        >(p_context);
	m_parentEntity            = bu::getHandle<Entity               >(p_context);
	m_useParentForward        = bu::get<bool                       >(p_context);
	m_useParentDown           = bu::get<bool                       >(p_context);
	m_externalPush            = bu::get<tt::math::Vector2          >(p_context);
	m_collisionParentEntity   = bu::getHandle<Entity>(p_context);
	m_collisionParentEntityScheduled = bu::getHandle<Entity>(p_context);
	m_collisionParentPosition = bu::get<tt::math::Vector2>(p_context);
	m_collisionParentSpeed    = bu::get<tt::math::Vector2>(p_context);
	
	m_collisionChildren.clear();
	const u32 collisionChildCount = bu::get<u32>(p_context);
	for (u32 i = 0; i < collisionChildCount; ++i)
	{
		m_collisionChildren.insert(bu::getHandle<Entity>(p_context));
	}
	
	m_collisionAncestor = bu::getHandle<Entity>(p_context);
	// No need to unserialize m_sortWeight will be recalculated.
	m_reevaluateCollisionParentScheduled = bu::get<bool>(p_context);
}


bool DirectionalMovementController::isCurrentMoveInterruptible() const
{
	// Can always interrupt move if there is no move
	if (m_currentMove == 0)
	{
		return true;
	}
	
	// Animation moves cannot be interrupted
	return m_currentMove->isAnimation() == false;
}


//--------------------------------------------------------------------------------------------------
// Private member functions

bool DirectionalMovementController::startNewMovementImpl(movement::Direction p_direction,
                                                         real                p_endDistance)
{
	m_distance            = 0.0f;
	m_endDistance         = p_endDistance;
	m_physicsMovementMode = PhysicsMovementMode_None;
	
	m_direction          = p_direction;
	
	Entity* entity = getEntityHandle().getPtr();
	TT_NULL_ASSERT(entity);
	if (entity->isSuspended())
	{
		return true;
	}
	
	bool moveSucceeded = true;
	const tt::math::PointRect rect = onNewTilePosition(
			*entity,
			entity->getRegisteredTileRect(),
			moveSucceeded);
	
	entity->updateRects(&rect); // Need forced update of registeredTiles.
	
	return moveSucceeded;
}


void DirectionalMovementController::endMovement()
{
	// We are in physics mode so switch to idle physics mode when ending a movement
	if (m_physicsMovementMode != PhysicsMovementMode_None)
	{
		m_physicsMovementMode = PhysicsMovementMode_Idle;
		m_directionControl = tt::math::Vector2::zero;
	}
	
	if (m_requestBitMask.checkFlag(RequestFlag_PresistentMove) == false)
	{
		m_requestedDirection  = movement::Direction_None;
	}
	m_direction = movement::Direction_None;
	
	// FIXME: Martijn change; only reset speedfactor when not in physicsmovement mode
	if (m_physicsMovementMode == PhysicsMovementMode_None)
	{
		m_speed.setValues(0.0f, 0.0f);
	}
	
	Entity* entity = getEntityHandle().getPtr();
	
	if (m_pathAgentID >= 0)
	{
		pathfinding::TileCache* tileCache =
				AppGlobal::getGame()->getPathMgr().getTileCacheForAgentRadius(entity->getPathAgentRadius());
		if (tileCache != 0)
		{
			tileCache->removeAgent(m_pathAgentID);
		}
		m_pathAgentID = -1;
	}
	
	const tt::math::Vector2 snappedPos = entity->getSnappedToTilePos();
	
	if (m_currentMove != 0)
	{
		if (m_currentMove->getFlags().checkFlag(movement::MoveBase::Flag_EndNeedsXPositionSnap))
		{
			entity->modifyPosition().x = snappedPos.x;
		}
		if (m_currentMove->getFlags().checkFlag(movement::MoveBase::Flag_EndNeedsYPositionSnap))
		{
			entity->modifyPosition().y = snappedPos.y;
		}
		// Reset current move to a move in direction none.
		m_currentMove = m_moveSet->getMove(movement::Direction_None, m_currentMove, *entity);
		
		if (m_currentMove != 0)
		{
			doMoveStartLogic(*entity);
		}
	}
}


void DirectionalMovementController::doAnimation(real p_deltaTime, Entity& p_entity)
{
	TT_ASSERT(p_entity.isInitialized());
	
	const tt::math::PointRect previousTiles(p_entity.getRegisteredTileRect());
	
	// Check for the end of this move. (Do checks first before doing the update.)
	if (m_animationMoveCanceled)
	{
		endAnimationMove(p_entity, previousTiles);
		return;
	}
	
	// Don't start animation when we're in a transition for which we don't want to use the 'to speed'.
	if (m_transition != 0 && m_transition->getSpeed() != movement::TransitionSpeed_UseTo)
	{
		return;
	}
	
	TT_ASSERT(m_currentMove != 0 && m_currentMove->isAnimation());
	const movement::MoveAnimation* animation = m_currentMove->getMoveAnimation();
	TT_NULL_ASSERT(animation);
	
	const real prevTime = m_currentTime;
	m_currentTime += p_deltaTime;
	
	const tt::math::Vector2 previousPosition(p_entity.getPosition());
	const real timeLength = animation->getTimeLength();
	
	TT_ASSERT(m_currentTime >= 0.0f);
	if (m_currentTime >= timeLength)
	{
		p_entity.modifyPosition() = m_startPosition + level::tileToWorld(animation->getStep());
		// FIXME: Collision check!
		endAnimationMove(p_entity, previousTiles);
		return;
	}
	
	typedef movement::MoveBase::SubSteps SubSteps;
	const SubSteps& subSteps = animation->getSubSteps();
	
	const s32 stepCount = static_cast<s32>(subSteps.size());
	if (stepCount <= 0)
	{
		// Nothing todo but wait for time.
		return;
	}
	
	TT_ASSERT(timeLength > 0.0f); // Sanity check. Other code should make sure this never happens.
	const real normalized     = m_currentTime / timeLength;
	TT_ASSERT(normalized >= -0.0001f && normalized <= 1.0001f);
	const real currentStep = stepCount * normalized;
	
	const real prevStep = stepCount * (prevTime / timeLength);
	
	tt::math::Vector2 position(m_startPosition);
	
	real stepsTodo   = currentStep;
	real stepsToPrev = prevStep;
	tt::math::Vector2 lastStep;
	for (SubSteps::const_iterator it = subSteps.begin();
	     it != subSteps.end() && stepsTodo > 0.0f; ++it)
	{
		if (stepsTodo > 1.0f)
		{
			lastStep     = level::tileToWorld(*it);
			position    += lastStep; // Move subStep
			stepsTodo   -= 1.0f; // One less step to do.
			stepsToPrev -= 1.0f;
			if (stepsToPrev <= 0.0f) // We need to check collision for ever subStep
			{
				stepsToPrev = 0.0f;
				p_entity.modifyPosition() = position;
				if (checkCollision(p_entity, lastStep))
				{
					endAnimationMove(p_entity, previousTiles);
					return;
				}
				updateLocalCollision(p_entity.calcRegisteredTileRect(), p_entity);
			}
		}
		else
		{
			TT_ASSERT(stepsTodo >= 0.0f);
			lastStep  = (level::tileToWorld(*it) * stepsTodo);
			position += lastStep;
			stepsTodo = 0.0f;
		}
	}
	TT_ASSERT(stepsTodo == 0.0f);
	
	p_entity.modifyPosition() = position;
	
	const tt::math::Vector2 fullMove(p_entity.getPosition() - previousPosition);
	
	switch (m_direction)
	{
	case movement::Direction_Up:    m_distance += fullMove.y; break;
	case movement::Direction_Down:  m_distance -= fullMove.y; break;
	case movement::Direction_Left:  m_distance -= fullMove.x; break;
	case movement::Direction_Right: m_distance += fullMove.x; break;
	default: break;
	}
	
	m_speed = lastStep; // Set speed for getActualMovementDirection
	
	if (checkCollision(p_entity, lastStep))
	{
		endAnimationMove(p_entity, previousTiles);
		return;
	}
	
	const tt::math::PointRect newTiles = calculateNewTileRect(p_entity, lastStep, false);
	p_entity.updateRects(&newTiles);
	
	if (previousTiles != p_entity.getRegisteredTileRect()) // Tile step within the animation.
	{
		level::TileRegistrationMgr& mgr(AppGlobal::getGame()->getTileRegistrationMgr());
		mgr.moveRegisterEntityHandle(previousTiles, p_entity.getRegisteredTileRect(), p_entity.getHandle());
		updateLocalCollision(p_entity.getRegisteredTileRect(), p_entity);
		p_entity.updateSurvey(true);
	}
}


void DirectionalMovementController::endAnimationMove(Entity& p_entity, const tt::math::PointRect& p_previousTiles)
{
	m_animationMoveCanceled = false;
	
	// FIXME: This is a whole new MoveStep, not (just) a new tile position.
	bool moveSucceeded = true;
	const tt::math::PointRect rect = onNewTilePosition(p_entity, p_previousTiles, moveSucceeded);
	p_entity.updateRects(&rect);
}


void DirectionalMovementController::doPhysicsMovement(real p_deltaTime, entity::Entity& p_entity,
                                                      const tt::math::PointRect& p_previousTiles)
{
	TT_ASSERT(m_physicsMovementMode != PhysicsMovementMode_None);
	TT_ASSERT(p_entity.isInitialized());
	TT_ASSERT(p_entity.isSuspended() == false);
	
	//==================================================================
	// "Physicsed" movement:
	
	bool moveEnded  = false;
	bool moveFailed = false;
	
	switch (m_physicsMovementMode)
	{
	case PhysicsMovementMode_Path:
		{
			pathfinding::PathMgr& pathMgr = AppGlobal::getGame()->getPathMgr();
			
			if (m_pathAgentID < 0)
			{
				// No path agent was created.
				p_entity.onPathMovementFailed(p_entity.getPosition());
				endMovement();
				return;
			}
			
			pathfinding::TileCache* tileCache = pathMgr.getTileCacheForAgentRadius(p_entity.getPathAgentRadius());
			// FIXME: What to do with non-existent tile cache in this case?
			if (tileCache == 0)
			{
				// For some reason the tile cache for this agent radius ceased to exist
				p_entity.onPathMovementFailed(p_entity.getPosition());
				endMovement();
				return;
			}
			
			const pathfinding::AgentState state = tileCache->getAgentState(m_pathAgentID);
			
			TT_ASSERT(state != pathfinding::AgentState_None);
			TT_ASSERT(state != pathfinding::AgentState_Invalid);
			if (state == pathfinding::AgentState_PathFailed)
			{
				const tt::math::Vector2 closestPoint = tileCache->getAgentEndPoint(m_pathAgentID);
				// Path failure.
				p_entity.onPathMovementFailed(closestPoint);
				endMovement();
				return;
			}
			
			if (m_targetEntity.isEmpty() == false)
			{
				entity::Entity* targetEntity = m_targetEntity.getPtr();
				if (targetEntity == 0)
				{
					moveFailed = true;
					break; // Keep old direction for this frame.
				}
				else if (state != pathfinding::AgentState_Busy) // If we're not still waiting for the result of a previous path result.
				{
					const tt::math::Vector2 newTargetPos(targetEntity->getPosition() +
					                                     targetEntity->applyOrientationToVector2(m_targetEntityOffset) +
					                                     p_entity.applyOrientationToVector2(m_settings.pathOffset));
					const tt::math::Vector2 targetDiff(m_targetPosition - newTargetPos);
					if (targetDiff.lengthSquared() > 0.1f)
					{
						m_targetPosition = newTargetPos;
						tileCache->updateTargetPosition(m_pathAgentID, m_targetPosition);
					}
				}
			}
			
			m_directionControl = (m_targetPosition - p_entity.getCenterPosition());
			const real length = m_directionControl.length();
			
			if (state != pathfinding::AgentState_Busy)
			{
				m_directionControl = tileCache->getAgentVelocity(m_pathAgentID);
				// Make the direction (gotten from pathfinding) the length of the distance to our target.
				// Needed for proper easeOutDistance and moveEndDistance code below.
				m_directionControl.normalize() *= length;
			}
		}
		break;
	case PhysicsMovementMode_Entity:
		{
			TT_ASSERT(m_targetEntity.isEmpty() == false);
			entity::Entity* targetEntity = m_targetEntity.getPtr();
			if (targetEntity == 0)
			{
				moveFailed = true;
				break; // Keep old direction for this frame.
			}
			m_targetPosition = targetEntity->getPosition() +
			                   targetEntity->applyOrientationToVector2(m_targetEntityOffset) +
			                   p_entity.applyOrientationToVector2(m_settings.pathOffset);
		}
		// No break;
		
	case PhysicsMovementMode_Point:
		{
			m_directionControl = (m_targetPosition - p_entity.getPosition());
		}
		// No break;
		
	case PhysicsMovementMode_Direction:
		// Nothing to do.
		break;
		
	case PhysicsMovementMode_Idle:
		// Nothing to do.
		break;
		
	default:
		TT_PANIC("Unknown physics movement mode: %d", m_physicsMovementMode);
		break;
	}
	
	// Don't override direction when in direciton mode.
	if (m_physicsMovementMode != PhysicsMovementMode_Direction &&
		m_physicsMovementMode != PhysicsMovementMode_Idle)
	{
		real length = m_directionControl.length();
		
		if (length > 0.0f)
		{
			const real max = m_settings.easeOutDistance;
			if (length > max) 
			{
				// Scale to (0.0 - max).
				const real mul = max / length;
				m_directionControl.x *= mul;
				m_directionControl.y *= mul;
			}
			
			// Scale to (0.0 - 1.0) range.
			const real invMax = 1.0f / max;
			m_directionControl.x *= invMax;
			m_directionControl.y *= invMax;
		}
		if (length < m_settings.moveEndDistance)
		{
			moveEnded = true;
		}
	}
	
	if (m_settings.shouldTurn)
	{
		static const real turnDistance = 0.1f;
		const bool shouldTurn = (p_entity.isOrientationForwardLeft()          && m_directionControl.x >  turnDistance) ||
		                        (p_entity.isOrientationForwardLeft() == false && m_directionControl.x < -turnDistance);
		
		if (shouldTurn)
		{
			p_entity.setOrientationForwardAsLeft(m_directionControl.x < 0.0f);
			if (m_settings.turnAnimationName.empty() == false)
			{
				p_entity.startAllPresentationObjectsForMovement(m_settings.turnAnimationName, tt::pres::Tags(),
				                                                pres::StartType_TransitionMove );
			}
			p_entity.onPhysicsTurn();
		}
	}
	
	State state;
	state.pos = p_entity.getPosition();
	state.vel = m_speed;
	
	tt::math::Vector2 thrust(m_directionControl * m_settings.thrust);
	
	integrate(state, p_deltaTime, m_settings, thrust + m_settings.extraForce + m_externalForce,
		m_hasSolidCollision ? m_settings.collisionDrag : m_settings.drag);
	
	m_speed = state.vel;
	
	/*
	TT_Printf("DirectionalMovementController::update - dir x: %f, y: %f\n"
	          "DirectionalMovementController::update - m_speed x: %f, y: %f\n",
	          m_directionControl.x, m_directionControl.y,
	          m_speed.x, m_speed.y);
	// */
	const tt::math::Vector2 oldPos(p_entity.getPosition());
	if (moveEnded)
	{
		// With moveEndDistance we can be far away from target, so no longer snap to target pos.
		//entity->modifyPosition() = controller->m_targetPosition;
	}
	else
	{
		const Entity* referenceEntity(m_referenceSpeedEntity.getPtr());
		if (referenceEntity != 0)
		{
			state.pos += referenceEntity->getSpeed() * p_deltaTime;
		}
		
		p_entity.modifyPosition() = state.pos;
	}
	
	m_settings.clampEntity(&p_entity, oldPos, &m_speed);
	
	tt::math::PointRect newRegisteredTiles = p_entity.calcRegisteredTileRect();
	
	m_hasSolidCollision = false;
	
	if (m_settings.hasCollisionWithSolid &&    // Should we check for collision?
	    p_previousTiles != newRegisteredTiles) // Check for tile move
	{
		// Could we have moved into collision?
		const tt::math::Vector2 deltaPos = p_entity.getPosition() - oldPos;
		
		if (checkCollision(p_entity, deltaPos))
		{
			// Movement was changed.
			/*
			TT_Printf("DirectionMovementController::update - after collision position x: %f, y: %f\n",
			          p_entity.getPosition().x, p_entity.getPosition().y);
			// */
			
			// Below a certain threshold the entity will be forced to stay still
			const real clippingThreshold = 0.05f;
			const tt::math::Vector2 clippedSpeed(fabs(deltaPos.x) > clippingThreshold ? deltaPos.x : 0.0f,
			                                     fabs(deltaPos.y) > clippingThreshold ? deltaPos.y : 0.0f);
			
			const movement::Directions directions(getTouchingCollisionDirections(p_entity));
			tt::math::Vector2 normal(tt::math::Vector2::zero);
			if      (directions.checkFlag(movement::Direction_Left))  normal.x =  1.0f;
			else if (directions.checkFlag(movement::Direction_Right)) normal.x = -1.0f;
			if      (directions.checkFlag(movement::Direction_Up))    normal.y = -1.0f;
			else if (directions.checkFlag(movement::Direction_Down))  normal.y =  1.0f;
			
			if (clippedSpeed.x == 0.0f) normal.x = 0.0f;
			if (clippedSpeed.y == 0.0f) normal.y = 0.0f;
			
			normal.normalize();
			
			// Handle bouncing and script callbacks only if movement was significant and in correct direction
			// (otherwise we keep getting those calls when trying to bounce into a wall/floor)
			if (clippedSpeed.x * normal.x < 0.0f || clippedSpeed.y * normal.y < 0.0f)
			{
				if (directions.checkFlag(movement::Direction_Left) || directions.checkFlag(movement::Direction_Right))
				{
					m_speed.x = -m_speed.x * m_settings.bouncyness;
				}
				
				if (directions.checkFlag(movement::Direction_Up) || directions.checkFlag(movement::Direction_Down))
				{
					m_speed.y = -m_speed.y * m_settings.bouncyness;
				}
			}
			else
			{
				// Update speed based on the possible collision;
				if (p_entity.getPosition().x == oldPos.x)
				{
					m_speed.x = 0.0f;
				}
				if (p_entity.getPosition().y == oldPos.y)
				{
					m_speed.y = 0.0f;
				}
				m_hasSolidCollision = true;
			}
			
			// If wall normal is different than the one before, signal solid collision to script
			if (normal.lengthSquared() > 0.0f && normal != m_lastSolidNormal)
			{
				// Communicate impact to script
				p_entity.onSolidCollision(normal, clippedSpeed);
				
				if (m_settings.isCollisionMovementFailure)
				{
					moveFailed = true;
				}
				m_lastSolidNormal = normal;
			}
			
			// Update registeredTiles based on new (snapped) position.
			newRegisteredTiles = p_entity.calcRegisteredTileRect();
		}
		else
		{
			m_lastSolidNormal = tt::math::Vector2::zero;
		}
		/*
		else
		{
			TT_Printf("DirectionMovementController::update - no position change x: %f, y: %f\n",
			          p_entity.getPosition().x, p_entity.getPosition().y);
		}
		*/
	}
	
	// Check if we're at a new Tile position.
	if (p_previousTiles != newRegisteredTiles)
	{
		/*
		const tt::math::PointRect regTiles = p_entity.calcRegisteredTileRect();
		TT_Printf("new Tile Pos prev %d, %d, %d, %d new %d, %d, %d, %d\n",
			p_previousTiles.getPos().x, p_previousTiles.getPos().y, p_previousTiles.getWidth(), p_previousTiles.getHeight(),
			regTiles.getPos().x,        regTiles.getPos().y,        regTiles.getWidth(),        regTiles.getHeight());
		// */
		
		bool moveSucceeded = true;
		const tt::math::PointRect rect = onNewTilePosition(p_entity,
		                                                   p_previousTiles,
		                                                   moveSucceeded);
		p_entity.updateRects(&rect);
	}
	else
	{
		p_entity.updateRects();
	}
	
	if (moveEnded)
	{
		p_entity.onMovementEnded(m_direction);
		endMovement();
	}
	else if (moveFailed)
	{
		p_entity.onMovementFailed(m_direction, "physics");
		endMovement();
	}
	else
	{
		// Move didn't end or fail.
		if (m_physicsMovementMode == PhysicsMovementMode_Path)
		{
			pathfinding::TileCache* tileCache =
					AppGlobal::getGame()->getPathMgr().getTileCacheForAgentRadius(p_entity.getPathAgentRadius());
			if (tileCache != 0)
			{
				tileCache->updateAgentPosition(m_pathAgentID, p_entity.getCenterPosition());
			}
		}
	}
}


void DirectionalMovementController::doNormalMovement(real p_deltaTime, entity::Entity& p_entity,
                                                     const tt::math::PointRect& p_previousTiles)
{
	TT_ASSERT(p_entity.isInitialized());
	TT_ASSERT(p_entity.isSuspended() == false);
	
	//==================================================================
	// Normal, Move-based movement:
	
	tt::math::Vector2 deltaPos(m_speed * p_deltaTime);
	
	if (p_entity.isSuspended()) // No own movement for suspended entities.
	{
		deltaPos.setValues(0.0f, 0.0f);
	}
	
	const tt::math::Vector2 oldPos(p_entity.getPosition());
	p_entity.modifyPosition() += deltaPos;
	
	pushOtherEntities(p_entity);
	
	if (tt::math::realEqual(deltaPos.x, 0.0f) == false)
	{
		// Remember the last horizontal direction. (Only done when moving horizontally.)
		p_entity.setFlowToTheRight(deltaPos.x > 0.0f);
	}
	
	if (p_entity.isSuspended() == false)
	{
		switch (m_direction)
		{
		case movement::Direction_Up:    m_distance += deltaPos.y; break;
		case movement::Direction_Down:  m_distance -= deltaPos.y; break;
		case movement::Direction_Left:  m_distance -= deltaPos.x; break;
		case movement::Direction_Right: m_distance += deltaPos.x; break;
		default: break;
		}
		
		// Check for movement end.
		if (m_endDistance        >= 0.0f                     &&
		    m_distance           >= m_endDistance            &&
		    m_direction          != movement::Direction_None &&
		    m_requestedDirection != movement::Direction_None ) // FIXME: Try to remove these direction checks. (They are needed so a startMovement none (with distance 0) continues it's current move.))
		{
			// Distance > endDistance. Stop moving.
			p_entity.onMovementEnded(m_direction);
			stopMovement();
		}
	}
	
	const tt::math::Vector2 nonSnappedPos(p_entity.getPosition());
	tt::math::Vector2 min           = oldPos;
	tt::math::Vector2 max           = p_entity.getPosition();
	const tt::math::Vector2 snapPos = p_entity.getSnappedToTilePos();
	bool forceSelectNewMove = false;
	
	if (min.x > max.x)
	{
		std::swap(min.x, max.x);
	}
	if (min.y > max.y)
	{
		std::swap(min.y, max.y);
	}
	bool movedPassedSnappedPos = false;
	
	if (min.x != max.x && snapPos.x >= min.x && snapPos.x <= max.x)
	{
		p_entity.modifyPosition().x = snapPos.x;
		movedPassedSnappedPos = true;
	}
	if (min.y != max.y && snapPos.y >= min.y && snapPos.y <= max.y)
	{
		p_entity.modifyPosition().y = snapPos.y;
		movedPassedSnappedPos = true;
	}
	
	// Check if we're at a new Tile position.
	if (movedPassedSnappedPos)
	{
		// Check collision
		const bool ignoreCollision = (m_currentMove != 0) ? 
			m_currentMove->getFlags().checkFlag(movement::MoveBase::Flag_IgnoreCollision) : false;
		
		// Check for tile collision
		if (m_settings.hasCollisionWithSolid && ignoreCollision == false)
		{
			// NOTE: The checkCollision call has side effects, so this if cannot be commented out completely
			checkCollision(p_entity, m_speed);
		}
		
		// Update survey for new pos.
		updateLocalCollision(p_entity.calcRegisteredTileRect(), p_entity);
		p_entity.updateSurvey(false);
		
		TT_NULL_ASSERT(m_moveSet);
		movement::MoveBasePtr move = m_moveSet->getMove(m_direction, m_currentMove, p_entity);
		
		if (move != m_currentMove)
		{
			forceSelectNewMove = true;
		}
		else
		{
			// Continue with same move. (Still do new tile checks below)
			p_entity.modifyPosition() = nonSnappedPos;
		}
	}
	
	if (forceSelectNewMove ||
	    p_previousTiles != p_entity.calcRegisteredTileRect())
	{
		bool moveSucceeded = true;
		const tt::math::PointRect rect(onNewTilePosition(p_entity, p_previousTiles, moveSucceeded));
		
		p_entity.updateRects(&rect);
	}
	else
	{
		p_entity.updateRects();
	}
}


void DirectionalMovementController::pushOtherEntities(const Entity& p_entity)
{
	// All entities with solid collision tiles push other entities.
	const EntityTiles* tiles = p_entity.getCollisionTiles();
	if (tiles == 0 || tiles->isSolid() == false)
	{
		return;
	}
	
	game::entity::EntityHandleSet entities;
	AppGlobal::getGame()->getTileRegistrationMgr().findRegisteredEntityHandles(
			p_entity.getRegisteredTileRect(), entities);
	
	// our rectangle is the size of our tile rectangle only smaller.
	const real smallDistance = 0.001f;
	const tt::math::Vector2 smallDistanceVec(smallDistance,smallDistance);
	const real twoSmallDistance = smallDistance + smallDistance;
	const tt::math::VectorRect ourRect(p_entity.getPosition() + p_entity.getLocalTileRect().getPosition() + smallDistanceVec,
	                                   level::tileToWorld(tiles->getWidth())  - twoSmallDistance,
	                                   level::tileToWorld(tiles->getHeight()) - twoSmallDistance);
	// Maybe replace with: const tt::math::VectorRect ourRect(p_entity.calcEntityTileRect());
	
	EntityMgr&             entityMgr(AppGlobal::getGame()->getEntityMgr());
	MovementControllerMgr& ctrlMgr  (entityMgr.getMovementControllerMgr());
	
	for (game::entity::EntityHandleSet::iterator it = entities.begin(); it != entities.end(); ++it)
	{
		if (*it == getEntityHandle())
		{
			continue;
		}
		
		Entity* otherEntity = entityMgr.getEntity(*it);
		if (otherEntity != 0 &&
		    otherEntity->canBePushed() &&
		    otherEntity->isInCollisionAncestry(getEntityHandle(), true) == false) // should not push if being carried
		{
			DirectionalMovementController* otherCtrl = ctrlMgr.getDirectionalController(
					otherEntity->getMovementControllerHandle());
			TT_ASSERTMSG(otherCtrl != 0,
			             "'%s' (ID %d) is trying to push other entity '%s' (ID %d) without a movement controller. " 
			             "This is not possible.",
			             p_entity.getType().c_str(), entityMgr.getEntityIDByHandle(p_entity.getHandle()),
			             otherEntity->getType().c_str(), entityMgr.getEntityIDByHandle(otherEntity->getHandle()));
			
			if (otherCtrl != 0 && otherCtrl->getParentEntity().isEmpty())
			{
				const tt::math::VectorRect& otherRect(otherEntity->getWorldRect());
				tt::math::VectorRect intersectRect;
				if (ourRect.intersects(otherRect, &intersectRect))
				{
					/*
					TT_Printf("our rect (%f, %f) (%f, %f) - other rect: (%f, %f) (%f, %f) - pre-push (%f, %f)\n",
						ourRect.getMin().x,   ourRect.getMin().y,   ourRect.getMaxEdge().x,   ourRect.getMaxEdge().y,
						otherRect.getMin().x, otherRect.getMin().y, otherRect.getMaxEdge().x, otherRect.getMaxEdge().y,
						otherCtrl->m_externalPush.x, otherCtrl->m_externalPush.y);
					// */
					
					const real extraPushDistance = 0.003f;
					
					// Select the direction which intersects the least. So we make the smallest correction.
					if (intersectRect.getWidth() < intersectRect.getHeight())
					{
						if (tt::math::realEqual(intersectRect.getLeft(), ourRect.getLeft()))
						{
							otherCtrl->m_externalPush.x -= intersectRect.getWidth() + extraPushDistance;
						}
						else if (tt::math::realEqual(intersectRect.getRight(), ourRect.getRight()))
						{
							otherCtrl->m_externalPush.x += intersectRect.getWidth() + extraPushDistance;
						}
					}
					else
					{
						if (tt::math::realEqual(intersectRect.getBottom(), ourRect.getBottom()))
						{
							otherCtrl->m_externalPush.y += intersectRect.getHeight() + extraPushDistance;
						}
						else if (tt::math::realEqual(intersectRect.getTop(), ourRect.getTop()))
						{
							otherCtrl->m_externalPush.y -= intersectRect.getHeight() + extraPushDistance;
						}
					}
				}
			}
		}
	}
}


void DirectionalMovementController::establishCollisionParentHierarchy(const Entity& p_entity)
{
	// Entities without their own collision tiles cannot carry other entities
	if (p_entity.hasCollisionTiles() == false)
	{
		return;
	}
	
	EntityMgr& entityMgr(AppGlobal::getGame()->getEntityMgr());
	
	const tt::math::PointRect ownTilesRect(p_entity.getCollisionTiles()->getPos(),
	                                       p_entity.getCollisionTiles()->getWidth(),
	                                       p_entity.getCollisionTiles()->getHeight());
	const tt::math::PointRect tilesToCheck(ownTilesRect.getMin()       - tt::math::Point2(1, 1),
	                                       ownTilesRect.getMaxInside() + tt::math::Point2(1, 1));
	
	EntityHandleSet entities;
	AppGlobal::getGame()->getTileRegistrationMgr().findRegisteredEntityHandles(tilesToCheck, entities);
	
	for (EntityHandleSet::iterator it = entities.begin(); it != entities.end(); ++it)
	{
		if (*it == p_entity.getHandle())
		{
			continue;
		}
		
		Entity* otherEntity = entityMgr.getEntity(*it);
		if (otherEntity != 0 && otherEntity->canBeCarried())
		{
			otherEntity->onPotentialCollisionParentStartsMove(p_entity.getHandle());
		}
	}
}


void DirectionalMovementController::overrideCollisionAncestorRecursive(const EntityHandle& p_ancestorToSet)
{
	const bool ourAncestorChanged = (p_ancestorToSet != m_collisionAncestor);
	m_collisionAncestor = p_ancestorToSet;
	
	EntityMgr&             entityMgr(AppGlobal::getGame()->getEntityMgr());
	MovementControllerMgr& ctrlMgr  (entityMgr.getMovementControllerMgr());
	
	for (EntityHandleSet::iterator it = m_collisionChildren.begin();
	     it != m_collisionChildren.end(); ++it)
	{
		Entity* child = entityMgr.getEntity(*it);
		if (child != 0)
		{
			DirectionalMovementController* childCtrl = ctrlMgr.getDirectionalController(
					child->getMovementControllerHandle());
			TT_NULL_ASSERT(childCtrl);  // collision children must have a movement controller
			if (childCtrl != 0)
			{
				childCtrl->overrideCollisionAncestorRecursive(p_ancestorToSet);
			}
		}
	}
	
	if (ourAncestorChanged)
	{
		m_requestBitMask.setFlag(RequestFlag_UpdateLocalCollision);
	}
}


void DirectionalMovementController::addCollisionChild(const EntityHandle& p_childHandle)
{
	TT_ASSERT(p_childHandle.isEmpty() == false);
	m_collisionChildren.insert(p_childHandle);
}


void DirectionalMovementController::removeCollisionChild(const EntityHandle& p_childHandle)
{
	TT_ASSERT(p_childHandle.isEmpty() == false);
	m_collisionChildren.erase(p_childHandle);
}


bool DirectionalMovementController::checkCollision(Entity&                    p_entity,
                                                   const tt::math::Vector2&   p_deltaPos)
{
	// Check for tile collision
	if (p_entity.isSuspended())       // No collision for suspended entities.
	{
		return false;
	}
	
	if (m_currentMove != 0)
	{
		if (m_currentMove->getFlags().checkFlag(movement::MoveBase::Flag_IgnoreCollision))
		{
			return false;
		}
	}
	
	const movement::TileCollisionHelper::CollisionResultMask& collisionMask(m_localCollision);
	
	if (collisionMask.isEmpty())
	{
		return false;
	}
	
	using movement::TileCollisionHelper;
	
	const tt::math::VectorRect worldTiles = p_entity.calcWorldRect();
	const tt::math::Vector2& worldTilesMin(   worldTiles.getMin()       );
	const tt::math::Vector2  worldTilesMax(   worldTiles.getMaxEdge()   );
	const tt::math::Point2&  collisionTilesMin(m_localCollisionTileRect.getMin()    );
	const tt::math::Point2   collisionTilesMax(m_localCollisionTileRect.getMaxEdge());
	
	/*
	{
		TT_Printf("[%06u] DirectionalMovementController::checkCollision ctrl handle 0X%X, entity 0X%X\n",
		          AppGlobal::getUpdateFrameCount(), getHandle().getValue(), p_entity.getHandle().getValue());
		TT_Printf("DirectionalMovementController::checkCollision - p_deltaPos x: %f, y: %f\n", p_deltaPos.x, p_deltaPos.y);
		TT_Printf("DirectionalMovementController::checkCollision - worldTiles Min x: %f, y: %f - Max x: %f, y: %f\n",
		          worldTilesMin.x, worldTilesMin.y, worldTilesMax.x, worldTilesMax.y);
		TT_Printf("DirectionalMovementController::checkCollision - collisionTiles Min x: %d, y: %d - Max x: %d, y: %d\n",
		          collisionTilesMin.x, collisionTilesMin.y, collisionTilesMax.x, collisionTilesMax.y);
		TT_Printf("DirectionalMovementController::checkCollision - Entity pos x: %f, y: %f\n",
		          p_entity.getPosition().x, p_entity.getPosition().y);
	}
	// */
	static const real epsilon = 0.00001f;
	
	//tt::math::Vector2 newPos(p_entity.getPosition());
	movement::Direction ySnapDir        = movement::Direction_None;
	movement::Direction xSnapDir        = movement::Direction_None;
	movement::Direction yCornerCheckDir = movement::Direction_None;
	
	if (p_deltaPos.y >= 0) // Moving Up
	{
		if (worldTilesMax.y > collisionTilesMax.y) // Over the edge
		{
			if (collisionMask.checkFlag(TileCollisionHelper::CollisionResult_Top))
			{
				ySnapDir       = movement::Direction_Up;
			}
			else
			{
				yCornerCheckDir = movement::Direction_Up;
			}
		}
	}
	if (p_deltaPos.y <= 0) // Moving Down
	{
		if (worldTilesMin.y < collisionTilesMin.y) // Over the edge
		{
			if (collisionMask.checkFlag(TileCollisionHelper::CollisionResult_Bottom))
			{
				ySnapDir       = movement::Direction_Down;
			}
			else
			{
				yCornerCheckDir = movement::Direction_Down;
			}
		}
	}
	
	if (p_deltaPos.x <= 0) // Moving Left
	{
		if (worldTilesMin.x < collisionTilesMin.x) // Over the edge
		{
			if (collisionMask.checkFlag(TileCollisionHelper::CollisionResult_Left))
			{
				xSnapDir        = movement::Direction_Left;
				yCornerCheckDir = movement::Direction_None;
			}
			else
			{
				switch (yCornerCheckDir)
				{
				case movement::Direction_Up:
					if (collisionMask.checkFlag(TileCollisionHelper::CollisionResult_TopLeft))
					{
						// We don't need to snap both. 
						if (p_deltaPos.x < -p_deltaPos.y)
						{
							ySnapDir = movement::Direction_Up;
						}
						else
						{
							xSnapDir = movement::Direction_Left;
						}
					}
					break;
				case movement::Direction_Down:
					if (collisionMask.checkFlag(TileCollisionHelper::CollisionResult_BottomLeft))
					{
						// We don't need to snap both. 
						if (p_deltaPos.x < p_deltaPos.y)
						{
							ySnapDir = movement::Direction_Down;
						}
						else
						{
							xSnapDir = movement::Direction_Left;
						}
					}
					break;
				case movement::Direction_None:
				default:
					break;
				}
			}
		}
	}
	if (p_deltaPos.x >= 0) // Moving Right
	{
		if (worldTilesMax.x > collisionTilesMax.x) // Over the edge
		{
			if (collisionMask.checkFlag(TileCollisionHelper::CollisionResult_Right))
			{
				xSnapDir        = movement::Direction_Right;
				yCornerCheckDir = movement::Direction_None;
			}
			else
			{
				switch (yCornerCheckDir)
				{
				case movement::Direction_Up:
					if (collisionMask.checkFlag(TileCollisionHelper::CollisionResult_TopRight))
					{
						// We don't need to snap both. 
						if (p_deltaPos.x < p_deltaPos.y)
						{
							ySnapDir = movement::Direction_Up;
						}
						else
						{
							xSnapDir = movement::Direction_Right;
						}
					}
					break;
				case movement::Direction_Down:
					if (collisionMask.checkFlag(TileCollisionHelper::CollisionResult_BottomRight))
					{
						// We don't need to snap both. 
						if (p_deltaPos.x < -p_deltaPos.y)
						{
							ySnapDir = movement::Direction_Down;
						}
						else
						{
							xSnapDir = movement::Direction_Right;
						}
					}
					break;
				case movement::Direction_None:
				default:
					break;
				}
			}
		}
	}
	
	switch (ySnapDir)
	{
	case movement::Direction_Up:
		p_entity.modifyPosition().y -= (worldTilesMax.y - collisionTilesMax.y) + epsilon;
		//TT_ASSERT(yCornerCheckDir != movement::Direction_None ||
		//          collidWithOthers                            ||
		//          getTouchingCollisionDirections(p_entity).checkFlag(movement::Direction_Up));
		break;
	case movement::Direction_Down:
		p_entity.modifyPosition().y -= (worldTilesMin.y - collisionTilesMin.y) - epsilon;
		//TT_ASSERT(yCornerCheckDir != movement::Direction_None ||
		//          collidWithOthers                            ||
		//          getTouchingCollisionDirections(p_entity).checkFlag(movement::Direction_Down));
		break;
	case movement::Direction_None:
		// Nothing to do.
		break;
	default:
		TT_PANIC("Unsupported ySnapDir: %d", ySnapDir);
		break;
	}
	
	switch (xSnapDir)
	{
	case movement::Direction_Left:
		p_entity.modifyPosition().x -= (worldTilesMin.x - collisionTilesMin.x) - epsilon;
		//TT_ASSERT(yCornerCheckDir != movement::Direction_None ||
		//          collidWithOthers                            ||
		//          getTouchingCollisionDirections(p_entity).checkFlag(movement::Direction_Left));
		break;
	case movement::Direction_Right:
		p_entity.modifyPosition().x -= (worldTilesMax.x - collisionTilesMax.x) + epsilon;
		//TT_ASSERT(yCornerCheckDir != movement::Direction_None ||
		//          collidWithOthers                            ||
		//          getTouchingCollisionDirections(p_entity).checkFlag(movement::Direction_Right));
		break;
	case movement::Direction_None:
		// Nothing to do.
		break;
	default:
		TT_PANIC("Unsupported xSnapDir: %d", ySnapDir);
		break;
	}
	
	/*
	{
		if (ySnapDir != movement::Direction_None)
		{
			TT_Printf("y snap - %s\n", movement::getDirectionName(ySnapDir));
		}
		if (xSnapDir != movement::Direction_None)
		{
			TT_Printf("x snap - %s\n", movement::getDirectionName(xSnapDir));
		}
		
		const tt::math::VectorRect newRect = p_entity.calcWorldRect();
		
		TT_Printf("DirectionalMovementController::checkCollision - newRect Min x: %f, y: %f - Max x: %f, y: %f\n",
		          newRect.getMin().x, newRect.getMin().y, newRect.getMaxEdge().x, newRect.getMaxEdge().y);
	}
	// */
	
	return xSnapDir != movement::Direction_None || ySnapDir != movement::Direction_None;
}


movement::Directions DirectionalMovementController::getTouchingCollisionDirections(const Entity& p_entity) const
{
	using movement::TileCollisionHelper;
	
	const tt::math::VectorRect worldTiles = p_entity.calcWorldRect();
	const tt::math::Vector2& worldTilesMin(   worldTiles.getMin()       );
	const tt::math::Vector2  worldTilesMax(   worldTiles.getMaxEdge()   );
	const tt::math::Point2&  localTilesMin(m_localCollisionTileRect.getMin()    );
	const tt::math::Point2   localTilesMax(m_localCollisionTileRect.getMaxEdge());
	
	const real touchingEpsilon = 0.00003f;
	
#if defined(TT_BUILD_DEV) && 0
	TT_Printf("DMC::getTouchingCollisionDirections: ");
	if (m_localCollision.checkFlag(TileCollisionHelper::CollisionResult_Left))
	{
		TT_Printf("Left ");
	}
	if (m_localCollision.checkFlag(TileCollisionHelper::CollisionResult_Right))
	{
		TT_Printf("Right ");
	}
	if (m_localCollision.checkFlag(TileCollisionHelper::CollisionResult_Bottom))
	{
		TT_Printf("Bottom (%f - %f) <= %d (== %d)", worldTilesMin.y, touchingEpsilon, localTilesMin.y, (worldTilesMin.y - touchingEpsilon <= localTilesMin.y));
	}
	if (m_localCollision.checkFlag(TileCollisionHelper::CollisionResult_Top))
	{
		TT_Printf("Top ");
	}
	TT_Printf("\n");
#endif
	
	movement::Directions collisionDirections;
	if (m_localCollision.checkFlag(TileCollisionHelper::CollisionResult_Left) &&
	    worldTilesMin.x - touchingEpsilon <= localTilesMin.x)
	{
		collisionDirections.setFlag(movement::Direction_Left);
	}
	if (m_localCollision.checkFlag(TileCollisionHelper::CollisionResult_Right) &&
	    worldTilesMax.x + touchingEpsilon >= localTilesMax.x)
	{
		collisionDirections.setFlag(movement::Direction_Right);
	}
	if (m_localCollision.checkFlag(TileCollisionHelper::CollisionResult_Bottom) &&
	    worldTilesMin.y - touchingEpsilon <= localTilesMin.y)
	{
		collisionDirections.setFlag(movement::Direction_Down);
	}
	if (m_localCollision.checkFlag(TileCollisionHelper::CollisionResult_Top) &&
	    worldTilesMax.y + touchingEpsilon >= localTilesMax.y)
	{
		collisionDirections.setFlag(movement::Direction_Up);
	}
	
	if (m_collisionParentEntity.isEmpty() == false)
	{
		Entity* collisionParent = m_collisionParentEntity.getPtr();
		if (collisionParent != 0)
		{
			switch (p_entity.getOrientationDown())
			{
			case movement::Direction_Down:
				if (worldTilesMin.y - touchingEpsilon <= collisionParent->calcEntityTileRect().getMaxEdge().y)
				{
					collisionDirections.setFlag(movement::Direction_Down);
				}
				break;
			case movement::Direction_Right:
				if (worldTilesMax.x + touchingEpsilon >= collisionParent->calcEntityTileRect().getMin().x)
				{
					collisionDirections.setFlag(movement::Direction_Right);
				}
				break;
			case movement::Direction_Up:
				if (worldTilesMax.y + touchingEpsilon >= collisionParent->calcEntityTileRect().getMin().y)
				{
					collisionDirections.setFlag(movement::Direction_Up);
				}
				break;
			case movement::Direction_Left:
				if (worldTilesMin.x - touchingEpsilon <= collisionParent->calcEntityTileRect().getMaxEdge().x)
				{
					collisionDirections.setFlag(movement::Direction_Left);
				}
				break;
			case movement::Direction_None:
				break;
			default:
				TT_PANIC("Unknown entity down orientation: %d", p_entity.getOrientationDown());
				break;
			}
		}
	}
	/*
	else
	{
		TT_Printf("DMC::getTouchingCollisionDirection H 0X%X- no parent - down collision: %d, bottom: %d, world.y %f, local.y %f\n", 
		          getHandle().getValue(), collisionDirections.checkFlag(movement::Direction_Down),
		          m_localCollision.checkFlag(TileCollisionHelper::CollisionResult_Bottom),
		          worldTilesMin.y, localTilesMin.y);
	}
	// */
	
	return collisionDirections;
}


movement::Directions DirectionalMovementController::getTileCollisionDirections(const Entity&) const
{
	using movement::TileCollisionHelper;
	
	movement::Directions collisionDirections;
	if (m_localCollision.checkFlag(TileCollisionHelper::CollisionResult_Left))
	{
		collisionDirections.setFlag(movement::Direction_Left);
	}
	if (m_localCollision.checkFlag(TileCollisionHelper::CollisionResult_Right))
	{
		collisionDirections.setFlag(movement::Direction_Right);
	}
	if (m_localCollision.checkFlag(TileCollisionHelper::CollisionResult_Bottom))
	{
		collisionDirections.setFlag(movement::Direction_Down);
	}
	if (m_localCollision.checkFlag(TileCollisionHelper::CollisionResult_Top))
	{
		collisionDirections.setFlag(movement::Direction_Up);
	}
	return collisionDirections;
}

tt::math::PointRect DirectionalMovementController::calculateNewTileRect(const Entity&            p_entity,
                                                                        const tt::math::Vector2& p_speed,
                                                                        bool                     p_preMove,
                                                                        bool                     p_doHackCheck ) const
{
	const tt::math::VectorRect& rect        = p_entity.getCollisionRect();
	const tt::math::VectorRect  worldRect   = p_entity.calcWorldRect();
	const tt::math::Vector2&    newPos      = worldRect.getMin();
	      tt::math::Point2      newPosTiled = level::worldToTile(newPos);
	const tt::math::Vector2     speed       = (p_entity.isSuspended() == false) ? 
	                                          p_speed + m_collisionParentSpeed : tt::math::Vector2::zero;
	
	const tt::math::PointRect regdRect = p_entity.calcRegisteredTileRect();
	
	const s32 tileWidth  = level::worldToTile(tt::math::ceil(rect.getWidth()));
	const s32 tileHeight = level::worldToTile(tt::math::ceil(rect.getHeight()));
	
	// Width and height for the rectangle to return
	const s32 rectWidth  = tt::math::realEqual(speed.x, 0.0f) ? regdRect.getWidth()  : tileWidth;
	const s32 rectHeight = tt::math::realEqual(speed.y, 0.0f) ? regdRect.getHeight() : tileHeight;
	
	if (p_preMove) // We will move this frame so we need to premove our position.
	{
		if (speed.x > 0.01f)
		{
			++newPosTiled.x;
		}
		else if (speed.x < -0.01f && ( p_doHackCheck ?
		                               tt::math::realEqual(newPos.x, static_cast<real>(newPosTiled.x)) :
		                               newPos.x == newPosTiled.x) )
		{
			--newPosTiled.x;
		}
		
		if (speed.y > 0.01f)
		{
			++newPosTiled.y;
		}
		else if (speed.y < -0.01f && newPos.y == newPosTiled.y)
		{
			--newPosTiled.y;
		}
	}
	else
	{
		// was already moved this frame. 
		// In this case we need to check if we need a specific side of the registered rect (if we're mid-tile/not on an edge.)
		if (speed.x > 0.01f && newPos.x != newPosTiled.x && tileWidth < regdRect.getWidth())
		{
			++newPosTiled.x;
		}
		
		if (speed.y > 0.01f && newPos.y != newPosTiled.y && tileHeight < regdRect.getHeight())
		{
			++newPosTiled.y;
		}
	}
	
	return tt::math::PointRect(newPosTiled, rectWidth, rectHeight);
}


tt::math::PointRect DirectionalMovementController::onNewTilePosition(Entity&                    p_entity,
                                                                     const tt::math::PointRect& p_previousTiles,
                                                                     bool&                      p_newMoveSucceeded_OUT)
{
	TT_ASSERT(p_entity.getHandle() == getEntityHandle());
	TT_ASSERT(p_entity.isInitialized());
	
	level::TileRegistrationMgr& tileRegMgr(AppGlobal::getGame()->getTileRegistrationMgr());
	tileRegMgr.unregisterEntityHandle(p_previousTiles, p_entity.getHandle());
	
	// --------------------- Tile Collision Check -------------------------
	
	// Check movement to new position.
	const bool ignoreCollision = (m_currentMove != 0) ? m_currentMove->getFlags().checkFlag(movement::MoveBase::Flag_IgnoreCollision) : false;
	
	// Check for tile collision
	if (m_settings.hasCollisionWithSolid && ignoreCollision == false)
	{
		
		{
			// Remove float errors by snapping to stand on solid pos when found in survey.
			// Do this before the collision check so we don't snap into level collision.
			const movement::Direction downDir = (p_entity.getOrientationDown() != movement::Direction_None) ?
			                                     p_entity.getOrientationDown() :  movement::Direction_Down;
			if (getTouchingCollisionDirections(p_entity).checkFlag(downDir)) // == SurveyResult_StandOnSolid
			{
				p_entity.snapToStandOnSolid();
			}
		}
		
		// This is an extra collision check to make sure our snap didn't move us through collision.
		// NOTE: The checkCollision call has side effects, so this if cannot be commented out completely
		if (checkCollision(p_entity, m_speed))
		{
			/*
			TT_Printf("DirectionalMovementController::onNewTilePosition - extra collision check was triggered.0x%08X ('%s')\n",
			          &p_entity, p_entity.getType().c_str());
			// */
		}
	}
	
	
	// --------------------- Next Move Step Checks -------------------------
	
	// Check current state, surroundings (collision, other specific tiles), etc. to decide which next move is valid.
	
	// This needs to be done after the collision tile snap above. (onFloor will otherwise fail).
	updateLocalCollision(p_entity.calcRegisteredTileRect(), p_entity);
	p_entity.updateSurvey(false);
	
	// --------------------- Select next move -------------------------
	// 
	// Selection is based on:
	// - Avaliable moves. (Depends on movement set for this entity type
	//                     and
	//                     Which are active for this instance.)
	// - Current direction of the controller.
	// - Which Moves are 'valid'. Based on:
	//                            - Survey done above (collision, special tiles, etc.)
	//                            - May transite from current move
	//                            - (Other things, like minimum speed?)
	// - Select next move from avalible set. (Based on highest priority, or something else?)
	// 
	p_newMoveSucceeded_OUT = true;
	
	// Remember this before changing m_currentMove.
	movement::MoveBasePtr previousMove = m_currentMove;
	
	if (m_physicsMovementMode == PhysicsMovementMode_None)
	{
		TT_NULL_ASSERT(m_moveSet);
		movement::MoveBasePtr move = m_moveSet->getMove(m_requestedDirection, m_currentMove, p_entity);
		m_direction = m_requestedDirection;
		
		// Did we change move?
		if (move != m_currentMove)
		{
			m_currentMove = move;
			
			if (m_currentMove == 0)
			{
				// Early out with (duplicate) restore/finish code.
				
				TT_PANIC("No current move");
				m_direction = movement::Direction_None;
				m_speed.setValues(0.0f, 0.0f);
				
				tileRegMgr.registerEntityHandle(p_entity.calcRegisteredTileRect(), p_entity.getHandle());
				
				p_newMoveSucceeded_OUT = false;
				
				return tt::math::PointRect(level::worldToTile(p_entity.getPosition() + p_entity.getLocalTileRect().getPosition()),
				                           level::worldToTile(p_entity.getLocalTileRect().getWidth()),
				                           level::worldToTile(p_entity.getLocalTileRect().getHeight()));
			}
			
			// --------------------- Set new move as current move. -------------------------
			
			doMoveStartLogic(p_entity);
		}
		else
		{
			//TT_Printf("same move\n");
			if (m_currentMove != 0 && m_currentMove->isAnimation())
			{
				// Need to (re)start animation.
				m_currentMove->startAllPresentationObjects(p_entity, *this);
			}
		}
		
		{
			// Needed at start of Animation
			m_currentTime   = 0;
			m_startPosition = p_entity.getPosition();
			m_rightFoot     = (m_rightFoot == false); // Toggle each 'step'.
		}
		m_animationMoveCanceled = false;
		
		//FIXME: Can we remove this check and only have the one in normal update?
		// Check for movement end.
		if (m_endDistance >= 0.0f                                 &&
		    tt::math::realGreaterEqual(m_distance, m_endDistance) &&
		    m_direction   != movement::Direction_None)
		{
			// Distance > endDistance. Stop moving.
			p_entity.onMovementEnded(m_direction);
			endMovement();
			p_newMoveSucceeded_OUT = false;
		}
		// Check for movement failed. (FIXME: Check for default (idle) move in some otherway.)
		else if ( m_currentMove != 0 &&
		          ( m_currentMove->getDirections().checkFlag(movement::Direction_None) ||
		            m_currentMove->getLocalDir() == entity::LocalDir_None )
		          &&
		          m_direction != movement::Direction_None)
		{
			doMoveFailedLogic(p_entity, m_direction, previousMove);
			p_newMoveSucceeded_OUT = false;
		}
		
		if (p_newMoveSucceeded_OUT == false && m_currentMove == 0)
		{
			// Make sure to select a new move. (Like fall or idle.)
			TT_ASSERT(m_direction == movement::Direction_None);
			m_currentMove = m_moveSet->getMove(m_direction, m_currentMove, p_entity);
			
			if (m_currentMove != 0)
			{
				doMoveStartLogic(p_entity);
			}
		}
	}
	
	// Find to which tiles we're moving.
	tt::math::PointRect newTiles(calculateNewTileRect(p_entity, (m_currentMove != 0) ? 
	                                                            applySpeedFactor(m_currentMove->getSpeed()) : m_speed,
	                                                  true));
	
	if (p_newMoveSucceeded_OUT)
	{
		// Successfully restarted persistent movement. Reset flag.
		m_requestBitMask.resetFlag(RequestFlag_PresistentMove);
	}
	
	/*
	TT_Printf("newTiles x: %d, y: %d, w: %d, h: %d. (newPos x: %f, y: %f. Tiled x: %d, y: %d. prevPos x: %f, y: %f)\n",
	          newTiles.getPos().x, newTiles.getPos().y, newTiles.getWidth(), newTiles.getHeight(),
	          newPos.x, newPos.y, newPosTiled.x, newPosTiled.y, prePos.x, prePos.y);
	// */
	
	// TODO: else can't move in new dir. Cancel move? Callback and stop controller?
	tileRegMgr.registerEntityHandle(p_entity.calcRegisteredTileRect(), p_entity.getHandle());
	
	updateLocalCollision(p_entity.calcRegisteredTileRect(), p_entity);
	
	p_entity.updateSurvey(true); // Update survey (Possible pos snap, etc.)
	
	p_entity.scheduleReevaluateCollisionParent();
	
	return newTiles;
}


void DirectionalMovementController::onNewTilePositionWithoutMoving(
		Entity&                    p_entity,
		const tt::math::PointRect& p_previousTiles)
{
	level::TileRegistrationMgr& mgr(AppGlobal::getGame()->getTileRegistrationMgr());
	tt::math::PointRect newTiles(calculateNewTileRect(p_entity, tt::math::Vector2::zero, false));
	p_entity.updateRects(&newTiles);
	updateLocalCollision(p_entity.getRegisteredTileRect(), p_entity);
	mgr.moveRegisterEntityHandle(p_previousTiles, p_entity.getRegisteredTileRect(), p_entity.getHandle());
	p_entity.updateSurvey(p_entity.isSuspended() == false);
	
	TT_ASSERT(p_entity.getRegisteredTileRect().contains(newTiles));
}


void DirectionalMovementController::updateLocalCollision(const tt::math::PointRect& p_tileRect, const entity::Entity& p_entity)
{
	const tt::math::PointRect& registeredTileRect = p_tileRect;
	const tt::math::PointRect registeredTileRectPlusOne(
			registeredTileRect.getPosition()    - tt::math::Point2(1,1),
			registeredTileRect.getWidth()  + 2, // One extra tile on each side is 2.
			registeredTileRect.getHeight() + 2);
	/*
	TT_Printf("[%06u] DMC::updateLocalCollision: [0x%08X] pos %f, %f\n",
	          AppGlobal::getUpdateFrameCount(), m_entityHandle.getValue(),
	          p_entity.getPosition().x, p_entity.getPosition().y);
	TT_Printf("[%06u] DMC::updateLocalCollision: [0x%08X] reg %d, %d, %d, %d - %d, %d, %d, %d\n",
	          AppGlobal::getUpdateFrameCount(),          m_entityHandle.getValue(),
	          registeredTileRect.getPosition().x,        registeredTileRect.getPosition().y,
	          registeredTileRect.getWidth(),             registeredTileRect.getHeight(),
	          registeredTileRectPlusOne.getPosition().x, registeredTileRectPlusOne.getPosition().y,
	          registeredTileRectPlusOne.getWidth(),      registeredTileRectPlusOne.getHeight());
	// */
	
	m_localCollisionTileRect = registeredTileRect;
	
	m_localCollision = movement::TileCollisionHelper::hasTileCollision(registeredTileRect,
	                                                                   registeredTileRectPlusOne,
	                                                                   p_entity, true, false);
	
	
	/*
	TT_Printf("[%06u] DMC::updateLocalCollision: [0x%08X] ",
	          AppGlobal::getUpdateFrameCount(), m_entityHandle.getValue());
	bool first = true;
	if (m_localCollision.checkFlag(movement::TileCollisionHelper::CollisionResult_Top) )
	{
		if (first) { first = false; } else { TT_Printf(", "); }
		TT_Printf("Top");
	}
	if (m_localCollision.checkFlag(movement::TileCollisionHelper::CollisionResult_Bottom) )
	{
		if (first) { first = false; } else { TT_Printf(", "); }
		TT_Printf("Bottom");
	}
	if (m_localCollision.checkFlag(movement::TileCollisionHelper::CollisionResult_Left))
	{
		if (first) { first = false; } else { TT_Printf(", "); }
		TT_Printf("Left");
	}
	if (m_localCollision.checkFlag(movement::TileCollisionHelper::CollisionResult_Right))
	{
		if (first) { first = false; } else { TT_Printf(", "); }
		TT_Printf("Right");
	}
	
	if (m_localCollision.checkFlag(movement::TileCollisionHelper::CollisionResult_BottomLeft))
	{
		if (first) { first = false; } else { TT_Printf(", "); }
		TT_Printf("BottomLeft");
	}
	if (m_localCollision.checkFlag(movement::TileCollisionHelper::CollisionResult_BottomRight))
	{
		if (first) { first = false; } else { TT_Printf(", "); }
		TT_Printf("BottomRight");
	}
	if (m_localCollision.checkFlag(movement::TileCollisionHelper::CollisionResult_TopLeft))
	{
		if (first) { first = false; } else { TT_Printf(", "); }
		TT_Printf("TopLeft");
	}
	if (m_localCollision.checkFlag(movement::TileCollisionHelper::CollisionResult_TopRight))
	{
		if (first) { first = false; } else { TT_Printf(", "); }
		TT_Printf("TopRight");
	}
	
	TT_Printf("%s\n", first ? "No collision result." : " collision results.");
	// */
}


void DirectionalMovementController::doMoveStartLogic(Entity& p_entity)
{
	// --------------------- Set new move as current move. -------------------------
	// - Check if new move needs a tile snap.
	
	if (m_currentMove != 0)
	{
		const tt::math::Vector2 snappedPos = p_entity.getSnappedToTilePos();
		if (m_currentMove->getFlags().checkFlag(movement::MoveBase::Flag_StartNeedsXPositionSnap))
		{
			if (p_entity.getPosition().x != snappedPos.x)
			{
				p_entity.modifyPosition().x = snappedPos.x;
				scheduleSurveyUpdate(); // The snap might have moved us to a smaller reg rect, this changes local collsion, which can mean we no longer stand on something.
			}
		}
		if (m_currentMove->getFlags().checkFlag(movement::MoveBase::Flag_StartNeedsYPositionSnap))
		{
			if (p_entity.getPosition().y != snappedPos.y)
			{
				p_entity.modifyPosition().y = snappedPos.y;
				scheduleSurveyUpdate(); // The snap might have moved us to a smaller reg rect, this changes local collsion, which can mean we no longer stand on something.
			}
		}
		
		if (m_currentMove != m_previousMove)
		{
			m_transition.reset();
			
			if (m_previousMove != 0)
			{
				//TT_Printf("DMC::doMoveStartLogic '%s' > '%s'\n", m_previousMove->getName().c_str(), m_currentMove->getName().c_str());
				
				m_transition = m_moveSet->findTransition(m_previousMove, m_currentMove);
				if (m_transition != 0)
				{
					// add controller tags
					tt::pres::Tags tags(getTags());
					tt::pres::Tags entityTags(p_entity.getStandOnTags());
					tags.insert(entityTags.begin(), entityTags.end());
					tt::pres::Tags animTags(m_transition->getAnimationTags());
					tags.insert(animTags.begin(), animTags.end());
					
					p_entity.startAllPresentationObjectsForMovement(m_transition->getAnimationName(),
					                                                tags,
					                                                pres::StartType_TransitionMove);
					
					const script::EntityBasePtr& script(p_entity.getEntityScript());
					if (m_transition->isTurn())
					{
						script->queueSqFun("onTurnStarted");
					}
					if (m_transition->hasStartCallback())
					{
						script->queueSqFun(m_transition->getStartCallback());
					}
					
					switch (m_transition->getSpeed())
					{
					case movement::TransitionSpeed_Zero:
						m_speed.setValues(0.0f, 0.0f);
						break;
					case movement::TransitionSpeed_UseTo:
						m_speed = applySpeedFactor(m_currentMove->getSpeed());
						break;
					case movement::TransitionSpeed_UseFrom:
						m_speed = applySpeedFactor(m_previousMove->getSpeed());
						break;
					default:
						TT_PANIC("Invalid transition speed type: %d", m_transition->getSpeed());	
					}
				}
			}
		}
		
		if (m_transition == 0)
		{
			// FIXME: Martijn change; only apply speedfactor when not in physicsmovement mode
			if (m_physicsMovementMode == PhysicsMovementMode_None)
			{
				m_speed = applySpeedFactor(m_currentMove->getSpeed());
			}
			m_currentMove->startAllPresentationObjects(p_entity, *this);
		}
	}
	
	if (m_currentMove != m_previousMove)
	{
		const script::EntityBasePtr& script(p_entity.getEntityScript());
		if (m_previousMove != 0 && m_previousMove->hasEndCallback())
		{
			script->queueSqFun(m_previousMove->getEndCallback(), m_previousMove->getName());
		}
		
		if (m_currentMove != 0 && m_currentMove->hasStartCallback())
		{
			script->queueSqFun(m_currentMove->getStartCallback(), m_currentMove->getName());
		}
		
		m_previousMove = m_currentMove;
	}
}


void DirectionalMovementController::doMoveFailedLogic(Entity& p_entity,
                                                      movement::Direction p_dir,
                                                      const movement::MoveBasePtr& p_failedMove)
{
	const bool        failedMoveWasPresistent = (p_failedMove == 0) ? false : p_failedMove->getFlags().checkFlag(movement::MoveBase::Flag_PersistentMove);
	const std::string failedMoveName          = (p_failedMove == 0) ? std::string() : p_failedMove->getName();
	
	if (m_requestBitMask.checkFlag(RequestFlag_PresistentMove) == false)
	{
		// Don't call failed when retrying for persistent movement.
		p_entity.onMovementFailed(p_dir, failedMoveName);
	}
	
	m_direction = movement::Direction_None;
	if (m_requestBitMask.checkFlag(RequestFlag_PresistentMove) == false)
	{
		if (failedMoveWasPresistent)
		{
			// A persistent move failed, turn on persistent move mode.
			m_requestBitMask.setFlag(RequestFlag_PresistentMove);
		}
		else
		{
			endMovement();
			
			// Make sure to select a new move. (Like fall or idle.)
			TT_ASSERT(m_direction == movement::Direction_None);
			m_currentMove = m_moveSet->getMove(m_direction, m_currentMove, p_entity);
			TT_NULL_ASSERT(m_currentMove);
			
			if (m_currentMove != 0)
			{
				doMoveStartLogic(p_entity);
			}
		}
	}
}


// Namespace end
}
}
}
}
