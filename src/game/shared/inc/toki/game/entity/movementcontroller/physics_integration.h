#if !defined(INC_TOKI_GAME_ENTITY_MOVEMENTCONTROLLER_PHYSICSINTEGRATION_H)
#define INC_TOKI_GAME_ENTITY_MOVEMENTCONTROLLER_PHYSICSINTEGRATION_H


#include <tt/math/Vector2.h>

#include <toki/game/entity/movementcontroller/PhysicsSettings.h>


namespace toki {
namespace game {
namespace entity {
namespace movementcontroller {


struct State
{
	tt::math::Vector2 pos; // Position
	tt::math::Vector2 vel; // Velocity
};


struct Derivative
{
	tt::math::Vector2 dPos; // derivative of postion (velocity)
	tt::math::Vector2 dVel; // derivative of velocity (acceleration)
};


inline tt::math::Vector2 acceleration(const State&             p_state,
                                      real                     /*p_deltaTime*/,
                                      const PhysicsSettings&   p_settings,
                                      const tt::math::Vector2& p_force,
                                      real                     p_drag)
{
	return ((-p_drag * p_state.vel + p_force) / p_settings.mass);
}


inline Derivative evaluate(const State&             p_initialState,
                           real                     p_deltaTime,
                           const Derivative&        p_derivative,
                           const PhysicsSettings&   p_settings,
                           const tt::math::Vector2& p_force,
                           real                     p_drag)
{
	State state;
	state.pos = p_initialState.pos + p_derivative.dPos * p_deltaTime;
	state.vel = (p_initialState.vel + p_derivative.dVel * p_deltaTime) * p_settings.speedFactor;
	
	Derivative output;
	output.dPos = state.vel;
	output.dVel = acceleration(state, p_deltaTime, p_settings, p_force, p_drag);
	return output;
}


// RK4 integration see: http://gafferongames.com/game-physics/integration-basics/
inline void integrate(State&                   p_state,
                      real                     p_deltaTime,
                      const PhysicsSettings&   p_settings,
                      const tt::math::Vector2& p_force,
                      real                     p_drag)
{
	Derivative a = evaluate(p_state,               0.0f, Derivative(), p_settings, p_force, p_drag);
	Derivative b = evaluate(p_state, p_deltaTime * 0.5f, a,            p_settings, p_force, p_drag);
	Derivative c = evaluate(p_state, p_deltaTime * 0.5f, b,            p_settings, p_force, p_drag);
	Derivative d = evaluate(p_state, p_deltaTime,        c,            p_settings, p_force, p_drag);
	
	const tt::math::Vector2 dPosDT = 1.0f / 6.0f * (a.dPos + 2.0f * (b.dPos + c.dPos) + d.dPos);
	const tt::math::Vector2 dVelDT = 1.0f / 6.0f * (a.dVel + 2.0f * (b.dVel + c.dVel) + d.dVel);
	
	p_state.pos += dPosDT * p_deltaTime;
	p_state.vel += dVelDT * p_deltaTime;
}

// Namespace end
}
}
}
}


#endif  // !defined(INC_TOKI_GAME_ENTITY_MOVEMENTCONTROLLER_PHYSICSINTEGRATION_H)
