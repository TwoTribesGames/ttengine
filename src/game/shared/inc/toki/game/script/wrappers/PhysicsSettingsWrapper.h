#if !defined(INC_TOKI_GAME_SCRIPT_WRAPPERS_PHYSICSSETTINGSWRAPPER_H)
#define INC_TOKI_GAME_SCRIPT_WRAPPERS_PHYSICSSETTINGSWRAPPER_H


#include <tt/math/Vector2.h>
#include <tt/script/VirtualMachine.h>

#include <toki/game/entity/movementcontroller/PhysicsSettings.h>


namespace toki   /*! */ {
namespace game   /*! */ {
namespace script /*! */ {
namespace wrappers {


/*! \brief The physics settings for a movement */
class PhysicsSettingsWrapper
{
public:
	PhysicsSettingsWrapper()
	:
	m_settings()
	{}
	
	PhysicsSettingsWrapper(const entity::movementcontroller::PhysicsSettings& p_settings)
	:
	m_settings(p_settings)
	{}
	
	/*! \brief Construct with values */
	PhysicsSettingsWrapper(real p_mass, real p_drag, real p_thrust, real p_easeOut, real p_moveEnd)
	:
	m_settings(p_mass, p_drag, p_thrust, p_easeOut, p_moveEnd)
	{}
	
	/*! \brief Set the mass property
	    \param p_mass Value for the mass property */
	inline void setMass(real p_mass) { m_settings.mass = p_mass; }
	
	/*! \brief Get the current value of the mass property
	    \return Value of the mass property */
	inline real getMass() const      { return m_settings.mass;   }
	
	/*! \brief Set the drag property
	    \param p_drag Value for the drag property */
	inline void setDrag(real p_drag) { m_settings.drag = p_drag; }
	
	/*! \brief Get the current value of the drag property
	    \return Value of the drag property */
	inline real getDrag() const      { return m_settings.drag;   }
	
	/*! \brief Set the collision drag property
	    \param p_drag Value for the drag property */
	inline void setCollisionDrag(real p_drag) { m_settings.collisionDrag = p_drag; }
	
	/*! \brief Get the current value of the drag property
	    \return Value of the drag property */
	inline real getCollisionDrag() const      { return m_settings.collisionDrag;   }
	
	/*! \brief Set the thrust property
	    \param p_thrust Value for the thrust property */
	inline void setThrust(real p_thrust) { m_settings.thrust = p_thrust; }
	
	/*! \brief Get the current value of the thrust property
	    \return Value of the thrust property */
	inline real getThrust() const        { return m_settings.thrust;     }
	
	/*! \brief Gets the extra force(s) which is applied to entity
	    \return Vector for all the extra forces. */
	inline const tt::math::Vector2& getExtraForce() const { return m_settings.extraForce; }
	
	/*! \brief Sets the extra force. */
	inline void setExtraForce(const tt::math::Vector2& p_force) { m_settings.extraForce = p_force; }
	
	/*! \brief Adds force to existing extra force. */
	inline void addToExtraForce(const tt::math::Vector2& p_force) { m_settings.extraForce += p_force; }
	
	/*! \brief Set the ease-out-distance property
	    \param p_easeOutDistance Value for the ease-out-distance property */
	inline void setEaseOutDistance(real p_easeOutDistance) { m_settings.easeOutDistance = p_easeOutDistance; }
	
	/*! \brief Get the current value of the ease-out-distance property
	    \return Value of the ease-out-distance property */
	inline real getEaseOutDistance() const                 { return m_settings.easeOutDistance;              }
	
	/*! \brief Set the move-end-distance property
	    \param p_moveEndDistance Value for the move-end-distance property */
	inline void setMoveEndDistance(real p_moveEndDistance) { m_settings.moveEndDistance = p_moveEndDistance; }
	
	/*! \brief Get the current value of the move-end-distance property
	    \return Value of the move-end-distance property */
	inline real getMoveEndDistance() const                 { return m_settings.moveEndDistance;              }
	
