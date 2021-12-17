include_entity("rewind/weapons/ProjectileGun");

class PlayerBotGun extends ProjectileGun
</
	placeable      = Placeable_Hidden
	movementset    = "Static"
	collisionRect  = [ 0.0, 0.0, 0.1, 0.1 ]
/>
{
	_soundEffect = null;
	_playSingle  = false;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onBulletTimeEnter()
	{
		stopLoop();
		
		_playSingle = true;
	}
	
	function onBulletTimeExit()
	{
		_playSingle = false;
		
		if (_isFiring && _soundEffect == null)
		{
			playLoop();
		}
	}
	
	function onWaterEnter()
	{
		base.onWaterEnter();
		
		if (_isFiring)
		{
			playLoop();
		}
	}
	
	function onWaterExit()
	{
		base.onWaterExit();
		
		stopLoop();
		
		if (_isFiring)
		{
			playLoop();
		}
	}
	
	function onDie()
	{
		stopFiring();
		
		base.onDie();
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Inherited Methods
	
	function startFiring()
	{
		base.startFiring();
		
		if (_firingRate > 0 && _soundEffect == null)
		{
			playLoop();
		}
	}
	
	function fire()
	{
		if (_canFire == false)
		{
			return false;
		}
		
		if (_playSingle)
		{
			playSoundEffect("bullet_player_slomo");
		}
		
		return base.fire();
	}
	
	function stopFiring()
	{
		base.stopFiring();
		
		playEnd();
		stopLoop();
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods
	
	function playLoop()
	{
		_soundEffect = playSoundEffect(_parent.isInWater() ?
			"bullet_player_underwater" :
			"bullet_player");
	}
	
	function playEnd()
	{
		if (_soundEffect != null)
		{
			playSoundEffect(_parent.isInWater() ?
				"bullet_player_underwater_end" :
				"bullet_player_end");
		}
	}
	
	function stopLoop()
	{
		if (_soundEffect != null)
		{
			_soundEffect.stop();
			_soundEffect = null;
		}
	}
}
