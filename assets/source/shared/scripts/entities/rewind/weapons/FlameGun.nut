include_entity("rewind/weapons/ProjectileGun");

class FlameGun extends ProjectileGun
</
	placeable      = Placeable_Hidden
	movementset    = "Static"
	collisionRect  = [ 0.0, 0.0, 0.1, 0.1 ]
/>
{
	_projectileType  = "Flame";
	_soundEffect     = null;
	
	////////////////////////////////////////////////////////////////////////////////////////////////

	function startFiring()
	{
		startSoundEffect();
		
		base.startFiring();
	}
	
	function stopFiring()
	{
		stopSoundEffect();
		
		base.stopFiring();
	}
	
	function handleFiringSymbol(p_currentSymbol, p_nextSymbol)
	{
		if (p_nextSymbol == '-')
		{
			stopSoundEffect();
		}
		else if (p_currentSymbol == '-' && p_nextSymbol != '-')
		{
			startSoundEffect();
		}
	}
	
	function startSoundEffect()
	{
		if (_soundEffect != null)
		{
			_soundEffect.stop();
			_soundEffect = null;
		}
		
		_soundEffect = playSoundEffect("enemy_flamethrower_on_loop");
	}

	function stopSoundEffect()
	{
		if (_soundEffect != null)
		{
			_soundEffect.stop();
			_soundEffect = null;
			playSoundEffect("enemy_flamethrower_off");
		}
	}
}
