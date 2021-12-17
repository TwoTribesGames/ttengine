include_entity("rewind/EntityChild");

class BaseThruster extends EntityChild
</ 
	placeable      = Placeable_Hidden
	movementset    = "Static"
	collisionRect  = [ 0.0, 0.0, 0.1, 0.1 ]
	group          = "Rewind"
/>
{
	// Settings (must be set)
	_thrusterPower          = null;
	_thrusterBoostPower     = null;
	_thrusterBoostDampening = null; 
	_maxThrusterBoosts      = null;
	_drag                   = null;
	
	// Audio/visual settings
	_presentationFile       = null;
	_soundCueActiveName     = null; // Looping sfx played while active
	
	// Internal values (don't change)
	_thrusterAppliedPower   = 0;
	_usedThrusterBoosts     = 0;
	_thrusterEnabled        = false;
	_presentation           = null;
	_movementSettings       = null;
	_soundCueActive         = null;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		::null_assert(_thrusterPower);
		::null_assert(_thrusterBoostPower);
		::null_assert(_thrusterBoostDampening);
		::null_assert(_maxThrusterBoosts);
		::null_assert(_drag);
		
		base.onInit();
		
		if (_presentationFile != null)
		{
			_presentation = createPresentationObject(_presentationFile);
			_presentation.setAffectedByOrientation(false);
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods
	
	function enableThruster()
	{
		if (_thrusterEnabled == false && (_usedThrusterBoosts < _maxThrusterBoosts || _maxThrusterBoosts == 0))
		{
			_thrusterAppliedPower = _thrusterBoostPower;
			_presentation.start("thrust", _parent.getSurroundingSpecificTags(), false, 0);
			
			_thrusterEnabled = true;
			_usedThrusterBoosts++;
			::setRumble(RumbleStrength_Low, 0.0);
			
			if (_soundCueActiveName != null)
			{
				_soundCueActive = playSoundEffect(_soundCueActiveName);
			}
			return true;
		}
		
		return false;
	}
	
	function disableThruster()
	{
		_thrusterEnabled = false;
		
		if (_presentation != null)
		{
			_presentation.stop();
		}
		
		if (_soundCueActive != null)
		{
			_soundCueActive.stop();
			_soundCueActive = null;
		}
	}
	
	function getThrustForce(p_thrusterAppliedPower, p_deltaTime)
	{
		::tt_panic("Should be implemented by derived class");
		return 0;
	}
	
	function resetBoostCount()
	{
		_usedThrusterBoosts = 0;
	}
	
	function setDrag(p_drag)
	{
		_drag = p_drag;
		_movementSettings.setDrag(_drag);
		_movementSettings.setCollisionDrag(_drag);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Helper methods
	
	// Used to calculate the influence to be able to quickly change direction in the air
	// FIXME: Static methods: move to helper?
	function calculateSteeringInfluence(p_targetSpeed, p_currentSpeed, p_minScale, p_maxScale, p_maxSpeedInTiles)
	{
		// Only apply when not at speed limit
		if (p_currentSpeed.length() < p_maxSpeedInTiles)
		{
			local cur = p_currentSpeed / p_maxSpeedInTiles;
			return ::lerp(p_minScale, p_maxScale, clamp((p_targetSpeed - cur).length() / 2.0, 0.0, 1.0));
		}
		return 1.0;
	}
	
	function calculateSteeringInfluenceX(p_targetSpeed, p_currentSpeed, p_minScale, p_maxScale, p_maxSpeedInTiles)
	{
		// Only apply when not at speed limit
		if (fabs(p_currentSpeed.x) < p_maxSpeedInTiles)
		{
			local cur = p_currentSpeed.x / p_maxSpeedInTiles;
			return ::lerp(p_minScale, p_maxScale, clamp(fabs(p_targetSpeed.x - cur) / 1.35, 0.0, 1.0));
		}
		return 1.0;
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Inherited methods
	
	function setSuspended(p_suspend)
	{
		if (p_suspend)
		{
			resetBoostCount();
			disableThruster();
		}
		
		base.setSuspended(p_suspend);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Update
	
	function childUpdate(p_deltaTime)
	{
		if (_thrusterEnabled)
		{
			// Update boost logic
			// dampen the thrust boost force if it was applied
			if (_thrusterAppliedPower > _thrusterPower + 0.01)
			{
				_thrusterAppliedPower = ::lerp(_thrusterAppliedPower, _thrusterPower, _thrusterBoostDampening);
			}
			else
			{
				_thrusterAppliedPower = _thrusterPower;
			}
			
			local force = getThrustForce(_thrusterAppliedPower, p_deltaTime);
			if (force.lengthSquared() > 0.0)
			{
				_movementSettings.addToExtraForce(force);
			}
			else
			{
				disableThruster();
			}
		}
	}
}
