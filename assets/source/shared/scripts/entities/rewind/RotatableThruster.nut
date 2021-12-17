include_entity("rewind/BaseThruster");

class RotatableThruster extends BaseThruster
</
	placeable      = Placeable_Hidden
	movementset    = "Static"
	collisionRect  = [ 0.0, 0.0, 0.1, 0.1 ]
	group          = "Rewind"
/>
{
	_resetAngleTimeout = 0.0;
	_previousAngle     = null;
	_fuel              = null;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Inherited methods
	
	function getThrustForce(p_thrusterAppliedPower, p_deltaTime)
	{
		if (_fuel != null && _fuel > 0.0)
		{
			_fuel -= p_deltaTime;
			if (_fuel <= 0.0)
			{
				if (_thrusterEnabled)
				{
					//_thrusterEnabled = false;
					disableThruster();
					_parent.customCallback("onFuelEmpty");
				}
				return ::Vector2(0, 0);
			}
		}
		local result = _parent.getDirectionInput();
		local angle = ::getAngleFromVector(result);
		if (_parent.hasDirectionInputOverride() == false && ::getCurrentControllerType() == ControllerType_Keyboard)
		{
			// Zero gravity keyboard lerping to make it less digital
			local length = result.length();
			if (_previousAngle == null || ::getAngleBetweenAngles(_previousAngle, angle) > 90)
			{
				_previousAngle = angle;
			}
			else if (angle != _previousAngle)
			{
				angle = ::angleLerp(_previousAngle, angle, 0.3);
			}
			_previousAngle = angle;
			result = ::getVectorFromAngle(angle) * length;
		}
		_presentation.setCustomRotation(-angle);
		
		return result * BaseThruster.calculateSteeringInfluence(result, _parent.getSpeed(), 0, 4, 50) *
			(p_thrusterAppliedPower / p_deltaTime);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Update
	
	function childUpdate(p_deltaTime)
	{
		// Don't call base here
		
		// FIXME: This must be removed and replaced with proper input handling
		if (_parent.moveControlsEnabled() == false)
		{
			return;
		}
		
		local direction = _parent.getDirectionInput();
		local length = direction.length();
		
		if (_fuel == null || _fuel >= 0)
		{
			if (length > 0.5 && _thrusterEnabled == false)
			{
				enableThruster();
			}
			else if (length <= 0.5 && _thrusterEnabled)
			{
				disableThruster();
			}
		}
		
		if (_thrusterEnabled == false)
		{
			_resetAngleTimeout += p_deltaTime;
		}
		else
		{
			_resetAngleTimeout = 0;
		}
		
		if (_resetAngleTimeout > 0.1)
		{
			_previousAngle = null;
		}
		
		base.childUpdate(p_deltaTime);
	}
}
