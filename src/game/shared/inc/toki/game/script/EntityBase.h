#if !defined(INC_TOKI_GAME_SCRIPT_ENTITYBASE_H)
#define INC_TOKI_GAME_SCRIPT_ENTITYBASE_H

#include <string>

#include <tt/code/fwd.h>
#include <tt/script/helpers.h>
#include <tt/platform/tt_error.h>

#include <toki/level/entity/EntityInstance.h>
#include <toki/game/event/input/fwd.h>
#include <toki/game/event/fwd.h>
#include <toki/game/script/Callback.h>
#include <toki/game/script/EntityScriptMgr.h>
#include <toki/game/script/EntityState.h>
#include <toki/game/script/wrappers/EntityWrapper.h>
#include <toki/game/script/wrappers/PointerEventWrapper.h>
#include <toki/game/script/wrappers/TileSensorWrapper.h>
#include <toki/game/script/wrappers/PresentationObjectWrapper.h>
#include <toki/script/serialization/fwd.h>

#if defined(TT_BUILD_FINAL)
#	define DEBUG_SQUIRREL_ENTITY_BASE_LIFETIME 0 // Needs to be 0 for final builds!
#else
#	define DEBUG_SQUIRREL_ENTITY_BASE_LIFETIME 0 // Change to 1 to debug EntityBase Squirrel instance's lifetime.
#endif


namespace toki {
namespace game {
namespace script {

/*! \brief Base class for all entities (in Squirrel).
           This class defines all functions that can be called from C++ to Squirrel (callbacks etc). */
class EntityBase
{
public:
	// Creation with onCreate(p_id) callback to let script determine whether or not to create this entity
	static EntityBasePtr create(const entity::EntityHandle& p_handle, const std::string& p_type, s32 p_id);
	~EntityBase();
	
	void init(game::entity::EntityProperties& p_properties, bool p_gameReloaded);
	void init(const HSQOBJECT& p_properties);
	void deinit();
	inline bool                        isInitialized() const { return m_currentState.isValid(); }
	inline const entity::EntityHandle& getHandle()     const { return m_wrapper.getHandle();    }
	
	inline void updateCallbacks() { updateCallbacks(1); }
	
	// callbacks
	
	/*! \brief Called right after the entity has been created (before onInit).
	    \param The editor ID of this entity. ID is -1 if the entity is spawned from script. NOTE: you CANNOT use getID() at this point,
	    \return Returns whether or not this entity should be created. If callback is not implemented, default is true.
	           NOTE: onCreate is non-deferred, i.e., it will be called immediately after entity is created */
	bool onCreate(s32 p_id);
	
	/*! \brief Called right after the entity has been initialized (before onSpawn).
	           NOTE: onInit is non-deferred, i.e., it will be called immediately after entity is initialized */
	inline void onInit()
	{
		callSqFun("onInit"); 
	}
	
	/*! \brief Called when this entity dies.
	           NOTE: onDie will flush the callbacks of this entity and will call onDie immediately after flusing the callbacks. */
	void onDie();
	
	/*! \brief Called when the entity is spawned. */
	inline void onSpawn()
	{
		queueSqFun("onSpawn");
		
		// Update so the callbacks deferred in onInit, onSpawn and possible enterState are flushed.
		updateCallbacks();
	}
	
	/*! \brief Called when the entity hears sound. */
	void onSound(const toki::game::event::Event& p_event) const;
	
	/*! \brief Called when the entity feels vibration. */
	void onVibration(const toki::game::event::Event& p_event) const;
	
	/*! \brief Called when the entity enters light. */
	inline void onLightEnter() const { queueSqFun("onLightEnter"); }
	
	/*! \brief Called when the entity exits light. */
	inline void onLightExit() const { queueSqFun("onLightExit"); }
	
	/*! \brief Called on Event source when event is spawned through spawnEventEx.
	    \param p_event the even that was spawned.
	    \param p_userParam the user parameter which was given with the spawn call.
	    \param p_entitiesFound All the entities on which the event had effect. */
	void onEventSpawned(const toki::game::event::Event& p_event, const std::string& p_userParam,
	                    const EntityBaseCollection& p_entitiesFound);
	
	
	/*! \brief Called when tile sensor is touching solid tiles. */
	inline void onTileSensorSolidTouchEnter(const entity::sensor::TileSensorHandle& p_tileSensor) const    { queueSqFun("onTileSensorSolidTouchEnter", wrappers::TileSensorWrapper(p_tileSensor)); }
	
