#if !defined(INC_TOKI_GAME_SCRIPT_WRAPPERS_ENTITYWRAPPER_H)
#define INC_TOKI_GAME_SCRIPT_WRAPPERS_ENTITYWRAPPER_H


#include <tt/math/Rect.h>
#include <tt/math/Vector2.h>
#include <tt/pres/fwd.h>
#include <tt/str/str_types.h>

#include <toki/game/entity/graphics/types.h>
#include <toki/game/entity/effect/types.h>
#include <toki/game/entity/sensor/fwd.h>
#include <toki/game/entity/fwd.h>
#include <toki/game/entity/PresStartSettings.h>
#include <toki/game/event/fwd.h>
#include <toki/game/fluid/types.h>
#include <toki/game/script/wrappers/fwd.h>
#include <toki/game/script/wrappers/SensorWrapper.h>
#include <toki/game/script/fwd.h>
#include <toki/game/types.h>
#include <toki/level/types.h>
#include <toki/utils/types.h>


namespace toki {
namespace game {
namespace script {
namespace wrappers {

/*! \brief 'EntityBase' in Squirrel. */
class EntityWrapper
{
public:
	inline ~EntityWrapper() { }
	
	inline const entity::EntityHandle& getHandle() const { return m_handle; }
	
	// bindings
	
	/*! \brief Suspends or unsuspends this entity. */
	void                      setSuspended(bool p_suspended);
	
	/*! brief Returns true if the entity is suspended. */
	bool                      isSuspended() const;
	
	/*! brief Make entity "screenspace". */
	void                      makeScreenSpaceEntity();
	
	/*! brief Returns if entity is "screenspace". */
	bool                      isScreenSpaceEntity() const;
	
	/*! \brief Moves this entity to a new world position. */
	void                      setPosition(const tt::math::Vector2& p_position);
	
	/*! \brief Moves this entity to a new world snapped position. */
	void                      setSnappedPosition(const tt::math::Vector2& p_position);
	
	/*! \brief Returns the current world position of this entity. */
	const tt::math::Vector2&  getPosition() const;
	
	/*! \brief Returns the snapped world position of this entity. */
	tt::math::Vector2         getSnappedPosition() const;
	
	/*! \brief Returns the offset needed to go from this entity's position to its center.*/
	tt::math::Vector2         getCenterOffset() const;
	
	/*! \brief Returns the current center position of this entity (within the world). */
	tt::math::Vector2         getCenterPosition() const;
	
	/*! \brief Set the 'down' for this entity on which orientation is based. (default is Direction_Down) */
	void setFloorDirection(movement::Direction p_orientationDown);
	
	/*! \brief Get the 'down' from orientation. */
	movement::Direction getFloorDirection() const;
	
	/*! \brief Checks whether p_surveyResult is part of the current survey result */
	bool hasSurveyResult(movement::SurveyResult p_surveyResult) const;
	
	/*! \brief Checks whether p_surveyResult (with entities orientation applied) is part of the current survey result. */
	bool hasSurveyResultLocal(movement::SurveyResult p_surveyResult) const;
	
	/*! \brief Disable or enable survey updates. (NOTE: Disabling might break other systems when those are used. (movement, sensor, etc.))*/
	void setUpdateSurvey(bool p_enabled);
	
	/*! \brief Is survey updated when needed? (see: setUpdateSurvey) */
	bool shouldUpdateSurvey() const;
	
	/*! \brief Clears the survey results */
	void clearSurvey();
	
	/*! \brief Returns a string with the stand on materialTheme (or 'entity' if entity and 'none' if nothing is found.) */
	std::string getStandOnStr() const;
	
	/*! \brief Set orientation 'flip'. True if forward (as seen from local space (orientation down rotation is already applied)) is left. */
	void setForwardAsLeft(bool p_forwardIsLeft);
	
	/*! \brief Should left be seen are going forward? ('flip') */
	bool isForwardLeft() const;
	
	/*! \brief Transform a local direction based on the entity's orientation. */
	movement::Direction getDirectionFromLocalDir(entity::LocalDir p_localDir) const;
	
	/*! \brief Transform a vector with entity's orientation. */
	tt::math::Vector2 applyOrientationToVector2(const tt::math::Vector2& p_vec) const;
	
	/*! \brief Get local direction based on the entity's orientation from 'world' direction. */
	entity::LocalDir getLocalDirFromDirection(movement::Direction p_worldDir) const;
	
	/*! \brief Transform a vector by removing entity's orientation. */
	tt::math::Vector2 removeOrientationFromVector(const tt::math::Vector2& p_vec) const;
	
