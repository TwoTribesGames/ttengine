#include <tt/engine/particles/ParticleEffect.h>
#include <tt/engine/particles/ParticleMgr.h>
#include <tt/script/ScriptEngine.h>

#include <toki/audio/AudioPlayer.h>
#include <toki/game/entity/sensor/Sensor.h>
#include <toki/game/entity/sensor/SensorMgr.h>
#include <toki/game/entity/Entity.h>
#include <toki/game/entity/EntityMgr.h>
#include <toki/game/event/Event.h>
#include <toki/game/event/EventMgr.h>
#include <toki/game/fluid/FluidMgr.h>
#include <toki/game/script/wrappers/EffectRectWrapper.h>
#include <toki/game/script/wrappers/EntityWrapper.h>
#include <toki/game/script/wrappers/EventWrapper.h>
#include <toki/game/script/wrappers/FluidSettingsWrapper.h>
#include <toki/game/script/wrappers/DarknessWrapper.h>
#include <toki/game/script/wrappers/LightWrapper.h>
#include <toki/game/script/wrappers/ParticleEffectWrapper.h>
#include <toki/game/script/wrappers/PhysicsSettingsWrapper.h>
#include <toki/game/script/wrappers/PowerBeamGraphicWrapper.h>
#include <toki/game/script/wrappers/PresentationObjectWrapper.h>
#include <toki/game/script/wrappers/SensorWrapper.h>
#include <toki/game/script/wrappers/ShapeWrapper.h>
#include <toki/game/script/wrappers/SoundCueWrapper.h>
#include <toki/game/script/wrappers/TextLabelWrapper.h>
#include <toki/game/script/wrappers/TileSensorWrapper.h>
#include <toki/game/script/sqbind_bindings.h>
#include <toki/game/script/TimerMgr.h>
#include <toki/game/script/Timer.h>
#include <toki/game/Game.h>
#include <toki/utils/types.h>
#include <toki/AppGlobal.h>


