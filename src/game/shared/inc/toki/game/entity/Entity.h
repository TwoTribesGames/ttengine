#if !defined(INC_TOKI_GAME_ENTITY_ENTITY_H)
#define INC_TOKI_GAME_ENTITY_ENTITY_H


#include <string>
#include <vector>

#include <squirrel/squirrel.h>

#include <tt/code/BufferWriteContext.h>
#include <tt/code/BufferReadContext.h>
#include <tt/engine/particles/fwd.h>
#include <tt/engine/particles/WorldObject.h>
#include <tt/engine/renderer/ColorRGB.h>
#include <tt/engine/renderer/QuadSprite.h>
#include <tt/math/Rect.h>
#include <tt/math/Vector2.h>
#include <tt/pres/fwd.h>
#include <tt/pres/CallbackTriggerInterface.h>
#include <tt/str/str_types.h>

#include <toki/game/entity/graphics/fwd.h>
#include <toki/game/entity/graphics/types.h>
#include <toki/game/entity/effect/fwd.h>
#include <toki/game/entity/effect/types.h>
#include <toki/game/entity/movementcontroller/DirectionalMovementController.h>
#include <toki/game/entity/sensor/fwd.h>
#include <toki/game/entity/fwd.h>
#include <toki/game/entity/PresStartSettings.h>
#include <toki/game/event/fwd.h>
#include <toki/game/fluid/fwd.h>
#include <toki/game/light/fwd.h>
#include <toki/game/movement/SurroundingsSurvey.h>
#include <toki/game/script/fwd.h>
#include <toki/game/types.h>
#include <toki/pres/fwd.h>
#include <toki/serialization/utils.h>
#include <toki/level/entity/fwd.h>
#include <toki/level/helpers.h>
#include <toki/utils/types.h>


namespace toki {
namespace game {
namespace entity {

class Entity
{
public:
	struct CreationParams
	{
		inline CreationParams()
		{ }
		
		// NOTE: Add variables here (and to the CreationParams constructor)
		//       for things that should be passed to the Entity constructor
	};
	typedef const CreationParams& ConstructorParamType;
	typedef std::vector<tt::math::Vector2> DetectionPoints;
	
	// Entity and Component functions
	inline bool                isInitialized() const { return m_state >= State_Initialized; /*|| m_state == State_Dying;*/ }	// IW CPU update
	inline bool                isDying() const       { return m_state == State_Dying; }
	inline const EntityHandle& getHandle()     const { return m_handle; }
	const std::string&         getType()       const;
	
	inline const script::EntityBasePtr& getEntityScript() const
	{
		TT_NULL_ASSERT(m_entityScript); return m_entityScript;
	}
	void setEntityScriptFromSerialize(const script::EntityBasePtr& p_script);
	
	void                  serializeCreationParams  (tt::code::BufferWriteContext* p_context) const;
	static CreationParams unserializeCreationParams(tt::code::BufferReadContext*  p_context);
	
	void serialize(  tt::code::BufferWriteContext* p_context) const;
	void unserialize(tt::code::BufferReadContext*  p_context);
	
	effect::EffectRectHandle addEffectRect(effect::EffectRectTarget p_targetType);
	void removeEffectRect(effect::EffectRectHandle& p_handle);
	void removeAllEffectRects();
	
	graphics::PowerBeamGraphicHandle addPowerBeamGraphic(
			graphics::PowerBeamType     p_type,
			const sensor::SensorHandle& p_source);
	void removePowerBeamGraphic(graphics::PowerBeamGraphicHandle& p_graphicHandle);
	void removeAllPowerBeamGraphics();
	
	graphics::TextLabelHandle addTextLabel(const std::string& p_localizationKey,
	                                       real p_width, real p_height,
	                                       utils::GlyphSetID p_glyphSetId);
	
	void removeTextLabel(graphics::TextLabelHandle& p_labelHandle);
	void removeAllTextLabels();
	