	/*! \brief Called when tile sensor is no longer touching solid tiles. */
	inline void onTileSensorSolidTouchExit(const entity::sensor::TileSensorHandle& p_tileSensor)  const    { queueSqFun("onTileSensorSolidTouchExit", wrappers::TileSensorWrapper(p_tileSensor)); }
	
	/*! \brief Called when tile sensor is touching water tiles. */
	inline void onTileSensorWaterTouchEnter(const entity::sensor::TileSensorHandle& p_tileSensor) const      { queueSqFun("onTileSensorWaterTouchEnter", wrappers::TileSensorWrapper(p_tileSensor)); }
	
	/*! \brief Called when tile sensor is no longer touching water tiles. */
	inline void onTileSensorWaterTouchExit(const entity::sensor::TileSensorHandle& p_tileSensor) const       { queueSqFun("onTileSensorWaterTouchExit", wrappers::TileSensorWrapper(p_tileSensor)); }
	
	/*! \brief Called when tile sensor is touching waterfall water tiles. */
	inline void onTileSensorWaterfallTouchEnter(const entity::sensor::TileSensorHandle& p_tileSensor) const  { queueSqFun("onTileSensorWaterfallTouchEnter", wrappers::TileSensorWrapper(p_tileSensor)); }
	
	/*! \brief Called when tile sensor is no longer touching waterfall water tiles. */
	inline void onTileSensorWaterfallTouchExit(const entity::sensor::TileSensorHandle& p_tileSensor) const   { queueSqFun("onTileSensorWaterfallTouchExit", wrappers::TileSensorWrapper(p_tileSensor)); }
	
	/*! \brief Called when tile sensor is touching lava tiles. */
	inline void onTileSensorLavaTouchEnter(const entity::sensor::TileSensorHandle& p_tileSensor) const { queueSqFun("onTileSensorLavaTouchEnter", wrappers::TileSensorWrapper(p_tileSensor)); }
	
	/*! \brief Called when tile sensor is no longer touching lava tiles. */
	inline void onTileSensorLavaTouchExit(const entity::sensor::TileSensorHandle& p_tileSensor) const { queueSqFun("onTileSensorLavaTouchExit", wrappers::TileSensorWrapper(p_tileSensor)); }
	
	/*! \brief Called when tile sensor is touching waterfall lava tiles. */
	inline void onTileSensorLavafallTouchEnter(const entity::sensor::TileSensorHandle& p_tileSensor) const { queueSqFun("onTileSensorLavafallTouchEnter", wrappers::TileSensorWrapper(p_tileSensor)); }
	
	/*! \brief Called when tile sensor is no longer touching waterfall lava tiles. */
	inline void onTileSensorLavafallTouchExit(const entity::sensor::TileSensorHandle& p_tileSensor) const { queueSqFun("onTileSensorLavafallTouchExit", wrappers::TileSensorWrapper(p_tileSensor)); }
	
	/*! \brief Called when touching water tiles. */
	inline void onWaterTouchEnter() const      { queueSqFun("onWaterTouchEnter");        }
	
	/*! \brief Called when no longer touching water tiles. */
	inline void onWaterTouchExit() const       { queueSqFun("onWaterTouchExit");         }
	
	/*! \brief Called when completely inside water tiles. */
	inline void onWaterEnclosedEnter() const   { queueSqFun("onWaterEnclosedEnter");     }
	
	/*! \brief Called when no longer completely inside water tiles. */
	inline void onWaterEnclosedExit() const    { queueSqFun("onWaterEnclosedExit");      }
	
	/*! \brief Called when touching waterfall water tiles. */
	inline void onWaterfallTouchEnter() const  { queueSqFun("onWaterfallTouchEnter");    }
	
	/*! \brief Called when no longer touching waterfall water tiles. */
	inline void onWaterfallTouchExit() const   { queueSqFun("onWaterfallTouchExit");     }
	
	/*! \brief Called when completely inside waterfall water tiles. */
	inline void onWaterfallEnclosedEnter() const { queueSqFun("onWaterfallEnclosedEnter"); }
	
	/*! \brief Called when no longer completely inside waterfall water tiles. */
	inline void onWaterfallEnclosedExit() const { queueSqFun("onWaterfallEnclosedExit");  }
	
	/*! \brief Called when touching lava tiles. */
	inline void onLavaTouchEnter() const { queueSqFun("onLavaTouchEnter");         }
	
