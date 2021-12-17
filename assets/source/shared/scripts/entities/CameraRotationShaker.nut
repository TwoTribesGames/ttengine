// A class to shake the rotation of the camera
class CameraRotationShaker extends EntityBase
</ 
	placeable      = Placeable_Hidden
	collisionRect  = [ 0.0, 0.0, 0.1, 0.1 ]
/>
{
	_amplitude  = null; // real for max FOV amplitude
	_easingType = EasingType_QuadraticInOut;
	
	_duration   = 0.5;
	
	_effect       = null;  // keep track of the effect so we can combine multiple shakes
	_positionCulling = true;
	_deathEvent      = null;
	
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
		
		::assert(_duration > 0, "CameraRotationShaker duration must be > 0");
		
		if (_duration > 0) 
		{
			_effect.setRotation(_amplitude, _duration / 4, _easingType);
			
			startTimer("atHalfPi", _duration / 4);
			startTimer("atOneAndAHalfPi", (_duration * 3) / 4);
			startTimer("stop", _duration);
		}
	}
	
	function onTimer(p_name)
	{
		if (p_name == "atHalfPi")
		{
			_effect.setRotation(-_amplitude, _duration / 2, _easingType);
		}
		else if (p_name == "atOneAndAHalfPi")
		{
			_effect.setRotation(0, _duration / 4, _easingType);
		}
		else if (p_name == "stop")
		{
			::killEntity(this);
		}
	}
	
	function onDie()
	{
		_deathEvent.publish(this);
	}
}
