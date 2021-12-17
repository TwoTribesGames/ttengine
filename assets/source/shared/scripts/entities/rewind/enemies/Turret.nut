include_entity("rewind/enemies/BaseTurret");

class Turret extends BaseTurret
</
	editorImage         = "editor.enemyturret"
	libraryImage        = "editor.library.enemyturret"
	placeable           = Placeable_Everyone
	movementset         = "StaticIdle"
	collisionRect       = [ 0.0, 0.75, 1.5, 1.5 ]
	group               = "01. Enemies"
	pathFindAgentRadius = 0.666
	pathCrowdSeparation = true
	stickToEntity       = true
/>
{
	</
		type        = "string"
		choice      = ::getNonLaserPresetNames(::g_turretGunPresets)
		order       = 0.0
		group       = "Firing"
	/>
	preset = "bullet";
	
	</
		type        = "bool"
		order       = 2.3
		group       = "Sight"
		conditional = "onlyFireWhenTargetInSight == true"
		description = "Setting this to true will enable the shield when the player is in sight."
	/>
	enableShieldWhenTargetInSight = false;
	
	</
		type        = "bool"
		order       = 3.0
		group       = "Shield"
	/>
	hasShield = false;
	
	</
		type        = "bool"
		order       = 3.1
		group       = "Shield"
		conditional = "hasShield == true"
	/>
	hasIndestructibleShield = false;
	
	</
		type        = "string"
		choice      = ["Detect", "left", "right", "up", "down", "none"]
		description = "Choose a specific direction to override the auto detection of the floor direction"
		group       = "StickTo"
		order       = 999
	/>
	stickToCollision = "Detect";
	
	// Constants
	static c_maxHealth                  = 5;
	static c_score                      = 50;
	static c_shieldRange                = 2.2;
	static c_shieldHealth               = 20;
	static c_gunOffset                  = ::Vector2(0, 0.3);
	static c_isRemovedFromWorld         = false;
	static c_dazedByEMPTimeout          = 3.0;
	static c_pickupDropCount            = 12;
	static c_killOnOutOfAmmo            = false;
	static c_isAffectedByVirus          = true;
	static c_energy                     = 4;
	static c_healthMultiplierWhenHacked = 15.0;
	
	showBarrel           = true;
	
	// Internals
	_patternMovement     = null;
	_followSensor        = null;
	_followMoveSettings  = null;
	_secondaryWeapon     = null;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInvalidProperties
	
	function onInvalidProperties(p_properties)
	{
		local valueFixes = {}
		
		valueFixes.rotation      <- { result = @(val) val.tointeger() };
		valueFixes.rotationSpeed <- { result = @(val) val.tointeger() };
		
		if ("shieldSpread" in p_properties && (p_properties.shieldSpread.tointeger() == 360))
		{
			p_properties.hasIndestructibleShield <- "true";
		}
		
		fixInvalidValues(p_properties, valueFixes);
		
		p_properties = removeNonexistentProperties(this, p_properties);
		
		return p_properties;
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		base.onInit();
		
		if (enableShieldWhenTargetInSight && hasShield == false)
		{
			editorWarning("enableShieldWhenTargetInSight without enabling hasShield");
		}
		
		if (stickToCollision == "Detect")
		{
			setFloorDirection(::getTouchingSolid(this, Direction_None)); // Fall back to down when it's not touching anything
		}
		else
		{
			local direction = getDirectionFromName(stickToCollision);
			if (isValidDirection(direction) == false) direction = Direction_Down;
			setFloorDirection(direction);
		}
		
		_presentation = createPresentationObject("presentation/" + getType().tolower());
		if (c_isAffectedByVirus && _presentation != null)
		{
			_presentation.addTag("ishackable");
		}
		
		addProperty("touchDamage", 1);
		addProperty("touchDamageDoesntKillPlayer");
		addProperty("healedByHealthBot");
		
		// For the follow player virus
		_followSensor = addTouchSensor(::CircleShape(0, 4), null, ::Vector2(0, 0));
		_followSensor.setExitCallback("onFollowExit");
		_followSensor.setDelay(0.0);
		_followSensor.setStopOnWaterPool(true); // He won't see you while you are under water!
		_followSensor.setEnabled(false);
		_followMoveSettings = PhysicsSettings(c_mass, 19.0, 550, 4, 2);
	}
	
	function onSpawn()
	{
		base.onSpawn();
		
		if (_shield != null)
		{
			_shield.setAngle(_angle);
			
			// Disable shield when it should only show when target is in sight.
			if (enableShieldWhenTargetInSight)
			{
				_shield.disableShield(false);
			}
		}
		
		updatePresentation();
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onFollowExit(p_entity, p_sensor)
	{
		moveToPlayer();
	}
	
	function onSightEnter(p_entity, p_sensor)
	{
		base.onSightEnter(p_entity, p_sensor);
		
		if (onlyFireWhenTargetInSight && enableShieldWhenTargetInSight && _shield != null)
		{
			_shield.enableShield();
		}
	}
	
	function onSightExit(p_entity, p_sensor)
	{
		base.onSightExit(p_entity, p_sensor);
		
		if (onlyFireWhenTargetInSight && enableShieldWhenTargetInSight && _shield != null)
		{
			_shield.disableShield();
		}
	}
	
	function onParentStartFiring(p_parent)
	{
		if (_gun.isFiring() == false)
		{
			_gun.startFiring();
		}
	}
	
	function onParentStopFiring(p_parent)
	{
		if (_gun.isFiring())
		{
			_gun.stopFiring();
		}
	}
	
	function onVirusUploaded(p_entity)
	{
		base.onVirusUploaded(p_entity);
		
		// Make sure it is flying
		if (getFloorDirection() != Direction_None)
		{
			spawnParticleOneShot("particles/wallturret_hacked", ::Vector2(0, 0), false, 0, false, ParticleLayer_UseLayerFromParticleEffect, 1.0);
			playSoundEffect("turret_detach");
			setFloorDirection(Direction_None);
			updatePresentation();
		}
		
		local preset = clone p_entity.c_bulletPropertiesNormal;
		
		// change presentation and muzzle flash
		preset._gunPresentationName   = "turretenemy_turret_bullet";
		preset._muzzleFlashEffectName = "muzzleflash";
		createGunFromPreset(preset, {});
		
		_gun.setInfiniteAmmo(true);
		_gun._weaponGroup = WeaponGroup.Player;
		removeProperty("touchDamage");
		removeProperty("healedByHealthBot");
		
		_secondaryWeapon = addChild("SecondaryWeapon", getCenterOffset());
		
		addProperty("weaponGroup", ::WeaponGroup.Player);
		setUpdateSurvey(true);
		
		if (getFloorDirection() == Direction_None)
		{
			_followSensor.setTarget(p_entity);
			_followSensor.setEnabled(true);
			moveToPlayer();
		}
		
		if (_sightSensor != null)
		{
			removeSensor(_sightSensor);
			_sightSensor = null;
		}
	}
	
	function onVirusRemoved()
	{
		base.onVirusRemoved();
		
		::killEntity(_secondaryWeapon);
		_secondaryWeapon = null;
		
		setUpdateSurvey(false);
		_gun.stopFiring();
		_gun.setInfiniteAmmo(false);
		_gun.setAmmo(0);
		addProperty("touchDamage", 1);
		addProperty("weaponGroup", ::WeaponGroup.Enemy);
		addProperty("healedByHealthBot");
		_followSensor.setEnabled(false);
		stopTimer("moveToPlayer");
		stopMovement();
	}
	
	function onVirusHealthEmpty()
	{
		::killEntity(this);
	}
	
	function onParentFiringSecondary(p_parent, p_secondary)
	{
		if (_secondaryWeapon != null)
		{
			_secondaryWeapon.select(p_secondary._type);
			_secondaryWeapon.setFiringAngle(_gun.getFiringAngle());
			_secondaryWeapon.setAmmo(1);
			_secondaryWeapon.fire();
			decreaseVirusHealth(0.1);
		}
	}
	
	function onMovementEnded(p_direction)
	{
		startCallbackTimer("moveToPlayer", 0.5);
	}
	
	function onMovementFailed(p_direction, p_moveName)
	{
		startCallbackTimer("moveToPlayer", 0.5);
	}
	
	function onPathMovementFailed(p_direction)
	{
		startCallbackTimer("moveToPlayer", 0.5);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Inherited methods
	
	function createShield()
	{
		if (hasShield)
		{
			return addChild("EnemyShield", getCenterOffset() + c_gunOffset,
				{
					_type   = hasIndestructibleShield ? ShieldType.Indestructible : ShieldType.Destructible,
					_range  = c_shieldRange,
					_health = c_shieldHealth
				}
			);
		}
		else
		{
			return null;
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods
	
	function moveToPlayer()
	{
		local player = ::getFirstEntityByTag("PlayerBot");
		if (::isValidEntity(player))
		{
			startPathMovementToEntity(player, ::Vector2(0, 3), _followMoveSettings);
		}
		else
		{
			_followSensor.setEnabled(false);
		}
	}
	
	function updatePresentation()
	{
		if (_presentation != null)
		{
			if (getFloorDirection() == Direction_None)
			{
				_presentation.addTag("flying");
				
				_patternMovement = ::PatternMovement(this);
				_patternMovement.startMovement(moveHover, ::Vector2(0, 0), ::frnd_minmax(0.0, 10.0));
				
				if (_healthBar._hitPresentation != null) _healthBar._hitPresentation.addTag("flying");
			}
			
			_presentation.start("idle", [], false, 0);
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Movement patterns
	
	function moveHover(p_time, p_previousResult)
	{
		local result = ::PatternMovement.generate.cos_y(p_time, 0.125, 0.5);
		result += ::PatternMovement.generate.cos_y(p_time, 0.178, 0.6255);
		result += ::PatternMovement.generate.cos_x(p_time, 0.078, 0.255);
		return result;
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Update
	
	function update(p_deltaTime)
	{
		base.update(p_deltaTime);
		
		if (_patternMovement != null && containsVirus() == false)
		{
			_patternMovement.update(p_deltaTime);
		}
		
		if (isEnabled() == false)
		{
			return;
		}
		
		if (containsVirus())
		{
			local player = ::getFirstEntityByTag("PlayerBot");
			if (player != null)
			{
				_angle = player._aimAngle;
				_gun.setFiringAngleTarget(_angle);
				if (_gun.isFiring())
				{
					decreaseVirusHealth(0.07 * p_deltaTime);
				}
			}
			else if (_gun.isFiring())
			{
				_gun.stopFiring();
			}
		}
		
		if (_shield != null)
		{
			_shield.setAngle(_gun.getFiringAngle());
		}
	}
}
Turret.setattributes("showBarrel", null);