	/*! \brief Called when no longer touching lava tiles. */
	inline void onLavaTouchExit() const { queueSqFun("onLavaTouchExit");          }
	
	/*! \brief Called when completely inside lava tiles. */
	inline void onLavaEnclosedEnter() const { queueSqFun("onLavaEnclosedEnter");      }
	
	/*! \brief Called when no longer completely inside lava tiles. */
	inline void onLavaEnclosedExit() const { queueSqFun("onLavaEnclosedExit");       }
	
	/*! \brief Called when touching waterfall lava tiles. */
	inline void onLavafallTouchEnter() const { queueSqFun("onLavafallTouchEnter");     }
	
	/*! \brief Called when no longer touching waterfall lava tiles. */
	inline void onLavafallTouchExit() const { queueSqFun("onLavafallTouchExit");      }
	
	/*! \brief Called when completely inside waterfall lava tiles. */
	inline void onLavafallEnclosedEnter() const { queueSqFun("onLavafallEnclosedEnter");  }
	
	/*! \brief Called when no longer completely inside waterfall lava tiles. */
	inline void onLavafallEnclosedExit() const { queueSqFun("onLavafallEnclosedExit");   }
	
	/*! \brief Called when a timer with the specified name is triggered.
	    \param p_name Name of the timer that was triggered (was specified in startTimer). */
	inline void onTimer(const std::string& p_name) const { callSqFun("onTimer", p_name);   }
	
	/*! \brief Called when this entity's movement has completed.
	    \param p_direction The direction in which the entity was moving. */
	inline void onMovementEnded( movement::Direction p_direction) { queueSqFun("onMovementEnded",  p_direction); }
	
	/*! \brief Called when this entity's movement could not be completed.
	    \param p_direction The direction in which the entity was moving.
	    \param p_moveName The name of the move that failed.*/
	inline void onMovementFailed(movement::Direction p_direction, const std::string& p_moveName) { queueSqFun("onMovementFailed", p_direction, p_moveName); }
	
	/*! \brief Called when this entity's path movement couldn't find a path.
	    \param p_closestPoint is the closest point to which which it could move with a path. */
	inline void onPathMovementFailed(const tt::math::Vector2& p_closestPoint) { queueSqFun("onPathMovementFailed", p_closestPoint); }
	
	/*! \brief Called when this entity hits solid collision (e.g., a wall)
	    \param p_collisionNormal is the normal of the collision.
	    \param p_speed is the impact speed. */
	inline void onSolidCollision(const tt::math::Vector2& p_collisionNormal, const tt::math::Vector2& p_speed)
	{
		queueSqFun("onSolidCollision", p_collisionNormal, p_speed);
	}
	
	/*! \brief Called when this entity's physics movement controller turned it. */
	inline void onPhysicsTurn() { queueSqFun("onPhysicsTurn"); }
	
	/*! \brief Called when entering the current state. */
	inline void onEnterState() { queueSqFun("onEnterState"); }
	
	/*! \brief Called when exiting the current state. */
	inline void onExitState() { queueSqFun("onExitState"); }
	
	/*! \brief Called when a pointer input device presses on this entity. */
	inline void onPointerPressed(const wrappers::PointerEventWrapper& p_event) const
	{
		queueSqFun("onPointerPressed", p_event);
	}
	
	/*! \brief Called when a pointer input device press is released on this entity. */
	inline void onPointerReleased(const wrappers::PointerEventWrapper& p_event) const
	{
		queueSqFun("onPointerReleased", p_event);
	}
	
	/*! \brief Called when input direction is changed.
	    \note This is the input direction unrelated to the movement direction in the movement controller.*/
	inline void onDirectionChanged(movement::Direction p_direction)
	{
		queueSqFun("onDirectionChanged", p_direction);
	}
	
	/*! \brief Called when the down button is pressed on an input device. */
	inline bool onButtonDownPressed() const
	{
		return handleButtonCallbackPressed("onButtonDownPressed");
	}
	
	/*! \brief Called when the down button is released on an input device.
	    \param p_pressDuration For how long the button was pressed, in seconds. */
	inline bool onButtonDownReleased(real p_pressDuration) const
	{
		return handleButtonCallbackReleased("onButtonDownReleased", p_pressDuration);
	}
	
	/*! \brief Called when the right button is pressed on an input device. */
	inline bool onButtonRightPressed() const
	{
		return handleButtonCallbackPressed("onButtonRightPressed");
	}
	
