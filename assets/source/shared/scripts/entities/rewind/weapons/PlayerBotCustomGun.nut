include_entity("rewind/weapons/PlayerBotGun");

class PlayerBotCustomGun extends PlayerBotGun
</
	placeable      = Placeable_Hidden
	movementset    = "Static"
	collisionRect  = [ 0.0, 0.0, 0.1, 0.1 ]
/>
{
	_playSingle    = false;
	_autoFire      = false;
	_autoRotation  = null;
	_secondaryType = null;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onBulletTimeEnter()
	{
		if (_secondaryType == null)
		{
			base.onBulletTimeEnter();
		}
	}
	
	function onBulletTimeExit()
	{
		if (_secondaryType == null)
		{
			base.onBulletTimeEnter();
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Inherited Methods
	
	function stopFiring()
	{
		if (_autoFire == false)
		{
			base.stopFiring();
		}
	}
	
	function playLoop()
	{
		if (_secondaryType == null)
		{
			base.playLoop();
		}
	}
	
	function playEnd()
	{
		if (_secondaryType == null)
		{
			base.playEnd();
		}
	}
	
	function stopLoop()
	{
		if (_secondaryType == null)
		{
			base.stopLoop();
		}
	}
	
	function fire()
	{
		if (_canFire == false)
		{
			return;
		}
		
		if (_secondaryType != null)
		{
			// Shut up the bullet sfx
			_playSingle = false;
			local hasFired = base.fire();
			_playSingle = true;
			
			if (hasFired)
			{
				if (_parent.isInWater())
				{
					playSoundEffect("playerbot_secondary_" + _secondaryType + "_submerged");
				}
				else
				{
					playSoundEffect("playerbot_secondary_" + _secondaryType);
				}
			}
		}
		else
		{
			base.fire();
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods
	
	function setAutoFireEnabled(p_enabled)
	{
		_autoFire = p_enabled;
	}
	
	function isAutoFireEnabled()
	{
		return _autoFire;
	}
	
	function setAutoRotation(p_rotation)
	{
		_autoRotation = p_rotation;
	}
	
	function resetAutoRotation()
	{
		_autoRotation = null;
	}
	
	function getAutoRotation()
	{
		return _autoRotation;
	}
}
