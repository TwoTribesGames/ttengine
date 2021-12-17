include_entity("rewind/EntityChild");

///////////////////////////////////////////////////////////////////////////////
// Gun Presets

::g_secondaryWeaponPresets <- {
	//-------------------------------------------------------------------------
	// PROJECTILES
	homingmissile  = {
		_gunType                     = "ProjectileGun",
		_muzzleFlashEffectName       = "muzzleflash_player_homingmissile",
		_projectileType              = "HomingMissile",
		_projectileSpeed             = 0.6,
		_projectileSpeedRandom       = 0.3,
		_projectileParentSpeedFactor = 0.0,
		_firingRate                  = 60.0,
		_barrelCount                 = 7,
		_barrelLength                = 1.0,
		_barrelSpread                = 25,
		_infiniteAmmo                = true,
		_flags                       = GunPresetFlags.Droppable | GunPresetFlags.Pickup,
		_projectileSpawnProps        =
		{
			_damageValue         = 3,
			_explosionRadius     = 3.0,
			_maxRotationPerFrame = 11,
			_timeout             = 10.0
		}
	},
	emp = {
		_gunType                     = "ProjectileGun",
		_muzzleFlashEffectName       = "muzzleflash_player",
		_projectileType              = "EMPBomb",
		_projectileSpeed             = 0.6,
		_projectileParentSpeedFactor = 0.25,
		_firingRate                  = 60,
		_barrelCount                 = 1,
		_barrelLength                = 1.0,
		_infiniteAmmo                = true,
		_flags                       = GunPresetFlags.Droppable | GunPresetFlags.Pickup
	},
	grenade = {
		_gunType                     = "ProjectileGun",
		_muzzleFlashEffectName       = "muzzleflash_player",
		_projectileType              = "Grenade",
		_projectileSpeed             = 0.9,
		_projectileParentSpeedFactor = 0.25,
		_projectileSpeedRandom       = 0.2,
		_firingRate                  = 60,
		_barrelCount                 = 8,
		_barrelLength                = 1.0,
		_barrelSpread                = 35,
		_infiniteAmmo                = true,
		_flags                       = GunPresetFlags.Droppable | GunPresetFlags.Pickup,
	},
	shotgun = {
		_gunType                     = "ProjectileGun",
		_muzzleFlashEffectName       = "muzzleflash_player",
		_projectileType              = "ShotgunBullet",
		_projectileSpeed             = 0.1, // should be a low value as the bullet itself isn't actually traveling, it cannot be 0 though
		_projectileParentSpeedFactor = 0.0,
		_firingRate                  = 60,
		_barrelCount                 = 1,
		_barrelLength                = 0.0,
		_infiniteAmmo                = true,
		_flags                       = GunPresetFlags.Droppable | GunPresetFlags.Pickup
	}
}