	sensor::SensorHandle addSensor(sensor::SensorType p_type, const sensor::ShapePtr& p_shape,
	                               const EntityHandle& p_target);
	sensor::TileSensorHandle addTileSensor(const sensor::ShapePtr& p_shape);
	
	void removeSensor(sensor::SensorHandle& p_sensor);
	void removeTileSensor(sensor::TileSensorHandle& p_sensor);
	void removeAllSensors(sensor::SensorType p_type);
	void removeAllTileSensors();
	void removeAllSensors();
	void updateDirtySensors(real64 p_gameTime);
	
	void registerSensorFilter(  const sensor::SensorHandle& p_sensor);
	void unregisterSensorFilter(const sensor::SensorHandle& p_sensor);
	void removeAllSensorFilter();
	
	// Light
	light::LightHandle addLight(const tt::math::Vector2& p_offset, real p_radius, real p_strength);
	void removeLight(light::LightHandle& p_sensor);
	void removeAllLights();
	
	// Darkness
	light::DarknessHandle addDarkness(real p_width, real p_height);
	void removeDarkness(light::DarknessHandle& p_sensor);
	void removeAllDarknesses();
	
	// Light Detection
	inline void setDetectableByLight(bool p_enabled) { m_isDetectableByLight = p_enabled; }
	inline bool isDetectableByLight() const
	{
		return m_isDetectableByLight && m_lightDetectionPoints.empty() == false;
	}
	inline const DetectionPoints& getLightDetectionPoints() const { return m_lightDetectionPoints; }
	void setLightDetectionPoints(const DetectionPoints& p_points);
	void removeAllLightDetectionPoints();
	inline bool isInLight() const { return m_isInLight; }
	void onLightEnter();
	void onLightExit();
	
	inline const DetectionPoints& getVibrationDetectionPoints() const { return m_vibrationDetectionPoints; }
	void setVibrationDetectionPoints(const DetectionPoints& p_points);
	void removeAllVibrationDetectionPoints();
	
	// Sight Detection
	inline void setDetectableBySight(bool p_enabled) { m_isDetectableBySight = p_enabled; }
	inline bool isDetectableBySight() const
	{
		return m_isDetectableBySight && m_sightDetectionPoints.empty() == false;
	}
	inline const DetectionPoints& getSightDetectionPoints() const { return m_sightDetectionPoints; }
	void setSightDetectionPoints(const DetectionPoints& p_points);
	void removeAllSightDetectionPoints();
	
	// Touch Detection
	inline void setDetectableByTouch(bool p_enabled) { m_isDetectableByTouch = p_enabled; }
	inline bool isDetectableByTouch() const
	{
		return m_isDetectableByTouch && m_touchShape != 0 && m_isPositionCulled == false;
	}
	inline const sensor::ShapePtr& getTouchShape() const { return m_touchShape; }
	void setTouchShape(const sensor::ShapePtr& p_shape, const tt::math::Vector2& p_offset);
	void removeTouchShape();
	
	// Light blocking
	void setLightBlocking(bool p_enabled) { m_isLightBlocking = p_enabled; }
	inline bool isLightBlocking() const { return m_isLightBlocking; }
	
	bool startNewMovement(movement::Direction p_dir, real p_endDistance);
	void stopMovement();
	void setMovementSet(const std::string& p_name);
	bool changeDirection(movement::Direction p_dir);
	movement::Direction getDirection() const;
	movement::Direction getActualMovementDirection() const;
	bool setSpeed(const tt::math::Vector2& p_speed);
	const tt::math::Vector2& getSpeed() const;
	bool setExternalForce(const tt::math::Vector2& p_force);
	const tt::math::Vector2& getExternalForce() const;
	bool setReferenceSpeedEntity(const EntityHandle& p_entity);
	bool isDoingDirectionPhysicsMovement() const;
	void setMovementDirection(const tt::math::Vector2& p_direction);
	void createMovementController(bool p_snapDown);
	void startMovementInDirection(const tt::math::Vector2& p_direction, const movementcontroller::PhysicsSettings& p_settings);
	void startPathMovement(const tt::math::Vector2& p_positionOrOffset, const movementcontroller::PhysicsSettings& p_settings, const EntityHandle& p_optinalTarget);
	void startMovementToPosition(const tt::math::Vector2& p_position, const movementcontroller::PhysicsSettings& p_settings);
	void startMovementToEntity(const EntityHandle& p_target,          const tt::math::Vector2& p_offset, const movementcontroller::PhysicsSettings& p_settings);
	void setPhysicsSettings(const movementcontroller::PhysicsSettings& p_settings);
	movementcontroller::PhysicsSettings getPhysicsSettings() const;
	
