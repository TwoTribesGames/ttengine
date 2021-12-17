class Bullet extends EntityBase
</
	placeable      = Placeable_Hidden
	collisionRect  = [ 0.0, 0.0, 0.5, 0.5 ]
/>
{
	static c_presentationFile      = "presentation/bullet";
	static c_score                 = 0;
	static c_waterDrag             = 0.15 * 60;
	static c_waterImpact           = 0.70;
	static c_disabledYAcceleration = -0.01 * 60;
	static c_impactForce           = 1200;
	
	_speed            = null;
	_presentation     = null;
	_radius           = null; // If null it will use the radius as set by the collision rect
	
	_angle            = 0;
	_group            = ::WeaponGroup.Player;
	_timeout          = 3;
	
	_shooter = null; // the entity that shot the bullet
	
	static c_positionCulling = false;
	</
		autoGetSet = true
	/>
	_damageValue  = 1;
	
	_touch        = null;
	
	// Private members
	_timeAlive        = 0;
	_hasTimedOut      = false;
	_isDisabled       = false;
	_inWater          = false;
	_hitsDeflectables = false;
	
	function onInit()
	{
		setUpdateSurvey(true);
		
		if (c_presentationFile != null)
		{
			_presentation = createPresentationObject(c_presentationFile);
			_presentation.setCustomRotation(-_angle);
		}
		
		if (_speed == null) _speed = ::Vector2(0, 0);
		
		setCanBePushed(false);
		setPositionCullingEnabled(c_positionCulling);
	}
	
	function onSpawn()
	{
		local rect = getCollisionRect();
		
		// Touchshape is used for deflecting bullets and shields and bullet sensing triggers
		setTouchShape(::CircleShape(0, rect.getWidth() / 2.0), rect.getPosition());
		
		// Check if it starts in water
		if (_inWater)
		{
			if (_presentation != null)
			{
				_presentation.start("disabled", [_group, "water"], false, 0);
			}
		}
		else
		{
			if (_radius == null)
			{
				_radius = rect.getWidth() / 2.0;
			}
			
			_touch = addTouchSensor(::CircleShape(0, _radius), null, ::Vector2(0,0));
			_touch.setEnterCallback("onBulletTouchEnter");
			_touch.setFilterCallback("onBulletFilter");
			
			// enable wave generation in water,
			enableDefaultWaterInteraction();
			
			if (_presentation != null)
			{
				_presentation.start("idle", [_group], false, 0);
			}
		}
	}
	
	function update(p_deltaTime)
	{
		_timeAlive += p_deltaTime;
		
		if (_timeAlive >= _timeout && _hasTimedOut == false)
		{
			onBulletTimedOut();
			_hasTimedOut = true;
		}
		
		if (_inWater || _isDisabled)
		{
			_speed.y += c_disabledYAcceleration * p_deltaTime;
			// Drag simulation (Stokes)
			if (_inWater) _speed -= _speed * (c_waterDrag * p_deltaTime);
			updateAngle();
		}
		
		local speed = (_speed * 60.0 * p_deltaTime);
		
		local prevPos = getPosition();
		
		// Hack for really fast moving bullets; do an extra check
		if (speed.lengthSquared() > 0.8)
		{
			local newPos = prevPos + (speed / 2.0);
			if (testCollision(newPos))
			{
				// Make sure we align the bullet with the tile boundary
				local hitPos = ::calcTileHitLocation(prevPos, newPos);
				setPosition(hitPos);
				onWallHit();
				return;
			}
		}
		
		local newPos = prevPos + speed;
		if (testCollision(newPos))
		{
			// Make sure we align the bullet with the tile boundary
			local hitPos = ::calcTileHitLocation(prevPos, newPos);
			setPosition(hitPos);
			onWallHit();
			return;
		}
		setPosition(newPos);
	}
	
	function testCollision(p_pos)
	{
		// "collision" only collide with the level, don't collide with entity tiles so the bullet
		// won't be removed before the touch can be detected.
		//so don't do this unless you want unreliable Collapsiblebridges: local collision = getCollisionType(newPos);
		
		// MARTIJN: I disregared the previous comment and replaced this code: getCollisionTypeLevelOnly(p_pos);
		local collision = ::getCollisionType(p_pos);
		return ::isSolid(collision);
	}
	
	function reflect(p_normal, p_bouncyness)
	{
		_speed = _speed.reflect(p_normal, p_bouncyness);
		updateAngle();
	}
	
	function updateAngle()
	{
		_angle = ::atan2(_speed.x, _speed.y);
		
		if (_presentation != null)
		{
			_presentation.setCustomRotation(-_angle * ::radToDeg);
		}
	}
	
	function getImpactForce()
	{
		return _speed.normalize() * c_impactForce;
	}
	
	function onWallHit()
	{
		if (_isDisabled || _inWater)
		{
			return;
		}
		
		local particleSpeedX = -_speed.x * 10;
		local particleSpeedY = -_speed.y * 10;
		
		if (_group == ::WeaponGroup.Player)
		{
			playHitAnim("bullet_hit_wall", particleSpeedX, particleSpeedY);
		}
		else
		{
			playHitAnim("bullet_enemy_hit_wall", particleSpeedX, particleSpeedY);
		}
		playSoundEffect("bullet_hitwall");
		
		removeBullet();
	}
	
	function onBulletTouchEnter(p_entity, p_sensor)
	{
		handleEntityImpact(p_entity);
	}
	
	function onShieldHit(p_shield)
	{
		playHitAnim(p_shield.isIndestructible() ?
			"bullet_hit_shield_indestructible" : "bullet_hit_shield_destructible", -_speed.x * 4, -_speed.y * 4);
		
		// This makes sure bullets bounce off of indestructible shields
		if (p_shield.isIndestructible() == false)
		{
			removeBullet();
		}
	}
	
	function onBulletTimedOut()
	{
		removeBullet();
	}
	
	function onCulled()
	{
		removeBullet();
	}
	
	function onBulletFilter(p_entity)
	{
		if (_isDisabled || p_entity.equals(_shooter))
		{
			// Shooter should never shoot itself and disabled bullets don't do anything
			return false;
		}
		
		if (p_entity.hasProperty("noticeBullets") == false)
		{
			return false;
		}
		
		if (p_entity instanceof ::Bullet && p_entity._shooter == _shooter)
		{
			// Bullets shouldn't hit eachother if shooter is the same
			return false;
		}
		
		if (p_entity instanceof ::Deflectable)
		{
			return _hitsDeflectables && p_entity._hitsDeflectables && p_entity._group != _group;
		}
		
		// Normal entities
		return p_entity.getProperty("weaponGroup") != _group;
	}
	
	function playHitAnim(p_animName, p_velocityX, p_velocityY, p_affectedEmitters = [0])
	{
		local effect = spawnParticleOneShot("particles/" + p_animName, ::Vector2(0, 0), false, 0, false,
			ParticleLayer_UseLayerFromParticleEffect, 1.0);
		
		foreach (emitter in p_affectedEmitters)
		{
			effect.setEmitterProperties(
				emitter,
				{
					velocity_x = p_velocityX,
					velocity_y = p_velocityY,
				}
			);
		}
	}
	
	function removeBullet()
	{
		// Make sure this bullet doesn't process any callbacks anymore
		removeQueuedCallbacks();
		::killEntity(this);
	}
	
	function _typeof()
	{
		return "Bullet";
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Fluids
	
	function enableDefaultWaterInteraction()
	{
		local settings = addFluidSettings(FluidType_Water);
		settings.setWaveGenerationEnabled(true);
	}
	
	function onWaterEnclosedEnter()
	{
		if (_inWater)
		{
			return;
		}
		
		_speed *= (1.0 - c_waterImpact);
		_isDisabled = true;
		_inWater    = true;
		
		// _timealive > 0.0 check is to make sure we don't start this presentation if the onSpawn() hasn't
		// been called yet. onWaterEnclosed is always called BEFORE onSpawn (if bullet is spawn inside water)
		if (_presentation != null && _timeAlive > 0.0)
		{
			_presentation.start("disabled", [_group, "water", "splash"], false, 0);
		}
		
		if (_touch != null)
		{
			_touch.setEnabled(false);
		}
	}
	
	function onWaterTouchExit()
	{
		_isDisabled = true;
		_inWater = false;
		
		_speed *= (1.0 - c_waterImpact);
		
		if (_presentation != null)
		{
			_presentation.start("disabled", [_group], false, 0);
		}
	}
	
	function onLavaTouchEnter()
	{
		local offset = -getCollisionRect().getWidth() / 2.0;
		spawnParticleOneShot("particles/lava_splash_bullet", getCenterPosition() + ::Vector2(0, offset), false, 0, true, ParticleLayer_UseLayerFromParticleEffect, 1.0);
		playSoundEffect("bullet_impact_lava");
		removeBullet();
	}
	
	function onLavaEnclosedEnter()
	{
		removeBullet();
	}
	
	function onLavafallTouchEnter()
	{
		local offset = getCollisionRect().getWidth() / 2.0;
		if (_speed.x < 0) offset = -offset;
		
		spawnParticleOneShot("particles/lavafall_splash_bullet", getCenterPosition() + ::Vector2(offset, 0), false, 0, true, ParticleLayer_UseLayerFromParticleEffect, 1.0);
	}
	
	function onLavafallEnclosedEnter()
	{
		removeBullet();
	}
	
	function onDeathRayEnter(p_ray)
	{
		spawnParticleOneShot("particles/lavafall_splash_bullet", getCenterPosition(), false, 0, true, ParticleLayer_UseLayerFromParticleEffect, 1.0);
		removeBullet();
	}
}