::g_secondaryWeaponPresetsEasyMode <- {
	//-------------------------------------------------------------------------
	// PROJECTILES
	homingmissile  = {
		_gunType                     = "ProjectileGun",
		_muzzleFlashEffectName       = "muzzleflash_player_homingmissile",
		_projectileType              = "HomingMissile",
		_projectileSpeed             = 0.6,
		_projectileSpeedRandom       = 0.3,
		_projectileParentSpeedFactor = 0.0,
		_firingRate                  = 60.0,
		_barrelCount                 = 7,
		_barrelLength                = 1.0,
		_barrelSpread                = 25,
		_infiniteAmmo                = true,
		_flags                       = GunPresetFlags.Droppable | GunPresetFlags.Pickup,
		_projectileSpawnProps        =
		{
			_damageValue         = 4,
			_explosionRadius     = 3.0,
			_maxRotationPerFrame = 11,
			_timeout             = 10.0
		}
	},
	emp = {
		_gunType                     = "ProjectileGun",
		_muzzleFlashEffectName       = "muzzleflash_player",
		_projectileType              = "EMPBomb",
		_projectileSpeed             = 0.6,
		_projectileParentSpeedFactor = 0.25,
		_firingRate                  = 60,
		_barrelCount                 = 1,
		_barrelLength                = 1.0,
		_infiniteAmmo                = true,
		_flags                       = GunPresetFlags.Droppable | GunPresetFlags.Pickup
	},
	grenade = {
		_gunType                     = "ProjectileGun",
		_muzzleFlashEffectName       = "muzzleflash_player",
		_projectileType              = "Grenade",
		_projectileSpeed             = 0.9,
		_projectileParentSpeedFactor = 0.25,
		_projectileSpeedRandom       = 0.2,
		_firingRate                  = 60,
		_barrelCount                 = 10,
		_barrelLength                = 1.0,
		_barrelSpread                = 35,
		_infiniteAmmo                = true,
		_flags                       = GunPresetFlags.Droppable | GunPresetFlags.Pickup,
	},
	shotgun = {
		_gunType                     = "ProjectileGun",
		_muzzleFlashEffectName       = "muzzleflash_player",
		_projectileType              = "ShotgunBullet",
		_projectileSpeed             = 0.1, // should be a low value as the bullet itself isn't actually traveling, it cannot be 0 though
		_projectileParentSpeedFactor = 0.0,
		_firingRate                  = 60,
		_barrelCount                 = 1,
		_barrelLength                = 0.0,
		_infiniteAmmo                = true,
		_flags                       = GunPresetFlags.Droppable | GunPresetFlags.Pickup
	}
}

// End Gun Presets
///////////////////////////////////////////////////////////////////////////////

class SecondaryWeapon extends EntityChild
</
	placeable      = Placeable_Hidden
	movementset    = "Static"
	collisionRect  = [ 0.0, 0.0, 0.1, 0.1 ]
/>
{
	static c_gunOffset = ::Vector2(0.0, 0.5);
	
	// Internal parameters
	_type       = null;
	_gun        = null;
	_ammo       = null
	_totalFired = 0;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		base.onInit();
		
		select(_type);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods
	
	function fire()
	{
		if (_gun != null)
		{
			if (_ammo > 0)
			{
				_gun.fire();
				_ammo--;
				_totalFired++;
				return true;
			}
			else
			{
				_parent.customCallback("onEmptySecondaryFired", _gun);
			}
		}
		return false;
	}
	
	function isFiring()
	{
		if (_gun != null)
		{
			return _gun.isFiring();
		}
		
		return false;
	}
	
	function setFiringAngle(p_angle)
	{
		if (_gun != null)
		{
			_gun.setFiringAngle(p_angle);
		}
	}
	
	function getFiringAngle()
	{
		if (_gun != null)
		{
			return _gun.getFiringAngle();
		}
		
		return 0.0;
	}
	
	function getTotalFired()
	{
		return _totalFired;
	}
	
	function select(p_type)
	{
		if (p_type != null)
		{
			_createGun(p_type);
			return true;
		}
		return false;
	}
	
	function addAmmo(p_amount)
	{
		_ammo += p_amount;
	}
	
	function setAmmo(p_amount)
	{
		_ammo = p_amount;
	}
	
	function isEmpty()
	{
		return _ammo == 0;
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Private Methods
	
	function _createGun(p_type)
	{
		_type = p_type;
		if (_gun != null)
		{
			// FIXME: Kinda harsh; perhaps reuse the existing gun?
			::killEntity(_gun);
		}
		
		// 0.5 is the offset as put in the presentation XML startvalues
		local offset = _parent.getCenterOffset() + c_gunOffset;
		local presets = ::ProgressMgr.isEasyModeEnabled() ? ::g_secondaryWeaponPresetsEasyMode : ::g_secondaryWeaponPresets;
		local preset = ::getGunPreset(presets, p_type);
		_gun = ::createGun(_parent, offset, preset);
	}
}