	void setParentEntity(const EntityHandle& p_parent, const tt::math::Vector2& p_offset = tt::math::Vector2::zero);
	EntityHandle getParentEntity() const;
	void setUseParentForward(bool p_useForward);
	bool hasUseParentForward() const;
	void setUseParentDown(bool p_useDown);
	bool hasUseParentDown() const;
	inline const movementcontroller::MovementControllerHandle& getMovementControllerHandle() const
	{
		return m_movementControllerHandle;
	}
	std::string getCurrentMoveName() const;
	
	bool canBeMovedInDirection(movement::Direction p_direction) const;
	
	inline real getPathAgentRadius()     const { return m_pathAgentRadius;     }
	inline bool hasPathCrowdSeparation() const { return m_pathCrowdSeparation; }
	
	// FluidSettings
	const fluid::FluidSettingsPtr getFluidSettings(fluid::FluidType p_type) const;
	fluid::FluidSettingsPtr getFluidSettings(fluid::FluidType p_type);
	void setFluidSettings(fluid::FluidType p_type, const fluid::FluidSettingsPtr& p_fluidSettings);
	bool hasFluidSettings(fluid::FluidType p_type) const;
	bool hasAnyFluidSettings() const;
	
	// Position functions. (Might get moved to another class.)
	inline const tt::math::Vector2& getPosition()       const { return m_pos;    }
	inline tt::math::Vector2 getCenterOffset() const
	{
		return tt::math::Vector2(m_localRect.getPosition().x + m_localRect.getHalfWidth(),
		                         m_localRect.getPosition().y + m_localRect.getHalfHeight());
	}
	inline tt::math::Vector2 getCenterPosition() const { return getPosition() + applyOrientationToVector2(getCenterOffset()); }
	
	void setPositionForced(const tt::math::Vector2& p_pos, bool p_snapPos);
	inline tt::math::Vector2& modifyPosition() { return m_pos; }
	
	void makeScreenSpaceEntity();
	bool isScreenSpaceEntity() const { return m_inScreenspace; }
	
	inline bool flowToTheRight() const { return m_flowToTheRight; }
	inline void setFlowToTheRight(bool p_right) { m_flowToTheRight = p_right; }
	
	// Position Snap functions
	tt::math::Vector2 getSnappedToTilePos()          const;
	tt::math::Vector2 getSnappedToTilePosLevelOnly() const;  // does not take any special cases (e.g. collision parent) into account
	void snapToStandOnSolid();
	
	inline void setSubmergeDepth(s32 p_depth) { m_submergeDepth = p_depth; }
	inline s32  getSubmergeDepth() const      { return m_submergeDepth;    }
	
	// Orientation getters / setters
	void setOrientationDown(movement::Direction p_orientationDown);
	inline movement::Direction getOrientationDown() const { return m_orientationDown; }
	inline void setOrientationForwardAsLeft(bool p_forwardIsLeft) { m_orientationForwardIsLeft = p_forwardIsLeft; handleOrientationChange(); }
	inline bool isOrientationForwardLeft() const { return m_orientationForwardIsLeft; }
	
