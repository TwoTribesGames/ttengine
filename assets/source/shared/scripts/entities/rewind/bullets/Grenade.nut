include_entity("rewind/bullets/Deflectable");

class Grenade extends Deflectable
</
	placeable      = Placeable_Hidden
	movementset    = "Static"
	collisionRect  = [ 0.0, 0.0, 0.5, 0.5 ]
	group          = "Rewind"
/>
{
	// Constants
	static c_presentationFile = "presentation/grenade";
	static c_deflectionDamage = 3;
	static c_gravity          = ::Vector2(0, -40);
	static c_waterDrag        =  0.12 * 60;
	static c_waterImpact      =  0.53;

	_damageValue     = 2;
	_explosionRadius = 1.5;

	_trailEffect = null;
	_isDefused = false;
	_moveSettings = null;
	_moveSettingsZeroG = null;

	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn

	function onInit()
	{
		base.onInit();

		local sensor = addTouchSensor(::CircleShape(0, 0.5));
		sensor.setEnterCallback("onTouchEnter");
		sensor.setFilterCallback("onTouchFilter");

		_moveSettings = PhysicsSettings(0.5, 0.0, 0.0, -1, -1);
		_moveSettings.setBouncyness(1.0);

		addProperty("deflectFromShield");
	}

	function onSpawn()
	{
		base.onSpawn();

		_timeout = ::frnd_minmax(3.0, 4.0);
		startCallbackTimer("defuse", _timeout);
		startCallbackTimer("onBulletTimedOut", _timeout + 3.0);

		registerZeroGravityAffectedEntity(this);

		_trailEffect = spawnParticleContinuous("particles/grenade_trail", ::Vector2(0, 0),
			true, 0, false, ParticleLayer_UseLayerFromParticleEffect, 1.0);

		startMovementInDirection(_speed.normalize(), _moveSettings);
		if (_inWater)
		{
			setSpeed(_speed * 30);
		}
		else
		{
			setSpeed(_speed * 60);
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks

	function defuse()
	{
		_isDefused = true;
		stopTimer("defuse");
		if (_trailEffect != null)
		{
			_trailEffect.stop(false);
			_trailEffect = null;
		}
		_presentation.start("defused", [], false, 1);

		if (_inWater == false)
		{
			_moveSettings.setDrag(0.1);
			_moveSettings.setCollisionDrag(5.0);
			_moveSettings.setBouncyness(0.5);
			// Restart movement
			if (::isInZeroGravity(this) == false)
			{
				_moveSettings.setExtraForce(c_gravity);
			}
			startMovementInDirection(::Vector2(0, 0), _moveSettings);
		}
	}

	function onTouchEnter(p_entity, p_sensor)
	{
		if (_isDefused == false && p_entity instanceof ::BaseEnemy && p_entity._healthBar != null &&
		    (p_entity._shield == null || p_entity._shield.isIndestructible() == false))
		{
			p_entity._healthBar.doDamage(_damageValue, this);
			spawnParticleOneShot("particles/grenade_hit", ::Vector2(0, 0), true, 0, false, ParticleLayer_UseLayerFromParticleEffect, 1.0);
			playSoundEffect("playerbot_grenade_explode");
			::Camera.shakeScreen(0.05, getCenterPosition(), CameraShakeFlag.NoRotation);
		}

		local normal = (getCenterPosition() - p_entity.getCenterPosition()).normalize();
		local speed = getSpeed().reflect(normal, 1.0);
		setSpeed(speed);
	}

	function onTouchFilter(p_entity)
	{
		// Check for missiles etc
		if ((p_entity instanceof ::Deflectable || p_entity.equals(_shooter)))
		{
			return false;
		}

		if (p_entity.hasProperty("noticeBullets") == false)
		{
			return false;
		}

		// Normal entities
		return p_entity.getProperty("weaponGroup") != _group;
	}

	function handleEntityImpact(p_entity)
	{
		//removeBullet();
	}

	function onZeroGravityEnter(p_source)
	{
		_moveSettings.setExtraForce(::Vector2(0, 0));
		startMovementInDirection(::Vector2(0, 0), _moveSettings);
	}

	function onZeroGravityExit(p_source)
	{
		_moveSettings.setExtraForce(c_gravity);
		startMovementInDirection(::Vector2(0, 0), _moveSettings);
	}

	function onShieldHit(p_shield)
	{
		local normal = (getCenterPosition() - p_shield.getCenterPosition()).normalize();
		local speed = getSpeed().reflect(normal, 1.0);
		setSpeed(speed);
	}

	function onSolidCollision(p_collisionNormal, p_speed)
	{
		if (_inWater)
		{
			customCallback("onBulletTimedOut");
			return;
		}
		spawnParticleOneShot("particles/grenade_land", ::Vector2(0, 0), true, 0, false, ParticleLayer_UseLayerFromParticleEffect, 1.0);
		playSoundEffect("playerbot_grenade_bounce");
	}

	function onWallHit()
	{
		// Prevent baseclass from calling onWallHit()
	}

	function onWaterEnclosedEnter()
	{
		_isDefused = true;
		_inWater   = true;
		setSpeed(getSpeed() * (1.0 - c_waterImpact));

		if (_presentation != null)
		{
			_presentation.start("disabled", [_group, "water", "splash"], false, 0);
		}

		if (_touch != null)
		{
			_touch.setEnabled(false);
		}

		_moveSettings.setDrag(3.0);
		_moveSettings.setBouncyness(0.0);
		_moveSettings.setExtraForce(::Vector2(0, -10));
		startMovementInDirection(::Vector2(0, 0), _moveSettings);
	}

	function onWaterTouchExit()
	{
		_inWater = false;
		defuse();
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Inherited methods

	function removeBullet()
	{
		unregisterZeroGravityAffectedEntity(this);
		if (_trailEffect != null)
		{
			_trailEffect.stop(false);
			_trailEffect = null;
		}

		base.removeBullet();
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Update

	function update(p_deltaTime)
	{
		if (_trailEffect != null)
		{
			_trailEffect.setEmitterProperties(
				0,
				{
					start_rotation = -::getAngleFromVector(getSpeed())
				}
			);
		}
	}
}
