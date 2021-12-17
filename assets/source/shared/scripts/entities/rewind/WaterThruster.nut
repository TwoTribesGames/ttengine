include_entity("rewind/RotatableThruster");

class WaterThruster extends RotatableThruster
</
	placeable      = Placeable_Hidden
	movementset    = "Static"
	collisionRect  = [ 0.0, 0.0, 0.1, 0.1 ]
	group          = "Rewind"
/>
{
	// Settings (must be set)
	_maxThrusterPower = null;
	
	// Internal values (don't change)
	_minThrusterPower = null;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		base.onInit();
		_minThrusterPower = _thrusterPower;
		
		::null_assert(_maxThrusterPower);
		::null_assert(_minThrusterPower);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods
	
	function resetThrusterPower()
	{
		_thrusterPower = _minThrusterPower;
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onWaterEnter()
	{
		local speedLength = _parent.getSpeed().length();
		local startPower = ::clamp(speedLength / 40 + _minThrusterPower, _minThrusterPower, _maxThrusterPower);
		_thrusterPower = ::max(_thrusterPower, startPower);
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
		
		if (direction.length() < 0.5)
		{
			_thrusterPower -= 0.1;
		}
		else
		{
			_thrusterPower += 0.075;
		}
		
		_thrusterPower = ::clamp(_thrusterPower, _minThrusterPower, _maxThrusterPower);
		
		base.childUpdate(p_deltaTime);
	}
}