	// Orientation helpers - Apply orientation
	inline movement::Direction getDirectionFromLocalDir(LocalDir p_localDir) const
	{ return entity::getDirectionFromLocalDir(p_localDir, m_orientationDown, m_orientationForwardIsLeft); }
	inline tt::math::Vector2 applyOrientationToVector2(const tt::math::Vector2& p_vec) const
	{ return entity::applyOrientationToVector2(p_vec, m_orientationDown, m_orientationForwardIsLeft); }
	inline tt::math::Point2 applyOrientationToPoint2(const tt::math::Point2& p_point) const
	{ return entity::applyOrientationToPoint2(p_point, m_orientationDown, m_orientationForwardIsLeft); }
	inline tt::math::VectorRect applyOrientationToVectorRect(const tt::math::VectorRect& p_rect) const
	{ return entity::applyOrientationToVectorRect(p_rect, m_orientationDown, m_orientationForwardIsLeft); }
	inline real applyOrientationToAngle(real p_angle) const
	{ return entity::applyOrientationToAngle(p_angle, m_orientationDown, m_orientationForwardIsLeft); }
	
	// Orientation helpers - Remove orientation
	inline LocalDir getLocalDirFromDirection(movement::Direction p_worldDir) const
	{ return entity::getLocalDirFromDirection(p_worldDir, m_orientationDown, m_orientationForwardIsLeft); }
	inline tt::math::Vector2 removeOrientationFromVector(const tt::math::Vector2& p_vec) const
	{ return entity::removeOrientationFromVector2(p_vec, m_orientationDown, m_orientationForwardIsLeft); }
	inline tt::math::VectorRect removeOrientationFromVectorRect(const tt::math::VectorRect& p_rect) const
	{ return entity::removeOrientationFromVectorRect(p_rect, m_orientationDown, m_orientationForwardIsLeft); }
	
	// Helpers to Transfrom from local space to world space. (Apply orientation and translate with m_pos.)
	inline tt::math::VectorRect getWorldVectorRectFromLocal(const tt::math::VectorRect& p_rect) const
	{ return applyOrientationToVectorRect(p_rect).translate(m_pos); }
	
	// Rects
	inline const tt::math::PointRect&  getRegisteredTileRect()  const { return m_registeredTileRect; }
	inline       tt::math::PointRect   calcRegisteredTileRect() const { return level::worldToTile(calcWorldRect()); }
	inline const tt::math::VectorRect& getLocalTileRect()       const { return m_localTileRect; }
	inline const tt::math::VectorRect& getWorldRect()           const { return m_worldRect;     }
	inline       tt::math::VectorRect  calcWorldRect()          const { return getWorldVectorRectFromLocal(m_localRect); }
	inline       tt::math::VectorRect  calcWorldTileRect()      const { return getWorldVectorRectFromLocal(m_localTileRect); }
	tt::math::VectorRect calcEntityTileRect() const;
	
	inline const tt::math::PointRect&  getWillEndMoveHereRect() const { return m_willEndMoveHereRect; }
	
	void updateRects(const tt::math::PointRect* p_moveToTileRect = 0);
	
	void setCollisionRect(const tt::math::VectorRect& p_rect);
	inline const tt::math::VectorRect& getCollisionRect() const { return m_localRect; }
	
	// Surrounding tiles survey.
	void updateSurvey(bool p_doScriptCallbacks);
	void doSurveyCallbacks();
	inline const movement::SurroundingsSurvey& getSurvey() const { return m_survey; }
	inline void setUpdateSurvey(bool p_enabled) { m_updateSurvey = p_enabled; if (p_enabled) { updateSurvey(true); } }
	inline bool shouldUpdateSurvey() const { return m_updateSurvey; }
	inline void clearSurvey() { m_survey = movement::SurroundingsSurvey(); m_prevSurvey = m_survey; }
	