	/*! \brief Called when the Right button is released on an input device.
	    \param p_pressDuration For how long the button was pressed, in seconds. */
	inline bool onButtonRightReleased(real p_pressDuration) const
	{
		return handleButtonCallbackReleased("onButtonRightReleased", p_pressDuration);
	}
	
	/*! \brief Called when the up button is pressed on an input device. */
	inline bool onButtonUpPressed() const
	{
		return handleButtonCallbackPressed("onButtonUpPressed");
	}
	
	/*! \brief Called when the Up button is released on an input device.
	    \param p_pressDuration For how long the button was pressed, in seconds. */
	inline bool onButtonUpReleased(real p_pressDuration) const
	{
		return handleButtonCallbackReleased("onButtonUpReleased", p_pressDuration);
	}
	
	/*! \brief Called when the left button is pressed on an input device. */
	inline bool onButtonLeftPressed() const 
	{
		return handleButtonCallbackPressed("onButtonLeftPressed");
	}
	
	/*! \brief Called when the Left button is released on an input device.
	    \param p_pressDuration For how long the button was pressed, in seconds. */
	inline bool onButtonLeftReleased(real p_pressDuration) const
	{
		return handleButtonCallbackReleased("onButtonLeftReleased", p_pressDuration);
	}
	
	/*! \brief Called when the VirusUpload button is pressed on an input device. */
	inline bool onButtonVirusUploadPressed() const
	{
		return handleButtonCallbackPressed("onButtonVirusUploadPressed");
	}
	
	/*! \brief Called when the VirusUpload button is released on an input device.
	    \param p_pressDuration For how long the button was pressed, in seconds. */
	inline bool onButtonVirusUploadReleased(real p_pressDuration) const
	{
		return handleButtonCallbackReleased("onButtonVirusUploadReleased", p_pressDuration);
	}
	
	/*! \brief Called when the DemoReset is pressed on an input device. */
	inline bool onButtonDemoResetPressed() const
	{
		return handleButtonCallbackPressed("onButtonDemoResetPressed");
	}
	
	/*! \brief Called when the DemoReset button is released on an input device.
	    \param p_pressDuration For how long the button was pressed, in seconds. */
	inline bool onButtonDemoResetReleased(real p_pressDuration) const
	{
		return handleButtonCallbackReleased("onButtonDemoResetReleased", p_pressDuration);
	}
	
	/*! \brief Called when the Respawn button is pressed on an input device. */
	inline bool onButtonRespawnPressed() const
	{
		return handleButtonCallbackPressed("onButtonRespawnPressed");
	}
	
	/*! \brief Called when the Respawn button is released on an input device.
	    \param p_pressDuration For how long the button was pressed, in seconds. */
	inline bool onButtonRespawnReleased(real p_pressDuration) const
	{
		return handleButtonCallbackReleased("onButtonRespawnReleased", p_pressDuration);
	}
	
	/*! \brief Called when the menu button is pressed on an input device. */
	inline bool onButtonMenuPressed() const
	{
		return handleButtonCallbackPressed("onButtonMenuPressed");
	}
	
	/*! \brief Called when the menu button is released on an input device.
	    \param p_pressDuration For how long the button was pressed, in seconds. */
	inline bool onButtonMenuReleased(real p_pressDuration) const
	{
		return handleButtonCallbackReleased("onButtonMenuReleased", p_pressDuration);
	}

	/*! \brief Called when the screen switch button is pressed on an input device. */
	inline bool onButtonScreenSwitchPressed() const
	{
		return handleButtonCallbackPressed("onButtonScreenSwitchPressed");
	}
	
	/*! \brief Called when the screen switch button is released on an input device.
	    \param p_pressDuration For how long the button was pressed, in seconds. */
	inline bool onButtonScreenSwitchReleased(real p_pressDuration) const
	{
		return handleButtonCallbackReleased("onButtonScreenSwitchReleased", p_pressDuration);
	}

	/*! \brief Called when the accept button is pressed on an input device. */
	inline bool onButtonAcceptPressed() const
	{
		return handleButtonCallbackPressed("onButtonAcceptPressed");
	}
	
	/*! \brief Called when the accept button is released on an input device.
	    \param p_pressDuration For how long the button was pressed, in seconds. */
	inline bool onButtonAcceptReleased(real p_pressDuration) const
	{
		return handleButtonCallbackReleased("onButtonAcceptReleased", p_pressDuration);
	}
	
	/*! \brief Called when the cancel button is pressed on an input device. */
	inline bool onButtonCancelPressed() const
	{
		return handleButtonCallbackPressed("onButtonCancelPressed");
	}
	
