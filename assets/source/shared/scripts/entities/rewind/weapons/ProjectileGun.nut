include_entity("rewind/weapons/BaseGun");
include_entity("rewind/bullets/Bullet");

class ProjectileGun extends BaseGun
</
	placeable      = Placeable_Hidden
	movementset    = "Static"
	collisionRect  = [ 0.0, 0.0, 0.1, 0.1 ]
/>
{
	// ProjectileGun specific parameters
	_muzzleFlashEffectName = null;
	
	// Projectile parameters
	_projectileType              = null;
	_projectileSpeed             = 0.2;
	_projectileSpeedRandom       = 0.0;
	_projectileParentSpeedFactor = 0.5;
	_projectilesPerBarrel        = 1;
	_projectilesSpread           = 360.0;
	_projectilesRandomSpread     = 0.0;
	
	_projectileSpawnProps  = {}; // extra spawn properties that'll be merged with the speed, rotation etc. of the gun
	_scoringProjectilesFired = 0;
	
	function onInit()
	{
		base.onInit();
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	
	function handleFiringSymbol(p_currentSymbol, p_nextSymbol)
	{
		// Don't do anything
	}
	
	function handleBarrelFiring(p_angle)
	{
		if (_parent != null && _infiniteAmmo == false && _ammo <= 0)
		{
			return;
		}
		
		local speedRange      = _projectileSpeedRandom / 2.0;
		local firingDirection = ::getVectorFromAngle(p_angle);
		local spawnPos = (firingDirection * _barrelLength);
		
		if (_muzzleFlashEffectName != null && _parent.isInWater() == false)
		{
			local effect = spawnParticleOneShot("particles/" + _muzzleFlashEffectName,
				spawnPos, true, 0, false, ParticleLayer_UseLayerFromParticleEffect, 1.0);
			
			effect.setEmitterProperties(
				0,
				{
					velocity_x = firingDirection.x * 10,
					velocity_y = firingDirection.y * 10
				}
			);
			effect.setEmitterProperties(
				1,
				{
					velocity_x = firingDirection.x * 10,
					velocity_y = firingDirection.y * 10
				}
			);
		}
		
		// Convert to worldposition
		spawnPos += getCenterPosition();
		
		local startAngle = _projectilesSpread == 360.0 ? 0 : - (_projectilesSpread / 2.0);
		local separation = 0;
		if (_projectilesPerBarrel > 1)
		{
			separation = _projectilesSpread == 360.0 ? 
				_projectilesSpread / _projectilesPerBarrel : _projectilesSpread / (_projectilesPerBarrel - 1);
		}
		
		// FIXME: Apparently the bullet speed and parent speed are not of the same magnitude
		// but differ a factor of 60
		local parentSpeed = ::Vector2(0, 0);
		if (_parent != null && _projectileParentSpeedFactor > 0.0)
		{
			local speedFactor = _projectileParentSpeedFactor / 60.0;
			if (_parent instanceof ::PlayerBot && _parent._carryParent != null)
			{
				parentSpeed += _parent._carryParent.getSpeed() * speedFactor;
			}
			else
			{
				parentSpeed += _parent.getSpeed() * speedFactor;
			}
		}
		
		// Fire the projectiles
		for (local i = 0; i < _projectilesPerBarrel; ++i)
		{
			local angle = p_angle + (i * separation) + startAngle;
			if (_projectilesRandomSpread > 0.0)
			{
				local spreadRange = _projectilesRandomSpread / 2.0;
				angle += ::frnd_minmax(-spreadRange, spreadRange);
			}
			
			local firingSpeed = _projectileSpeed + ::frnd_minmax(-speedRange, speedRange);
			
			// merge the settings from the gun with the custom projectile props
			local projectileSpawnProps = ::mergeTables(
				{
					_angle   = angle,
					_speed   = parentSpeed + ::getVectorFromAngle(angle) * firingSpeed,
					_group   = _weaponGroup,
					_shooter = _parent.weakref()
				},
				_projectileSpawnProps);
			
			local projectile = ::spawnEntity(_projectileType, spawnPos, projectileSpawnProps);
			
			if (_parent != null && projectile != null)
			{
				if (_parent instanceof ::PlayerBot)
				{
					_parent.customCallback("onProjectileFired", projectile);
				}
				else if (projectile.c_score > 0)
				{
					++_scoringProjectilesFired;
					if (_scoringProjectilesFired >= 50)
					{
						// Farming check
						::Game.setFarmingEnabled(true, false);
					}
				}
			}
		}
	}
}