	/*! \brief Set the custom debug text that is shown for this entity when 'D' is pressed.
	    \param p_text Text to display. */
	void setDebugText(const std::string& p_text);
	
	/*! \brief Created a presentation file for the entity, so that animations from that file can be played.
	    Note: Presentation Object is not affected by the movement of the entity by default. Use object.setAffectedByMovement to change that.
	    \param p_filename Name of the presentation file to load, without file extension.
	    \return The presentation object */
	PresentationObjectWrapper createPresentationObject(const std::string& p_filename);
	
	/*! \brief Created a presentation file for the entity, so that animations from that file can be played.
	    Note: Presentation Object is not affected by the movement of the entity by default. Use object.setAffectedByMovement to change that.
	    \param p_filename Name of the presentation file to load, without file extension.
	    \param p_layer Layer that this presentation should be displayed in.
	    \return The presentation object */
	PresentationObjectWrapper createPresentationObjectInLayer(const std::string& p_filename,
	                                                          ParticleLayer p_layer);
	
	/*! \brief Destroys a presentation object of this entity
	    \param p_object The presentation object that will be destroyed */
	void destroyPresentationObject(PresentationObjectWrapper& p_object);
	
	/*! \brief Destroys all presentation object of this entity */
	void destroyAllPresentationObjects();
	
	/*! \brief Start all presentation objects with the same name and tags. */
	void startAllPresentationObjects(const std::string& p_name, const tt::str::Strings& p_tagsToStart);
	
	/*! \brief Stops all presentation objects. */
	void stopAllPresentationObjects();
	
	/*! \brief Returns whether a presentation object is part of this entity */
	bool hasPresentationObject(const PresentationObjectWrapper& p_object);
	
	/*! \brief Sets whether the active presentation tags should be displayed near the entity,
	           and changes in tags should be mentioned in the debug output. */
	void setShowPresentationTags(bool p_show);
	
	/*! \brief Indicates whether presentation tags (and their changes) are being displayed. */
	bool shouldShowPresentationTags() const;
	/*! \brief Spawns a one-shot particle effect (lifetime of the effect is managed by the particle manager).
	    \param p_filename               Filename of the particle effect to start, without file extension.
	    \param p_position               Absolute or relative position to spawn the particle effect at (the meaning of this value depends on p_positionIsInWorldSpace).
	    \param p_followEntity           Whether the particle effect should follow the entity by which it was spawned (cannot be used in combination with p_positionIsInWorldSpace).
	    \param p_spawnDelay             By how much to delay the particles in the specified particle effect (adds to the delay specified in the particle file).
	    \param p_positionIsInWorldSpace If true, p_position specifies an absolute world position. If false, p_position is an offset relative to the entity (taking entity orientation into account).
	    \param p_particleLayer          In which particle layer the effect should be rendered. Use ParticleLayer_UseLayerFromParticleEffect to keep the RenderGroup from the particle file.
	    \param p_scale                  The scale of this particle effect. */
	ParticleEffectWrapper spawnParticleOneShot(const std::string&       p_filename,
	                                           const tt::math::Vector2& p_position,
	                                           bool                     p_followEntity,
	                                           real                     p_spawnDelay,
	                                           bool                     p_positionIsInWorldSpace,
	                                           ParticleLayer            p_particleLayer,
	                                           real                     p_scale);
	
	/*! \brief Spawns a continuous particle effect. The script is responsible for managing the lifetime
	           of this effect (by keeping a reference to the object that is returned from this function).
	    \param p_filename               Filename of the particle effect to start, without file extension.
	    \param p_position               Absolute or relative position to spawn the particle effect at (the meaning of this value depends on p_positionIsInWorldSpace).
	    \param p_followEntity           Whether the particle effect should follow the entity by which it was spawned (cannot be used in combination with p_positionIsInWorldSpace).
	    \param p_spawnDelay             By how much to delay the particles in the specified particle effect (adds to the delay specified in the particle file).
	    \param p_positionIsInWorldSpace If true, p_position specifies an absolute world position. If false, p_position is an offset relative to the entity (taking entity orientation into account).
	    \param p_particleLayer          In which particle layer the effect should be rendered. Use ParticleLayer_UseLayerFromParticleEffect to keep the RenderGroup from the particle file.
	    \param p_scale                  The scale of this particle effect. */
	ParticleEffectWrapper spawnParticleContinuous(const std::string&       p_filename,
	                                              const tt::math::Vector2& p_position,
	                                              bool                     p_followEntity,
	                                              real                     p_spawnDelay,
	                                              bool                     p_positionIsInWorldSpace,
	                                              ParticleLayer            p_particleLayer,
	                                              real                     p_scale);
	