	/*! \brief Called when the cancel button is released on an input device.
	    \param p_pressDuration For how long the button was pressed, in seconds. */
	inline bool onButtonCancelReleased(real p_pressDuration) const
	{
		return handleButtonCallbackReleased("onButtonCancelReleased", p_pressDuration);
	}
	
	/*! \brief Called when the faceup button is pressed on an input device. */
	inline bool onButtonFaceUpPressed() const
	{
		return handleButtonCallbackPressed("onButtonFaceUpPressed");
	}
	
	/*! \brief Called when the faceup button is released on an input device.
	    \param p_pressDuration For how long the button was pressed, in seconds. */
	inline bool onButtonFaceUpReleased(real p_pressDuration) const
	{
		return handleButtonCallbackReleased("onButtonFaceUpReleased", p_pressDuration);
	}
	
	/*! \brief Called when the faceleft button is pressed on an input device. */
	inline bool onButtonFaceLeftPressed() const
	{
		return handleButtonCallbackPressed("onButtonFaceLeftPressed");
	}
	
	/*! \brief Called when the faceleft button is released on an input device.
	    \param p_pressDuration For how long the button was pressed, in seconds. */
	inline bool onButtonFaceLeftReleased(real p_pressDuration) const
	{
		return handleButtonCallbackReleased("onButtonFaceLeftReleased", p_pressDuration);
	}
	
	/*! \brief Called when the jump button is pressed on an input device. */
	inline bool onButtonJumpPressed() const
	{
		return handleButtonCallbackPressed("onButtonJumpPressed");
	}
	
	/*! \brief Called when the jump button is released on an input device.
	    \param p_pressDuration For how long the button was pressed, in seconds. */
	inline bool onButtonJumpReleased(real p_pressDuration) const
	{
		return handleButtonCallbackReleased("onButtonJumpReleased", p_pressDuration);
	}
	
	/*! \brief Called when the primary fire button is pressed on an input device. */
	inline bool onButtonPrimaryFirePressed() const
	{
		return handleButtonCallbackPressed("onButtonPrimaryFirePressed");
	}
	
	/*! \brief Called when the primary fire button is released on an input device.
	    \param p_pressDuration For how long the button was pressed, in seconds. */
	inline bool onButtonPrimaryFireReleased(real p_pressDuration) const
	{
		return handleButtonCallbackReleased("onButtonPrimaryFireReleased", p_pressDuration);
	}
	
	/*! \brief Called when the secondary fire button is pressed on an input device. */
	inline bool onButtonSecondaryFirePressed() const
	{
		return handleButtonCallbackPressed("onButtonSecondaryFirePressed");
	}
	
	/*! \brief Called when the secondary fire button is released on an input device.
	    \param p_pressDuration For how long the button was pressed, in seconds. */
	inline bool onButtonSecondaryFireReleased(real p_pressDuration) const
	{
		return handleButtonCallbackReleased("onButtonSecondaryFireReleased", p_pressDuration);
	}
	
	/*! \brief Called when the select weapon 1 button is pressed on an input device. */
	inline bool onButtonSelectWeapon1Pressed() const
	{
		return handleButtonCallbackPressed("onButtonSelectWeapon1Pressed");
	}
	
	/*! \brief Called when the select weapon 2 button is pressed on an input device. */
	inline bool onButtonSelectWeapon2Pressed() const
	{
		return handleButtonCallbackPressed("onButtonSelectWeapon2Pressed");
	}
	
	/*! \brief Called when the select weapon 3 button is pressed on an input device. */
	inline bool onButtonSelectWeapon3Pressed() const
	{
		return handleButtonCallbackPressed("onButtonSelectWeapon3Pressed");
	}
	
	/*! \brief Called when the select weapon 4 button is pressed on an input device. */
	inline bool onButtonSelectWeapon4Pressed() const
	{
		return handleButtonCallbackPressed("onButtonSelectWeapon4Pressed");
	}
	
	/*! \brief Called when the toggle weapons is pressed on an input device. */
	inline bool onButtonToggleWeaponsPressed() const
	{
		return handleButtonCallbackPressed("onButtonToggleWeaponsPressed");
	}
	
	// Debug buttons
#if !defined(TT_BUILD_FINAL)
	/*! \brief Called when the Cheat button is pressed on an input device. */
	inline bool onButtonDebugCheatPressed() const
	{
		return handleButtonCallbackPressed("onButtonDebugCheatPressed");
	}
	
