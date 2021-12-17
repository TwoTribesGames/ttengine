// A class to shake the camera
class CameraPositionShaker extends EntityBase
</ 
	placeable      = Placeable_Hidden
	collisionRect  = [ 0.0, 0.0, 0.1, 0.1 ]
/>
{
	_amplitude  = null; // vector2 for max amplitudes along axes
	_frequency  = 0.2;
	_frequencyRange = 0.01;
	
	_duration   = 0.5;  // if the duration is < 0 it's assumed to be a constant shake and it won't start automatically
	
	_effect      = null;  // keep track of the effect so we can combine multiple shakes
	_shakeToggle = false; // used to alternate between extremes
	_shakeHud    = true;  // flag to indicate if HUD needs to shake too
	
	_positionCulling = true;
	_deathEvent      = null;
	
	_lastShakeDuration = 0.01;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		setPositionCullingEnabled(_positionCulling);
		::removeEntityFromWorld(this);
		_deathEvent = ::EventPublisher("deathevent");
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onCulled()
	{
		::killEntity(this);
	}
	
	function onUnculled()
	{
		// Make sure this effect ONLY starts on screen
		_effect = ::Camera.createCameraEffect();
		
		if (_duration > 0) 
		{
			startTimer("timeout", _duration);
		}
		
		_amplitude.x = brnd() ? -_amplitude.x : _amplitude.x;
		_amplitude.y = brnd() ? -_amplitude.y : _amplitude.y;
		
		// start shaking immediately
		startShaking();
	}
	
	function onTimer(p_name)
	{
		if (p_name == "timeout")
		{
			stopShaking();
			
			startTimer("kill", _lastShakeDuration);
		}
		else if (p_name == "startShake")
		{
			startShaking();
		}
		else if (p_name == "kill")
		{
			::killEntity(this);
		}
	}
	
	function onDie()
	{
		_deathEvent.publish(this);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods
	
	// sets the camera on a random offset
	function startShaking()
	{
		local offset = (_shakeToggle ? _amplitude : -_amplitude);
		local shakeDuration = 1.0 / ::max(0.001, _frequency + frnd_minmax(-_frequencyRange, _frequencyRange));
		if (_effect != null)
		{
			_effect.setOffset(offset, shakeDuration, EasingType_QuadraticInOut);
		}
		
		// FIXME: HUD Shaking should be moved to its own class
		if (_shakeHud)
		{
			::Hud.setOffset(offset / 50);
			
			// Wait couple of frames before resetting
			_hudResetCounter = 5;
		}
		
		_shakeToggle = (_shakeToggle == false);
		startTimer("startShake", shakeDuration);
		
		_lastShakeDuration = shakeDuration;
	}
	
	function stopShaking()
	{
		if (_effect != null)
		{
			_effect.setOffset(::Vector2(0, 0), _lastShakeDuration, EasingType_QuadraticInOut);
		}
		Hud.setOffset(::Vector2(0, 0));
		stopAllTimers();
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Update
	
	// FIXME: HUD Shaking should be moved to its own class
	_hudResetCounter = 0;
	function update(p_deltaTime)
	{
		if (_hudResetCounter > 0)
		{
			--_hudResetCounter;
			if (_hudResetCounter == 0)
			{
				Hud.setOffset(::Vector2(0, 0));
			}
		}
	}
}