	// Collision tiles
	inline const EntityTiles* getCollisionTiles() const { return m_collisionTiles.get(); }
	inline EntityTiles*       getCollisionTiles()       { return m_collisionTiles.get(); }
	inline bool               hasCollisionTiles() const { return m_collisionTiles != 0;  }
	void setCollisionTiles(const std::string& p_tiles);
	void setCollisionTilesActive(bool p_active);
	inline bool areCollisionTilesActive() const { return m_collisionTilesActive; }
	void removeCollisionTiles();
	inline void setCollisionTilesOffset(const tt::math::Point2& p_offset) { m_collisionTileOffset = p_offset; }
	void setEntityCollisionRectAndTiles(s32 p_width, s32 p_height, level::CollisionType p_collisionType);
	
	// Tile registration
	inline bool isTileRegistrationEnabled() const { return m_tileRegistrationEnabled; }
	void enableTileRegistration();
	void disableTileRegistration();
	
	// Appearance
	pres::PresentationObjectHandle createPresentationObject(const std::string& p_filename,
	                                                        const tt::pres::Tags& p_requiredTags,
	                                                        ParticleLayer p_layer);
	
	void destroyPresentationObject(pres::PresentationObjectHandle& p_handle);
	void destroyAllPresentationObjects();
	bool hasPresentationObject(const pres::PresentationObjectHandle& p_handle) const;
	
	void startAllPresentationObjects(const std::string& p_name,
	                                 const tt::pres::Tags& p_tagsToStart,
	                                 pres::StartType p_startType = pres::StartType_Normal);
	void stopAllPresentationObjects();
	
	void startAllPresentationObjectsForMovement(const std::string& p_name,
	                                            const tt::pres::Tags& p_tagsToStart,
	                                            pres::StartType p_startType = pres::StartType_Normal);
	void stopAllPresentationObjectsForMovement();
	
	inline void incrementDebugPresentationObjectIdx()
	{
		m_debugPresentationObjectIdx++;
		if (m_debugPresentationObjectIdx >= m_presentationObjects.size())
		{
			m_debugPresentationObjectIdx = 0;
		}
	}
	
	inline void setShowPresentationTags(bool p_show) { m_debugShowTagChanges = p_show; }
	inline bool shouldShowPresentationTags() const   { return m_debugShowTagChanges;   }
	tt::pres::Tags getStandOnTags() const;
	const char* const getStandOnStr() const;
	
	enum SpawnType
	{
		SpawnType_NoSpawn,
		SpawnType_OneShot,
		SpawnType_Continuous
	};
	
	tt::engine::particles::ParticleEffectPtr spawnParticle(SpawnType                p_spawnType,
	                                                       const std::string&       p_filename,
	                                                       const tt::math::Vector2& p_position,
	                                                       bool                     p_followEntity,
	                                                       real                     p_spawnDelay,
	                                                       bool                     p_positionIsInWorldSpace,
	                                                       ParticleLayer            p_particleLayer,
	                                                       real                     p_scale = 1.0f) const;
	
	tt::math::Vector3 getPositionForParticles(const tt::math::Vector2& p_offset) const;
	
	void renderDebug(bool p_hudRenderPass);
	void setGraphicFlippedHorizontal(bool p_enable);
	
	void kill();
	
	void setSuspended(bool p_suspended);
	inline bool isSuspended() const            { return m_suspended;        }
	
	/*! \brief Set whether this entity can be pushed by other entities, if this entity is in the way. */
	inline void setCanBePushed(bool p_canBePushed) { m_canBePushed = p_canBePushed; }
	
	/*! \brief Indicates whether this entity can be pushed by other entities, if this entity is in the way. */
	inline bool canBePushed() const { return m_canBePushed; }
	
	/*! \brief Set whether this entity can be carried by other entities. */
	void setCanBeCarried(bool p_canBeCarried);
	
	/*! \brief Indicates whether this entity can be carried by other entities. */
	inline bool canBeCarried() const { return m_canBeCarried; }
	
	/*! \brief Set whether this entity can be paused (default is true) */
	inline void setCanBePaused(bool p_canBePaused) { m_canBePaused = p_canBePaused; }
	
	/*! \brief Indicates whether this entity can be paused. */
	inline bool canBePaused() const { return m_canBePaused; }
	