	/*! \brief Called when the Cheat button is released on an input device.
	    \param p_pressDuration For how long the button was pressed, in seconds. */
	inline bool onButtonDebugCheatReleased(real p_pressDuration) const
	{
		return handleButtonCallbackReleased("onButtonDebugCheatReleased", p_pressDuration);
	}
	
	/*! \brief Called when the Restart button is pressed on an input device. */
	inline bool onButtonDebugRestartPressed() const
	{
		return handleButtonCallbackPressed("onButtonDebugRestartPressed");
	}
	
	/*! \brief Called when the Restart button is released on an input device.
	    \param p_pressDuration For how long the button was pressed, in seconds. */
	inline bool onButtonDebugRestartReleased(real p_pressDuration) const
	{
		return handleButtonCallbackReleased("onButtonDebugRestartReleased", p_pressDuration);
	}
	
	/*! \brief Called when the Debug button is pressed on an input device. */
	inline bool onButtonDebugPressed() const
	{
		return handleButtonCallbackPressed("onButtonDebugPressed");
	}
	
	/*! \brief Called when the Debug button is released on an input device.
	    \param p_pressDuration For how long the button was pressed, in seconds. */
	inline bool onButtonDebugReleased(real p_pressDuration) const
	{
		return handleButtonCallbackReleased("onButtonDebugReleased", p_pressDuration);
	}
	
	inline bool onButtonDebugPreviousSpawnPointPressed() const
	{
		return handleButtonCallbackPressed("onButtonDebugPreviousSpawnPointPressed");
	}
	
	inline bool onButtonDebugPreviousSpawnPointReleased(real p_pressDuration) const
	{
		return handleButtonCallbackReleased("onButtonDebugPreviousSpawnPointReleased", p_pressDuration);
	}
	
	inline bool onButtonDebugNextSpawnPointPressed() const
	{
		return handleButtonCallbackPressed("onButtonDebugNextSpawnPointPressed");
	}
	
	inline bool onButtonDebugNextSpawnPointReleased(real p_pressDuration) const
	{
		return handleButtonCallbackReleased("onButtonDebugNextSpawnPointReleased", p_pressDuration);
	}
	
	inline bool onButtonDebugLastSpawnPointPressed() const
	{
		return handleButtonCallbackPressed("onButtonDebugLastSpawnPointPressed");
	}
	
	inline bool onButtonDebugLastSpawnPointReleased(real p_pressDuration) const
	{
		return handleButtonCallbackReleased("onButtonDebugLastSpawnPointReleased", p_pressDuration);
	}
#endif
	
	/*! \brief Called when a presentation object has ended. 
	           Only works if end callback name was provided. See PresentationObject.startEx for more details
	    \param p_pres the presentation object
	    \param p_name the name provided to this callback */
	inline void onPresentationObjectEnded(const wrappers::PresentationObjectWrapper& p_pres,
	                                      const std::string& p_name) const
	{
		queueSqFun("onPresentationObjectEnded", p_pres, p_name);
	}
	
	/*! \brief Called when a presentation object was canceled.
	           (Another animation was stared before the one with a callback was ended.)
	           Only works if end callback name was provided. See PresentationObject.startEx for more details
	    \param p_pres the presentation object
	    \param p_name the name provided to this callback */
	inline void onPresentationObjectCanceled(const wrappers::PresentationObjectWrapper& p_pres,
	                                         const std::string& p_name) const
	{
		queueSqFun("onPresentationObjectCanceled", p_pres, p_name);
	}
	
	/*! \brief Called when a presentation object triggers a callback
	    \param p_pres the presentation object
	    \param p_data The data provided by this callback */
	inline void onPresentationObjectCallback(const wrappers::PresentationObjectWrapper& p_pres,
	                                         const std::string& p_data) const
	{
		queueSqFun("onPresentationObjectCallback", p_pres, p_data);
	}
	
	/*! \brief Called when this entity is being carried (via carry movement) by another entity.
	    \param p_carryingEntity The entity that is carrying this entity. */
	void onCarryBegin(const entity::EntityHandle& p_carryingEntity);
	
	/*! \brief Called when this entity is no longer carried (via carry movement) by another entity. */
	void onCarryEnd();
	
	/*! \brief Called after game progress restore. This callback is NOT queued and executed immediately.
	    \note Build-in restoreIDs are: "editor", "recorder" and "shutdown". */
	inline void onProgressRestored(const std::string& p_restoreID) const
	{
		// Immediate call this method (no queuing)
		ms_mgr->getVM()->callSqMemberFun("onProgressRestored", getSqState(), m_instance, p_restoreID);
	}
	
