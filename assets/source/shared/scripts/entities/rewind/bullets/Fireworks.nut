include_entity("rewind/bullets/Deflectable");

class Fireworks extends Deflectable
</
	placeable      = Placeable_Hidden
	movementset    = "Static"
	collisionRect  = [ 0.0, 0.0, 0.05, 0.05 ]
	group          = "Rewind"
/>
{
	static c_presentationFile = "presentation/fireworks";
	static c_colors = ["blue", "green", "red", "purple"];
	
	_color          = null;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		base.onInit();
		
		_color = c_colors[::rnd_minmax(0, c_colors.len()-1)];
		_timeout = ::frnd_minmax(0.3, 0.9);
		startTimer("disappear", _timeout);
		::makeEntityUndetectable(this);
	}
	
	function onSpawn()
	{
		base.onSpawn();
		
		registerZeroGravityAffectedEntity(this);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onTouchEnter(p_entity, p_sensor)
	{
		// Don't do anything
	}
	
	function onTouchFilter(p_entity)
	{
		// Don't do anything
	}
	
	function onShieldHit(p_shield)
	{
		// Don't do anything
	}
	
	function onWallHit()
	{
		// Don't do anything
	}
	
	function onTimer(p_name)
	{
		if (p_name == "disappear" && _inWater == false && _isDisabled == false)
		{
			_presentation.start("disappear", [], false, 0);
		}
		else if (p_name == "removeBullet")
		{
			::Camera.shakeScreen(0.1, getCenterPosition(), CameraShakeFlag.Default);
			::ColorGrading.oneShot("fireworks", 0.05, 0.05, 0.05);
			::setRumble(RumbleStrength_Low, 0.0);
			
			base.removeBullet();
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Inherited methods
	
	function handleEntityImpact(p_entity)
	{
		// Don't do anything
	}
	
	function impact()
	{
		// Don't do anything
	}
	
	function removeBullet()
	{
		unregisterZeroGravityAffectedEntity(this);
		spawnParticleOneShot("particles/fireworks_disappear_" + _color, ::Vector2(0, 0), false, 0, false, ParticleLayer_UseLayerFromParticleEffect, ::frnd_minmax(0.6, 1.4));
		playSoundEffect("fireworks_explode");
		startTimer("removeBullet", 0.15);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Update
	
	function update(p_deltaTime)
	{
		base.update(p_deltaTime);
		
		if (_timeAlive > 0.25 && _isDisabled == false && ::isInZeroGravity(this) == false)
		{
			_speed.y -= 0.01 * 60 * p_deltaTime;
		}
		
		// Dir is a Vector2, which is an instance. Hence no need to call setDirection afterwards
		local bouncyness = 0.5;
		if (hasSurveyResult(SurveyResult_WallLeft)  && _speed.x < 0.0)
		{
			_speed.x = -_speed.x * bouncyness;
		}
		else if (hasSurveyResult(SurveyResult_WallRight) && _speed.x > 0.0)
		{
			_speed.x = -_speed.x * bouncyness;
		}
		else if (hasSurveyResult(SurveyResult_Ceiling)   && _speed.y > 0.0)
		{
			_speed.y = -_speed.y * bouncyness;
		}
		else if (hasSurveyResult(SurveyResult_Floor)     && _speed.y < 0.0)
		{
			_speed.y = -_speed.y * bouncyness;
			_speed.x = _speed.x * 0.85;
			if (_speed.length() < 0.25)
			{
				_speed.y = 0;
			}
		}
	}
}
