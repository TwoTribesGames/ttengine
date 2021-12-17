include_entity("rewind/bullets/Missile");

class HomingMissile extends Missile
</ 
	placeable      = Placeable_Hidden
	movementset    = "Static"
	collisionRect  = [ 0.0, 0.0, 0.5, 0.5 ]
	group          = "Rewind"
/>
{
	static c_presentationFile = "presentation/missile_homing";
	
	_propulsionSound = null;
	
	_sightSensor = null;
	_sightRadius = 10;
	
	_timeout     = 16;
	
	</
		autoGetSet = true
	/>
	_target = null;
	
	// Seek setting
	_seekCooldown   = 0.0;
	
	// FIXME: Is per frame, so behavior changes in bullettime
	_huntSpeedFactor               = 1.003; // multiply the speed with this factor each frame when missile acquires target
	_huntMaxRotationPerFrameFactor = 0.99; // multiply the _maxRotationPerFrame with this factor each frame when missile acquires target
	
	_damageValue     = 3;
	_explosionRadius = 7.5;
	
	// Wobble settings
	static c_wobbleStartTime = 0.2;
	_wobbleAmplitude = 3.0;
	_wobbleFrequency = 20;
	
	// Homing settings
	static c_rotationDamping  = 0.5; // multiplication factor to calculate the rotation rate.
	_maxRotationPerFrame = 0.1;  // max degrees of change per game frame
	_sightShape          = null;
	_sightSpread         = 50;
	_potentialTargets    = null;
	_hitsDeflectables    = true;
	
	//-------------------------------------------------------------------------
	// Overloaded methods
	function onInit()
	{
		base.onInit();
		
		_potentialTargets = [];
		
		_sightShape = ::ConeShape(0, _sightRadius + ::rnd_minmax(5.0, 12.0), ::wrapAngle(_angle), _sightSpread);
		_sightSensor = addSightSensor(_sightShape);
		_sightSensor.setEnterCallback("onMissileSightEnter");
		_sightSensor.setFilterCallback("onMissileSightFilter");
		_sightSensor.setStopOnWaterPool(true);
		
		// 33% of all missiles home at the closest target
		_sightSensor.setDistanceSort(::frnd() < 0.33);
	}
	
	function onSpawn()
	{
		base.onSpawn();
		
		if (_seekCooldown > 0)
		{
			startTimer("seek", _seekCooldown);
			_sightSensor.setEnabled(false);
		}
		else
		{
			_sightSensor.setEnabled(true);
		}
		
		_maxRotationPerFrame *= ::degToRad;
	}
	
	function onProgressRestored(p_id)
	{
		// reconnect the shape with the sensor because that gets lost
		_sightSensor.setShape(_sightShape);
	}
	
	function onTimer(p_name)
	{
		if (p_name == "seek")
		{
			_sightSensor.setEnabled(true);
		}
	}
	
	function onShieldHit(p_shield)
	{
		removeHoming();
		
		base.onShieldHit(p_shield);
	}
	
	function onBulletHit(p_bullet)
	{
		_presentation.stop();
		_presentation.start("idle", [], false, 0);
		spawnParticleOneShot("particles/hit_sparks", ::Vector2(0, 0), false, 0, false, ParticleLayer_UseLayerFromParticleEffect, 1.0);
		// This defuses the bomb to make it less harmfull when it hits close to you.
		removeHoming();
		_explosionRadius = 1;
		_damageValue = 1;
		
		base.onBulletHit(p_bullet);
	}
	
	function update(p_deltaTime)
	{
		if (isSuspended()) return;
		
		_sightSpread += 700 * p_deltaTime;
		if (_sightSpread >= 360)
		{
			_sightSpread = 360;
		}
		_sightShape.setSpread(_sightSpread);
		
		if (_target != null && _isDisabled == false)
		{
			_speed               *= _huntSpeedFactor;
			_maxRotationPerFrame *= _huntMaxRotationPerFrameFactor;
			
			if (::isValidEntity(_target) == false)
			{
				_sightSensor.removeAllSensedEntities();
				_sightSensor.setEnabled(true);
				_target = null;
			}
			else
			{
				homeToTarget(_target, c_rotationDamping, _maxRotationPerFrame, p_deltaTime);
			}
		}
		
		base.update(p_deltaTime);
	}
	
	//-------------------------------------------------------------------------
	// Class specific methods
	
	function homeToTarget(p_target, p_damping, p_maxRotation, p_deltaTime)
	{
		// Inspired by: http://en.sfml-dev.org/forums/index.php?topic=5523.0
		local dxdy = p_target.getCenterPosition() - getCenterPosition();
		local v    = _speed;
		
		local dxdyLen = dxdy.length();
		local vLen    = v.length();
		
		// Watch out for division by zero
		if (dxdyLen < 0.0001 || vLen < 0.0001)
		{
			return;
		}
		
		// Normalize
		local vNorm    = v    / vLen;
		local dxdyNorm = dxdy / dxdyLen;
		local perpDot   = ::Vector2.perpDot(vNorm, dxdyNorm);
		local angleDiff = asin(perpDot) * p_damping;
		angleDiff = ::clamp(angleDiff, -p_maxRotation, p_maxRotation) * 60 * p_deltaTime;
		_speed = v.rotate(angleDiff * ::radToDeg);
		updateAngle();
	}
	
	function removeHoming()
	{
		// Basically becomes normal rocket
		setTarget(null);
		_sightSensor.setEnabled(false);
		stopAllTimers();
	}
	
	function isEntityOfTargetClass(p_entity)
	{
		return p_entity.hasProperty("attractHomingMissiles") && 
		       _shooter != null && _shooter.getProperty("weaponGroup") != p_entity.getProperty("weaponGroup");
	}
	
	function hunt()
	{
		// pick a random target from the list
		local index = rnd_minmax(0, _potentialTargets.len()-1);
		_target = _potentialTargets[index]
		if (_target == null)
		{
			_sightSensor.removeAllSensedEntities();
		}
		else
		{
			_sightSensor.setEnabled(false);
		}
		setTarget(_target);
	}
	
	function onMissileSightEnter(p_entity, p_sensor)
	{
		// Only hunt if existing target is not valid
		if (::isValidEntity(getTarget()) == false)
		{
			_potentialTargets.push(p_entity.weakref());
			if (hasTimer("hunt") == false)
			{
				startCallbackTimer("hunt", ::frnd_minmax(0.1, 0.3));
			}
		}
	}
	
	function onMissileSightFilter(p_entity)
	{
		return isEntityOfTargetClass(p_entity);
	}
}