	/*! \brief Plays a sound effect. Effect is positional, volume is relative to viewer.
	    \param p_effectName The name of the sound effect (XACT cue) to play.
	    \return SoundCue object that can be used to control the sound effect. */
	SoundCueWrapper playSoundEffect(const std::string& p_effectName);
	
	/*! \brief Plays a sound effect from a soundbank. Effect is positional, volume is relative to viewer.
	    \param p_soundbank The name of the soundbank
	    \param p_effectName The name of the sound effect (XACT cue) to play.
	    \return SoundCue object that can be used to control the sound effect. */
	SoundCueWrapper playSoundEffectFromSoundbank(const std::string& p_soundbank, const std::string& p_effectName);
	
	/*! \brief Set whether this entity is cullable when going outside culling rectangle. */
	void setPositionCullingEnabled(bool p_enabled);
	
	/*! \brief Set whether this entity should be culled if parent is culled. Use null to reset the culling parent. */
	void setPositionCullingParent(const EntityWrapper* p_parent);
	
	/*! \brief Indicates whether this entity has position culling (either itself or its culling parent have position culling enabled). */
	bool isPositionCullingEnabled() const;
	
	/*! \brief Indicates whether this entity is currently being culled (and it or its culling parent lies outside culling rectangle). */
	bool isPositionCulled() const;
	
	/*! \brief Indicates whether this entity is currently on screen or not. */
	bool isOnScreen() const;
	
	/*! \brief Set whether this entity can be pushed by other entities, if this entity is in the way.
	           The default is false: entities cannot be pushed. */
	void setCanBePushed(bool p_canBePushed);
	
	/*! \brief Indicates whether this entity can be pushed by other entities, if this entity is in the way. */
	bool canBePushed() const;
	
	/*! \brief Set whether this entity can be carried by another entity (via carry movement). */
	void setCanBeCarried(bool p_canBeCarried);
	
	/*! \brief Indicates whether this entity can be carried by another entity (via carry movement). */
	bool canBeCarried() const;
	
	/*! \brief Indicates whether this entity is being carried by another entity (via carry movement). */
	bool isBeingCarried() const;
	
	/*! \brief Set whether this entity can be paused (default is true) */
	void setCanBePaused(bool p_canBePaused);
	
	/*! \brief Indicates whether this entity can be paused. */
	bool canBePaused() const;
	
	/*! \brief Create movement controller for this entity.
	    \param p_snapDown If true the entity will snap down to the bottom edge of the tile.
	           (Down is relative to entity orientation.) */
	void createMovementController(bool p_snapDown);
	
	/*! \brief Makes the entity move in the specified direction.
	    \param p_dir The direction to move in.
	    \param p_endDistance How far in the specified direction should be moved. A negative number means "infinite".
	    \return false When starting a movement fails (or is instantly ended.) */
	bool startMovement(movement::Direction p_dir, real p_endDistance);
	
	/*! \brief Stop current movement.
	    \note Can be called without an active movement. */
	void stopMovement();
	
	/*! \brief Start movement to the specified direction.
	    \param p_direction Direction to move to. (Expects normalized vector though it can larger.)
	    \param p_settings Physics settings of the movement
	    \note Unlimited at the moment! All directions are allowed. */
	void startMovementInDirection(const tt::math::Vector2&      p_direction,
	                              const PhysicsSettingsWrapper& p_settings);
	
	/*! \brief Start path movement to the specified position or entity.
	    \param p_positionOrOffset The target position if p_optionalTarget is null, otherwise offset to p_optinalTarget.
	    \param p_settings Physics settings of the movement
	    \param p_optionalTarget Specify entity here to move to it. (position stays updated.)
	                            Specify null to move to position p_positionOrOffset. */
	void startPathMovement(const tt::math::Vector2&      p_positionOrOffset,
	                       const PhysicsSettingsWrapper& p_settings,
	                       const EntityWrapper*          p_optionalTarget);
	
