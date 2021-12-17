include_entity("rewind/bullets/Deflectable");

class Missile extends Deflectable
</ 
	placeable      = Placeable_Hidden
	movementset    = "Static"
	collisionRect  = [ 0.0, 0.0, 0.5, 0.5 ]
	group          = "Rewind"
/>
{
	// Constants
	static c_presentationFile = "presentation/missile";
	static c_deflectionDamage = 6;
	static c_waterDrag        =  0.10 * 60;
	static c_waterImpact      =  0.10;
	
	_propulsionSound = null;
	
	_damageValue     = 3;
	_explosionRadius = 5;
	
	// Wobble settings
	static c_wobbleEnabled   = true;
	static c_wobbleStartTime = 0.2;
	_wobbleAmplitude = 2.0;
	_wobbleFrequency = 20;
	
	_hitsDeflectables = true;
	
	//-------------------------------------------------------------------------
	// Overloaded methods
	function onInit()
	{
		base.onInit();
	}
	
	function onSpawn()
	{
		base.onSpawn();
		
		_timeAlive = 0;
		_wobbleAmplitude = ::frnd_minmax(_wobbleAmplitude / 2, _wobbleAmplitude);
		_wobbleFrequency = ::frnd_minmax(_wobbleFrequency / 2, _wobbleFrequency);
		
		_wobbleAmplitude = brnd() ? -_wobbleAmplitude : _wobbleAmplitude;
		
		if (_isDisabled == false)
		{
			_propulsionSound = playSoundEffect("missile_loop");
		}
	}
	
	function handleEntityImpact(p_entity)
	{
		impact();
	}
	
	function onBulletHit(p_bullet)
	{
		_presentation.stop();
		_presentation.start("idle", [], false, 0);
		spawnParticleOneShot("particles/hit_sparks", ::Vector2(0, 0), false, 0, false, ParticleLayer_UseLayerFromParticleEffect, 1.0);
		// This defuses the bomb to make it less harmfull when it hits close to you.
		_explosionRadius = 1;
		_damageValue = 1;
		
		base.onBulletHit(p_bullet);
	}
	
	function onShieldHit(p_shield)
	{
		if (p_shield.isIndestructible() == false)
		{
			// Fake explosion
			spawnParticleOneShot("particles/explosion", ::Vector2(0, 0), false, 0, false, ParticleLayer_UseLayerFromParticleEffect, 2.0);
			playSoundEffect("explosion");
			
			base.onShieldHit(p_shield);
			return;
		}
		_shooter = p_shield._parent;
		_group = p_shield.c_weaponGroup;
		_hitsDeflectables = false;
		_presentation.start("idle", [_group, "deflected"], false, 0);
	}
	
	function onWaterTouchEnter()
	{
		stopPropulsionSound();
	}
	
	function onWallHit()
	{
		// Prevent baseclass from calling onWallHit()
		if (_inWater || _isDisabled)
		{
			local effectname = _inWater ? "particles/underwater_remove_missile" : "particles/abovewater_remove_missile";
			spawnParticleOneShot(effectname, ::Vector2(0, 0), false, 0, false,
				ParticleLayer_UseLayerFromParticleEffect, 1.0);
			removeBullet();
		}
		else
		{
			impact();
		}
	}
	
	function update(p_deltaTime)
	{
		if (isSuspended()) return;
		
		base.update(p_deltaTime);
		
		if (c_wobbleEnabled && _timeAlive >= c_wobbleStartTime && _isDisabled == false)
		{
			local x = ((_timeAlive - c_wobbleStartTime)) * _wobbleFrequency;
			local rot = ::cos(x) * _wobbleAmplitude * (x < ::PI ? 0.5 : 1.0);
			_speed = _speed.rotate(rot * ::radToDeg * p_deltaTime);
		}
		
		updateAngle();
	}
	
	function playHitAnim(p_animName, p_velocityX, p_velocityY, p_affectedEmitters = [0])
	{
		// empty to override base effect
	}
	
	function impact()
	{
		createExplosion(getCenterPosition(), _explosionRadius, _shooter,
			{
				_damageValue      = _damageValue,
				_parentVesselType = this.getclass()
			}
		);
		
		playSoundEffect("missile_impact");
		
		removeBullet();
	}
	
	function stopPropulsionSound()
	{
		if (_propulsionSound != null)
		{
			_propulsionSound.stop();
			_propulsionSound = null;
		}
	}
	
	function removeBullet()
	{
		stopPropulsionSound();
		
		base.removeBullet();
	}
	
	function _typeof()
	{
		return "Missile";
	}
	
	//-------------------------------------------------------------------------
	// Class specific methods
}