	inline void onValidateScriptState() const
	{
		// Immediate call this method (no queuing)
		// FIXME: use m_class->getBaseClass() instead of getSqState().
		ms_mgr->getVM()->callSqMemberFun("onValidateScriptState", getSqState(), m_instance);
	}
	
	/*! \brief Called after game is reloaded and entities have just been re-created. 
	           So after the onCreate() and before onInit() and onSpawn(). This callback is NOT queued and executed immediately.
	           NOTE: Don't use kill() during this callback. */
	inline void onReloadGame() const
	{
		// Immediate call this method (no queuing)
		ms_mgr->getVM()->callSqMemberFun("onReloadGame", getSqState(), m_instance);
	}
	
	/*! \brief Called right before entity gets killed by killSpawnSection call */
	inline void onPreSpawnSectionKill() const
	{
		// Immediate call this method (no queuing)
		ms_mgr->getVM()->callSqMemberFun("onPreSpawnSectionKill", getSqState(), m_instance);
	}
	
	void                      setState(const std::string& p_newState);
	inline const std::string& getState() const { return m_currentState.getName(); }
	inline const std::string& getPreviousState() const { return m_previousStateName; }
	
	inline HSQOBJECT getSqState() const { return m_currentState.getSqState(); }
	const std::string& getType()  const;
	
	static void setScriptMgr(EntityScriptMgr* p_mgr);
	static void resetScriptMgr();
	
	inline const EntityScriptClassPtr& getClass() const { return m_class; }
	inline HSQOBJECT getSqInstance() const { return m_instance; }
	
	void addTag(const std::string& p_tag);
	void removeTag(const std::string& p_tag);
	inline const tt::str::Strings& getTags() const { return m_tags; }
	
	inline void queueSqFun(const std::string& p_function) const
	{
		CallbackPtr ptr(std::make_shared<Callback>(ms_mgr->getVM()->getVM(), m_currentState.getSqState(), p_function));
		m_callbacks.push_back(ptr);
		ms_mgr->registerForCallbacksUpdate(m_this.lock());
	}
	
	template <typename P1Type>
	inline void queueSqFun(const std::string& p_function, const P1Type& p_arg) const
	{
		CallbackPtr ptr(std::make_shared<Callback>(ms_mgr->getVM()->getVM(), m_currentState.getSqState(), p_function));
		ptr->addParameter(p_arg);
		m_callbacks.push_back(ptr);
		ms_mgr->registerForCallbacksUpdate(m_this.lock());
	}
	
	template <typename P1Type, typename P2Type>
	inline void queueSqFun(const std::string& p_function, const P1Type& p_arg1, const P2Type& p_arg2) const
	{
		CallbackPtr ptr(std::make_shared<Callback>(ms_mgr->getVM()->getVM(), m_currentState.getSqState(), p_function));
		ptr->addParameter(p_arg1);
		ptr->addParameter(p_arg2);
		m_callbacks.push_back(ptr);
		ms_mgr->registerForCallbacksUpdate(m_this.lock());
	}
	
	inline void callSqFun(const std::string& p_function) const
	{
		ms_mgr->getVM()->callSqMemberFun(p_function, getSqState(), m_instance);
	}
	
	template <typename P1Type>
	inline void callSqFun(const std::string& p_function, const P1Type& p_arg) const
	{
		ms_mgr->getVM()->callSqMemberFun(p_function, getSqState(), m_instance, p_arg);
	}
	
	template <typename P1Type, typename P2Type>
	inline void callSqFun(const std::string& p_function, const P1Type* p_arg, const P2Type& p_arg2) const
	{
		ms_mgr->getVM()->callSqMemberFun(p_function, getSqState(), m_instance, p_arg, p_arg2);
	}
	
	template <typename P1Type, typename P2Type>
	inline void callSqFun(const std::string& p_function, const P1Type& p_arg, const P2Type& p_arg2) const
	{
		ms_mgr->getVM()->callSqMemberFun(p_function, getSqState(), m_instance, p_arg, p_arg2);
	}
	
	template <typename RetType>
	inline void callSqFunWithReturn(RetType* p_returnValue_OUT, const std::string& p_function) const
	{
		ms_mgr->getVM()->callSqMemberFunWithReturn(p_returnValue_OUT, p_function, getSqState(), m_instance);
	}
	
