include_entity("triggers/Trigger");
include_entity("rewind/enemies/BaseEnemy");

class Asteroid extends BaseEnemy
</
	placeable           = Placeable_Hidden
	movementset         = "Static"
	collisionRect       = [ 0.0, 0.0, 2.0, 2.0 ]
/>
{
	</
		type = "float"
		min = -180
		max = 180
		order = 2
	/>
	rotation = 0;
	
	</
		type = "float"
		min = 0
		max = 50
		order = 3
	/>
	speed = 1.0;
	
	</
		type = "bool"
		order = 4
	/>
	zeroGravity = true;
	
	// Constants
	static c_maxRotationSpeed = 30;
	
	// Internal values
	_spawnedByAsteroid = false;
	_movementSettings  = null;
	_radius            = null; // is determined by collision rect
	
	_rotation      = 0;
	_rotationSpeed = 0;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		base.onInit();
		
		setUpdateSurvey(zeroGravity == false);
		setCanBeCarried(true);
		
		local rect = getCollisionRect();
		local diameter = rect.getWidth();
		_radius = diameter / 2.0;
		setTouchShape(::CircleShape(0, _radius * 0.99), rect.getPosition());
		
		local corpse = ::CorpseParams("Corpse", { _presentationFile = "asteroid", _rumbleStrength = c_rumbleStrength });
		corpse.addPresentationCustomValues(["diameter", diameter]);
		addProperty("weaponGroup", ::WeaponGroup.Neutral);
		addProperty("hasCorpse", corpse);
		removeProperty("noticeEMP");
		addProperty("touchDamage", 1);
		addProperty("touchDamageDoesntKillPlayer");
		addProperty("noRandomPickupsOnDie");
		addProperty("noCameraShakeOnDie");
		removeProperty("opensPipeGates");
		removeProperty("attractHomingMissiles");
		
		if (zeroGravity)
		{
			_movementSettings = PhysicsSettings(c_mass, 0.01, 0.0, -1, -1);
			_movementSettings.setBouncyness(0.98);
		}
		else
		{
			_movementSettings = PhysicsSettings(c_mass, 0.5, 0.0, -1, -1);
			_movementSettings.setBouncyness(0.15);
			_movementSettings.setCollisionDrag(40);
		}
		
		_presentation = createPresentationObject("presentation/asteroid");
		_presentation.setAffectedByMovement(true);
		_presentation.addCustomValue("diameter", diameter);
		_presentation.addCustomValue("frame", ::rnd_minmax(0, 2));
		
		// Rotation settings
		if (zeroGravity)
		{
			_rotationSpeed = ::frnd_minmax(c_maxRotationSpeed / 2.0, c_maxRotationSpeed);
		}
		_rotation = ::frnd_minmax(0, 360);
		_presentation.setCustomRotation(_rotation);
	}
	
	function onSpawn()
	{
		base.onSpawn();
		
		_presentation.start("idle", [], false, 0);
		
		if (_spawnedByAsteroid)
		{
			spawnParticleOneShot("particles/asteroid_cooldown", ::Vector2(0, 0), true, 0, false, ParticleLayer_UseLayerFromParticleEffect, _radius * 2);
			
			if (zeroGravity == false)
			{
				setSpeed(::Vector2(::rnd_minmax(-20, 20), ::rnd_minmax(-10, 10)));
			}
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onEnabled()
	{
		base.onEnabled();
		
		local movementSpeed = ::Vector2(0, speed);
		movementSpeed = movementSpeed.rotate(-rotation);
		
		if (zeroGravity == false)
		{
			_movementSettings.setExtraForce(::Vector2(0, -150));
		}
		startMovementInDirection(::Vector2(0, 0), _movementSettings);
		setSpeed(movementSpeed);
	}
	
	function onDisabled()
	{
		base.onEnabled();
		
		setSpeed(::Vector2(0, 0));
	}
	
	function onLavaTouchEnter()
	{
		local pos = getCenterPosition();
		spawnParticleOneShot("particles/asteroid_lavahit",  ::Vector2(0, -_radius / 2), false, 0, false, ParticleLayer_UseLayerFromParticleEffect, _radius / 10.0);
		for (local i = 0; i < _radius; i++)
		{
			// Spawn equally across diameter
			::spawnEntity("BlastwaveDebris", ::Vector2((pos.x - _radius) + (i * 2.0) + 1.0, pos.y - _radius),
			{
				speed           = (5 * c_mass) + 15,
				minSpeed        = (3 * c_mass) + 15,
				rotation        = -65 + (i.tofloat() / _radius) * 130.0, // ... and make them splash outwards
				positionCulling = false
			});
		}
		
		playSoundEffect("asteroid_lavahit");
	}
	
	function onLavaEnclosedEnter()
	{
		addProperty("dieQuietly");
		::killEntity(this);
	}
	
	function onSolidCollision(p_collisionNormal, p_speed)
	{
		// FIXME: Add more realistic physics response here
		local speed = getSpeed();
		local perpDot = ::Vector2.perpDot(p_collisionNormal, speed);
		if ((_rotationSpeed > 0 && perpDot < 0) || (_rotationSpeed < 0 && perpDot > 0))
		{
			_rotationSpeed = -_rotationSpeed;
		}
		
		// Add some rotation speed based on the impact speed
		local speedLength = speed.length();
		
		_rotationSpeed += (_rotationSpeed < 0 ? -speedLength : speedLength) * 0.9;
		_rotationSpeed *= 0.94;
	}
	
	function onBulletHit(p_bullet)
	{
		// Extra effect for asteroids
		p_bullet.playHitAnim("bullet_hit_asteroid", -p_bullet._speed.x * 10, -p_bullet._speed.y * 10);
		
		// FIXME: Add more realistic physics response here
		base.onBulletHit(p_bullet);
		
		// Simulate impulse
		setSpeed(getSpeed() + (p_bullet._speed / c_mass));
	}
	
	function onExplosionHit(p_explosion)
	{
		// Simulate impulse (set speed based on damage value)
		local speed = -(p_explosion.getCenterPosition() - getCenterPosition()).normalize() *
		              p_explosion.getDamageValue(this);
		setSpeed(getSpeed() + (speed * 1.25 / c_mass));
		
		base.onExplosionHit(p_explosion);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Update
	
	function update(p_deltaTime)
	{
		if (enabled == false)
		{
			return;
		}
		
		base.update(p_deltaTime);
		
		if (zeroGravity)
		{
			_rotation += _rotationSpeed * p_deltaTime;
			_presentation.setCustomRotation(_rotation);
		}
	}
}