	/*! \brief Start movement to the specified position.
	    \param p_position World position to move to.
	    \param p_mass Mass of the entity. (Used by physics).
	    \param p_drag The drag applied to entity when moving. (-drag * velocitiy).
	    \param p_thrust The strength of the force used to move.
	    \param p_easeOutDistance At what distance should the thrust be lowered. (1.0f is a good minimum!)
	    \param p_moveEndDistance At what distance from the end point the movement is considered ended. (default was: 0.01f)
	    \note Unlimited at the momement! All directions are allowed. */
	void startMovementToPosition(const tt::math::Vector2& p_position,
	                             real p_mass, real p_drag, real p_thrust, real p_easeOutDistance, real p_moveEndDistance);
	
	/*! \brief Start movement to the specified position.
	    \param p_position World position to move to.
	    \param p_settings Physics settings of the movement
	    \note Unlimited at the momement! All directions are allowed. */
	void startMovementToPositionEx(const tt::math::Vector2&      p_position,
	                               const PhysicsSettingsWrapper& p_settings);

	/*! \brief Start movement to the specified entity.
	    \param p_target Entity to move to.
	    \param p_offset Offset from entity position.
	    \param p_mass Mass of the entity. (Used by physics).
	    \param p_drag The drag applied to entity when moving. (-drag * velocitiy).
	    \param p_thrust The strength of the force used to move.
	    \param p_easeOutDistance At what distance should the thrust be lowered. (1.0f is a good minimum!)
	    \param p_moveEndDistance At what distance from the entity the movement is considered ended. (default was: 0.01f)
	    \note Unlimited at the momement! All directions are allowed. */
	void startMovementToEntity(const EntityWrapper* p_target,
	                           const tt::math::Vector2& p_offset,
	                           real p_mass, real p_drag, real p_thrust, real p_easeOutDistance, real p_moveEndDistance);
	
	/*! \brief Start movement to the specified entity.
	    \param p_target Entity to move to.
	    \param p_offset Offset from entity position.
	    \param p_settings Physics settings of the movement.
	    \note Unlimited at the momement! All directions are allowed. */
	void startMovementToEntityEx(const EntityWrapper*          p_target,
	                             const tt::math::Vector2&      p_offset,
	                             const PhysicsSettingsWrapper& p_settings);
	
	/*! \brief Set physics settings. (NOTE: needs Movement Controller.) */
	void setPhysicsSettings(const PhysicsSettingsWrapper& p_settings);
	
	/*! \brief Get copy of physics settings. (NOTE: needs Movement Controller.)*/
	PhysicsSettingsWrapper getPhysicsSettings() const;
	
	/*! \brief Returns the name of the current move. */
	std::string getCurrentMoveName() const;
	
	/*! \brief Get position of another entity (movement parent).
	    \param p_parent Entity which becomes the parent. */
	void setParentEntity(const EntityWrapper* p_parent);
	
	/*! \brief Get position of another entity (movement parent) with an offset.
	    \param p_parent Entity which becomes the parent.
	    \param p_offset The offset which is applied to the parent position.*/
	void setParentEntityWithOffset(const EntityWrapper*     p_parent,
	                               const tt::math::Vector2& p_offset);
	
	/*! \brief Retrieves the current parent of this entity. */
	EntityBase* getParentEntity() const;
	
	/*! \brief Reset (movement) parent. */
	void resetParentEntity();
	
	/*! \brief Use parent's forwardIsLeft setting for own forwardIsLeft. */
	void setUseParentForward(bool p_useForward);
	
	/*! \brief Should this entity use parent's forwardIsLeft setting as its own forwardIsLeft. */
	bool hasUseParentForward() const;
	
	/*! \brief Use parent's FloorDirection setting for own FloorDirection. */
	void setUseParentFloorDirection(bool p_useFloorDirection);
	
	/*! \brief Should this entity use parent's FloorDirection setting as its own FloorDirection. */
	bool hasUseParentFloorDirection() const;
	
	/*! \brief Change the movementset.
	    \note Entity needs to have movement. */
	void setMovementSet(const std::string& p_name);
	
	/*! \brief Returns the current direction of the entity's movement. */
	movement::Direction getDirection() const;
	
	/*! \brief Returns the direction of the current movement/speed.
	    \note Try to use actual movement speed to determine the direction, 
	          other wise fallback to requested dir. */
	movement::Direction getActualMovementDirection() const;
	
	/*! \brief Sets the current speed of the entity.
	           \return if speed could be set successfully */
	bool setSpeed(const tt::math::Vector2& p_speed);
	
	/*! \brief Returns the current speed of the entity. */
	const tt::math::Vector2& getSpeed() const;
	
	/*! \brief Sets the external force of the entity.
	           \return if force could be set successfully */
	bool setExternalForce(const tt::math::Vector2& p_force);
	