	template <typename RetType, typename P1Type>
	inline void callSqFunWithReturn(RetType* p_returnValue_OUT, const std::string& p_function, const P1Type* p_arg) const
	{
		ms_mgr->getVM()->callSqMemberFunWithReturn(p_returnValue_OUT, p_function, getSqState(), m_instance, p_arg);
	}
	
	template <typename RetType, typename P1Type>
	inline void callSqFunWithReturn(RetType* p_returnValue_OUT, const std::string& p_function, const P1Type& p_arg) const
	{
		ms_mgr->getVM()->callSqMemberFunWithReturn(p_returnValue_OUT, p_function, getSqState(), m_instance, p_arg);
	}
	
	void removeCallbacks();
	
	void addObjectToSQSerializer(toki::script::serialization::SQSerializer& p_serializer) const;
	void serialize(const toki::script::serialization::SQSerializer& p_serializer,
	               tt::code::BufferWriteContext* p_context) const;
	void serializeSQSerializerObjects(const toki::script::serialization::SQSerializer& p_serializer,
	                                    tt::code::BufferWriteContext* p_context) const;
	static EntityBasePtr unserialize(const toki::script::serialization::SQUnserializer& p_unserializer,
	                                 tt::code::BufferReadContext* p_context);
	void unserializeSQSerializerObjects(const toki::script::serialization::SQUnserializer& p_unserializer,
	                                      tt::code::BufferReadContext* p_context);
	
	static const EntityBase* getEntityBase(const entity::EntityHandle& p_handle);
	
private:
	// Creation without script callback
	static EntityBasePtr create(const entity::EntityHandle& p_handle, const std::string& p_type);
	
	EntityBase(const entity::EntityHandle& p_handle, const std::string& p_type,
	           const EntityScriptClassPtr& p_class);
	
	void updateCallbacks(s32 p_maxUpdates);
	
	void updateInvalidLevelProperties(HSQUIRRELVM p_vm,
	                                  level::entity::EntityInstance::Properties& p_properties) const;
	
	inline bool hasSqFun(const std::string& p_function) const
	{
		return ms_mgr->getVM()->hasSqMemberFun(p_function, getSqState());
	}
	
	inline bool handleButtonCallbackPressed(const std::string& p_callback) const
	{
		if (hasSqFun(p_callback))
		{
			bool isHandled = false;
			callSqFunWithReturn(&isHandled, p_callback);
			return isHandled;
		}
		// Has not been handled
		return false;
	}
	
	inline bool handleButtonCallbackReleased(const std::string& p_callback, real p_duration) const
	{
		if (hasSqFun(p_callback))
		{
			bool isHandled = false;
			callSqFunWithReturn(&isHandled, p_callback, p_duration);
			return isHandled;
		}
		// Has not been handled
		return false;
	}
	
	template <typename P1Type, typename P2Type, typename P3Type>
	inline void queueSqFun(const std::string& p_function, const P1Type& p_arg1, const P2Type& p_arg2, const P3Type& p_arg3) const
	{
		CallbackPtr ptr(std::make_shared<Callback>(ms_mgr->getVM()->getVM(), m_currentState.getSqState(), p_function));
		ptr->addParameter(p_arg1);
		ptr->addParameter(p_arg2);
		ptr->addParameter(p_arg3);
		m_callbacks.push_back(ptr);
		ms_mgr->registerForCallbacksUpdate(m_this.lock());
	}
	
	const EntityBase& operator=(const EntityBase&); // no assignment allowed
	
	
	static EntityScriptMgr* ms_mgr;
	
	EntityScriptClassPtr m_class;
	
	EntityState m_currentState;
	EntityState m_targetState;        // Used to prevent multiple onExitState callbacks. Does NOT have to be serialized.
	std::string m_previousStateName;  // Used for scripting convenience. Not used in code.
	wrappers::EntityWrapper m_wrapper;
	
	tt::str::Strings m_tags;
	
	typedef std::vector<CallbackPtr> Callbacks;
	mutable Callbacks m_callbacks;
	
	static inline wrappers::EntityWrapper& getEmptyWrapper()
	{
		static wrappers::EntityWrapper emptyWrapper;
		return emptyWrapper;
	}
	
	HSQOBJECT m_instance;
	EntityBaseWeakPtr m_this;
};

// Namespace end
}
}
}


// HACK: Included here so EntityBase isn't incomplete
#include <toki/game/script/sqbind_bindings.h>

#endif  // !defined(INC_TOKI_GAME_SCRIPT_ENTITYBASE_H)
