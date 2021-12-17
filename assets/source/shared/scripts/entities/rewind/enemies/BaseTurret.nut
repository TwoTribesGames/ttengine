include_entity("rewind/enemies/BaseEnemy");
include_entity("rewind/weapons/LaserGun");
include_entity("rewind/weapons/ProjectileGun");

::g_turretGunPresets <- {
	//-------------------------------------------------------------------------
	// PROJECTILES
	bullet = {
		_gunType                     = "ProjectileGun",
		_gunPresentationName         = "turretenemy_turret_bullet",
		_muzzleFlashEffectName       = "muzzleflash",
		_projectileType              = "EnemyBullet",
		_projectileSpeed             = 0.5,
		_projectileParentSpeedFactor = 0.0,
		_firingRate                  = 7.5,
		_barrelCount                 = 1,
		_barrelLength                = 1.7,
		_projectileSpawnProps        =
		{
			_timeout  = 3.0
		}
	},
	bullet_fast = {
		_gunType                     = "ProjectileGun",
		_gunPresentationName         = "turretenemy_turret_bullet",
		_muzzleFlashEffectName       = "muzzleflash",
		_projectileType              = "EnemyBullet",
		_projectileSpeed             = 0.5,
		_projectileParentSpeedFactor = 0.0,
		_firingRate                  = 15,
		_barrelCount                 = 1,
		_barrelLength                = 1.7,
		_projectileSpawnProps        =
		{
			_timeout  = 2.0
		}
	},
	emp = {
		_gunType                     = "ProjectileGun",
		_gunPresentationName         = "turretenemy_turret_missile",
		_muzzleFlashEffectName       = "muzzleflash_player",
		_projectileType              = "EMPBomb",
		_projectileSpeed             = 0.5,
		_projectileSpeedRandom       = 0.2,
		_projectileParentSpeedFactor = 0.0,
		_firingRate                  = 0.5,
		_barrelCount                 = 1,
		_barrelLength                = 1.0,
		_barrelRandomSpread          = 10
	},
	grenade = {
		_gunType                     = "ProjectileGun",
		_gunPresentationName         = "turretenemy_turret_missile",
		_muzzleFlashEffectName       = "muzzleflash_player",
		_projectileType              = "Grenade",
		_projectileSpeed             = 0.5,
		_projectileSpeedRandom       = 0.2,
		_projectileParentSpeedFactor = 0.0,
		_firingRate                  = 2.0,
		_barrelCount                 = 1,
		_barrelLength                = 1.0,
		_barrelRandomSpread          = 10
	},
	grenadespread = {
		_gunType                     = "ProjectileGun",
		_gunPresentationName         = "turretenemy_turret_missile",
		_muzzleFlashEffectName       = "muzzleflash",
		_projectileType              = "Grenade",
		_projectileSpeed             = 0.6,
		_projectileSpeedRandom       = 0.2,
		_projectileParentSpeedFactor = 0.0,
		_projectilesPerBarrel        = 3,
		_projectilesSpread           = 40,
		_projectilesRandomSpread     = 5,
		_firingRate                  = 0.6,
		_barrelCount                 = 1,
		_barrelLength                = 1.0,
		_barrelRandomSpread          = 10
	},
	depthbomb = {
		_gunType                     = "ProjectileGun",
		_gunPresentationName         = "turretenemy_turret_missile",
		_muzzleFlashEffectName       = "muzzleflash",
		_projectileType              = "DepthBomb",
		_projectileSpeed             = 0.4,
		_projectileSpeedRandom       = 0.4,
		_projectileParentSpeedFactor = 0.0,
		_projectilesPerBarrel        = 5,
		_projectilesSpread           = 22,
		_projectilesRandomSpread     = 15,
		_firingRate                  = 0.5,
		_barrelCount                 = 1,
		_barrelLength                = 4.5,
		_barrelRandomSpread          = 10
	},
	missile = {
		_gunType                     = "ProjectileGun",
		_gunPresentationName         = "turretenemy_turret_missile",
		_muzzleFlashEffectName       = "muzzleflash",
		_projectileType              = "Missile",
		_projectileSpeed             = 0.3,
		_projectileParentSpeedFactor = 0.0,
		_firingRate                  = 1,
		_barrelCount                 = 1,
		_barrelLength                = 1.7,
		_projectileSpawnProps        =
		{
			_timeout  = 10.0
		}
	},
	missile_steelworks = {
		_gunType                     = "ProjectileGun",
		_gunPresentationName         = "turretenemy_turret_missile",
		_muzzleFlashEffectName       = "muzzleflash",
		_projectileType              = "Missile",
		_projectileSpeed             = 0.3,
		_projectileSpeedRandom       = 0.1,
		_projectileParentSpeedFactor = 0.0,
		_firingRate                  = ::frnd_minmax(0.8, 1.1),
		_barrelCount                 = 1,
		_barrelLength                = 1.7,
		_barrelRandomSpread          = 0,
		_projectileSpawnProps        =
		{
			_timeout  = 20.0
		}
	},
	missile_trainride = {
		_gunType               = "ProjectileGun",
		_gunPresentationName   = "turretenemy_turret_missile",
		_muzzleFlashEffectName = "muzzleflash",
		_projectileType        = "Missile",
		_projectileSpeed       = 0.5,
		_firingRate            = 10.0,
		_barrelCount           = 1,
		_barrelLength          = 1.0,
		_barrelRandomSpread    = 20,
		_ammo                  = 35,
		_projectileSpawnProps  =
		{
			_bulletHitDeflectAngle   = 60.0
			_timeout                 = 30.0,
			_damageValue             = 3.0
			_explosionRadius         = 2.0,
		}
	},
	missile_asteroidfield = {
		_gunType                     = "ProjectileGun",
		_gunPresentationName         = "turretenemy_turret_missile",
		_muzzleFlashEffectName       = "muzzleflash",
		_projectileType              = "Missile",
		_projectileSpeed             = 0.15,
		_projectileParentSpeedFactor = 0.0,
		_firingRate                  = 0.25,
		_barrelCount                 = 1,
		_barrelLength                = 1.7,
		_ammo                        = 1,
		_projectileSpawnProps        =
		{
			_timeout                 = 100.0,
			_damageValue             = 7.0
			_explosionRadius         = 3.0,
		}
	},
	homingmissile = {
		_gunType                     = "ProjectileGun",
		_gunPresentationName         = "turretenemy_turret_missile",
		_muzzleFlashEffectName       = "muzzleflash",
		_projectileType              = "HomingMissile",
		_projectileSpeed             = 0.35,
		_projectileParentSpeedFactor = 0.0,
		_projectileSpeedRandom       = 0.1,
		_projectilesPerBarrel        = 2,
		_projectilesSpread           = 30,
		_projectilesRandomSpread     = 5,
		_firingRate                  = 1,
		_barrelCount                 = 1,
		_barrelLength                = 1.7,
		_projectileSpawnProps        =
		{
			_sightRadius         = 15,
			_damageValue         = 3,
			_explosionRadius     = 3.0,
			_maxRotationPerFrame = 4,
			_seekCooldown        = 0.3,
			_timeout             = 5.0
		}
	},
	homingmissile_walkerturret = {
		_gunType                     = "ProjectileGun",
		_gunPresentationName         = "turretenemy_turret_missile",
		_muzzleFlashEffectName       = "muzzleflash",
		_projectileType              = "HomingMissile",
		_projectileSpeed             = 0.35,
		_projectileParentSpeedFactor = 0.0,
		_projectileSpeedRandom       = 0.1,
		_projectilesPerBarrel        = 3,
		_projectilesSpread           = 40,
		_projectilesRandomSpread     = 0,
		_firingRate                  = 1.0,
		_barrelCount                 = 1,
		_barrelLength                = 2.7,
		_projectileSpawnProps        =
		{
			_sightRadius         = 15,
			_damageValue         = 3,
			_explosionRadius     = 3.0,
			_maxRotationPerFrame = 4,
			_seekCooldown        = 0.1,
			_timeout             = 5.0
		}
	},
	homingmissile_walkerturret_warproom = {
		_gunType                        = "ProjectileGun",
		_gunPresentationName            = "turretenemy_turret_missile",
		_muzzleFlashEffectName          = "muzzleflash",
		_projectileType                 = "HomingMissile",
		_projectileSpeed                = 0.35,
		_projectileParentSpeedFactor    = 0.0,
		_projectileSpeedRandom          = 0.1,
		_projectilesPerBarrel           = 2,
		_projectilesSpread              = 40,
		_projectilesRandomSpread        = 0,
		_firingRate                     = 0.75,
		_barrelCount                    = 1,
		_barrelLength                   = 1.7,
		_projectileSpawnProps           =
		{
			_sightRadius         = 15,
			_damageValue         = 3,
			_explosionRadius     = 3.0,
			_maxRotationPerFrame = 3,
			_seekCooldown        = 0.1,
			_timeout             = 5.0
		}
	},
	homingmissile_trainride  = {
		_gunType               = "ProjectileGun",
		_gunPresentationName   = "turretenemy_turret_missile",
		_muzzleFlashEffectName = "muzzleflash_player",
		_projectileType        = "HomingMissile",
		_projectileSpeed       = 0.65,
		_projectileSpeedRandom = 0.2,
		_projectilesPerBarrel  = 4,
		_projectilesSpread     = 30,
		_firingRate            = 4,
		_barrelCount           = 1,
		_barrelLength          = 1.0,
		_barrelSpread          = 25,
		_projectileSpawnProps  =
		{
			_huntSpeedFactor     = 1.004,
			_sightRadius         = 100,
			_damageValue         = 3,
			_explosionRadius     = 3.0,
			_maxRotationPerFrame = 4,
			_seekCooldown        = 0.2,
			_timeout             = 10.0
		}
	},
	flame = {
		_gunType                     = "FlameGun",
		_gunPresentationName         = "turretenemy_turret_flamethrower",
		_muzzleFlashEffectName       = null,
		_projectileType              = "Flame",
		_projectileSpeed             = 0.30,
		_projectileParentSpeedFactor = 1.0,
		_firingRate                  = 25.0,
		_barrelCount                 = 1,
		_barrelLength                = 2.1,
		_projectileSpawnProps        =
		{
			_timeout  = 0.8
		}
	},
	flame_endboss = {
		_gunType                     = "FlameGun",
		_gunPresentationName         = "turretenemy_turret_flamethrower",
		_muzzleFlashEffectName       = null,
		_projectileType              = "Flame",
		_projectileSpeed             = 0.30,
		_projectileParentSpeedFactor = 0.0,
		_firingRate                  = 25.0,
		_barrelCount                 = 1,
		_barrelLength                = 0.1,
		_projectileSpawnProps        =
		{
			_timeout  = 0.8
		}
	},
	firemissile_result = {
		_gunType               = "ProjectileGun",
		_gunPresentationName   = "turretenemy_turret_bullet",
		_muzzleFlashEffectName = null,
		_projectileType        = "Flame",
		_projectileSpeed       = 0.01,
		_firingRate            = 5.0,
		_barrelCount           = 1,
		_barrelLength          = 0,
		_projectileSpawnProps  =
		{
			_timeout  = 1.5
		}
	}

	//-------------------------------------------------------------------------
	// LASERS
	laser = {
		_gunType             = "LaserGun",
		_gunPresentationName = "turretenemy_turret_laser",
		_barrelLength        = 1.5,
		_ammoDrainRate       = 0.2,
		_damageValue         = 1.0,
		_damageCooldown      = 0.08,
		_length              = 20.0,
		_firingPattern       = "0",
		_firingRate          = 2.0,
	},
	laser_asteroidfield = {
		_gunType             = "LaserGun",
		_gunPresentationName = "turretenemy_turret_laser",
		_barrelLength        = 1.5,
		_ammoDrainRate       = 0.2,
		_damageValue         = 0.5,
		_damageCooldown      = 0.08,
		_length              = 20.0,
		_firingPattern       = "0",
		_firingRate          = 2.0,
	},
	laser_noturret = {
		_gunType             = "LaserGun",
		_gunPresentationName = "turretenemy_turret_laser",
		_barrelLength        = 0.0,
		_ammoDrainRate       = 0.2,
		_damageValue         = 1.0,
		_damageCooldown      = 0.08,
		_length              = 20.0,
		_firingPattern       = "0",
		_firingRate          = 10,
		_hasPreview          = false
	},
	laser_slow = {
		_gunType             = "LaserGun",
		_gunPresentationName = "turretenemy_turret_laser",
		_barrelLength        = 1.5,
		_ammoDrainRate       = 0.2,
		_damageValue         = 1.0,
		_damageCooldown      = 0.08,
		_length              = 20.0,
		_firingPattern       = "0",
		_firingRate          = 0.75,
	},
	laser_ultraslow = {
		_gunType             = "LaserGun",
		_gunPresentationName = "turretenemy_turret_laser",
		_barrelLength        = 0.0,
		_ammoDrainRate       = 0.2,
		_damageValue         = 1.0,
		_damageCooldown      = 0.08,
		_length              = 20.0,
		_firingPattern       = "0",
		_firingRate          = 0.375,
	},
	laser_unstable_wavesfight = {
		_gunType             = "LaserGun",
		_gunPresentationName = "turretenemy_turret_laser",
		_barrelLength        = 1.5,
		_barrelCount         = 5,
		_barrelSpread        = 15,
		_ammoDrainRate       = 0.2,
		_damageValue         = 1.0,
		_damageCooldown      = 0.08,
		_length              = 20.0,
		_firingPattern       = "0",
		_firingRate          = 0.33,
	},
	laser_triple = {
		_gunType             = "LaserGun",
		_gunPresentationName = "turretenemy_turret_laser",
		_barrelLength        = 1.5,
		_barrelCount         = 3,
		_barrelSpread        = 15,
		_ammoDrainRate       = 0.2,
		_damageValue         = 1.0,
		_damageCooldown      = 0.08,
		_length              = 20.0,
		_firingPattern       = "0",
		_firingRate          = 1,
	},
	laser_steelworks_wavesfight = {
		_gunType             = "LaserGun",
		_gunPresentationName = "turretenemy_turret_laser",
		_barrelLength        = 1.5,
		_ammoDrainRate       = 0.2,
		_damageValue         = 1.0,
		_damageCooldown      = 0.08,
		_length              = 20.0,
		_firingPattern       = "0",
		_firingRate          = 0.666,
	},
	laser_ultrafast = {
		_gunType             = "LaserGun",
		_gunPresentationName = "turretenemy_turret_laser",
		_barrelLength        = 1.5,
		_ammoDrainRate       = 0.2,
		_damageValue         = 1.0,
		_damageCooldown      = 0.08,
		_length              = 20.0,
		_firingPattern       = "0",
		_firingRate          = 10.00,
		_hasPreview          = false
	},
	laser_asteroidfield = {
		_gunType             = "LaserGun",
		_gunPresentationName = "turretenemy_turret_laser",
		_barrelLength        = 0.0,
		_ammoDrainRate       = 0.2,
		_damageValue         = 1.0,
		_damageCooldown      = 0.08,
		_length              = 20.0,
		_firingPattern       = "0",
		_firingRate          = 0.5,
	}
}