	void reevaluateCollisionParent(const EntityHandle& p_caller);
	
	void makeSurroundingEntitiesScheduleReevaluateCollisionParent();
	void scheduleReevaluateCollisionParent();
	
	void onPotentialCollisionParentStartsMove(const EntityHandle& p_caller);
	
	/*! \brief Returns the direct collision parent of this entity. */
	EntityHandle getCollisionParentEntity() const;
	
	/*! \brief Returns the top-most collision parent of this entity, as stored in DirectionalMovementController. */
	EntityHandle getCachedCollisionAncestor() const;
	
	/*! \brief Returns the top-most collision parent of this entity (the one at the root of the collision entity tree). */
	EntityHandle getCollisionAncestor(bool p_useScheduledParents) const;
	
	/*! \brief Returns how many parents this entity has (how deep the collision entity tree to this entity is). */
	s32 getCollisionAncestryDepth(bool p_useScheduledParents) const;
	
	/*! \brief Indicates whether the specified entity is among one of the direct or indirect parents of this entity. */
	bool isInCollisionAncestry(const EntityHandle& p_entity, bool p_useScheduledParents) const;
	
	bool hasCollisionParent(bool p_useScheduledParents) const;
	
	// Events
	void onMovementEnded(movement::Direction p_direction);
	void onMovementFailed(movement::Direction p_direction, const std::string& p_moveName);
	void onPathMovementFailed(const tt::math::Vector2& p_closestPoint);
	void onSolidCollision(const tt::math::Vector2& p_collisionNormal, const tt::math::Vector2& p_speed);
	void onPhysicsTurn();
	void onTileChange();
	void handleEvent(const toki::game::event::Event& p_event);
	
	void onCarryBegin(const EntityHandle& p_carryingEntity);
	void onCarryEnd();
	
	// Created with handle   - State_Created
	Entity(const CreationParams& p_creationParams, const EntityHandle& p_ownHandle);
	// NOTE: We can NOT have an ~Entity destructor because of how HandleArrayMgr works (with temp object.)
	
	inline void setDebugText(const std::string& p_text) { m_scriptDefinedDebugString = p_text; }
	
	static Entity* getPointerFromHandle(const EntityHandle& p_handle);
	void invalidateTempCopy();
	
	const movementcontroller::DirectionalMovementController* getDirectionalMovementController() const;
	movementcontroller::DirectionalMovementController*       getDirectionalMovementController();
	
	inline const SensorHandles& getSensorHandles() const
	{ return m_sensors; }
	
	// Culling
	inline void setPositionCullingEnabled(bool p_enabled) { m_positionCullingEnabled = p_enabled; }
	inline bool hasPositionCullingParent() const          { return m_positionCullingParent.isEmpty() == false; }
	inline void setPositionCullingParent(const EntityHandle& p_parent) { m_positionCullingParent = p_parent; }
	
	/*! \brief Indicates whether this entity has position culling when going outside culling rectangle. */
	inline bool isPositionCullingEnabled() const { return m_positionCullingEnabled; }
	
	/*! \brief Indicates whether this entity has its position culling initialized or not. */
	inline bool isPositionCullingInitialized() const { return m_positionCullingInitialized; }
	
	/*! \brief Indicates whether this entity is currently being culled. If culling is not yet initialized (done in update); return false. */
	inline bool isPositionCulled() const { return m_positionCullingInitialized ? m_isPositionCulled : false; }
	
	/*! \brief Indicates whether this entity is currently on screen or not. */
	inline bool isOnScreen() const { return m_isOnScreen; }
	
	void initPositionCulling  (const tt::math::VectorRect& p_cullingRect);
	void updatePositionCulling(const tt::math::VectorRect& p_cullingRect, const tt::math::VectorRect& p_uncullingRect);
	