	/*! \brief Set speed factor. (The final speed is multiplied with this. 1.0 is default.) */
	inline void setSpeedFactor(real p_factor) { m_settings.speedFactor = p_factor; }
	/*! \brief Return speed factor. (The final speed is multiplied with this. 1.0 is default.) */
	inline real getSpeedFactor() const        { return m_settings.speedFactor;      }
	
	/*! \brief Set bouncyness when colliding with a solid wall. 0.0 (no bounce) is default. */
	inline void setBouncyness(real p_bouncyness) { m_settings.bouncyness = p_bouncyness; }
	/*! \brief Returns bouncyness when colliding with a solid wall. 0.0 (no bounce) is default. */
	inline real getBouncyness() const            { return m_settings.bouncyness;      }
	
	/*! \brief Set if entity should collide with solid tiles. */
	inline void setCollisionWithSolid(bool p_collide) { m_settings.hasCollisionWithSolid = p_collide; }
	/*! \brief Returns if entity should colide with solid tiles. */
	inline bool hasCollisionWithSolid() const         { return m_settings.hasCollisionWithSolid;      }
	
	/*! \brief Set if collide should result in movement failure. */
	inline void setCollisionIsMovementFailure(bool p_enable) { m_settings.isCollisionMovementFailure = p_enable; }
	/*! \brief Returns if collide should result in movement failure. */
	inline bool isCollisionIsMovementFailure() const         { return m_settings.isCollisionMovementFailure;     }
	
	/*! \brief Set whether physics movement should turn the entity (change SetForwardAsLeft) when its direction Y changes. */
	inline void setShouldTurn(bool p_turnEnabled) { m_settings.shouldTurn = p_turnEnabled; }
	/*! \brief Get whether physics movement should turn the entity (change SetForwardAsLeft) when its direction Y changes. */
	inline bool shouldTurn() const { return m_settings.shouldTurn; };
	
	/*! \brief Set the name of the presentation animation that should be played when turning. */
	inline void setTurnAnimationName(const std::string& p_name) { m_settings.turnAnimationName = p_name; }
	/*! \brief Get the name of the presentation animation that should be played when turning. */
	inline const std::string& getTurnAnimationName() const      { return m_settings.turnAnimationName;   }
	
	/*! \brief Set the name of the presentation animation that should be played when doing physics movement. */
	inline void setPresentationAnimationName(const std::string& p_name) { m_settings.presentationAnimationName = p_name; }
	/*! \brief Get the name of the presentation animation that should be played when doing physics movement. */
	inline const std::string& getPresentationAnimationName() const      { return m_settings.presentationAnimationName;   }
	
	/*! \brief Set path offset. This is added to targetpos and targetoffset when doing path finding.
	           (own orientation is applied). */
	inline void setPathOffset(const tt::math::Vector2 p_offset) { m_settings.pathOffset = p_offset; }
	/*! \brief Get path offset. (See: setPathOffset for more details.) */
	inline const tt::math::Vector2& getPathOffset() { return m_settings.pathOffset; }
	
	/*! \brief Clears any clamp settings. */
	inline void clearClamp()                                     { m_settings.clearClamp();         }
	/*! \brief Sets rectangle as clamp shape. */
	inline void setRectClamp(const tt::math::VectorRect& p_rect) { m_settings.setRectClamp(p_rect); }
	/*! \brief Sets circle as clamp shape. */
	inline void setCircleClamp(const tt::math::Vector2& p_centerPos, real p_radius) { m_settings.setCircleClamp(p_centerPos, p_radius); }
	
	inline const entity::movementcontroller::PhysicsSettings& getSettings() const { return m_settings; }
	
	static void bind(const tt::script::VirtualMachinePtr& p_vm);
	
private:
	entity::movementcontroller::PhysicsSettings m_settings;
};

// Constructor with values in squirrel
PhysicsSettingsWrapper* PhysicsSettingsWrapper_constructor(HSQUIRRELVM v);

// Namespace end
}
}
}
}

#endif  // !defined(INC_TOKI_GAME_SCRIPT_WRAPPERS_PHYSICSSETTINGSWRAPPER_H)