class BaseTurret extends BaseEnemy
</
	editorImage         = "editor.baseturret"
	libraryImage        = "editor.library.baseturret"
	placeable           = Placeable_Everyone
	movementset         = "StaticIdle"
	collisionRect       = [ 0.0, 0.75, 1.5, 1.5 ]
	group               = "01. Enemies"
	stickToEntity       = true
/>
{
	</
		type        = "string"
		choice      = ::getGunPresetNames(::g_turretGunPresets)
		order       = 0.0
		group       = "Firing"
	/>
	preset = "bullet";

	</
		type        = "integer"
		min         = 1
		max         = 150
		order       = 0.1
		group       = "Firing"
		conditional = "preset contains laser"
	/>
	laserLength = 20;

	</
		type        = "bool"
		order       = 0.2
		group       = "Firing"
	/>
	barrelCountFromPreset = false;

	</
		type        = "integer"
		choice      = [1, 2, 4, 5, 8]
		order       = 0.3
		group       = "Firing"
		conditional = "barrelCountFromPreset == false"
	/>
	barrelCount = 1;

	</
		type        = "string"
		order       = 0.4
		group       = "Firing"
		description = "'0' = selected projectile / '-' = nothing / '?' = Fire previous only once"
	/>
	firingPattern = "0";

	</
		type        = "integer"
		min         = 0
		max         = 100
		order       = 0.5
		group       = "Firing"
		description = "Loops if longer than firing pattern length"
	/>
	firingPatternStartPosition = 0;

	</
		type        = "integer"
		order       = 0.6
		min         = 0
		max         = 50
		group       = "Firing"
		description = "Ammo for this turret; use 0 to use the (infinite) preset default"
	/>
	ammo = 0;

	</
		type        = "bool"
		order       = 1.0
		group       = "Barrel"
		description = "Show the barrel and muzzle flash"
	/>
	showBarrel = false;

	</
		type        = "integer"
		min         = -200
		max         = 200
		order       = 1.1
		group       = "Barrel"
	/>
	rotationSpeed = 0;

	</
		type        = "integer"
		min         = -180
		max         = 180
		order       = 1.2
		group       = "Barrel"
	/>
	rotation = 180;
	_angle = 0;

	</
		type        = "integer"
		min         = 0
		max         = 180
		order       = 1.3
		group       = "Barrel"
		description = "Use 0 for no spread limit"
	/>
	rotationSpreadLimit = 0
	_rotationSpreadUBound = 0;
	_rotationSpreadLBound = 0;

	</
		type        = "bool"
		order       = 2.0
		group       = "Sight"
		description = "Targets on player when player is in range."
	/>
	targetPlayer = false;

	</
		type        = "float"
		min         = 5
		max         = 100
		order       = 2.1
		group       = "Sight"
		conditional = "targetPlayer == true"
	/>
	sightRange = 16;

	</
		type        = "bool"
		order       = 2.2
		group       = "Sight"
		conditional = "targetPlayer == true"
		description = "Setting this to true will only fire at the player when Turret sees it."
	/>
	onlyFireWhenTargetInSight = false;

	// Constants
	static c_maxHealth          = null; // invincible
	static c_mass               = 1.0; // used when following the player
	static c_gunOffset          = ::Vector2(0, 0.3);
	static c_isRemovedFromWorld = true;
	static c_killOnOutOfAmmo    = true;
	static c_energy             = null;

	// Internals
	_gun                 = null;
	_sightSensor         = null;

	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInvalidProperties

	function onInvalidProperties(p_properties)
	{
		local valueFixes = {}

		valueFixes.rotation      <- { result = @(val) val.tointeger() };
		valueFixes.rotationSpeed <- { result = @(val) val.tointeger() };

		if (("overrideAmmoFromPreset" in p_properties) == false ||
		    p_properties.overrideAmmoFromPreset == false)
		{
			valueFixes.ammo <- 0;
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

		local gunPreset = clone getGunPreset(::g_turretGunPresets, preset);
		if (showBarrel == false)
		{
			gunPreset._gunPresentationName   <- null;

			if (gunPreset._gunType == "ProjectileGun")
			{
				gunPreset._muzzleFlashEffectName <- null;
			}
		};

		createGunFromPreset(gunPreset,
			{
				firingPattern              = firingPattern,
				firingPatternStartPosition = firingPatternStartPosition,
				ammo                       = ammo,
				laserLength                = laserLength
			}
		);

		rotation= ::wrapAngle(rotation);

		_rotationSpreadUBound = ::wrapAngle(rotation - rotationSpreadLimit / 2);
		_rotationSpreadLBound = ::wrapAngle(rotation + rotationSpreadLimit / 2);

		_gun.setFiringAngle(rotation);

		if (targetPlayer)
		{
			local player = ::getFirstEntityByTag("PlayerBot");
			if (player != null)
			{
				_sightSensor = addSightSensor(::CircleShape(0, sightRange), player);
				_sightSensor.setDefaultEnterAndExitCallback();
				_sightSensor.setStopOnWaterPool(true);
			}
		}

		if (c_isRemovedFromWorld)
		{
			::removeEntityFromWorld(this);
			addProperty("dieQuietly");
		}

		_angle = rotation;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks

	function onEnabled()
	{
		base.onEnabled();

		if (containsVirus() == false && (onlyFireWhenTargetInSight == false || targetPlayer == false))
		{
			// Always fire
			_gun.startFiring();
		}

		if (_sightSensor != null)
		{
			_sightSensor.setEnabled(true);
		}
	}

	function onDisabled()
	{
		base.onDisabled();

		_gun.stopFiring();
		stopTimer("aim");

		if (_sightSensor != null)
		{
			_sightSensor.setEnabled(false);
		}
	}

	function onSightEnter(p_entity, p_sensor)
	{
		_gun.setTargetEntity(p_entity);
		if (onlyFireWhenTargetInSight)
		{
			startTimer("startFiring", 0.25);
		}
	}

	function onSightExit(p_entity, p_sensor)
	{
		stopTimer("startFiring");
		_gun.removeTargetEntity();
		_angle = _gun.getFiringAngleTarget();

		if (onlyFireWhenTargetInSight)
		{
			_gun.stopFiring();
		}
	}

	function onTimer(p_name)
	{
		base.onTimer(p_name);

		if (p_name == "startFiring")
		{
			_gun.startFiring();
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Inherited methods

	function createShield()
	{
		return null;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods

	function createGunFromPreset(p_preset, p_customProperties)
	{
		if (_gun != null)
		{
			// FIXME: Kinda harsh; perhaps reuse the existing gun?
			::killEntity(_gun);
			_gun = null;
		}

		if (barrelCountFromPreset == false)
		{
			p_preset._barrelCount <- barrelCount;
		}

		if (p_preset._gunType == "LaserGun")
		{
			p_preset._length <- p_customProperties.laserLength;
		}
		_gun = ::createGun(this, getCenterOffset() + c_gunOffset, p_preset);

		if ("firingPattern" in p_customProperties && p_customProperties.firingPattern != "")
		{
			local offset = p_customProperties.firingPatternStartPosition % p_customProperties.firingPattern.len();
			_gun._firingPatternStartIdx = offset;
			_gun.setFiringPattern(p_customProperties.firingPattern);
		}

		if ("ammo" in p_customProperties && p_customProperties.ammo > 0)
		{
			_gun.setInfiniteAmmo(false);
			_gun.setAmmo(p_customProperties.ammo);
		}
		else
		{
			_gun.setInfiniteAmmo(true);
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Update

	function update(p_deltaTime)
	{
		base.update(p_deltaTime);

		if (isEnabled() == false)
		{
			return;
		}

		if (_gun.hasExpired() && c_killOnOutOfAmmo)
		{
			::killEntity(this);
			return;
		}

		if (_gun.getTargetEntity() == null && rotationSpeed != 0.0)
		{
			// check limits
			if (rotationSpreadLimit > 0 && ::isAngleInRange(_angle, _rotationSpreadLBound, _rotationSpreadUBound) == false)
			{
				rotationSpeed = abs(rotationSpeed);

				local diffLBound = ::getAngleBetweenAngles(_angle, _rotationSpreadLBound);
				local diffUBound = ::getAngleBetweenAngles(_angle, _rotationSpreadUBound);

				// First check for bounds (< 2 seems to work just fine for even fast moving turrets)
				// Note: check with diffUBound is not required since rotationSpeed is already positive at this point
				if (diffLBound < 2)
				{
					rotationSpeed = -rotationSpeed;
				}
				else if (diffLBound > 20 && diffUBound > 20)
				{
					// Off center; return to center immediately
					_angle = rotation;
					if (diffLBound < diffUBound)
					{
						// Make sure rotation speed continues in correct direction
						rotationSpeed = -rotationSpeed ;
					}
				}
			}

			_angle= ::wrapAngle(_angle + (p_deltaTime * rotationSpeed));
			_gun.setFiringAngleTarget(_angle);
		}
	}
}
