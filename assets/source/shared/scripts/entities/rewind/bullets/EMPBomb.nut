include_entity("rewind/bullets/Deflectable");

class EMPBomb extends Deflectable
</
	placeable      = Placeable_Hidden
	movementset    = "Static"
	collisionRect  = [ 0.0, 0.0, 0.05, 0.05 ]
	group          = "Rewind"
/>
{
	// Constants
	static c_presentationFile = "presentation/emp_grenade";
	static c_beamTimeout = 0.5;
	static c_waterDrag   = 0.12 * 60;
	static c_waterImpact = 0.53;
	
	_proximitySensor    = null;
	
	_timeout     = 3.5;
	_beams       = null;
	_stuck       = false;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		base.onInit();
		
		_beams = [];
		
		_proximitySensor = addTouchSensor(::CircleShape(0, 9.0));
		_proximitySensor.setEnterCallback("onTouchEnter");
		_proximitySensor.setFilterCallback("onTouchFilter");
		
		local light = addLight(::Vector2(0, 0), 0.0, 1.0);
		light.setRadius(8.0, 0.1);
		light.setTexture("empbomb");
		
		startTimer("disappear", _timeout - 0.5);
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
		p_entity.customCallback("onEMPHit");
		
		// Create electricity beam to it
		local sensor = addSightSensor(::CircleShape(0, 100), null, ::Vector2(0, 0));
		sensor.setTarget(p_entity);
		local beam = addPowerBeamGraphic(PowerBeamType_Electricity, sensor);
		_beams.push({beam = beam, sensor = sensor, timeout = c_beamTimeout, soundeffect = playSoundEffect("emp_hit_spark_loop")});
		playSoundEffect("emp_hit_impact")
	}
	
	function onTouchFilter(p_entity)
	{
		if (_isDisabled)
		{
			return false;
		}
		
		return p_entity != _shooter && p_entity.hasProperty("noticeEMP") && p_entity.containsVirus() == false;
	}
	
	function onShieldHit(p_shield)
	{
		// Don't do anything
	}
	
	function onWallHit()
	{
		// Prevent baseclass from calling onWallHit()
		if (_inWater || _isDisabled)
		{
			local effectname = _inWater ? "particles/underwater_remove_missile" : "particles/abovewater_remove_missile";
			spawnParticleOneShot(effectname, ::Vector2(0, 0), false, 0, false,
				ParticleLayer_UseLayerFromParticleEffect, 1.0);
			removeBullet();
		}
	}
	
	function onTimer(p_name)
	{
		if (p_name == "disappear" && _isDisabled == false)
		{
			_presentation.start("disappear", _inWater ? ["submerged"] : [], false, 0);
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Inherited methods
	
	function handleEntityImpact(p_entity)
	{
		if (p_entity.hasProperty("deflectEMPBomb"))
		{
			// Deflect bomb
			local normal = (getCenterPosition() - p_entity.getCenterPosition()).normalize();
			reflect(normal, 0.8);
		}
	}
	
	function impact()
	{
	}
	
	function removeBullet()
	{
		unregisterZeroGravityAffectedEntity(this);
		spawnParticleOneShot("particles/emp_disappear", ::Vector2(0, 0), false, 0, false, ParticleLayer_UseLayerFromParticleEffect, 1.0);
		
		// Ensure sfx are gone
		for (local i = 0; i < _beams.len(); ++i)
		{
			if (_beams[i].soundeffect != null)
			{
				_beams[i].soundeffect.stop();
				_beams[i].soundeffect = null;
			}
		}
		
		base.removeBullet();
	}
	
	function onWallHit()
	{
		if (_stuck)
		{
			return;
		}
		
		_stuck = true;
		setPosition(getPosition() - _speed);
		spawnParticleOneShot("particles/grenade_land", ::Vector2(0, 0), true, 0, false, ParticleLayer_UseLayerFromParticleEffect, 1.0);
		playSoundEffect("emp_bounce");
		_speed = ::Vector2(0, 0);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Update
	
	function update(p_deltaTime)
	{
		base.update(p_deltaTime);
		
		// Do beam bookkeeping
		for (local i = 0; i < _beams.len();)
		{
			_beams[i].timeout -= p_deltaTime;
			if (_beams[i].timeout <= 0.0)
			{
				removeSensor(_beams[i].sensor);
				removePowerBeamGraphic(_beams[i].beam);
				_beams[i].soundeffect.stop();
				_beams[i].soundeffect = null;
				_beams.remove(i);
				// Don't increment index; continue with same index as that one has just been removed
			}
			else
			{
				++i;
			}
		}
		
		if (_stuck == false && _timeAlive > 0.25 && _isDisabled == false && ::isInZeroGravity(this) == false)
		{
			_speed.y -= 0.01 * 60 * p_deltaTime;
		}
	}
}