	void updateIsOnScreen(const tt::math::VectorRect& p_screenRect);
	
#if !defined(TT_BUILD_FINAL)
	std::string getDebugInfo() const;
#endif
	
private:
	enum State
	{
		State_Created,     // Object is created.               (It 'exists' in code outside the manager)
		State_Loaded,      // Resource are loaded, script run. (All resource initialization and loading is done, but entity is not in game.)
		State_Initialized, // Initialized and in the game.     (It 'exists' in the game.)
		State_Dying        // Still initialized but kill has been called on this entity
	};
	
	// Bridge object to allow particle effects to follow an entity with an offset
	class ParticleEntityFollower : public tt::engine::particles::WorldObject
	{
	public:
		inline ParticleEntityFollower(const EntityHandle&      p_entityToFollow,
		                              const tt::math::Vector2& p_followOffset)
		:
		m_entityToFollow(p_entityToFollow),
		m_followOffset(p_followOffset)
		{ }
		virtual ~ParticleEntityFollower() { }
		
		virtual tt::math::Vector3 getPosition() const;
		inline virtual real       getScale() const             { return 1.0f; }
		inline virtual real       getScaleForParticles() const { return -1.0f; } // Use negative scale to signal that particle effect should use its own scale
		virtual bool              isCulled() const;
		
	private:
		EntityHandle      m_entityToFollow;
		tt::math::Vector2 m_followOffset;
	};
	
	class EntityCallbackTrigger : public tt::pres::CallbackTriggerInterface
	{
	public:
		inline EntityCallbackTrigger(const EntityHandle& p_entity)
		:
		m_entity(p_entity)
		{ }
		
		virtual ~EntityCallbackTrigger() { }
		
		// Presentation callbacks
		virtual void callback(const std::string& p_data, const tt::pres::PresentationObjectPtr& p_object);
		
	private:
		EntityHandle m_entity;
	};
	
	typedef std::vector<graphics::PowerBeamGraphicHandle> PowerBeamGraphicHandles;
	typedef std::vector<graphics::TextLabelHandle>        TextLabelHandles;
	typedef std::vector<pres::PresentationObjectHandle  > PresentationObjects;
	typedef std::vector<effect::EffectRectHandle        > EffectRectHandles;
	
	inline State getEntityState() const { return m_state; }
	
	bool load(const std::string& p_type, s32 p_id);
	void init(const tt::math::Vector2& p_position);
	void init(const tt::math::Vector2& p_position, EntityProperties& p_properties, bool p_gameReloaded);
	void init(const tt::math::Vector2& p_position, const HSQOBJECT&  p_properties);
	
	void deinit();
	
	tt::math::PointRect calcCollisionTilesRegdRect() const;
	void setLocalRects(const tt::math::VectorRect& p_rect);
	
	// Determines on which entity this entity is standing (if any)
	EntityHandle determineStandOnParent(const EntityHandle& p_caller) const;
	
	bool setCollisionParentEntity(const EntityHandle& p_parent);
	EntityHandle getCollisionParentEntityScheduled() const;
	EntityHandle getCollisionAncestor(s32*                p_ancestorCount,
	                                  const EntityHandle& p_stopOnThisEntity,
	                                  bool                p_useScheduledParents) const;
	
	void setMovementController(const movementcontroller::MovementControllerHandle& p_movementControllerHandle,
	                                 movementcontroller::MovementControllerMgr&    p_controllerMgr);
	
	movementcontroller::DirectionalMovementController*       createDirectionalMovementController();
	
	void handleOrientationChange();
	void updatePresentationObjectsPosition();
	
#if !defined(TT_BUILD_FINAL)
	// Debug helper to easily get the names of the specified set of tags, sorted alphabetically.
	// (only available in non-final, because the original string of a Hash is only available in non-final)
	static tt::str::Strings getSortedTagNames(const tt::pres::Tags& p_tags);
#else
	static inline tt::str::Strings getSortedTagNames(const tt::pres::Tags&) { return tt::str::Strings(); }
#endif
	
	void setPositionCulled(bool p_isCulled); // Used by EntityMgr
	
	// Order of members is 64-bit aligned
	
