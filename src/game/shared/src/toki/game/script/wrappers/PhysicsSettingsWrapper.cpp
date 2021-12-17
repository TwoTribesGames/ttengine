#include <tt/script/helpers.h>

#include <toki/game/script/wrappers/PhysicsSettingsWrapper.h>

namespace toki {
namespace game {
namespace script {
namespace wrappers {


void PhysicsSettingsWrapper::bind(const tt::script::VirtualMachinePtr& p_vm)
{
	TT_SQBIND_SETVM(p_vm);
	TT_SQBIND_INIT_NAME(PhysicsSettingsWrapper, "PhysicsSettings");
	TT_SQBIND_SET_CONSTRUCTOR(PhysicsSettingsWrapper, PhysicsSettingsWrapper_constructor);
	
	TT_SQBIND_METHOD(PhysicsSettingsWrapper, setMass);
	TT_SQBIND_METHOD(PhysicsSettingsWrapper, getMass);
	
	TT_SQBIND_METHOD(PhysicsSettingsWrapper, setDrag);
	TT_SQBIND_METHOD(PhysicsSettingsWrapper, getDrag);
	
	TT_SQBIND_METHOD(PhysicsSettingsWrapper, setCollisionDrag);
	TT_SQBIND_METHOD(PhysicsSettingsWrapper, getCollisionDrag);
	
	TT_SQBIND_METHOD(PhysicsSettingsWrapper, setThrust);
	TT_SQBIND_METHOD(PhysicsSettingsWrapper, getThrust);
	
	TT_SQBIND_METHOD(PhysicsSettingsWrapper, getExtraForce);
	TT_SQBIND_METHOD(PhysicsSettingsWrapper, setExtraForce);
	TT_SQBIND_METHOD(PhysicsSettingsWrapper, addToExtraForce);
	
	TT_SQBIND_METHOD(PhysicsSettingsWrapper, setEaseOutDistance);
	TT_SQBIND_METHOD(PhysicsSettingsWrapper, getEaseOutDistance);
	
	TT_SQBIND_METHOD(PhysicsSettingsWrapper, setMoveEndDistance);
	TT_SQBIND_METHOD(PhysicsSettingsWrapper, getMoveEndDistance);
	
	TT_SQBIND_METHOD(PhysicsSettingsWrapper, setSpeedFactor);
	TT_SQBIND_METHOD(PhysicsSettingsWrapper, getSpeedFactor);
	
	TT_SQBIND_METHOD(PhysicsSettingsWrapper, setBouncyness);
	TT_SQBIND_METHOD(PhysicsSettingsWrapper, getBouncyness);
	
	TT_SQBIND_METHOD(PhysicsSettingsWrapper, setCollisionWithSolid);
	TT_SQBIND_METHOD(PhysicsSettingsWrapper, hasCollisionWithSolid);
	
	TT_SQBIND_METHOD(PhysicsSettingsWrapper, setCollisionIsMovementFailure);
	TT_SQBIND_METHOD(PhysicsSettingsWrapper, isCollisionIsMovementFailure);
	
	TT_SQBIND_METHOD(PhysicsSettingsWrapper, setShouldTurn);
	TT_SQBIND_METHOD(PhysicsSettingsWrapper, shouldTurn);
	
	TT_SQBIND_METHOD(PhysicsSettingsWrapper, setTurnAnimationName);
	TT_SQBIND_METHOD(PhysicsSettingsWrapper, getTurnAnimationName);
	
	TT_SQBIND_METHOD(PhysicsSettingsWrapper, setPresentationAnimationName);
	TT_SQBIND_METHOD(PhysicsSettingsWrapper, getPresentationAnimationName);
	
	TT_SQBIND_METHOD(PhysicsSettingsWrapper, setPathOffset);
	TT_SQBIND_METHOD(PhysicsSettingsWrapper, getPathOffset);
	
	TT_SQBIND_METHOD(PhysicsSettingsWrapper, clearClamp);
	TT_SQBIND_METHOD(PhysicsSettingsWrapper, setRectClamp);
	TT_SQBIND_METHOD(PhysicsSettingsWrapper, setCircleClamp);
}


PhysicsSettingsWrapper* PhysicsSettingsWrapper_constructor(HSQUIRRELVM v) 
{
	const SQInteger params = sq_gettop(v) - 1; // Stack has params + context.
	
	if (params != 5)
	{
		TT_PANIC("PhysicsSettingsWrapper incorrect number of parameters. (Got: %d, expected: 5)", params);
		return 0; // need 5 params
	}
	
	SQFloat mass;
	SQFloat drag;
	SQFloat thrust;
	SQFloat easeOut;
	SQFloat moveEnd;
	
	if (SQ_FAILED(sq_getfloat(v, 2, &mass)))
	{
		TT_PANIC("PhysicsSettingsWrapper 1st parameter is not a float value");
		return 0;
	}
	
	if (SQ_FAILED(sq_getfloat(v, 3, &drag)))
	{
		TT_PANIC("PhysicsSettingsWrapper 2nd parameter is not a float value");
		return 0;
	}
	
	if (SQ_FAILED(sq_getfloat(v, 4, &thrust)))
	{
		TT_PANIC("PhysicsSettingsWrapper 3rd parameter is not a float value");
		return 0;
	}
	
	if (SQ_FAILED(sq_getfloat(v, 5, &easeOut)))
	{
		TT_PANIC("PhysicsSettingsWrapper 4th parameter is not a float value");
		return 0;
	}
	
	if (SQ_FAILED(sq_getfloat(v, 6, &moveEnd)))
	{
		TT_PANIC("PhysicsSettingsWrapper 5th parameter is not a float value");
		return 0;
	}
	
	return new PhysicsSettingsWrapper(mass, drag, thrust, easeOut, moveEnd);
}

// Namespace end
}
}
}
}