namespace toki {
namespace game {
namespace script {
namespace wrappers {

//--------------------------------------------------------------------------------------------------
// Public member functions

EntityWrapper::EntityWrapper(entity::EntityHandle p_handle)
:
m_handle(p_handle)
{
}


void EntityWrapper::setSuspended(bool p_suspended)
{
	entity::Entity* entity = getModifiableEntity();
	if (entity != 0)
	{
		entity->setSuspended(p_suspended);
	}
}


bool EntityWrapper::isSuspended() const
{
	const entity::Entity* entity = getEntity();
	if (entity != 0)
	{
		return entity->isSuspended();
	}
	
	return false;
}


void EntityWrapper::makeScreenSpaceEntity()
{
	entity::Entity* entity = getModifiableEntity();
	if (entity != 0)
	{
		entity->makeScreenSpaceEntity();
	}
}


bool EntityWrapper::isScreenSpaceEntity() const
{
	const entity::Entity* entity = getEntity();
	if (entity != 0)
	{
		return entity->isScreenSpaceEntity();
	}
	
	return false;
}


void EntityWrapper::setPosition(const tt::math::Vector2& p_position)
{
	entity::Entity* entity = getModifiableEntity();
	if (entity != 0)
	{
		entity->setPositionForced(p_position, false);
	}
}


void EntityWrapper::setSnappedPosition(const tt::math::Vector2& p_position)
{
	entity::Entity* entity = getModifiableEntity();
	if (entity != 0)
	{
		entity->setPositionForced(p_position, true);
	}
}


const tt::math::Vector2& EntityWrapper::getPosition() const
{
	const entity::Entity* entity = getEntity();
	if (entity != 0)
	{
		return entity->getPosition();
	}
	
	return tt::math::Vector2::zero;
}


tt::math::Vector2 EntityWrapper::getSnappedPosition() const
{
	const entity::Entity* entity = getEntity();
	if (entity != 0)
	{
		return entity->getSnappedToTilePosLevelOnly();
	}
	
	return tt::math::Vector2::zero;
}


tt::math::Vector2 EntityWrapper::getCenterOffset() const
{
	const entity::Entity* entity = getEntity();
	if (entity != 0)
	{
		return entity->getCenterOffset();
	}
	
	return tt::math::Vector2::zero;
}


tt::math::Vector2 EntityWrapper::getCenterPosition() const
{
	const entity::Entity* entity = getEntity();
	if (entity != 0)
	{
		return entity->getCenterPosition();
	}
	
	return tt::math::Vector2::zero;
}


void EntityWrapper::setFloorDirection(movement::Direction p_orientationDown)
{
	entity::Entity* entity = getModifiableEntity();
	if (entity != 0)
	{
		entity->setOrientationDown(p_orientationDown);
	}
}


movement::Direction EntityWrapper::getFloorDirection() const
{
	const entity::Entity* entity = getEntity();
	return (entity != 0) ? entity->getOrientationDown() : movement::Direction_Invalid;
}


bool EntityWrapper::hasSurveyResult(movement::SurveyResult p_surveyResult) const
{
	const entity::Entity* entity = getEntity();
	if (entity != 0)
	{
		TT_ASSERT(entity->shouldUpdateSurvey());
		
		// Don't use survey but determine fresh result.
		if (p_surveyResult == movement::SurveyResult_StandOnSolid)
		{
			const entity::movementcontroller::DirectionalMovementController* controller = 
			entity->getDirectionalMovementController();
			const movement::Direction downDir = (entity->getOrientationDown() != movement::Direction_None) ?
			                                     entity->getOrientationDown() :  movement::Direction_Down;
			return (controller != 0 &&
			        controller->getTouchingCollisionDirections(*entity).checkFlag(downDir));
		}
		
		movement::SurroundingsSurvey survey = entity->getSurvey();
		return survey.getCheckMask().checkFlag(p_surveyResult);
	}
	
	return false;
}


bool EntityWrapper::hasSurveyResultLocal(movement::SurveyResult p_surveyResult) const
{
	const entity::Entity* entity = getEntity();
	if (entity != 0)
	{
		TT_ASSERT(entity->shouldUpdateSurvey());
		
		return hasSurveyResult(movement::rotateForDown(p_surveyResult,
		                                               entity->getOrientationDown(),
		                                               entity->isOrientationForwardLeft()));
	}
	
	return false;
}


void EntityWrapper::setUpdateSurvey(bool p_enabled)
{
	entity::Entity* entity = getModifiableEntity();
	if (entity != 0)
	{
		entity->setUpdateSurvey(p_enabled);
	}
}


bool EntityWrapper::shouldUpdateSurvey() const
{
	const entity::Entity* entity = getEntity();
	if (entity != 0)
	{
		return entity->shouldUpdateSurvey();
	}
	
	return true;
}


void EntityWrapper::clearSurvey()
{
	entity::Entity* entity = getModifiableEntity();
	if (entity != 0)
	{
		entity->clearSurvey();
	}
}


std::string EntityWrapper::getStandOnStr() const
{
	const entity::Entity* entity = getEntity();
	std::string str;
	if (entity != 0)
	{
		str = entity->getStandOnStr();
	}
	
	return (str.empty()) ? "none" : str;
}


void EntityWrapper::setForwardAsLeft(bool p_forwardIsLeft)
{
	entity::Entity* entity = getModifiableEntity();
	if (entity != 0)
	{
		entity->setOrientationForwardAsLeft(p_forwardIsLeft);
	}
}


bool EntityWrapper::isForwardLeft() const
{
	const entity::Entity* entity = getEntity();
	return (entity != 0) ? entity->isOrientationForwardLeft() : false;
}


movement::Direction EntityWrapper::getDirectionFromLocalDir(entity::LocalDir p_localDir) const
{
	const entity::Entity* entity = getEntity();
	return (entity != 0) ? entity->getDirectionFromLocalDir(p_localDir) : movement::Direction_Invalid;
}


tt::math::Vector2 EntityWrapper::applyOrientationToVector2(const tt::math::Vector2& p_vec) const
{
	const entity::Entity* entity = getEntity();
	return (entity != 0) ? entity->applyOrientationToVector2(p_vec) : p_vec;
}


entity::LocalDir EntityWrapper::getLocalDirFromDirection(movement::Direction p_worldDir) const
{
	const entity::Entity* entity = getEntity();
	return (entity != 0) ? entity->getLocalDirFromDirection(p_worldDir) : entity::LocalDir_Invalid;
}


tt::math::Vector2 EntityWrapper::removeOrientationFromVector(const tt::math::Vector2& p_vec) const
{
	const entity::Entity* entity = getEntity();
	return (entity != 0) ? entity->removeOrientationFromVector(p_vec) : p_vec;
}


void EntityWrapper::setDebugText(const std::string& p_text)
{
	entity::Entity* entity = getModifiableEntity();
	if (entity != 0)
	{
		entity->setDebugText(p_text);
	}
}


PresentationObjectWrapper EntityWrapper::createPresentationObject(const std::string& p_filename)
{
	return createPresentationObjectImpl(p_filename, tt::pres::Tags(), ParticleLayer_InFrontOfEntities);
}


PresentationObjectWrapper EntityWrapper::createPresentationObjectInLayer(const std::string& p_filename,
                                                                         ParticleLayer p_layer)
{
	return createPresentationObjectImpl(p_filename, tt::pres::Tags(), p_layer);
}


void EntityWrapper::destroyPresentationObject(PresentationObjectWrapper& p_object)
{
	entity::Entity* entity = getModifiableEntity();
	if (entity != 0)
	{
		entity->destroyPresentationObject(p_object.getHandle());
	}
}


void EntityWrapper::destroyAllPresentationObjects()
{
	entity::Entity* entity = getModifiableEntity();
	if (entity != 0)
	{
		entity->destroyAllPresentationObjects();
	}
}


void EntityWrapper::startAllPresentationObjects(const std::string& p_name, const tt::str::Strings& p_tagsToStart)
{
	entity::Entity* entity = getModifiableEntity();
	if (entity != 0)
	{
		tt::pres::Tags startTags;
		for (tt::str::Strings::const_iterator it = p_tagsToStart.begin(); it != p_tagsToStart.end(); ++it)
		{
			startTags.insert(tt::pres::Tag(*it));
		}
		
		entity->startAllPresentationObjects(p_name, startTags, pres::StartType_Script);
	}
}


void EntityWrapper::stopAllPresentationObjects()
{
	entity::Entity* entity = getModifiableEntity();
	if (entity != 0)
	{
		entity->stopAllPresentationObjects();
	}
}


bool EntityWrapper::hasPresentationObject(const PresentationObjectWrapper& p_object)
{
	const entity::Entity* entity = getEntity();
	if (entity != 0)
	{
		return entity->hasPresentationObject(p_object.getHandle());
	}
	
	return false;
}


void EntityWrapper::setShowPresentationTags(bool p_show)
{
	entity::Entity* entity = getModifiableEntity();
	if (entity != 0)
	{
		entity->setShowPresentationTags(p_show);
	}
}


bool EntityWrapper::shouldShowPresentationTags() const
{
	const entity::Entity* entity = getEntity();
	return (entity != 0) ? entity->shouldShowPresentationTags() : false;
}


ParticleEffectWrapper EntityWrapper::spawnParticleOneShot(const std::string&       p_filename,
                                                          const tt::math::Vector2& p_position,
                                                          bool                     p_followEntity,
                                                          real                     p_spawnDelay,
                                                          bool                     p_positionIsInWorldSpace,
                                                          ParticleLayer            p_particleLayer,
                                                          real                     p_scale)
{
	tt::engine::particles::ParticleEffectPtr effect;
	ParticleEffectWrapper::SpawnInfo spawnInfo;
	
	spawnInfo.oneShot                = true;
	spawnInfo.filename               = p_filename;
	spawnInfo.position               = p_position;
	spawnInfo.followEntity           = p_followEntity;
	spawnInfo.spawningEntity         = m_handle;
	spawnInfo.spawnDelay             = p_spawnDelay;
	spawnInfo.positionIsInWorldSpace = p_positionIsInWorldSpace;
	spawnInfo.particleLayer          = p_particleLayer;
	spawnInfo.scale                  = p_scale;
	
	entity::Entity* entity = getModifiableEntity();
	if (entity != 0)
	{
		effect = entity->spawnParticle(
			entity::Entity::SpawnType_OneShot,
			p_filename,
			p_position,
			p_followEntity,
			p_spawnDelay,
			p_positionIsInWorldSpace,
			p_particleLayer,
			p_scale);
	}
	
	return ParticleEffectWrapper(effect, spawnInfo);
}


ParticleEffectWrapper EntityWrapper::spawnParticleContinuous(const std::string&       p_filename,
                                                             const tt::math::Vector2& p_position,
                                                             bool                     p_followEntity,
                                                             real                     p_spawnDelay,
                                                             bool                     p_positionIsInWorldSpace,
                                                             ParticleLayer            p_particleLayer,
                                                             real                     p_scale)
{
	tt::engine::particles::ParticleEffectPtr effect;
	ParticleEffectWrapper::SpawnInfo spawnInfo;
	
	spawnInfo.oneShot                = false;
	spawnInfo.filename               = p_filename;
	spawnInfo.position               = p_position;
	spawnInfo.followEntity           = p_followEntity;
	spawnInfo.spawningEntity         = m_handle;
	spawnInfo.spawnDelay             = p_spawnDelay;
	spawnInfo.positionIsInWorldSpace = p_positionIsInWorldSpace;
	spawnInfo.particleLayer          = p_particleLayer;
	spawnInfo.scale                  = p_scale;
	
	entity::Entity* entity = getModifiableEntity();
	if (entity != 0)
	{
		effect = entity->spawnParticle(
			entity::Entity::SpawnType_Continuous,
			p_filename,
			p_position,
			p_followEntity,
			p_spawnDelay,
			p_positionIsInWorldSpace,
			p_particleLayer,
			p_scale);
	}
	
	return ParticleEffectWrapper(effect, spawnInfo);
}


SoundCueWrapper EntityWrapper::playSoundEffect(const std::string& p_effectName)
{
	return playSoundEffectFromSoundbank("Effects", p_effectName);
}


SoundCueWrapper EntityWrapper::playSoundEffectFromSoundbank(const std::string& p_soundbank,
                                                            const std::string& p_effectName)
{
	entity::Entity* entity = getModifiableEntity();
	
	audio::SoundCue::PlayInfo playInfo;
	
	// NOTE: Disabled screen culling for sound effects.
	//       We might need to replace this with distance-based culling
	//       (if we find that too many sound effects are triggered in large levels).
	if (entity != 0 /*&& entity->isOnScreen()*/)
	{
		playInfo.soundbank                = p_soundbank;
		playInfo.name                     = p_effectName;
		playInfo.isPositional             = true;
		playInfo.entityForPositionalSound = m_handle;
		return SoundCueWrapper(audio::SoundCue::create(playInfo));
	}
	
	return SoundCueWrapper();
}


void EntityWrapper::setPositionCullingEnabled(bool p_enabled)
{
	entity::Entity* entity = getModifiableEntity();
	if (entity != 0)
	{
		entity->setPositionCullingEnabled(p_enabled);
	}
}


void EntityWrapper::setPositionCullingParent(const EntityWrapper* p_parent)
{
	entity::Entity* entity = getModifiableEntity();
	if (entity != 0)
	{
		entity::Entity* parent = p_parent != 0 ? p_parent->getHandle().getPtr() : 0;
		if (parent != 0)
		{
			entity->setPositionCullingParent(p_parent->getHandle());
			return;
		}
	}
	
	entity->setPositionCullingParent(entity::EntityHandle());
}


bool EntityWrapper::isPositionCullingEnabled() const
{
	const entity::Entity* entity = getEntity();
	return (entity != 0) ? entity->isPositionCullingEnabled() : false;
}


bool EntityWrapper::isPositionCulled() const
{
	const entity::Entity* entity = getEntity();
	if (entity != 0)
	{
		TT_ASSERTMSG(entity->isPositionCullingInitialized(),
			"Position culling is not yet initialized for entity '%s'. Use this AFTER onSpawn().",
			entity->getType().c_str());
		return entity->isPositionCulled();
	}
	
	return true;
}


bool EntityWrapper::isOnScreen() const
{
	const entity::Entity* entity = getEntity();
	if (entity != 0)
	{
		return entity->isOnScreen();
	}
	
	return false;
}


void EntityWrapper::setCanBePushed(bool p_canBePushed)
{
	entity::Entity* entity = getModifiableEntity();
	if (entity != 0)
	{
		entity->setCanBePushed(p_canBePushed);
	}
}


bool EntityWrapper::canBePushed() const
{
	const entity::Entity* entity = getEntity();
	return (entity != 0) ? entity->canBePushed() : false;
}


void EntityWrapper::setCanBeCarried(bool p_canBeCarried)
{
	entity::Entity* entity = getModifiableEntity();
	if (entity != 0)
	{
		entity->setCanBeCarried(p_canBeCarried);
	}
}


bool EntityWrapper::canBeCarried() const
{
	const entity::Entity* entity = getEntity();
	return (entity != 0) ? entity->canBeCarried() : false;
}


bool EntityWrapper::isBeingCarried() const
{
	const entity::Entity* entity = getEntity();
	return (entity != 0) ? entity->hasCollisionParent(true) : false;
}


void EntityWrapper::setCanBePaused(bool p_canBePaused)
{
	entity::Entity* entity = getModifiableEntity();
	if (entity != 0)
	{
		entity->setCanBePaused(p_canBePaused);
	}
}


bool EntityWrapper::canBePaused() const
{
	const entity::Entity* entity = getEntity();
	return (entity != 0) ? entity->canBePaused() : true;
}


void EntityWrapper::createMovementController(bool p_snapDown)
{
	entity::Entity* entity = getModifiableEntity();
	entity->createMovementController(p_snapDown);
}


bool EntityWrapper::startMovement(movement::Direction p_dir, real p_endDistance)
{
	entity::Entity* entity = getModifiableEntity();
	return (entity != 0) ? entity->startNewMovement(p_dir, p_endDistance) : false;
}


void EntityWrapper::stopMovement()
{
	entity::Entity* entity = getModifiableEntity();
	if (entity != 0)
	{
		entity->stopMovement();
	}
}


void EntityWrapper::startMovementInDirection(const tt::math::Vector2& p_direction,
                                             const wrappers::PhysicsSettingsWrapper& p_settings)
{
	entity::Entity* entity = getModifiableEntity();
	if (entity != 0)
	{
		entity->startMovementInDirection(p_direction, p_settings.getSettings());
	}
}


void EntityWrapper::startPathMovement(const tt::math::Vector2&      p_positionOrOffset,
                                      const PhysicsSettingsWrapper& p_settings,
                                      const EntityWrapper*          p_optionalTarget)
{
	entity::Entity* entity = getModifiableEntity();
	if (entity != 0)
	{
		entity->startPathMovement(p_positionOrOffset, p_settings.getSettings(), (p_optionalTarget != 0) ? 
		                          p_optionalTarget->getHandle() : entity::EntityHandle() );
	}
}


void EntityWrapper::startMovementToPosition(const tt::math::Vector2& p_position,
                                            real p_mass, real p_drag, real p_thrust,
                                            real p_easeOutDistance, real p_moveEndDistance)
{
	entity::Entity* entity = getModifiableEntity();
	if (entity != 0)
	{
		entity::movementcontroller::PhysicsSettings settings(p_mass, p_drag, p_thrust, p_easeOutDistance, p_moveEndDistance);
		entity->startMovementToPosition(p_position, settings);
	}
}


void EntityWrapper::startMovementToPositionEx(const tt::math::Vector2& p_position,
                                              const wrappers::PhysicsSettingsWrapper& p_settings)
{
	entity::Entity* entity = getModifiableEntity();
	if (entity != 0)
	{
		entity->startMovementToPosition(p_position, p_settings.getSettings());
	}
}


void EntityWrapper::startMovementToEntity(const EntityWrapper* p_target,
                                          const tt::math::Vector2& p_offset,
                                          real p_mass, real p_drag, real p_thrust,
                                          real p_easeOutDistance, real p_moveEndDistance)
{
	if (p_target == 0)
	{
		TT_PANIC("Trying to startMovementToEntity with null target");
		return;
	}
	
	entity::Entity* entity = getModifiableEntity();
	if (entity != 0)
	{
		entity::movementcontroller::PhysicsSettings settings(p_mass, p_drag, p_thrust, p_easeOutDistance, p_moveEndDistance);
		entity->startMovementToEntity(p_target->getHandle(), p_offset, settings);
	}
}


void EntityWrapper::startMovementToEntityEx(const EntityWrapper* p_target,
                                            const tt::math::Vector2& p_offset,
                                            const wrappers::PhysicsSettingsWrapper& p_settings)
{
	if (p_target == 0)
	{
		TT_PANIC("Trying to startMovementToEntity with null target");
		return;
	}
	
	entity::Entity* entity = getModifiableEntity();
	if (entity != 0)
	{
		entity->startMovementToEntity(p_target->getHandle(), p_offset, p_settings.getSettings());
	}
}


void EntityWrapper::setPhysicsSettings(const PhysicsSettingsWrapper& p_settings)
{
	entity::Entity* entity = getModifiableEntity();
	if (entity != 0)
	{
		entity->setPhysicsSettings(p_settings.getSettings());
	}
}


PhysicsSettingsWrapper EntityWrapper::getPhysicsSettings() const
{
	const entity::Entity* entity = getEntity();
	if (entity != 0)
	{
		return PhysicsSettingsWrapper(entity->getPhysicsSettings());
	}
	
	return PhysicsSettingsWrapper();
}


std::string EntityWrapper::getCurrentMoveName() const
{
	const entity::Entity* entity = getEntity();
	return (entity != 0) ? entity->getCurrentMoveName() : "";
}


void EntityWrapper::setParentEntity(const EntityWrapper* p_parent)
{
	if (p_parent == 0)
	{
		TT_PANIC("Trying to setParentEntity with null parent");
		return;
	}
	
	entity::Entity* entity = getModifiableEntity();
	if (entity != 0)
	{
		entity->setParentEntity(p_parent->getHandle(), tt::math::Vector2::zero);
	}
}


void EntityWrapper::setParentEntityWithOffset(const EntityWrapper*     p_parent,
                                              const tt::math::Vector2& p_offset)
{
	if (p_parent == 0)
	{
		TT_PANIC("Trying to setParentEntityWithOffset with null parent");
		return;
	}
	
	entity::Entity* entity = getModifiableEntity();
	if (entity != 0)
	{
		entity->setParentEntity(p_parent->getHandle(), p_offset);
	}
}


EntityBase* EntityWrapper::getParentEntity() const
{
	const entity::Entity* entity = getEntity();
	if (entity == 0)
	{
		return 0;
	}
	
	entity::Entity* parentEntity = entity->getParentEntity().getPtr();
	if (parentEntity == 0 ||
	    parentEntity->isInitialized() == false)
	{
		return 0;
	}
	
	return parentEntity->getEntityScript().get();
}


void EntityWrapper::resetParentEntity()
{
	entity::Entity* entity = getModifiableEntity();
	if (entity != 0)
	{
		entity->setParentEntity(entity::EntityHandle());
	}
}


void EntityWrapper::setUseParentForward(bool p_useForward)
{
	entity::Entity* entity = getModifiableEntity();
	if (entity != 0)
	{
		entity->setUseParentForward(p_useForward);
	}
}


bool EntityWrapper::hasUseParentForward() const
{
	const entity::Entity* entity = getEntity();
	if (entity == 0)
	{
		return false;
	}
	return entity->hasUseParentForward();
}


void EntityWrapper::setUseParentFloorDirection(bool p_useFloorDirection)
{
	entity::Entity* entity = getModifiableEntity();
	if (entity != 0)
	{
		entity->setUseParentDown(p_useFloorDirection);
	}
}


bool EntityWrapper::hasUseParentFloorDirection() const
{
	const entity::Entity* entity = getEntity();
	if (entity == 0)
	{
		return false;
	}
	return entity->hasUseParentDown();
}


void EntityWrapper::setMovementSet(const std::string& p_name)
{
	entity::Entity* entity = getModifiableEntity();
	if (entity != 0)
	{
		entity->setMovementSet(p_name);
	}
}


movement::Direction EntityWrapper::getDirection() const
{
	const entity::Entity* entity = getEntity();
	return (entity != 0) ? entity->getDirection() : movement::Direction_Invalid;
}


movement::Direction EntityWrapper::getActualMovementDirection() const
{
	const entity::Entity* entity = getEntity();
	return (entity != 0) ? entity->getActualMovementDirection() : movement::Direction_Invalid;
}


bool EntityWrapper::setSpeed(const tt::math::Vector2& p_speed)
{
	entity::Entity* entity = getModifiableEntity();
	if (entity != 0)
	{
		return entity->setSpeed(p_speed);
	}
	return false;
}


const tt::math::Vector2& EntityWrapper::getSpeed() const
{
	const entity::Entity* entity = getEntity();
	return (entity != 0) ? entity->getSpeed() : tt::math::Vector2::zero;
}


bool EntityWrapper::setExternalForce(const tt::math::Vector2& p_force)
{
	entity::Entity* entity = getModifiableEntity();
	if (entity != 0)
	{
		return entity->setExternalForce(p_force);
	}
	return false;
}


const tt::math::Vector2& EntityWrapper::getExternalForce() const
{
	const entity::Entity* entity = getEntity();
	return (entity != 0) ? entity->getExternalForce() : tt::math::Vector2::zero;
}


bool EntityWrapper::setReferenceSpeedEntity(const EntityWrapper* p_entity)
{
	entity::Entity* entity = getModifiableEntity();
	if (entity != 0)
	{
		if (p_entity != 0)
		{
			return entity->setReferenceSpeedEntity(p_entity->getHandle());
		}
		
		// else reset
		return entity->setReferenceSpeedEntity(entity::EntityHandle());
	}
	
	return false;
}


void EntityWrapper::setSpeedFactor(const tt::math::Vector2& p_factor)
{
	using entity::movementcontroller::DirectionalMovementController;
	DirectionalMovementController* controller = getMovementController();
	if (controller != 0)
	{
		controller->setSpeedFactor(p_factor);
	}
}


const tt::math::Vector2& EntityWrapper::getSpeedFactor() const
{
	using entity::movementcontroller::DirectionalMovementController;
	const DirectionalMovementController* controller = getMovementController();
	if (controller != 0)
	{
		return controller->getSpeedFactor();
	}
	return tt::math::Vector2::allOne;
}


void EntityWrapper::setSpeedFactorX(real p_x)
{
	using entity::movementcontroller::DirectionalMovementController;
	DirectionalMovementController* controller = getMovementController();
	if (controller != 0)
	{
		controller->setSpeedFactorX(p_x);
	}
}


void EntityWrapper::setSpeedFactorY(real p_y)
{
	using entity::movementcontroller::DirectionalMovementController;
	DirectionalMovementController* controller = getMovementController();
	if (controller != 0)
	{
		controller->setSpeedFactorY(p_y);
	}
}


real EntityWrapper::getSpeedFactorX() const
{
	using entity::movementcontroller::DirectionalMovementController;
	const DirectionalMovementController* controller = getMovementController();
	if (controller != 0)
	{
		return controller->getSpeedFactorX();
	}
	return 1.0f;
}


real EntityWrapper::getSpeedFactorY() const
{
	using entity::movementcontroller::DirectionalMovementController;
	const DirectionalMovementController* controller = getMovementController();
	if (controller != 0)
	{
		return controller->getSpeedFactorY();
	}
	return 1.0f;
}


bool EntityWrapper::isCurrentMoveInterruptible() const
{
	using entity::movementcontroller::DirectionalMovementController;
	const DirectionalMovementController* controller = getMovementController();
	if (controller != 0)
	{
		return controller->isCurrentMoveInterruptible();
	}
	return false;
}


int EntityWrapper::updateStickToPosition(HSQUIRRELVM v)
{
	const SQInteger argc = sq_gettop(v);
	if (argc != 3)
	{
		TT_PANIC("updateStickToPosition(Entity, Vector2) has %d argument(s), expected 2.", argc);
		return 0;
	}
	
	SQObjectType arg1Type = sq_gettype(v, 2);
	if (arg1Type == OT_INSTANCE)
	{
		EntityBase* parentBase = SqBind<EntityBase>::GetterPtr().get(v, 2);
		if (parentBase != 0)
		{
			entity::Entity* parent(parentBase->getHandle().getPtr());
			entity::Entity* thisEntity(getModifiableEntity());
			if (parent != 0 && thisEntity != 0)
			{
				SQObjectType arg2Type = sq_gettype(v, 3);
				tt::math::Vector2 newPos(parent->getCenterPosition() - thisEntity->getCenterOffset());
				if (arg2Type == OT_NULL)
				{
					thisEntity->setPositionForced(newPos, false);
				}
				else
				{
					const tt::math::Vector2& offset(SqBind<tt::math::Vector2>::get(v, 3));
					thisEntity->setPositionForced(newPos + offset, false);
				}
				SqBind<EntityBase>::push(v, parentBase);
				return 1;
			}
		}
		sq_pushnull(v);
		return 1;
	}
	
	TT_PANIC("updateStickToPosition provided with invalid first argument; "
			    "expected null or instance of type Entity, got type %d", arg1Type);
	sq_pushnull(v);
	return 1;
}


int EntityWrapper::addSightSensor(HSQUIRRELVM v)
{
	return addSensor(entity::sensor::SensorType_Sight, v);
}


SensorWrapper EntityWrapper::addSightSensorWorldSpace(const ShapeWrapper*      p_shape,
                                                      const EntityWrapper*     p_target,
                                                      const tt::math::Vector2& p_position)
{
	if (p_shape == 0 && p_target == 0)
	{
		TT_PANIC("Sightsensor should have a shape and/or a target");
		return SensorWrapper();
	}
	
	return addSensorWorldSpace(entity::sensor::SensorType_Sight, p_shape, p_target, p_position);
}


int EntityWrapper::addTouchSensor(HSQUIRRELVM v)
{
	return addSensor(entity::sensor::SensorType_Touch, v);
}


SensorWrapper EntityWrapper::addTouchSensorWorldSpace(const ShapeWrapper*      p_shape,
                                                      const EntityWrapper*     p_target,
                                                      const tt::math::Vector2& p_position)
{
	if (p_shape == 0)
	{
		TT_PANIC("Touchsensor should have a shape");
		return SensorWrapper();
	}
	
	return addSensorWorldSpace(entity::sensor::SensorType_Touch, p_shape, p_target, p_position);
}


int EntityWrapper::addTileSensor(HSQUIRRELVM v)
{
	const SQInteger argc = sq_gettop(v);
	if (argc < 2 || argc > 3)
	{
		TT_PANIC("addTileSensor(Shape, [Vector2]) has %d argument(s), expected 1 or 2.", argc);
		return 0;
	}
	
	// Arg1: Shape
	entity::sensor::ShapePtr shape;
	if (sq_gettype(v, 2) != OT_NULL)
	{
		shape = SqBind<ShapeWrapper>::get(v, 2).getShape();
	}
	
	// Arg2: Optional Vector2 (offset)
	tt::math::Vector2 offset;
	if (argc >= 3)
	{
		offset = SqBind<tt::math::Vector2>::get(v, 3);
	}
	
	entity::Entity* entity = getModifiableEntity();
	
	if (entity != 0)
	{
		TileSensorWrapper wrapper(entity->addTileSensor(shape));
		wrapper.setOffset(offset);
		SqBind<TileSensorWrapper>::push(v, wrapper);
	}
	else
	{
		sq_pushnull(v);
	}
	return 1;
}


TileSensorWrapper EntityWrapper::addTileSensorWorldSpace(const ShapeWrapper*        p_shape,
                                                         const tt::math::Vector2&   p_position)
{
	entity::sensor::ShapePtr shape;
	if (p_shape != 0)
	{
		shape = p_shape->getShape();
	}
	
	
	entity::Entity* entity = getModifiableEntity();
	if (entity == 0)
	{
		return TileSensorWrapper();
	}
	
	TileSensorWrapper wrapper(entity->addTileSensor(shape));
	wrapper.setWorldPosition(p_position);
	return wrapper;
}


void EntityWrapper::removeSensor(SensorWrapper* p_sensor)
{
	if (p_sensor == 0)
	{
		return;
	}
	
	entity::Entity* entity = getModifiableEntity();
	if (entity != 0)
	{
		entity->removeSensor(p_sensor->getHandle());
	}
}


void EntityWrapper::removeTileSensor(TileSensorWrapper* p_sensor)
{
	if (p_sensor == 0)
	{
		return;
	}
	
	entity::Entity* entity = getModifiableEntity();
	if (entity != 0)
	{
		entity->removeTileSensor(p_sensor->getHandle());
	}
}


void EntityWrapper::removeAllSensors()
{
	entity::Entity* entity = getModifiableEntity();
	if (entity != 0)
	{
		entity->removeAllSensors();
	}
}


void EntityWrapper::removeAllSightSensors()
{
	entity::Entity* entity = getModifiableEntity();
	if (entity != 0)
	{
		entity->removeAllSensors(entity::sensor::SensorType_Sight);
	}
}


void EntityWrapper::removeAllTouchSensors()
{
	entity::Entity* entity = getModifiableEntity();
	if (entity != 0)
	{
		entity->removeAllSensors(entity::sensor::SensorType_Touch);
	}
}


void EntityWrapper::removeAllTileSensors()
{
	entity::Entity* entity = getModifiableEntity();
	if (entity != 0)
	{
		entity->removeAllTileSensors();
	}
}


void EntityWrapper::recheckSensorFilter()
{
	entity::Entity* entity = getModifiableEntity();
	if (entity != 0)
	{
		entity->removeAllSensorFilter();
	}
}


void EntityWrapper::setState(const std::string& p_newState)
{
	entity::Entity* entity = getModifiableEntity();
	if (entity != 0 && entity->getEntityScript() != 0)
	{
		entity->getEntityScript()->setState(p_newState);
	}
}


std::string EntityWrapper::getState() const
{
	const entity::Entity* entity = getEntity();
	if (entity != 0 && entity->getEntityScript() != 0)
	{
		return entity->getEntityScript()->getState();
	}
	
	return std::string();
}


std::string EntityWrapper::getPreviousState() const
{
	const entity::Entity* entity = getEntity();
	if (entity != 0 && entity->getEntityScript() != 0)
	{
		return entity->getEntityScript()->getPreviousState();
	}
	
	return std::string();
}


std::string EntityWrapper::getType() const
{
	const entity::Entity* entity = getEntity();
	return (entity != 0) ? entity->getType() : std::string();
}


bool EntityWrapper::isInitialized() const
{
	const entity::Entity* entity = getEntity();
	return entity != 0 && entity->isInitialized();
}


bool EntityWrapper::isDying() const
{
	const entity::Entity* entity = getEntity();
	return entity != 0 && entity->isDying();
}


s32 EntityWrapper::getID() const
{
	if (AppGlobal::hasGame() && AppGlobal::getGame()->hasEntityMgr())
	{
		return AppGlobal::getGame()->getEntityMgr().getEntityIDByHandle(m_handle);
	}
	return -1;
}


void EntityWrapper::editorWarning(const std::string& p_warningStr)
{
	if (AppGlobal::hasGame())
	{
		return AppGlobal::getGame()->editorWarning(m_handle, p_warningStr);
	}
	
	return;
}


void EntityWrapper::registerEntityByTag(const std::string& p_tag)
{
	entity::Entity* entity = getModifiableEntity();
	if (entity != 0)
	{
		entity->getEntityScript()->addTag(p_tag);
	}
}


void EntityWrapper::unregisterEntityByTag(const std::string& p_tag)
{
	entity::Entity* entity = getModifiableEntity();
	if (entity != 0)
	{
		entity->getEntityScript()->removeTag(p_tag);
	}
}


const tt::str::Strings& EntityWrapper::getTags() const
{
	const entity::Entity* entity = getEntity();
	static tt::str::Strings empty;
	return (entity != 0) ? entity->getEntityScript()->getTags() : empty;
}


void EntityWrapper::setEntityCollisionTiles(const std::string& p_tiles)
{
	entity::Entity* entity = getModifiableEntity();
	if (entity != 0)
	{
		entity->setCollisionTiles(p_tiles);
	}
}


void EntityWrapper::setEntityCollisionTilesActive(bool p_active)
{
	entity::Entity* entity = getModifiableEntity();
	if (entity != 0)
	{
		entity->setCollisionTilesActive(p_active);
	}
}


bool EntityWrapper::areEntityCollisionTilesActive() const
{
	const entity::Entity* entity = getEntity();
	return (entity != 0) ? entity->areCollisionTilesActive() : false;
}


void EntityWrapper::removeEntityCollisionTiles()
{
	entity::Entity* entity = getModifiableEntity();
	if (entity != 0)
	{
		entity->removeCollisionTiles();
	}
}


void EntityWrapper::setEntityCollisionTilesOffset(s32 p_x, s32 p_y)
{
	entity::Entity* entity = getModifiableEntity();
	if (entity != 0)
	{
		entity->setCollisionTilesOffset(tt::math::Point2(p_x, p_y));
	}
}


void EntityWrapper::setCollisionRect(const tt::math::Vector2& p_centerPos, real p_width, real p_height)
{
	tt::math::VectorRect rect(p_centerPos, p_width, p_height);
	rect.setCenterPosition(p_centerPos);
	setCollisionRectWithVectorRect(rect);
}


void EntityWrapper::setCollisionRectWithVectorRect(const tt::math::VectorRect& p_rect)
{
	entity::Entity* entity = getModifiableEntity();
	if (entity != 0)
	{
		entity->setCollisionRect(p_rect);
	}
}


void EntityWrapper::setEntityCollisionRectAndTiles(s32 p_width, s32 p_height, level::CollisionType p_collisionType)
{
	entity::Entity* entity = getModifiableEntity();
	if (entity != 0)
	{
		entity->setEntityCollisionRectAndTiles(p_width, p_height, p_collisionType);
	}
}


tt::math::VectorRect EntityWrapper::getCollisionRect() const
{
	const entity::Entity* entity = getEntity();
	return (entity != 0) ? entity->getCollisionRect() : tt::math::VectorRect();
}


tt::math::VectorRect EntityWrapper::getCollisionRectWorldSpace() const
{
	const entity::Entity* entity = getEntity();
	return (entity != 0) ? entity->getWorldRect() : tt::math::VectorRect();
}

// Martijn: not needed for RIVE
/*
void EntityWrapper::spawnEvent(event::EventType p_type, const tt::math::Vector2& p_worldPosition,
                               real p_radius)
{
	event::EventMgr& mgr = AppGlobal::getGame()->getEventMgr();
	mgr.registerEvent(event::Event(p_type, p_worldPosition, p_radius, m_handle));
}


void EntityWrapper::spawnEventEx(event::EventType p_type, const tt::math::Vector2& p_worldPosition,
                                 real p_radius, const std::string& p_userParam)
{
	event::EventMgr& mgr = AppGlobal::getGame()->getEventMgr();
	event::Event newEvent(p_type, p_worldPosition, p_radius, m_handle);
	newEvent.setCallback(p_userParam);
	mgr.registerEvent(newEvent);
}
// */

void EntityWrapper::startTimer(const std::string& p_name, real p_timeout) const
{
	TimerMgr::startTimer(p_name, p_timeout, m_handle);
}


void EntityWrapper::startCallbackTimer(const std::string& p_callback, real p_timeout) const
{
	TimerMgr::startCallbackTimer(p_callback, p_timeout, m_handle);
}


void EntityWrapper::stopTimer(const std::string& p_name) const
{
	TimerMgr::stopTimer(p_name, m_handle);
}


void EntityWrapper::stopAllTimers() const
{
	TimerMgr::stopAllTimers(m_handle);
}


void EntityWrapper::suspendTimer(const std::string& p_name) const
{
	TimerMgr::suspendTimer(p_name, m_handle);
}


void EntityWrapper::suspendAllTimers() const
{
	TimerMgr::suspendAllTimers(m_handle);
}


void EntityWrapper::resumeTimer(const std::string& p_name) const
{
	TimerMgr::resumeTimer(p_name, m_handle);
}


void EntityWrapper::resumeAllTimers() const
{
	TimerMgr::resumeAllTimers(m_handle);
}


bool EntityWrapper::hasTimer(const std::string& p_name) const
{
	return TimerMgr::getTimer(p_name, m_handle) != 0;
}


real EntityWrapper::getTimerTimeout(const std::string& p_name) const
{
	TimerPtr timer = TimerMgr::getTimer(p_name, m_handle);
	return (timer != 0) ? timer->getTimeout() : -1.0f;
}


void EntityWrapper::setSubmergeDepth(s32 p_depthInTiles)
{
	entity::Entity* entity = getModifiableEntity();
	if (entity != 0)
	{
		entity->setSubmergeDepth(p_depthInTiles);
	}
}


s32 EntityWrapper::getSubmergeDepth() const
{
	const entity::Entity* entity = getEntity();
	return (entity != 0) ? entity->getSubmergeDepth() : 0;
}


void EntityWrapper::enableTileRegistration()
{
	entity::Entity* entity = getModifiableEntity();
	if (entity != 0)
	{
		entity->enableTileRegistration();
	}
}


void EntityWrapper::disableTileRegistration()
{
	entity::Entity* entity = getModifiableEntity();
	if (entity != 0)
	{
		entity->disableTileRegistration();
	}
}


int EntityWrapper::customCallback(HSQUIRRELVM v)
{
	entity::Entity* entity = getModifiableEntity();
	if (entity == 0)
	{
		return 0;
	}
	EntityBasePtr script(entity->getEntityScript());
	TT_NULL_ASSERT(script);
	tt::script::SqTopRestorerHelper helper(v);
	
	if (sq_gettype(v, 2) != OT_STRING)
	{
		TT_PANIC("customCallback first parameter should be of type 'string'");
		return 0;
	}
	const SQChar* callback;
	if (SQ_FAILED(sq_getstring(v, 2, &callback)))
	{
		TT_PANIC("sq_getstring failed!");
		callback = "";
	}
	
	sq_remove(v, 2);
	
	const SQInteger params = sq_gettop(v);
	TT_ASSERT(params >= 1);
	
	// Get function
	sq_pushobject(v, script->getSqState()); // Push class here.
	sq_pushstring(v, callback, -1);
	sq_get(v, -2); //get the function from the class
	
	const SQObjectType type(sq_gettype(v, sq_gettop(v)));
	if (type != OT_CLOSURE && type != OT_NATIVECLOSURE)
	{
		return 0;
	}
	
	// Copy the params on the stack
	for (s32 i = 1; i <= params; ++i)
	{
		sq_push(v, i);
	}
	
	if (SQ_FAILED(sq_call(v, params, SQFalse, TT_SCRIPT_RAISE_ERROR)))
	{
		TT_PANIC("Calling squirrel function '%s' failed. Check if number of arguments match!",
			callback);
	}
	
	return 0;
}


void EntityWrapper::removeQueuedCallbacks()
{
	entity::Entity* entity = getModifiableEntity();
	if (entity != 0 && entity->getEntityScript() != 0)
	{
		entity->getEntityScript()->removeCallbacks();
	}
}


EffectRectWrapper EntityWrapper::addEffectRect(entity::effect::EffectRectTarget p_targetType)
{
	entity::Entity* entity = getModifiableEntity();
	if (entity != 0)
	{
		return EffectRectWrapper(entity->addEffectRect(p_targetType));
	}
	else
	{
		return EffectRectWrapper();
	}
}


void EntityWrapper::removeEffectRect(EffectRectWrapper& p_effectRect)
{
	entity::Entity* entity = getModifiableEntity();
	if (entity != 0)
	{
		entity->removeEffectRect(p_effectRect.getHandle());
	}
}


PowerBeamGraphicWrapper EntityWrapper::addPowerBeamGraphic(
		entity::graphics::PowerBeamType p_type,
		const SensorWrapper&            p_sensor)
{
	entity::Entity* entity = getModifiableEntity();
	if (entity != 0)
	{
		return PowerBeamGraphicWrapper(entity->addPowerBeamGraphic(p_type, p_sensor.getHandle()));
	}
	else
	{
		return PowerBeamGraphicWrapper();
	}
}


void EntityWrapper::removePowerBeamGraphic(PowerBeamGraphicWrapper& p_graphic)
{
	entity::Entity* entity = getModifiableEntity();
	if (entity != 0)
	{
		entity->removePowerBeamGraphic(p_graphic.getHandle());
	}
}


void EntityWrapper::removeAllPowerBeamGraphics()
{
	entity::Entity* entity = getModifiableEntity();
	if (entity != 0)
	{
		entity->removeAllPowerBeamGraphics();
	}
}


TextLabelWrapper EntityWrapper::addTextLabel(const std::string& p_localizationKey, 
                                             real p_width, real p_height,
                                             utils::GlyphSetID p_glyphSetID)
{
	entity::Entity* entity = getModifiableEntity();
	if (entity != 0)
	{
		return TextLabelWrapper(entity->addTextLabel(p_localizationKey, p_width, p_height, p_glyphSetID));
	}
	else
	{
		return TextLabelWrapper();
	}
}


void EntityWrapper::removeTextLabel(TextLabelWrapper p_graphic)
{
	entity::Entity* entity = getModifiableEntity();
	if (entity != 0)
	{
		entity->removeTextLabel(p_graphic.getHandle());
	}
}


void EntityWrapper::removeAllTextLabels()
{
	entity::Entity* entity = getModifiableEntity();
	if (entity != 0)
	{
		entity->removeAllTextLabels();
	}
}


LightWrapper EntityWrapper::addLight(const tt::math::Vector2& p_offset, real p_radius, real p_strength)
{
	entity::Entity* entity = getModifiableEntity();
	
	return (entity != 0) ? LightWrapper(entity->addLight(p_offset, p_radius, p_strength)) : LightWrapper();
}


void EntityWrapper::removeLight(LightWrapper* p_light)
{
	if (p_light == 0)
	{
		return;
	}
	
	entity::Entity* entity = getModifiableEntity();
	if (entity != 0)
	{
		entity->removeLight(p_light->getHandle());
	}
}


void EntityWrapper::removeAllLights()
{
	entity::Entity* entity = getModifiableEntity();
	if (entity != 0)
	{
		entity->removeAllLights();
	}
}


DarknessWrapper EntityWrapper::addDarkness(real p_width, real p_height)
{
	entity::Entity* entity = getModifiableEntity();
	
	return (entity != 0) ? 
		DarknessWrapper(entity->addDarkness(p_width, p_height)) : DarknessWrapper();
}


void EntityWrapper::removeDarkness(DarknessWrapper* p_darkness)
{
	if (p_darkness == 0)
	{
		return;
	}
	
	entity::Entity* entity = getModifiableEntity();
	if (entity != 0)
	{
		entity->removeDarkness(p_darkness->getHandle());
	}
}


void EntityWrapper::removeAllDarknesses()
{
	entity::Entity* entity = getModifiableEntity();
	if (entity != 0)
	{
		entity->removeAllDarknesses();
	}
}


void EntityWrapper::setDetectableByLight(bool p_enabled)
{
	entity::Entity* entity = getModifiableEntity();
	if (entity != 0)
	{
		entity->setDetectableByLight(p_enabled);
	}
}


bool EntityWrapper::isDetectableByLight() const
{
	const entity::Entity* entity = getEntity();
	if (entity != 0)
	{
		return entity->isDetectableByLight();
	}
	
	return false;
}


bool EntityWrapper::isInLight() const
{
	const entity::Entity* entity = getEntity();
	if (entity != 0)
	{
		return entity->isInLight();
	}
	
	return false;
}


void EntityWrapper::setLightBlocking(bool p_enabled)
{
	entity::Entity* entity = getModifiableEntity();
	if (entity != 0)
	{
		entity->setLightBlocking(p_enabled);
	}
}


bool EntityWrapper::isLightBlocking() const
{
	const entity::Entity* entity = getEntity();
	if (entity != 0)
	{
		return entity->isLightBlocking();
	}
	
	return false;
}


int EntityWrapper::setVibrationDetectionPoints(HSQUIRRELVM v)
{
	entity::Entity* entity = getModifiableEntity();
	if (entity == 0)
	{
		return 0;
	}
	tt::script::SqTopRestorerHelper helper(v);
	
	if (sq_gettype(v, -1) != OT_ARRAY)
	{
		TT_PANIC("setVibrationDetectionPoints first parameter should be of type 'array'");
		return 0;
	}
	
	entity::Entity::DetectionPoints points;
	sq_pushnull(v); //null iterator
	while(SQ_SUCCEEDED(sq_next(v, -2)))
	{
		tt::math::Vector2 point(SqBind<tt::math::Vector2>::get(v, -1));
		points.push_back(point);
		sq_pop(v, 2);
	}
	sq_poptop(v);
	
	entity->setVibrationDetectionPoints(points);
	
	return 0;
}


void EntityWrapper::removeAllVibrationDetectionPoints()
{
	entity::Entity* entity = getModifiableEntity();
	if (entity != 0)
	{
		entity->removeAllVibrationDetectionPoints();
	}
}


void EntityWrapper::setDetectableBySight(bool p_enabled)
{
	entity::Entity* entity = getModifiableEntity();
	if (entity != 0)
	{
		entity->setDetectableBySight(p_enabled);
	}
}


bool EntityWrapper::isDetectableBySight() const
{
	const entity::Entity* entity = getEntity();
	if (entity != 0)
	{
		return entity->isDetectableBySight();
	}
	
	return false;
}


void EntityWrapper::setDetectableByTouch(bool p_enabled)
{
	entity::Entity* entity = getModifiableEntity();
	if (entity != 0)
	{
		entity->setDetectableByTouch(p_enabled);
	}
}


bool EntityWrapper::isDetectableByTouch() const
{
	const entity::Entity* entity = getEntity();
	if (entity != 0)
	{
		return entity->isDetectableByTouch();
	}
	
	return false;
}


int EntityWrapper::setSightDetectionPoints(HSQUIRRELVM v)
{
	entity::Entity* entity = getModifiableEntity();
	if (entity == 0)
	{
		return 0;
	}
	tt::script::SqTopRestorerHelper helper(v);
	
	if (sq_gettype(v, -1) != OT_ARRAY)
	{
		TT_PANIC("setSightDetectionPoints first parameter should be of type 'array'");
		return 0;
	}
	
	entity::Entity::DetectionPoints points;
	sq_pushnull(v); //null iterator
	while(SQ_SUCCEEDED(sq_next(v, -2)))
	{
		tt::math::Vector2 point(SqBind<tt::math::Vector2>::get(v, -1));
		points.push_back(point);
		sq_pop(v, 2);
	}
	sq_poptop(v);
	
	entity->setSightDetectionPoints(points);
	
	return 0;
}


void EntityWrapper::removeAllSightDetectionPoints()
{
	entity::Entity* entity = getModifiableEntity();
	if (entity != 0)
	{
		entity->removeAllSightDetectionPoints();
	}
}


int EntityWrapper::setLightDetectionPoints(HSQUIRRELVM v)
{
	entity::Entity* entity = getModifiableEntity();
	if (entity == 0)
	{
		return 0;
	}
	tt::script::SqTopRestorerHelper helper(v);
	
	if (sq_gettype(v, -1) != OT_ARRAY)
	{
		TT_PANIC("setLightDetectionPoints first parameter should be of type 'array'");
		return 0;
	}
	
	entity::Entity::DetectionPoints points;
	sq_pushnull(v); //null iterator
	while(SQ_SUCCEEDED(sq_next(v, -2)))
	{
		tt::math::Vector2 point(SqBind<tt::math::Vector2>::get(v, -1));
		points.push_back(point);
		sq_pop(v, 2);
	}
	sq_poptop(v);
	
	entity->setLightDetectionPoints(points);
	
	return 0;
}


void EntityWrapper::removeAllLightDetectionPoints()
{
	entity::Entity* entity = getModifiableEntity();
	if (entity != 0)
	{
		entity->removeAllLightDetectionPoints();
	}
}


void EntityWrapper::setTouchShape(const ShapeWrapper* p_shape, const tt::math::Vector2& p_offset)
{
	entity::Entity* entity = getModifiableEntity();
	if (entity != 0)
	{
		entity::sensor::ShapePtr shape;
		if (p_shape != 0)
		{
			shape = p_shape->getShape();
		}
		entity->setTouchShape(shape, p_offset);
	}
}


void EntityWrapper::removeTouchShape()
{
	entity::Entity* entity = getModifiableEntity();
	if (entity != 0)
	{
		entity->removeTouchShape();
	}
}


FluidSettingsWrapper EntityWrapper::addFluidSettings(fluid::FluidType p_type)
{
	entity::Entity* entity = getModifiableEntity();
	if (entity != 0)
	{
		if (entity->hasFluidSettings(p_type) == false)
		{
			fluid::FluidSettingsPtr settings(new fluid::FluidSettings());
			entity->setFluidSettings(p_type, settings);
			return FluidSettingsWrapper(p_type, entity->getHandle());
		}
		else
		{
			TT_PANIC("Cannot add FluidSettings of type '%s' to entity as they are already added.",
			         fluid::getFluidTypeName(p_type));
		}
	}
	
	return FluidSettingsWrapper();
}


void EntityWrapper::removeFluidSettings(fluid::FluidType p_type)
{
	entity::Entity* entity = getModifiableEntity();
	if (entity != 0)
	{
		if (entity->hasFluidSettings(p_type))
		{
			entity->setFluidSettings(p_type, fluid::FluidSettingsPtr());
		}
		else
		{
			TT_PANIC("Cannot remove FluidSettings of type '%s' from entity as they haven't been added.",
			         fluid::getFluidTypeName(p_type));
		}
	}
}


FluidSettingsWrapper EntityWrapper::getFluidSettings(fluid::FluidType p_type) const
{
	const entity::Entity* entity = getEntity();
	if (entity != 0)
	{
		if (entity->hasFluidSettings(p_type))
		{
			return FluidSettingsWrapper(p_type, entity->getHandle());
		}
		else
		{
			TT_PANIC("Cannot get FluidSettings of type '%s' from entity as they haven't been added.",
			         fluid::getFluidTypeName(p_type));
		}
	}
	
	return FluidSettingsWrapper();
}


SensorWrappers EntityWrapper::getAllSensors() const
{
	const entity::Entity* entity = getEntity();
	SensorWrappers sensors;
	
	if (entity != 0)
	{
		const entity::SensorHandles& handles = entity->getSensorHandles();
		sensors.reserve(handles.size());
		
		for (entity::SensorHandles::const_iterator it = handles.begin(); it != handles.end(); ++it)
		{
			sensors.push_back(SensorWrapper(*it));
		}
	}
	
	return sensors;
}


int EntityWrapper::weakref(HSQUIRRELVM v)
{
	TT_ASSERT(sq_gettop(v) == 1);
	sq_weakref(v, -1);
	return 1;
}


void EntityWrapper::bind(const tt::script::VirtualMachinePtr& p_vm)
{
	TT_SQBIND_SETVM(p_vm);
	
	TT_SQBIND_INIT_NO_INSTANCING_NAME(EntityWrapper, "EntityBase");
	TT_SQBIND_METHOD(EntityWrapper, setSuspended);
	TT_SQBIND_METHOD(EntityWrapper, isSuspended);
	TT_SQBIND_METHOD(EntityWrapper, makeScreenSpaceEntity);
	TT_SQBIND_METHOD(EntityWrapper, isScreenSpaceEntity);
	TT_SQBIND_METHOD(EntityWrapper, setPosition);
	TT_SQBIND_METHOD(EntityWrapper, setSnappedPosition);
	TT_SQBIND_METHOD(EntityWrapper, getPosition);
	TT_SQBIND_METHOD(EntityWrapper, getSnappedPosition);
	TT_SQBIND_METHOD(EntityWrapper, getCenterOffset);
	TT_SQBIND_METHOD(EntityWrapper, getCenterPosition);
	TT_SQBIND_METHOD(EntityWrapper, setFloorDirection);
	TT_SQBIND_METHOD(EntityWrapper, getFloorDirection);
	TT_SQBIND_METHOD(EntityWrapper, hasSurveyResult);
	TT_SQBIND_METHOD(EntityWrapper, hasSurveyResultLocal);
	TT_SQBIND_METHOD(EntityWrapper, setUpdateSurvey);
	TT_SQBIND_METHOD(EntityWrapper, shouldUpdateSurvey);
	TT_SQBIND_METHOD(EntityWrapper, clearSurvey);
	TT_SQBIND_METHOD(EntityWrapper, getStandOnStr);
	TT_SQBIND_METHOD(EntityWrapper, setForwardAsLeft);
	TT_SQBIND_METHOD(EntityWrapper, isForwardLeft);
	TT_SQBIND_METHOD(EntityWrapper, getDirectionFromLocalDir);
	TT_SQBIND_METHOD(EntityWrapper, applyOrientationToVector2);
	TT_SQBIND_METHOD(EntityWrapper, getLocalDirFromDirection);
	TT_SQBIND_METHOD(EntityWrapper, removeOrientationFromVector);
	TT_SQBIND_METHOD(EntityWrapper, setDebugText);
	TT_SQBIND_METHOD(EntityWrapper, createPresentationObject);
	TT_SQBIND_METHOD(EntityWrapper, createPresentationObjectInLayer);
	TT_SQBIND_METHOD(EntityWrapper, destroyPresentationObject);
	TT_SQBIND_METHOD(EntityWrapper, destroyAllPresentationObjects);
	TT_SQBIND_METHOD(EntityWrapper, startAllPresentationObjects);
	TT_SQBIND_METHOD(EntityWrapper, stopAllPresentationObjects);
	TT_SQBIND_METHOD(EntityWrapper, hasPresentationObject);
	TT_SQBIND_METHOD(EntityWrapper, setShowPresentationTags);
	TT_SQBIND_METHOD(EntityWrapper, shouldShowPresentationTags);
	TT_SQBIND_METHOD(EntityWrapper, spawnParticleOneShot);
	TT_SQBIND_METHOD(EntityWrapper, spawnParticleContinuous);
	TT_SQBIND_METHOD(EntityWrapper, playSoundEffect);
	TT_SQBIND_METHOD(EntityWrapper, playSoundEffectFromSoundbank);
	TT_SQBIND_METHOD(EntityWrapper, setPositionCullingEnabled);
	TT_SQBIND_METHOD(EntityWrapper, setPositionCullingParent);
	TT_SQBIND_METHOD(EntityWrapper, isPositionCullingEnabled);
	TT_SQBIND_METHOD(EntityWrapper, isPositionCulled);
	TT_SQBIND_METHOD(EntityWrapper, isOnScreen);
	TT_SQBIND_METHOD(EntityWrapper, setCanBePushed);
	TT_SQBIND_METHOD(EntityWrapper, canBePushed);
	TT_SQBIND_METHOD(EntityWrapper, setCanBeCarried);
	TT_SQBIND_METHOD(EntityWrapper, canBeCarried);
	TT_SQBIND_METHOD(EntityWrapper, isBeingCarried);
	TT_SQBIND_METHOD(EntityWrapper, setCanBePaused);
	TT_SQBIND_METHOD(EntityWrapper, canBePaused);
	TT_SQBIND_METHOD(EntityWrapper, createMovementController);
	TT_SQBIND_METHOD(EntityWrapper, startMovement);
	TT_SQBIND_METHOD(EntityWrapper, stopMovement);
	TT_SQBIND_METHOD(EntityWrapper, startMovementInDirection);
	TT_SQBIND_METHOD(EntityWrapper, startPathMovement);
	TT_SQBIND_METHOD(EntityWrapper, startMovementToPosition);
	TT_SQBIND_METHOD(EntityWrapper, startMovementToPositionEx);
	TT_SQBIND_METHOD(EntityWrapper, startMovementToEntity);
	TT_SQBIND_METHOD(EntityWrapper, startMovementToEntityEx);
	TT_SQBIND_METHOD(EntityWrapper, setPhysicsSettings);
	TT_SQBIND_METHOD(EntityWrapper, getPhysicsSettings);
	TT_SQBIND_METHOD(EntityWrapper, getCurrentMoveName);
	TT_SQBIND_METHOD(EntityWrapper, setParentEntity);
	TT_SQBIND_METHOD(EntityWrapper, setParentEntityWithOffset);
	TT_SQBIND_METHOD(EntityWrapper, getParentEntity);
	TT_SQBIND_METHOD(EntityWrapper, resetParentEntity);
	TT_SQBIND_METHOD(EntityWrapper, setUseParentForward);
	TT_SQBIND_METHOD(EntityWrapper, hasUseParentForward);
	TT_SQBIND_METHOD(EntityWrapper, setUseParentFloorDirection);
	TT_SQBIND_METHOD(EntityWrapper, hasUseParentFloorDirection);
	TT_SQBIND_METHOD(EntityWrapper, setMovementSet);
	TT_SQBIND_METHOD(EntityWrapper, getDirection);
	TT_SQBIND_METHOD(EntityWrapper, getActualMovementDirection);
	TT_SQBIND_METHOD(EntityWrapper, setSpeed);
	TT_SQBIND_METHOD(EntityWrapper, getSpeed);
	TT_SQBIND_METHOD(EntityWrapper, setExternalForce);
	TT_SQBIND_METHOD(EntityWrapper, getExternalForce);
	TT_SQBIND_METHOD(EntityWrapper, setReferenceSpeedEntity);
	TT_SQBIND_METHOD(EntityWrapper, setSpeedFactor);
	TT_SQBIND_METHOD(EntityWrapper, getSpeedFactor);
	TT_SQBIND_METHOD(EntityWrapper, setSpeedFactorX);
	TT_SQBIND_METHOD(EntityWrapper, setSpeedFactorY);
	TT_SQBIND_METHOD(EntityWrapper, getSpeedFactorX);
	TT_SQBIND_METHOD(EntityWrapper, getSpeedFactorY);
	TT_SQBIND_METHOD(EntityWrapper, isCurrentMoveInterruptible);
	TT_SQBIND_METHOD(EntityWrapper, setState);
	TT_SQBIND_METHOD(EntityWrapper, getState);
	TT_SQBIND_METHOD(EntityWrapper, getPreviousState);
	TT_SQBIND_METHOD(EntityWrapper, getType);
	TT_SQBIND_METHOD(EntityWrapper, registerEntityByTag);
	TT_SQBIND_METHOD(EntityWrapper, unregisterEntityByTag);
	TT_SQBIND_METHOD(EntityWrapper, equals);
	TT_SQBIND_METHOD(EntityWrapper, isInitialized);
	TT_SQBIND_METHOD(EntityWrapper, isDying);
	TT_SQBIND_METHOD(EntityWrapper, isValid);
	TT_SQBIND_METHOD(EntityWrapper, getTags);
	TT_SQBIND_SQUIRREL_METHOD(EntityWrapper, updateStickToPosition);
	TT_SQBIND_SQUIRREL_METHOD(EntityWrapper, addSightSensor);
	TT_SQBIND_METHOD(EntityWrapper, addSightSensorWorldSpace);
	TT_SQBIND_SQUIRREL_METHOD(EntityWrapper, addTouchSensor);
	TT_SQBIND_METHOD(EntityWrapper, addTouchSensorWorldSpace);
	TT_SQBIND_SQUIRREL_METHOD(EntityWrapper, addTileSensor);
	TT_SQBIND_METHOD(EntityWrapper, addTileSensorWorldSpace);
	TT_SQBIND_METHOD(EntityWrapper, removeSensor);
	TT_SQBIND_METHOD(EntityWrapper, removeTileSensor);
	TT_SQBIND_METHOD(EntityWrapper, removeAllSensors);
	TT_SQBIND_METHOD(EntityWrapper, removeAllSightSensors);
	TT_SQBIND_METHOD(EntityWrapper, removeAllTouchSensors);
	TT_SQBIND_METHOD(EntityWrapper, removeAllTileSensors);
	TT_SQBIND_METHOD(EntityWrapper, recheckSensorFilter);
	// Martijn: not needed for RIVE
	/*
	TT_SQBIND_METHOD(EntityWrapper, spawnEvent);
	TT_SQBIND_METHOD(EntityWrapper, spawnEventEx);
	// */
	TT_SQBIND_METHOD(EntityWrapper, setCollisionRect);
	TT_SQBIND_METHOD(EntityWrapper, setCollisionRectWithVectorRect);
	TT_SQBIND_METHOD(EntityWrapper, setEntityCollisionRectAndTiles);
	TT_SQBIND_METHOD(EntityWrapper, getCollisionRect);
	TT_SQBIND_METHOD(EntityWrapper, getCollisionRectWorldSpace);
	TT_SQBIND_METHOD(EntityWrapper, setEntityCollisionTiles);
	TT_SQBIND_METHOD(EntityWrapper, setEntityCollisionTilesActive);
	TT_SQBIND_METHOD(EntityWrapper, areEntityCollisionTilesActive);
	TT_SQBIND_METHOD(EntityWrapper, removeEntityCollisionTiles);
	TT_SQBIND_METHOD(EntityWrapper, setEntityCollisionTilesOffset);
	TT_SQBIND_METHOD(EntityWrapper, startTimer);
	TT_SQBIND_METHOD(EntityWrapper, startCallbackTimer);
	TT_SQBIND_METHOD(EntityWrapper, stopTimer);
	TT_SQBIND_METHOD(EntityWrapper, stopAllTimers);
	TT_SQBIND_METHOD(EntityWrapper, suspendTimer);
	TT_SQBIND_METHOD(EntityWrapper, suspendAllTimers);
	TT_SQBIND_METHOD(EntityWrapper, resumeTimer);
	TT_SQBIND_METHOD(EntityWrapper, resumeAllTimers);
	TT_SQBIND_METHOD(EntityWrapper, hasTimer);
	TT_SQBIND_METHOD(EntityWrapper, getTimerTimeout);
	TT_SQBIND_METHOD(EntityWrapper, getHandleValue);
	TT_SQBIND_METHOD(EntityWrapper, getID);
	TT_SQBIND_METHOD(EntityWrapper, editorWarning);
	TT_SQBIND_METHOD(EntityWrapper, setSubmergeDepth);
	TT_SQBIND_METHOD(EntityWrapper, getSubmergeDepth);
	TT_SQBIND_SQUIRREL_METHOD(EntityWrapper, customCallback);
	TT_SQBIND_METHOD(EntityWrapper, removeQueuedCallbacks);	
	TT_SQBIND_METHOD(EntityWrapper, addEffectRect);
	TT_SQBIND_METHOD(EntityWrapper, removeEffectRect);
	TT_SQBIND_METHOD(EntityWrapper, addPowerBeamGraphic);
	TT_SQBIND_METHOD(EntityWrapper, removePowerBeamGraphic);
	TT_SQBIND_METHOD(EntityWrapper, removeAllPowerBeamGraphics);
	TT_SQBIND_METHOD(EntityWrapper, addTextLabel);
	TT_SQBIND_METHOD(EntityWrapper, removeTextLabel);
	TT_SQBIND_METHOD(EntityWrapper, removeAllTextLabels);
	TT_SQBIND_METHOD(EntityWrapper, enableTileRegistration);
	TT_SQBIND_METHOD(EntityWrapper, disableTileRegistration);
	TT_SQBIND_METHOD(EntityWrapper, addLight);
	TT_SQBIND_METHOD(EntityWrapper, removeLight);
	TT_SQBIND_METHOD(EntityWrapper, removeAllLights);
	TT_SQBIND_METHOD(EntityWrapper, addDarkness);
	TT_SQBIND_METHOD(EntityWrapper, removeDarkness);
	TT_SQBIND_METHOD(EntityWrapper, removeAllDarknesses);
	TT_SQBIND_METHOD(EntityWrapper, setDetectableByLight);
	TT_SQBIND_METHOD(EntityWrapper, isDetectableByLight);
	TT_SQBIND_METHOD(EntityWrapper, isInLight);
	TT_SQBIND_METHOD(EntityWrapper, setLightBlocking);
	TT_SQBIND_METHOD(EntityWrapper, isLightBlocking);
	TT_SQBIND_SQUIRREL_METHOD(EntityWrapper, setVibrationDetectionPoints);
	TT_SQBIND_METHOD(EntityWrapper, removeAllSightDetectionPoints);
	TT_SQBIND_METHOD(EntityWrapper, setDetectableBySight);
	TT_SQBIND_METHOD(EntityWrapper, isDetectableBySight);
	TT_SQBIND_METHOD(EntityWrapper, setDetectableByTouch);
	TT_SQBIND_METHOD(EntityWrapper, isDetectableByTouch);
	TT_SQBIND_SQUIRREL_METHOD(EntityWrapper, setSightDetectionPoints);
	TT_SQBIND_METHOD(EntityWrapper, removeAllSightDetectionPoints);
	TT_SQBIND_SQUIRREL_METHOD(EntityWrapper, setLightDetectionPoints);
	TT_SQBIND_METHOD(EntityWrapper, removeAllLightDetectionPoints);
	TT_SQBIND_METHOD(EntityWrapper, setTouchShape);
	TT_SQBIND_METHOD(EntityWrapper, removeTouchShape);
	TT_SQBIND_METHOD(EntityWrapper, addFluidSettings);
	TT_SQBIND_METHOD(EntityWrapper, removeFluidSettings);
	TT_SQBIND_METHOD(EntityWrapper, getFluidSettings);
	TT_SQBIND_METHOD(EntityWrapper, getAllSensors);
	TT_SQBIND_SQUIRREL_METHOD(EntityWrapper, weakref);
}


//--------------------------------------------------------------------------------------------------
// Private member functions

PresentationObjectWrapper EntityWrapper::createPresentationObjectImpl(const std::string& p_filename,
                                                                      const tt::pres::Tags& p_tags,
                                                                      ParticleLayer p_layer)
{
	entity::Entity* entity = getModifiableEntity();
	if (entity == 0)
	{
		return PresentationObjectWrapper();
	}
	
	pres::PresentationObjectHandle handle = entity->createPresentationObject(p_filename, p_tags, p_layer);
	
	return PresentationObjectWrapper(handle);
}


SensorWrapper EntityWrapper::addSensor(entity::sensor::SensorType      p_type,
                                       const entity::sensor::ShapePtr& p_shape,
                                       const entity::EntityHandle&     p_target)
{
	entity::Entity* entity = getModifiableEntity();
	
	using namespace entity::sensor;
	return (entity != 0) ? SensorWrapper(entity->addSensor(p_type, p_shape, p_target)) : SensorWrapper();
}


int EntityWrapper::addSensor(entity::sensor::SensorType p_type, HSQUIRRELVM v)
{
	const SQInteger argc = sq_gettop(v);
	if (argc < 2 || argc > 4)
	{
		TT_PANIC("add%sSensor(Shape, [Entity], [Vector2]) has %d argument(s), expected 1, 2 or 3",
		         getSensorTypeName(p_type), argc);
		return 0;
	}
	
	// Arg1: Shape
	entity::sensor::ShapePtr shape;
	if (sq_gettype(v, 2) != OT_NULL)
	{
		shape = SqBind<ShapeWrapper>::get(v, 2).getShape();
	}
	
	// Arg2: Optional Entity (target)
	entity::EntityHandle target;
	if (argc >= 3)
	{
		SQObjectType arg3Type = sq_gettype(v, 3);
		if (arg3Type == OT_INSTANCE)
		{
			EntityBase* targetEntity = SqBind<EntityBase>::GetterPtr().get(v, 3);
			if (targetEntity != 0)
			{
				target = targetEntity->getHandle();
			}
			else
			{
				TT_PANIC("addSensor type %d provided with invalid third argument; "
				         "expected null or instance of type Entity, got invalid instance (binding failed)", p_type);
			}
		}
		else if (arg3Type != OT_NULL)
		{
			TT_PANIC("addSensor type %d provided with invalid third argument; "
			         "expected null or instance of type Entity, got type %d", p_type, arg3Type);
		}
	}
	
	// Sanity checking
	if (entity::sensor::Sensor::validate(p_type, shape, target) == false)
	{
		return 0;
	}
	
	// Arg3: Optional Vector2 (offset)
	tt::math::Vector2 offset;
	if (argc == 4)
	{
		offset = SqBind<tt::math::Vector2>::get(v, 4);
	}
	
	SensorWrapper wrapper(addSensor(p_type, shape, target));
	wrapper.setOffset(offset);
	SqBind<SensorWrapper>::push(v, wrapper);
	return 1;
}


SensorWrapper EntityWrapper::addSensorWorldSpace(entity::sensor::SensorType p_type,
                                                 const ShapeWrapper*        p_shape,
                                                 const EntityWrapper*       p_target,
                                                 const tt::math::Vector2&   p_position)
{
	entity::sensor::ShapePtr shape;
	if (p_shape != 0)
	{
		shape = p_shape->getShape();
	}
	
	entity::EntityHandle target;
	if (p_target != 0)
	{
		target = p_target->getHandle();
	}
	
	SensorWrapper wrapper(addSensor(p_type, shape, target));
	wrapper.setWorldPosition(p_position);
	return wrapper;
}


entity::Entity* EntityWrapper::getModifiableEntity()
{
	// Entities can only be modified if they're in their initialized state
	entity::Entity* entity = m_handle.getPtr();
	return (entity != 0 && entity->isInitialized()) ? entity : 0;
}


const entity::movementcontroller::DirectionalMovementController* EntityWrapper::getMovementController() const
{
	const entity::Entity* entity = getEntity();
	if (entity == 0 || entity->isInitialized() == false)
	{
		return 0;
	}
	return entity->getDirectionalMovementController();
}


entity::movementcontroller::DirectionalMovementController* EntityWrapper::getMovementController()
{
	entity::Entity* entity = getModifiableEntity();
	if (entity == 0 || entity->isInitialized() == false)
	{
		return 0;
	}
	return entity->getDirectionalMovementController();
}


// Namespace end
}
}
}
}
