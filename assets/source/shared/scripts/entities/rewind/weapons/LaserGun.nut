include_entity("rewind/weapons/BaseGun");

class LaserBarrel extends GunBarrel
{
	_shape          = null;
	_sensor         = null;
	_touchedEntity  = null;
	_damageCooldown = 0;
	_powerBeam      = null;
	_length         = 0;
};

enum LaserState
{
	Off,
	Preview,
	On
};

class LaserGun extends BaseGun
</
	placeable      = Placeable_Hidden
	movementset    = "Static"
	collisionRect  = [ 0.0, 0.0, 0.1, 0.1 ]
/>
{
	</
		autoGetSet = true
	/>
	_damageValue             = null;  // Damage done every tick
	_damageCooldown          = null;  // Wait x seconds before doing next damage
	_ammoDrainRate           = null;  // How fast does the ammo drain
	_length                  = null;  // Maximum Length of our laserbeam
	_initialDamageMultiplier = 5;     // _damageValue is multiplied with this on initial impact
	_hasPreview              = true;
	
	_state            = LaserState.Off;
	_soundEffect      = null;
	_shooter          = null;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		base.onInit();
		
		// Remove GunBarrels so they can be replaced with laser barrels
		_barrels = [];
		
		// Create the lasers
		for (local i = 0; i < _barrelCount; ++i)
		{
			local laser = LaserBarrel();
			laser._shape = RayShape(::Vector2(0, 0));
			laser._sensor = addTouchSensor(laser._shape, null, ::Vector2(0, 0));
			laser._sensor.setEnterCallback("onLaserTouchEnter");
			laser._sensor.setExitCallback("onLaserTouchExit");
			laser._sensor.setFilterCallback("onLaserFilter");
			laser._sensor.setStopOnWaterPool(true);
			laser._sensor.setDistanceSort(true);
			laser._sensor.setEnabled(false);
			laser._length = _length;
			
			_barrels.push(laser);
		}
		
		updateBarrelAngles();
		
		_shooter = _parent.weakref();
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Class specific methods
	
	function switchOff()
	{
		if (_state == LaserState.Off)
		{
			return;
		}
		
		_state = LaserState.Off;
		
		foreach (laser in _barrels)
		{
			laser._sensor.setEnabled(false);
			if (laser._powerBeam != null)
			{
				removePowerBeamGraphic(laser._powerBeam);
				laser._powerBeam = null;
			}
		}
		
		if (_soundEffect != null)
		{
			_soundEffect.stop();
			_soundEffect = null;
		}
	}
	
	function getLaserBarrelFromSensor(p_sensor)
	{
		foreach (laser in _barrels)
		{
			if (laser._sensor.equals(p_sensor))
			{
				return laser;
			}
		}
		
		return null;
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onProgressRestored(p_id)
	{
		// FIXME: Fix this lost shape thing!
		foreach (laser in _barrels)
		{
			laser._sensor.setShape(laser._shape);
		}
	}
	
	function onLaserTouchEnter(p_entity, p_sensor)
	{
		local laser = getLaserBarrelFromSensor(p_sensor);
		if (laser == null)
		{
			::tt_panic("Couldn't find sensor in barrels. This shouldn't happen.");
			return;
		}
		
		laser._touchedEntity = p_entity.weakref();
		p_entity.customCallback("onLaserEnter", this);
		
		// Set initial damage
		{
			local damage = _damageValue;
			_damageValue *= _initialDamageMultiplier;
			p_entity.customCallback("onLaserHit", this);
			_damageValue = damage;
			laser._damageCooldown = _damageCooldown;
		}
	}
	
	function onLaserTouchExit(p_entity, p_sensor)
	{
		local laser = getLaserBarrelFromSensor(p_sensor);
		if (laser == null)
		{
			::tt_panic("Couldn't find sensor in barrels. This shouldn't happen.");
			return;
		}
		
		if (p_entity.equals(laser._touchedEntity))
		{
			laser._touchedEntity = null;
		}
		
		p_entity.customCallback("onLaserExit", this);
	}
	
	function onLaserFilter(p_entity)
	{
		if (_state != LaserState.On || p_entity.equals(_parent))
		{
			// Shooter should never shoot itself and laser should be switched on
			return false;
		}
		
		if (p_entity instanceof ::RewindEntity)
		{
			if (p_entity.hasProperty("ignoreLasers"))
			{
				return false;
			}
			
			if (p_entity.hasProperty("weaponGroup") && p_entity.getProperty("weaponGroup") != _weaponGroup)
			{
				// Allow hits if weaponGroup is not the same
				return true;
			}
		}
		
		return false;
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Inherited methods
	
	function startFiring()
	{
		// Add additional time for preview laser, but only once
		if (_hasPreview)
		{
			setFiringPattern("-?" + getFiringPattern());
		}
		
		base.startFiring();
	}
	
	function handleFiringSymbol(p_currentSymbol, p_nextSymbol)
	{
		local targetState = LaserState.Off;
		switch (p_currentSymbol)
		{
			case '0': targetState = LaserState.On; break;
			case '-': targetState = p_nextSymbol == '0' ? LaserState.Preview : LaserState.Off; break;
		}
		
		if (_state == targetState)
		{
			// Still same state; don't do anything
			return;
		}
		
		switch (targetState)
		{
			case LaserState.Off:
			{
				switchOff();
			}
			break;
			
			case LaserState.Preview:
			{
				foreach (laser in _barrels)
				{
					laser._sensor.setEnabled(true);
					if (laser._powerBeam != null)
					{
						removePowerBeamGraphic(laser._powerBeam);
					}
					laser._powerBeam = addPowerBeamGraphic(PowerBeamType_PreviewLaser, laser._sensor);
				}
			}
			break;
			
			case LaserState.On:
			{
				foreach (laser in _barrels)
				{
					laser._sensor.setEnabled(true);
					laser._sensor.removeAllSensedEntities();
					if (laser._powerBeam != null)
					{
						removePowerBeamGraphic(laser._powerBeam);
					}
					laser._powerBeam = addPowerBeamGraphic(PowerBeamType_Laser, laser._sensor);
				}
				
				playSoundEffect("laser_fire_start");
				_soundEffect = playSoundEffect("laser_fire_loop");
			}
			break;
		}
		
		_state = targetState;
	}
	
	function stopFiring()
	{
		base.stopFiring();
		
		switchOff();
	}
	
	function updateFiringAngle()
	{
		base.updateFiringAngle();
		
		foreach (laser in _barrels)
		{
			local rad = laser._angle * ::degToRad;
			local dirX = ::sin(rad);
			local dirY = ::cos(rad);
			laser._shape.setOffsetEndPos(::Vector2(laser._length * dirX, laser._length * dirY));
			laser._sensor.setOffset(::Vector2(_barrelLength * dirX, _barrelLength * dirY));
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Update
	
	function childUpdate(p_deltaTime)
	{
		base.childUpdate(p_deltaTime);
		
		if (_state == LaserState.On && _infiniteAmmo == false)
		{
			_ammo -= _ammoDrainRate;
			if (_ammo < 0.0)
			{
				switchOff();
			}
		}
		
		if (_state != LaserState.On)
		{
			return;
		}
		
		foreach (laser in _barrels)
		{
			if (laser._touchedEntity != null)
			{
				laser._damageCooldown -= p_deltaTime;
				if (laser._damageCooldown <= 0.0)
				{
					// Do damage
					laser._touchedEntity.customCallback("onLaserHit", this);
					laser._damageCooldown = _damageCooldown;
				}
			}
		}
	}
}
