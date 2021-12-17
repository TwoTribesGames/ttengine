include_entity("rewind/BaseThruster");

class Thruster extends BaseThruster
</
	placeable      = Placeable_Hidden
	movementset    = "Static"
	collisionRect  = [ 0.0, 0.0, 0.1, 0.1 ]
	group          = "Rewind"
/>
{
	// Settings (must be set)
	_flySteerForce  = null;
	_flyDrag        = null;
	_walkSteerForce = null;
	_walkDrag       = null;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		::null_assert(_flySteerForce);
		::null_assert(_flyDrag);
		::null_assert(_walkSteerForce);
		::null_assert(_walkDrag);
		
		// Default to something
		setDrag(_flyDrag);
		
		base.onInit();
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Inherited methods
	
	// Implemented from baseclass
	function getThrustForce(p_thrusterAppliedPower, p_deltaTime)
	{
		// Handle jump
		return ::Vector2(0, p_thrusterAppliedPower / p_deltaTime);;
	}
	
	function enableThruster()
	{
		setDrag(_flyDrag);
		return base.enableThruster();
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onLand()
	{
		resetBoostCount();
		setDrag(_walkDrag);
	}
	
	function onLiftOff()
	{
		setDrag(_flyDrag);
	}
	
	function onButtonJumpPressed()
	{
		// FIXME: This must be removed and replaced with proper input handling
		if (_parent.moveControlsEnabled() == false)
		{
			return;
		}
		
		// FIXME: Why is this isSuspended check here? Are callbacks processd by suspended entities?
		if (isSuspended() == false && enableThruster())
		{
			// Remove Y speed, to give a more digital feel to jumping
			local speed = _parent.getSpeed();
			speed.y = 0.0;
			_parent.setSpeed(speed);
			
			// FIXME: Hack to instantly reset the gravity back to default to prevent higher jumps
			::Gravity.set(GravityType.Normal);
		}
	}
	
	function onButtonJumpReleased(p_duration)
	{
		disableThruster();
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Update
	
	function childUpdate(p_deltaTime)
	{
		base.childUpdate(p_deltaTime);
		
		// FIXME: This must be removed and replaced with proper input handling
		if (_parent.moveControlsEnabled() == false) return;
		
		// Handle walking/flying here
		// FIXME: Check for deltatime
		local result = _parent.getDirectionInput().getHorizontal();
		
		// Check for deadzone, only on floor
		if (_parent._isOnFloor && ::fabs(result.x) < 0.25)
		{
			return;
		}
		
		local threshold = _parent.getWalkThreshold();
		
		// let's make it semi digital so it becomes easier to execute jumps
		if      (result.x >  0 && result.x <=  threshold) result.x =  0.35;
		else if (result.x <  0 && result.x >= -threshold) result.x = -0.35;
		else if (result.x >  threshold) result.x =  1.0;
		else if (result.x < -threshold) result.x = -1.0;
		
		if (_parent._isOnFloor)
		{
			result *= _walkSteerForce;
		}
		else
		{
			// In air
			result = result * _flySteerForce * calculateSteeringInfluenceX(result, _parent.getSpeed(), 0, 10, 30);
		}
		
		_movementSettings.addToExtraForce(result);
	}
}