	movement::Direction m_orientationDown;          // Which direction is down for this entity. (What is it standing on.)
	State               m_state;
	bool                m_suspended;
	bool                m_orientationForwardIsLeft; // Forward can be right or fliped and than forward is left.
	bool                m_inScreenspace;
	bool                m_flowToTheRight; // When fluid flow direction balances out should this entity flow to the right?
	
	s32                 m_submergeDepth;  // How many tiles need to be below the water line before the entity floats.
	u32                 m_debugPresentationObjectIdx;
	
	bool                m_tileRegistrationEnabled;
	bool                m_updateSurvey;
	bool                m_debugShowTagChanges;
	bool                m_isDetectableByLight;
	
	bool                m_isInLight;
	bool                m_isLightBlocking;
	bool                m_scriptIsInLight; // What script 'thinks'. Used to call the correct callback.
	bool                m_isDetectableBySight;
	
	bool                m_isDetectableByTouch;
	bool                m_positionCullingEnabled;
	bool                m_positionCullingInitialized;
	bool                m_isPositionCulled;
	
	bool                m_isOnScreen;
	bool                m_canBePushed;
	bool                m_canBeCarried;
	bool                m_canBePaused;
	
	real                m_pathAgentRadius;     // not serialized: always retrieved from EntityInfo. FIXME: Remove this redundant state. Get it from EntityInfo directly.
	bool                m_pathCrowdSeparation; // not serialized: always retrieved from EntityInfo. FIXME: Remove this redundant state. Get it from EntityInfo directly.
	bool                m_collisionTilesActive;    // local copy, so that flag persists even when m_collisionTiles is destroyed
	
	EntityHandle        m_handle; // Own handle
	
	tt::math::Vector2   m_pos;
	
	tt::math::VectorRect m_worldRect;     // collision rect, in world space
	tt::math::VectorRect m_localRect;     // collision rect, in (local) model space
	tt::math::VectorRect m_localTileRect; // tile rect in (local) model space
	tt::math::PointRect  m_willEndMoveHereRect;  // tile rect (in world space) where the entity will end up after its move
	
	tt::math::PointRect  m_registeredTileRect; // Rect with the tiles that are registered.
	
	movement::SurroundingsSurvey m_survey;
	movement::SurroundingsSurvey m_prevSurvey;
	
	PresentationObjects          m_presentationObjects;
	
	movementcontroller::MovementControllerHandle m_movementControllerHandle;
	
	fluid::FluidSettingsPtr m_fluidSettings[fluid::FluidType_Count];
	
	PowerBeamGraphicHandles m_powerBeamGraphics;
	TextLabelHandles        m_textLabels;
	EffectRectHandles       m_effectRects;
	
	SensorHandles           m_sensors;
	SensorHandles           m_filteredBySensors;
	
	typedef std::vector<sensor::TileSensorHandle> TileSensorHandles;
	TileSensorHandles m_tileSensors;
	
	typedef std::vector<light::LightHandle> LightHandles;
	LightHandles m_lights;
	
	typedef std::vector<light::DarknessHandle> DarknessHandles;
	DarknessHandles m_darknesses;
	
	DetectionPoints      m_vibrationDetectionPoints;
	DetectionPoints      m_sightDetectionPoints;
	DetectionPoints      m_lightDetectionPoints;
	sensor::ShapePtr     m_touchShape;
	tt::math::Vector2    m_touchShapeOffset;
	
	EntityHandle         m_positionCullingParent;
	
	EntityTilesPtr      m_collisionTiles;
	tt::math::PointRect m_collisionTilesRegdRect;  // registered tile rect for the entity's own collision tiles
	tt::math::Point2    m_collisionTileOffset;
	
	script::EntityBasePtr m_entityScript;
	
	std::string m_scriptDefinedDebugString;
	
	
	friend class EntityMgr; // For creation and culling.
};

// Namespace end
}
}
}


#endif  // !defined(INC_TOKI_GAME_ENTITY_ENTITY_H)
