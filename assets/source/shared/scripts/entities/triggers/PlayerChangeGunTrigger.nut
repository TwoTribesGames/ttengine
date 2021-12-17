include_entity("triggers/Trigger");

class PlayerChangeGunTrigger extends Trigger
</
	editorImage    = "editor.playerchangeguntrigger"
	libraryImage   = "editor.library.playerchangeguntrigger"
	placeable      = Placeable_Developer
	sizeShapeColor = Colors.Trigger
	displayName    = "Trigger - Player Change Gun"
	group          = "04.4 Player Triggers"
	stickToEntity  = true
/>
{
	</
		type   = "string"
		choice = ["bullet", "homingmissile", "emp", "grenade"]
		order  = -1
		group  = "Specific Settings"
	/>
	gunType = "bullet";
	
	</
		type   = "float"
		min    = 0.5
		max    = 50
		order  = 0
		group  = "Specific Settings"
	/>
	firingRate = 20.0;
	
	</
		type   = "integer"
		min    = 0
		max    = 500
		order  = 1
		group  = "Specific Settings"
	/>
	ammo = 0;
	
	</
		type   = "integer"
		min    = 1
		max    = 5
		order  = 2
		group  = "Specific Settings"
	/>
	barrelCount = 1;
	
	</
		type   = "integer"
		min    = 10
		max    = 360
		order  = 3
		group  = "Specific Settings"
	/>
	barrelSpread = 360;
	
	</
		type   = "integer"
		min    = 0
		max    = 90
		order  = 4
		group  = "Specific Settings"
	/>
	barrelRandomSpread = 10;
	
	</
		type   = "bool"
		order  = 5
		group  = "Specific Settings"
	/>
	autoFire = false;
	
	</
		type   = "float"
		order  = 6
		min    = -10.0
		max    = 10.0
		group  = "Specific Settings"
	/>
	autoRotation = 0.0;
	
	</
		type   = "bool"
		order  = 7
		group  = "Specific Settings"
	/>
	removeSecondary = false;
	
	_unlockedSecondaries = null;
	
	function onTriggerEnterFirst(p_entity)
	{
		local playerBot = ::getFirstEntityByTag("PlayerBot");
		if (playerBot == null)
		{
			return;
		}
		
		local isFiring = playerBot._primaryWeapon.isFiring();
		local presets = ::ProgressMgr.isEasyModeEnabled() ? ::g_secondaryWeaponPresetsEasyMode : ::g_secondaryWeaponPresets;
		local props = gunType == "bullet" ? {} : ::getGunPreset(presets, gunType);
		playerBot.createCustomWeapon(
			::mergeTables(
			{
				_gunType            = "PlayerBotCustomGun",
				_firingRate         = firingRate,
				_barrelCount        = barrelCount,
				_barrelSpread       = barrelSpread,
				_barrelRandomSpread = barrelRandomSpread
			}, props)
		);
		if (gunType != "bullet")
		{
			playerBot._primaryWeapon._secondaryType = gunType;
		}
		if (isFiring || autoFire)
		{
			playerBot.startFiringPrimary();
		}
		playerBot._primaryWeapon.setAutoFireEnabled(autoFire);
		
		if (ammo > 0)
		{
			playerBot._primaryWeapon.setAmmo(ammo);
			playerBot._primaryWeapon.setInfiniteAmmo(false);
		}
		
		if (autoRotation != 0.0)
		{
			playerBot.setAimControlsEnabled(false);
			playerBot._primaryWeapon.setAutoRotation(autoRotation);
		}
		
		if (removeSecondary)
		{
			_unlockedSecondaries = clone playerBot._secondaryWeapon._unlockedTypes;
			playerBot._secondaryWeapon.lockAll();
		}
	}
	
	function onTriggerExitLast(p_entity)
	{
		local playerBot = ::getFirstEntityByTag("PlayerBot");
		if (playerBot == null)
		{
			return;
		}
		
		if (autoRotation != 0.0)
		{
			playerBot.setAimControlsEnabled(true);
		}
		
		local isFiring = playerBot._primaryWeapon.isFiring();
		playerBot.createPrimaryWeapon();
		if (isFiring && autoFire == false)
		{
			playerBot.startFiringPrimary();
		}
		
		if (removeSecondary)
		{
			playerBot._secondaryWeapon._unlockedTypes = _unlockedSecondaries;
			playerBot._secondaryWeapon._updateHud();
		}
	}
}