	/*! \brief Returns the current external force of the entity. */
	const tt::math::Vector2& getExternalForce() const;
	
	/*! \brief Set the speed relative to this reference entity. */
	bool setReferenceSpeedEntity(const EntityWrapper* p_entity);
	
	/*! \brief Set the speed factor for this entity. */
	void setSpeedFactor(const tt::math::Vector2& p_factor);
	
	/*! \brief Get the speed factor for this entity. */
	const tt::math::Vector2& getSpeedFactor() const;
	
	/*! \brief Set the X speed factor for this entity. */
	void setSpeedFactorX(real p_x);
	
	/*! \brief Set the Y speed factor for this entity. */
	void setSpeedFactorY(real p_y);
	
	/*! \brief Get the X speed factor for this entity. */
	real getSpeedFactorX() const;
	
	/*! \brief Get the Y speed factor for this entity. */
	real getSpeedFactorY() const;
	
	/*! \brief Returns if the current move is interruptable move. */
	bool isCurrentMoveInterruptible() const;
	
	/*! \brief Optimization helper: opdates this entity's position based on the stickto params
	    \param p_parent [Entity] The stickto parent entity (cannot be null)
	    \param p_offset [Vector2] The stickto offset
	    \returns the updated stickto entity (can be set to null if it wasn't valid)
	*/
	int updateStickToPosition(HSQUIRRELVM v);
	
	/*! \brief Adds a sight sensor for this entity in local space
	    \param p_shape [Shape] The sensor shape (can be null)
	    \param p_target [Entity] The sensor target (optional; can be null)
	    \param p_position [Vector2] The sensor offset (optional)
	    \return The sensor object */
	int addSightSensor(HSQUIRRELVM v);
	
	/*! \brief Adds a sight sensor for this entity in world space
	    \param p_shape The sensor shape (can be null)
	    \param p_target The sensor target (can be null)
	    \param p_position The sensor position in world space
	    \return The sensor object */
	SensorWrapper addSightSensorWorldSpace(const ShapeWrapper* p_shape, const EntityWrapper* p_target,
	                                       const tt::math::Vector2& p_position);
	
	/*! \brief Adds a touch sensor for this entity in local space
	    \param p_shape [Shape] The sensor shape (can be null)
	    \param p_target [Entity] The sensor target (optional; can be null)
	    \param p_position [Vector2] The sensor offset (optional)
	    \return The sensor object */
	int addTouchSensor(HSQUIRRELVM v);
	
	/*! \brief Adds a touch sensor for this entity in world space
	    \param p_shape The sensor shape (must NOT be null)
	    \param p_target The sensor target (can be null)
	    \param p_position The sensor position in world space
	    \return The sensor object */
	SensorWrapper addTouchSensorWorldSpace(const ShapeWrapper* p_shape, const EntityWrapper* p_target,
	                                       const tt::math::Vector2& p_position);
	
	/*! \brief Adds a tile sensor for this entity in local space.
	    \param p_shape [Shape] The sensor shape. (Can be null.)
	    \param p_position [Vector2] The sensor offset (optional).
	    \return The TileSensor object. */
	int addTileSensor(HSQUIRRELVM v);
	
	/*! \brief Adds a tile sensor for this entity in world space.
	    \param p_shape The sensor shape. (Must NOT be null!)
	    \param p_position The sensor position in world space.
	    \return The TileSensor object. */
	TileSensorWrapper addTileSensorWorldSpace(const ShapeWrapper* p_shape, const tt::math::Vector2& p_position);
	
	/*! \brief Removes a specific sensor for this entity
	    \param p_sensor the sensor */
	void removeSensor(SensorWrapper* p_sensor);
	
	/*! \brief Removes a specific tile sensor for this entity
	    \param p_sensor the sensor */
	void removeTileSensor(TileSensorWrapper* p_sensor);
	
	/*! \brief Removes all sensors for this entity */
	void removeAllSensors();
	
	/*! \brief Removes all sight sensors for this entity */
	void removeAllSightSensors();
	
	/*! \brief Removes all touch sensors for this entity */
	void removeAllTouchSensors();

	/*! \brief Removes all tile sensors from this entity. */
	void removeAllTileSensors();
	
	/*! \brief Forces all sensors which have filtered this entity to recheck it. */
	void recheckSensorFilter();
	
	/*! \brief Sets the state for the entity.
	    \param p_newState Name of the state to change to. */
	void setState(const std::string& p_newState);
	
	/*! \brief Returns the name of the current state the entity is in. */
	std::string getState() const;
	
