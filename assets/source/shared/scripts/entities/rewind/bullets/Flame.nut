class Flame extends Bullet
</
	placeable      = Placeable_Hidden
	collisionRect  = [ 0.0, 0.0, 3.5, 3.5 ]
/>
{
	static c_presentationFile = "presentation/flame";
	static c_impactForce      = 0;
	
	// Settings (defaults)
	_damageValue  = 0.45;
	
	_startScale = 1.0;
	_endScale   = 3.0;
	
	// Settings (must be set)
	_timeout    = null;
	
	// Internal values (don't change)
	_easing = null;
	_yFactor = null;
	_shape = null;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		base.onInit();
	}
	
	function onSpawn()
	{
		_presentation.addCustomValue("lifeTime", _timeout);
		_presentation.addCustomValue("startScale", _startScale);
		_presentation.addCustomValue("endScale", _endScale);
		
		base.onSpawn();
		
		_easing = EasingReal(_startScale, _endScale, _timeout, EasingType_CubicOut);
		setCollisionRectWithVectorRect(::VectorRect(::Vector2(0, 0), _startScale, _startScale));
		_yFactor = ::frnd_minmax(300, 1000);
		
		// Override _touch from base bullet
		if (_touch != null)
		{
			_shape = ::CircleShape(0, _startScale);
			_touch.setShape(_shape);
			_touch.setEnterCallback("onFlameTouchEnter");
			_touch.setExitCallback("onFlameTouchExit");
			_touch.setFilterCallback("onBulletFilter");
		}
		
		startTimer("smoke",  _timeout * 0.7);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Update
	
	function update(p_deltaTime)
	{
		base.update(p_deltaTime);
		
		if (isSuspended())
		{
			return;
		}
		
		_easing.update(p_deltaTime);
		local diameter = _easing.getValue();
		_shape.setMaxRadius(diameter / 2.0);
		
		// Add going up effect to flame at last 25% of life
		local fraction = (_timeout - _timeAlive) / _timeout;
		if (fraction < 0.25)
		{
			_speed.y += 0.75 * p_deltaTime;
		}
		else if (fraction < 0.30)
		{
			// Suspend touch sensor when it's fading
			_touch.setEnabled(false);
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onTimer(p_name)
	{
		local scale = _easing.getValue();
		local effect = spawnParticleOneShot("particles/flame_death", getCenterPosition(), false, 0, true, ParticleLayer_UseLayerFromParticleEffect, scale);
		local angle = ::getAngleFromVector(_speed);
		effect.setInitialExternalImpulse(-angle, scale);
	}
	
	function onFlameTouchEnter(p_entity, p_sensor)
	{
		p_entity.customCallback("onFlameHit", this);
	}
	
	function onWallHit()
	{
		playHitAnim("flame_enemy_hit_wall", -_speed.x * 10, -_speed.y * 20, [0, 1]);
		
		removeBullet();
	}
	
	function onShieldHit(p_shield)
	{
	}
	
	function onWaterTouchEnter()
	{
		spawnParticleOneShot("particles/water_splash_flame", ::Vector2(0, 0), false, 0, false,
			ParticleLayer_UseLayerFromParticleEffect, 1.0);
		
		removeBullet();
	}
	
	function onLavaTouchEnter()
	{
		spawnParticleOneShot("particles/lava_splash_flame", ::Vector2(0, 0), false, 0, false,
			ParticleLayer_UseLayerFromParticleEffect, 1.0);
		
		removeBullet();
	}
	
	function onLavafallTouchEnter()
	{
		spawnParticleOneShot("particles/lava_splash_flame", ::Vector2(0, 0), false, 0, false,
			ParticleLayer_UseLayerFromParticleEffect, 1.0);
		
		removeBullet();
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods
	
	function _typeof()
	{
		return "Flame";
	}
}
