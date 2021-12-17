include_entity("rewind/EntityChild");

class HealthBar extends EntityChild
</
	placeable      = Placeable_Hidden
	movementset    = "Static"
	collisionRect  = [ 0.0, 0.0, 0.1, 0.1 ]
/>
{
	</
		autoGetSet = true
	/>
	_maxHealth = null;
	
	_health          = null;
	_invincible      = false;
	_lastDamager     = null;
	_hitPresentation = null;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		base.onInit();
		
		setDetectableByTouch(true);
		setDetectableBySight(false);
		setDetectableByLight(false);
		
		removeAllSightDetectionPoints();
		removeAllLightDetectionPoints();
		
		local rect = getCollisionRect();
		setTouchShape(::BoxShape(rect.getWidth(), rect.getHeight()), rect.getPosition());
	}
	
	function onSpawn()
	{
		base.onSpawn();
		
		if (_health == null)
		{
			_health = _maxHealth; // automatically assume full health when nothing is defined
		}
		
		if (_health > 1)
		{
			// Load presentation based on parent type
			local file = "presentation/hiteffect_" + _parent.getType().tolower();
			if (::presentationFileExists(file))
			{
				_hitPresentation = createPresentationObjectInLayer(file, ParticleLayer_InFrontOfShoeboxZeroTwo);
			}
		}
		
		initHealth(_health);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onBulletHit(p_bullet)
	{
		local damageValue = p_bullet.getDamageValue();
		
		if (damageValue > 0)
		{
			if (_parent._shield == null || _parent._shield.isIndestructible() == false)
			{
				doDamage(damageValue, p_bullet);
			}
		}
	}
	
	function onFlameHit(p_flame)
	{
		if (_parent._shield != null && _parent._shield.isIndestructible())
		{
			return;
		}
		
		local damageValue = p_flame.getDamageValue();
		if (damageValue > 0)
		{
			doDamage(damageValue, p_flame, "flame");
		}
	}
	
	function onLaserHit(p_laser)
	{
		local damageValue = p_laser.getDamageValue();
		if (damageValue > 0)
		{
			doDamage(damageValue, p_laser);
		}
	}
	
	function onExplosionHit(p_explosion)
	{
		if (_parent._shield != null && _parent._shield.isIndestructible())
		{
			return;
		}
		
		local damage = p_explosion.getDamageValue(_parent);
		if (damage > 0)
		{
			doDamage(damage, p_explosion);
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods
	
	function initHealth(p_health)
	{
		if (p_health == 0)
		{
			::tt_panic("Cannot init a healthbar with health 0");
		}
		
		_health = ::clamp(p_health, 1, getMaxHealth());
		
		if (_hitPresentation != null && _health < _maxHealth)
		{
			startHitPresentation("hit");
		}
	}
	
	function getNormalizedHealth()
	{
		return _health == null ? 1.0 : _health.tofloat() / _maxHealth.tofloat();
	}
	
	function setNormalizedHealth(p_health)
	{
		local prevHealth = _health;
		setHealth(getMaxHealth() * p_health);
		startHitPresentation(_health > prevHealth ? "heal" : "hit");
	}
	
	function getHealth()
	{
		return _health;
	}
	
	function setHealth(p_health)
	{
		_health = ::clamp(p_health, 0, getMaxHealth());
		recheckSensorFilter();
		
		if (_health == 0)
		{
			_parent.customCallback("onHealthBarEmpty", this, _lastDamager);
		}
	}
	
	function startHitPresentation(p_name, p_extraTag = null)
	{
		if (_hitPresentation != null)
		{
			local normalizedHealth = getNormalizedHealth();
			local tags = [];
			if      (p_extraTag != null)       tags.push(p_extraTag);
			if      (normalizedHealth <= 0.25) tags.push("high_damage");
			else if (normalizedHealth <= 0.50) tags.push("medium_damage");
			else if (normalizedHealth <= 0.75) tags.push("low_damage");
			
			_hitPresentation.start(p_name, tags, false, 0);
		}
	}
	
	function doTouchDamage(p_amount, p_damager)
	{
		return doDamage(p_amount, p_damager);
	}
	
	function doDamage(p_amount, p_damager, p_extraTag = null)
	{
		if (p_amount < 0 || _invincible)
		{
			return false;
		}
		
		_lastDamager = p_damager != null ? p_damager.weakref() : null;
		setHealth(_health - p_amount);
		
		startHitPresentation("hit", p_extraTag);
		_parent.customCallback("onDamage", this, p_amount);
		
		return true;
	}
	
	function doHeal(p_amount)
	{
		if (p_amount <= 0 || _health == _maxHealth)
		{
			return false;
		}
		
		setHealth(_health + p_amount);
		
		startHitPresentation("heal");
		_parent.customCallback("onHeal", this, -p_amount);
		return true;
	}
	
	function setInvincible(p_invincible)
	{
		_invincible = p_invincible;
	}
	
	function isEmpty()
	{
		return _health <= 0;
	}
	
	function isFull()
	{
		return _health == _maxHealth;
	}
}