	/*! \brief Returns the name of the previous state the entity was in. */
	std::string getPreviousState() const;
	
	/*! \brief Returns the name of this entity's type (i.e. script class). */
	std::string getType() const;
	
	/*! \brief Indicates whether this entity is valid (safe to use). */
	inline bool isValid() const { return m_handle.isEmpty() == false; }
	
	/*! \brief Indicates whether this entity is initialized.*/
	bool isInitialized() const;
	
	/*! \brief Indicates whether this entity is dying. */
	bool isDying() const;
	
	/*! \brief Retrieves the unique handle (used in C++) for this entity */
	inline s32 getHandleValue() const { return m_handle.getValue(); }
	
	/*! \brief Retrieves the ID, as displayed in the editor, for this entity.
	           Note: returns -1 for entities that weren't placed in the editor (e.g., spawned entities) */
	s32 getID() const;
	
	/*! \brief Trigger a warning in the editor for this entity.
	    \param p_warningStr string with the warning. */
	void editorWarning(const std::string& p_warningStr);
	
	/*! \brief Registers this entity using a tag name.
	    \param p_tag Tag name to register this entity with. */
	void registerEntityByTag(const std::string& p_tag);
	
	/*! \brief Unregisters this entity using a tag name.
	    \param p_tag Tag name to unregister */
	void unregisterEntityByTag(const std::string& p_tag);
	
	/*! \brief Returns a string array of all the tags registered with this entity. */
	const tt::str::Strings& getTags() const;
	
	/*! \brief Sets collision tiles for the entity.
	    \param p_tiles Tiles in a format generated by Copy in the in-game editor. */
	void setEntityCollisionTiles(const std::string& p_tiles);
	
	/*! \brief Sets whether the entity collision tiles are marked as "active" (certain sight can see through them). */
	void setEntityCollisionTilesActive(bool p_active);
	
	/*! \brief Indicates whether the entity collision tiles are marked as "active". */
	bool areEntityCollisionTilesActive() const;
	
	/*! \brief Removes the entity's collision tiles. */
	void removeEntityCollisionTiles();
	
	/*! \brief Set the (tile) offset which is applied to EntityCollisionTiles. */
	void setEntityCollisionTilesOffset(s32 p_x, s32 p_y);
	
	/*! \brief Sets the entity's collision rectangle.
	    \param p_centerPos The center position of the rectangle (relative to the entity position).
	    \param p_width The width of the rectangle.
	    \param p_height The height of the rectangle. */
	void setCollisionRect(const tt::math::Vector2& p_centerPos, real p_width, real p_height);
	
	/*! \brief Sets the entity's collision rectangle using a VectorRect.
	    \param p_rect The rectangle to set. */
	void setCollisionRectWithVectorRect(const tt::math::VectorRect& p_rect);
	
	/*! \brief Change the collision rect and collsion tiles for the entity. */
	void setEntityCollisionRectAndTiles(s32 p_width, s32 p_height, level::CollisionType p_collisionType);
	
	/*! \brief Returns the entity's current collision rectangle (local space). */
	tt::math::VectorRect getCollisionRect() const;
	
	/*! \brief Returns the entity's current collision rectangle (world space). */
	tt::math::VectorRect getCollisionRectWorldSpace() const;
	
	// Martijn: not needed for RIVE
	/*! \brief Spawn event for this entity
	    \param p_type the type of the Event
	    \param p_worldPosition the position in the world
	    \param p_radius the radius of the event
	void spawnEvent(event::EventType p_type, const tt::math::Vector2& p_worldPosition,
	                real p_radius);
	*/
	
	/*! \brief Spawn event for this entity.
	    \param p_type the type of the Event.
	    \param p_worldPosition the position in the world.
	    \param p_radius the radius of the event.
	    \param p_userParam string which is passed to callback 
	void spawnEventEx(event::EventType p_type, const tt::math::Vector2& p_worldPosition,
	                  real p_radius, const std::string& p_userParam);
	*/
	
	/*! \brief Adds a timer for this entity. Overwrites (and warns) if timer already exists */
	void startTimer(const std::string& p_name, real p_timeout) const;
	
	/*! \brief Adds a callback timer for this entity. Overwrites (and warns) if timer already exists.
	           A callback timer fires a callback p_callback without arguments */
	void startCallbackTimer(const std::string& p_callback, real p_timeout) const;
	
	/*! \brief Removes a timer for this entity */
	void stopTimer(const std::string& p_name) const;
	
	/*! \brief Removes all timers for this entity */
	void stopAllTimers() const;
	
