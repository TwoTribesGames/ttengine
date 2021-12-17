include_entity("rewind/weapons/SecondaryWeapon");

class PlayerBotSecondaryWeapon extends SecondaryWeapon
</
	placeable      = Placeable_Hidden
	movementset    = "Static"
	collisionRect  = [ 0.0, 0.0, 0.1, 0.1 ]
/>
{
	// Store / Restore Constants
	static c_internalsStoredOnLevelExit      = ["_type", "_ammo", "_unlockedTypes"];
	static c_internalsStoredOnDeath          = ["_type"];
	static c_internalsStoredOnMissionRestart = ["_type"];
	static c_internalsStoredForUpdate        = ["_type", "_ammo", "_unlockedTypes"];
	
	// Internal parameters
	_hudPresentations     = null;
	_selectedType         = null;
	_previousSelectedType = null;
	_unlockedTypes        = null;
	_isHudVisible         = true;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		if (_unlockedTypes == null)
		{
			_unlockedTypes = [];
		}
		
		_hudPresentations = {};
		
		foreach (key, value in ::g_secondaryWeaponPresets)
		{
			_hudPresentations[key] <- ::Hud.createElement(this, "presentation/hud_weapons", HudAlignment.Right);
		}
		
		base.onInit();
	}
	
	function onSpawn()
	{
		base.onSpawn();
		
		_updateHud();
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onHUDShow()
	{
		if (_isHudVisible == false)
		{
			_isHudVisible = true;
			_updateHud();
		}
	}
	
	function onHUDHide()
	{
		if (_isHudVisible)
		{
			_isHudVisible = false;
			_updateHud();
		}
	}
	
	function onDie()
	{
		base.onDie();
		
		foreach (pres in _hudPresentations)
		{
			::Hud.destroyElement(pres);
		}
	}
	
	function onProgressRestored(p_id)
	{
		base.onProgressRestored(p_id);
		
		_selectedType = null;
		select(_type);
	}
	
	function onRestoreFromUpdate()
	{
		base.onRestoreFromUpdate();
		
		_selectedType = null;
		select(_type);
	}
	
	function onControllerSwitched(p_controller)
	{
		_updateHud();
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods
	
	function fire()
	{
		local hasFired = base.fire();
		
		if (hasFired)
		{
			if (_parent.isInWater())
			{
				::setRumble(RumbleStrength_Low, 0.0);
				playSoundEffect("playerbot_secondary_" + _type + "_submerged");
			}
			else
			{
				::setRumble(RumbleStrength_Medium, 0.0);
				playSoundEffect("playerbot_secondary_" + _type);
			}
		}
		
		_updateHud();
		
		return hasFired;
	}
	
	function selectPrevious()
	{
		select(_previousSelectedType);
	}
	
	function select(p_type)
	{
		if (_unlockedTypes.find(p_type) != null)
		{
			return base.select(p_type);
		}
		return false;
	}
	
	function unlock(p_type)
	{
		if (_unlockedTypes.find(p_type) == null)
		{
			_unlockedTypes.push(p_type);
		}
		
		// No ammo with first gun
		_ammo = _unlockedTypes.len() > 1 ? 1 : 0;
		select(p_type);
	}
	
	function lockAll()
	{
		_unlockedTypes.clear();
		_ammo = 0;
		_updateHud();
	}
	
	function addAmmo(p_amount)
	{
		base.addAmmo(p_amount);
		
		_updateHud();
	}
	
	function setAmmo(p_amount)
	{
		base.setAmmo(p_amount);
		
		_updateHud();
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Private Methods
	
	function _createGun(p_type)
	{
		base._createGun(p_type);
		
		_previousSelectedType = _type;
		
		_updateHud();
		
		// Hud is updated, set _selectedType
		_selectedType = _type;
	}
	
	function _updateHud()
	{
		local gunCount = _gun == null ? 0 : _unlockedTypes.len();
		
		// 0 guns collected: hide all presentations
		if (gunCount == 0 || _isHudVisible == false)
		{
			// Hide all
			foreach (pres in _hudPresentations)
			{
				pres.stop();
			}
			return;
		}
		
		foreach (key, pres in _hudPresentations)
		{
			local tags = [key];
			
			tags.push(::getControllerTypeString());
			tags.push(::getPlatformString());
			
			if (::getCurrentControllerType() != ControllerType_Keyboard)
			{
				tags.push("controller");
			}
			
			if (key == _type && _selectedType != _type)
			{
				// Selecting
				tags.push("select");
			}
			else if (key == _type && _selectedType == _type)
			{
				// Selecting
				tags.push("selected");
			}
			else if (key == _selectedType && _selectedType != _type)
			{
				// Deselecting
				tags.push("deselect");
			}
			else 
			{
				// Already deselected
				tags.push("deselected");
			}
			
			if (isEmpty()) // selected gun
			{
				tags.push("empty");
			}
			
			local gunAvailable = _unlockedTypes.find(key) != null;
			pres.start(gunAvailable ? key : key + "_inactive", tags, false, 0);
		}
	}
}
