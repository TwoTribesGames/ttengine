include_entity("rewind/bullets/Bullet");

class ShotgunBullet extends Bullet
</
	placeable      = Placeable_Hidden
	movementset    = "Static"
	collisionRect  = [ 0.0, 0.125, 0.25, 0.25 ]
	group          = "Rewind"
/>
{
	static c_presentationFile = null;
	
	_damageValue = 20;
	_range       = 30;
	_spread      = 60;
	
	static c_numberOfFragments     = 10;
	static c_maxDamage             = 25;
	
	_damageCount     = 0;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		base.onInit();
		
		if (::ProgressMgr.isEasyModeEnabled())
		{
			_range  *= 1.3;
			_spread *= 1.3;
		}
		
		setUpdateSurvey(false);
		::removeEntityFromWorld(this);
	}
	
	function onSpawn()
	{
		base.onSpawn();
		
		if (_inWater)
		{
			local effect = spawnParticleOneShot("particles/shotgun_underwater", ::Vector2(0,0), false, 0, false,
				ParticleLayer_UseLayerFromParticleEffect, 1.0);
			
			effect.setInitialExternalImpulse(-_angle, 1.0);
			
			removeBullet();
			return;
		}
		
		local sensor = addSightSensor(::ConeShape(0, _range, _angle, _spread), null, ::Vector2(0.0, 0.0));
		sensor.setEnterCallback("onShotgunEnter");
		sensor.setFilterCallback("onShotgunFilter");
		sensor.setDistanceSort(true);
		
		local light = addLight(::Vector2(0, 0), 0.0, 0.0);
		light.setRadius(_range / 1.5, 0.03);
		light.setSpread(200, 0.0);
		light.setDirection(_angle);
		light.setStrength(1.0, 0.01); // To fix first frame spread issue; spread is unused otherwise
		light.setTexture("shotgun");
		
		// Spawn 'ghost' fragments
		for (local i = 0; i < c_numberOfFragments; ++i)
		{
			local angle =  _angle + ::frnd_minmax(-_spread / 2.0, _spread / 2.0);
			::spawnEntity("ShotgunBulletFragment", getCenterPosition(),
				{
					_angle   = angle,
					_speed   = ::getVectorFromAngle(angle) * frnd_minmax(1.5, 2.0),
					_group   = _group,
					_shooter = _shooter
				}
			);
		}
		
		local effect = spawnParticleOneShot("particles/shotgun_bullet", ::Vector2(0,0), false, 0, false,
			ParticleLayer_UseLayerFromParticleEffect, _range / 17.0);
		
		effect.setInitialExternalImpulse(-_angle, 1.0);
		
		// This 'bullet' should only live briefly
		startTimer("remove", 0.115);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onTimer(p_name)
	{
		removeBullet();
	}
	
	function onShieldHit(p_shield)
	{
		// Don't do anything
	}
	
	function onShotgunEnter(p_entity, p_sensor)
	{
		// Scale damage value based on distance
		local dist = getCenterPosition().distanceTo(p_entity.getCenterPosition());
		local prevDamageValue = _damageValue;
		
		local damageLeft = c_maxDamage - _damageCount;
		if (damageLeft < 0.5)
		{
			_damageCount = c_maxDamage;
			return;
		}
		
		// Damage drop off starts at 50% of range
		_damageValue = ::min(damageLeft * (1.0 - dist / _range) * 2.0, damageLeft);
		
		if (p_entity instanceof ::Deflectable)
		{
			p_entity._health -= _damageValue * 2.0;
		}
		
		local absorbedByShield = false;
		if (("_shield" in p_entity) && p_entity._shield != null && p_entity._shield.isEnabled())
		{
			if (p_entity._shield.isIndestructible())
			{
				absorbedByShield = true;
			}
			else
			{
				local shieldAngle   = p_entity._shield._angle;
				local lboundShotgun = wrapAngle(_angle - (_spread / 2.0));
				local uboundShotgun = wrapAngle(_angle + (_spread / 2.0));
				local lboundShield  = wrapAngle(shieldAngle - (p_entity._shield.c_spreadDestructibleShield / 2.0));
				local uboundShield  = wrapAngle(shieldAngle + (p_entity._shield.c_spreadDestructibleShield / 2.0));
				
				if (::isAngleInRange(lboundShotgun, lboundShield, uboundShield) || 
				    ::isAngleInRange(uboundShotgun, lboundShield, uboundShield))
				{
					absorbedByShield = true;
				}
			}
			
			if (absorbedByShield)
			{
				p_entity._shield.customCallback("onShieldEnter", this, null);
			}
		}
		
		if (absorbedByShield == false && _damageCount < c_maxDamage)
		{
			local hb = ("_healthBar" in p_entity) ? p_entity._healthBar : null;
			local prevHealthBarAmount = (hb != null)  ? hb.getHealth() : 0;
			
			p_entity.customCallback("onBulletHit", this);
			
			local curHealthBarAmount = (hb != null)  ? hb.getHealth() : 0;
			
			_damageCount += prevHealthBarAmount - curHealthBarAmount;
		}
		
		local scale = p_entity.getProperty("shotgunKnockbackScale");
		if (scale != null)
		{
			p_entity.setSpeed(::getVectorFromAngle(_angle) * scale);
		}
		
		_damageValue = prevDamageValue;
	}
	
	function onShotgunFilter(p_entity)
	{
		// Check for missiles etc
		if (p_entity instanceof ::Deflectable)
		{
			return _group != p_entity._group;
		}
		
		if (p_entity instanceof ::RewindEntity && p_entity.isInWater())
		{
			return false;
		}
		
		if (p_entity.hasProperty("noticeBullets") == false)
		{
			return false;
		}
		
		// Normal entities
		return p_entity.equals(_shooter) == false && p_entity.getProperty("weaponGroup") != _group;
	}
	
	function handleEntityImpact(p_entity)
	{
	}
	
	function onWallHit()
	{
		// Prevent baseclass from calling onWallHit()
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Inherited methods
	
	function removeBullet()
	{
		base.removeBullet();
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Update
	
	function update(p_deltaTime)
	{
	}
}