	/*! \brief Suspends a timer for this entity */
	void suspendTimer(const std::string& p_name) const;
	
	/*! \brief Suspends all timers for this entity */
	void suspendAllTimers() const;
	
	/*! \brief Resumes a timer for this entity */
	void resumeTimer(const std::string& p_name) const;
	
	/*! \brief Resumes all timers for this entity */
	void resumeAllTimers() const;
	
	/*! \brief Returns whether a timer with p_name exists for this entity */
	bool hasTimer(const std::string& p_name) const;
	
	/*! \brief Returns the remaining time for a timer with name p_name, returns -1 if timer doesn't exist */
	real getTimerTimeout(const std::string& p_name) const;
	
	/*! \brief Sets how many tiles (in height) of the entity should be underwater before the entity starts to float. */
	void setSubmergeDepth(s32 p_depthInTiles);
	
	/*! \brief Returns how many tiles (in height) of the entity should be underwater before the entity starts to float. */
	s32 getSubmergeDepth() const;
	
	/*! \brief Enables the tile registration for this entity (default is enabled) */
	void enableTileRegistration();
	
	/*! \brief Disable the tile registration for this entity. Use this for markers and collectibles etc. */
	void disableTileRegistration();
	
	/*! \brief Call a custom callback
		\param Callback [string] 
		\param Parameters Variable parameters list */
	int customCallback(HSQUIRRELVM v);
	
	/* \brief Remove all queued callbacks. NOTE: Very lowlevel feature, should probably be removed in future*/
	void removeQueuedCallbacks();
	
	/*! \brief Adds a effect rect to this entity.
	    \param p_targetType Against which type of target should the rect check it's distance. */
	EffectRectWrapper addEffectRect(entity::effect::EffectRectTarget p_targetType);
	
	/*! \brief Remove an effect rect from this entity. */
	void removeEffectRect(EffectRectWrapper& p_effectRect);
	
	/*! \brief Adds a powerbeam graphic for a sensor to this entity.
	           NOTE: Sensor must be a sight sensor with a target set.
	    \param p_type   The type of graphic to create.
	    \param p_sensor The sensor that this powerbeam graphic should apply to */
	PowerBeamGraphicWrapper addPowerBeamGraphic(
			entity::graphics::PowerBeamType p_type,
			const SensorWrapper&            p_sensor);
	
	/*! \brief Removes a powerbeam graphic from this entity. */
	void removePowerBeamGraphic(PowerBeamGraphicWrapper& p_graphic);
	
	/*! \brief Removes all powerbeam graphics from this entity. */
	void removeAllPowerBeamGraphics();
	
	/*! \brief Adds a Text Label to this entity.
	    \param p_localizationKey The key for the localization text that should be used (Maybe empty).
	    \param p_width  The width in world units (1.0 is tile size.)
	    \param p_height The height in world units (1.0 is tile size.)
	    \param p_glyphSetId Which glyph set should be used. */
	TextLabelWrapper addTextLabel(const std::string& p_localizationKey,
	                              real p_width, real p_height,
	                              utils::GlyphSetID p_glyphSetId);
	
	/*! \brief Removes a Text Label from this entity. */
	void removeTextLabel(TextLabelWrapper p_graphic);
	
	/*! \brief Removes all Text Labels from this entity. */
	void removeAllTextLabels();
	
	/*! \brief Adds a light for this entity
	    \param p_offset The light offset
	    \param p_radius The light radius
	    \return The light object */
	LightWrapper addLight(const tt::math::Vector2& p_offset, real p_radius, real p_strength);
	
	/*! \brief Removes a specific light for this entity
	    \param p_light the light */
	void removeLight(LightWrapper* p_light);
	
	/*! \brief Removes all lights for this entity */
	void removeAllLights();
	
	/*! \brief Adds a darkness for this entity
	    \param p_width The width of the darkness area
	    \param p_height The height of the darkness area
	    \return The light object */
	DarknessWrapper addDarkness(real p_width, real p_height);
	
	/*! \brief Removes a specific darkness for this entity
	    \param p_light the light */
	void removeDarkness(DarknessWrapper* p_darkness);
	
	/*! \brief Removes all darknesses for this entity */
	void removeAllDarknesses();
	
	/*! \brief Set whether or not this entity can be detected by light and should receive onLightEnter/Exit callbacks */
	void setDetectableByLight(bool p_enabled);
	
	/*! \brief Indicates whether or not this entity is affected by light and should receive onLightEnter/Exit callbacks */
	bool isDetectableByLight() const;
	
	/*! \brief Indicates whether or not this entity is in light. Returns false if an entity is not affected by light */
	bool isInLight() const;
	
	/*! \brief Sets whether this entity blocks light or not */
	void setLightBlocking(bool p_enabled);
	
	/*! \brief Indicates whether this entity blocks light */
	bool isLightBlocking() const;
	
	/*! \brief Sets the vibration detection points for this entity.
	    \param Detection points [array of Vector2]. Offset from center position. Uses entity orientation. */
	int setVibrationDetectionPoints(HSQUIRRELVM v);
	
	/*! \brief Removes all vibration detection points for this entity. */
	void removeAllVibrationDetectionPoints();
	
	/*! \brief Set whether or not this entity can be detected by sight sensors of other entities */
	void setDetectableBySight(bool p_enabled);
	
	/*! \brief Indicates whether or not this entity can be detected by sight sensors of other entities */
	bool isDetectableBySight() const;
	
	/*! \brief Set whether or not this entity can be detected by touch sensors of other entities */
	void setDetectableByTouch(bool p_enabled);
	
	/*! \brief Indicates whether or not this entity can be detected by touch sensors of other entities */
	bool isDetectableByTouch() const;
	
	/*! \brief Sets the sight detection points for this entity
		\param Sight detection points [array of Vector2] */
	int setSightDetectionPoints(HSQUIRRELVM v);
	
	/*! \brief Removes all sight detection points for this entity. You can use setDetectableBySight(false) for this as well */
	void removeAllSightDetectionPoints();
	
	/*! \brief Sets the light detection points for this entity
		\param Light detection points points [array of Vector2] */
	int setLightDetectionPoints(HSQUIRRELVM v);
	
	/*! \brief Removes all light detection points for this entity. You can use setDetectableByLight(false) for this as well */
	void removeAllLightDetectionPoints();
	
	/*! \brief Sets the touch shape for this entity */
	void setTouchShape(const ShapeWrapper* p_shape, const tt::math::Vector2& p_offset);
	
	/*! \brief Removes the touch shape for this entity. You can use setDetectableByTouch(false) for this as well */
	void removeTouchShape();
	
	/*! \brief Adds fluid settings of specific type for this entity */
	FluidSettingsWrapper addFluidSettings(fluid::FluidType p_type);
	
	/*! \brief Removes fluid settings of specific type for this entity */
	void removeFluidSettings(fluid::FluidType p_type);
	
	/*! \brief Gets fluid settings of specific type for this entity. If the fluidsettings haven't been added, it will trigger an assert */
	FluidSettingsWrapper getFluidSettings(fluid::FluidType p_type) const;
	
	/*! \brief Gets all sensors for this entity. */
	SensorWrappers getAllSensors() const;
	
	/*! \brief Returns the weakreference of this Entity */
	int weakref(HSQUIRRELVM v);
	
	static void bind(const tt::script::VirtualMachinePtr& p_vm);
	
	/*! \brief Check whether two entities are the same instance (use this instead of '==' or '!='). */
	inline bool equals(const EntityWrapper* p_rhs) const
	{
		return p_rhs != 0 && m_handle == p_rhs->m_handle;
	}
	
private:
	explicit EntityWrapper(entity::EntityHandle p_handle = entity::EntityHandle());
	
	PresentationObjectWrapper createPresentationObjectImpl(const std::string& p_filename,
	                                                       const tt::pres::Tags& p_tags,
	                                                       ParticleLayer p_layer);
	
	SensorWrapper addSensor(entity::sensor::SensorType p_type, const entity::sensor::ShapePtr& p_shape,
	                        const entity::EntityHandle& p_target);
	int addSensor(entity::sensor::SensorType p_type, HSQUIRRELVM v);
	SensorWrapper addSensorWorldSpace(entity::sensor::SensorType p_type,
	                                  const ShapeWrapper* p_shape, const EntityWrapper* p_target,
	                                  const tt::math::Vector2& p_position);
	
	inline const entity::Entity* getEntity() const
	{
		return m_handle.getPtr();
	}
	
	entity::Entity* getModifiableEntity();
	const entity::movementcontroller::DirectionalMovementController* getMovementController() const;
	      entity::movementcontroller::DirectionalMovementController* getMovementController();
	
	entity::EntityHandle m_handle;
	
	friend class toki::game::script::EntityBase;
};


// Namespace end
}
}
}
}


#endif  // !defined(INC_TOKI_GAME_SCRIPT_WRAPPERS_ENTITYWRAPPER_H)
