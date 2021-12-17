include_entity("rewind/bullets/Deflectable");
include_entity("rewind/EntityChild");

class Shield extends EntityChild
</
	placeable      = Placeable_Hidden
	movementset    = "Static"
	collisionRect  = [ 0.0, 0.0, 0.1, 0.1 ]
	group          = "Rewind"
/>
{
	// Constants
	static c_spreadDestructibleShield = 180;
	static c_weaponGroup              = null;
	
	// Creation values
	_range         = null;
	_health        = null;
	
	// Internal values
	_presentation  = null;
	_shape         = null;
	_sensor        = null;
	_enabled       = true;
	_enabledSet    = false;
	_initialHealth = null;
	_angle         = 0;
	_type          = null;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		base.onInit();
		
		_initialHealth = _health;
		
		_presentation = createPresentation();
		if (_presentation != null)
		{
			// Shared by all shields
			_presentation.setAffectedByOrientation(false);
			_presentation.addCustomValue("shieldRange", _range);
		}
		
		_shape  = createShape();
		_sensor = addTouchSensor(_shape, null, ::Vector2(0, 0));
		_sensor.setEnterCallback("onShieldEnter");
		_sensor.setFilterCallback("onShieldFilter");
	}
	
	function onSpawn()
	{
		base.onSpawn();
		
		updateVisual();
		_enabled ? enableShield() : disableShield(false);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onShieldFilter(p_entity)
	{
		// FIXME: Clean up hardcoded checks
		return (p_entity instanceof ::Bullet) &&
			p_entity._group != c_weaponGroup && p_entity._isDisabled == false;
	}
	
	function onShieldEnter(p_entity, p_sensor)
	{
		p_entity.customCallback("onShieldHit", this);
		_parent.customCallback("onShieldGotHit", this, p_entity);
		
		local shouldDeflect = p_entity instanceof ::Deflectable;
		if (isIndestructible() || shouldDeflect)
		{
			// Deflect code
			local shieldNormal = (p_entity.getPosition() - getPosition()).normalize();
			p_entity.reflect(shieldNormal, 0.8);
		}
		
		local amount = shouldDeflect ? getDeflectionDamage(p_entity) : p_entity.getDamageValue();
		if (amount > 0)
		{
			damage(amount);
		}
		
		_presentation.startEx("hit", [p_entity.getType()], false, 0, getPresentationCallback("idle"));
	}
	
	function onExplosionHit(p_explosion)
	{
		// Indestructible shield; only play hit anim
		if (isIndestructible())
		{
			_presentation.startEx("hit", [], false, 0, getPresentationCallback("idle"));
			
			return;
		}
		
		damage(p_explosion.getDamageValue(this._parent));
	}
	
	function onFlameHit(p_flame)
	{
		damage(p_flame.getDamageValue());
	}
	
	function onProgressRestored(p_id)
	{
		// reconnect the shape with the sensor because that gets lost... ?
		_sensor.setShape(_shape);
	}
	
	function onPresentationObjectEnded(p_object, p_name)
	{
		if (p_name == "idle")
		{
			_presentation.start("idle", [], false, 0);
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods
	
	function createShape()
	{
		if (_type == ShieldType.Indestructible)
		{
			return ::CircleShape(0, _range);
		}
		else
		{
			return ::ConeShape(0, _range, 0, c_spreadDestructibleShield);
		}
	}
	
	function isIndestructible()
	{
		return _enabled && _type == ShieldType.Indestructible;
	}
	
	function setAngle(p_angle)
	{
		if (_type == ShieldType.Indestructible)
		{
			// Always same graphic, return
			return;
		}
		
		_angle = p_angle;
		_shape.setAngle(p_angle);
		_presentation.setCustomRotation(-p_angle); // presentation uses flipped angle
	}
	
	function updateVisual()
	{
		if (_type == ShieldType.Indestructible)
		{
			// Always same graphic, return
			return;
		}
		
		local damage = (_initialHealth - _health) / _initialHealth.tofloat();
		_presentation.addCustomValue("shieldDamage", (damage * -180.0).tointeger());
		if (_enabled)
		{
			_presentation.stop();
			_presentation.start("idle", [], false, 0);
		}
	}
	
	function getDeflectionDamage(p_entity)
	{
		return isIndestructible() ? 0 : p_entity.c_deflectionDamage;
	}
	
	function isDamaged()
	{
		if (_type == ShieldType.Indestructible)
		{
			return false;
		}
		
		return _health < _initialHealth;
	}
	
	function damage(p_hitpoints)
	{
		if (_type == ShieldType.Indestructible)
		{
			return;
		}
		
		_health -= p_hitpoints;
		if (_health <= 0)
		{
			::killEntity(this);
		}
		else
		{
			updateVisual();
		}
	}
	
	function isEnabled()
	{
		return (_enabledSet && _enabled);
	}
	
	function enableShield()
	{
		if (isEnabled() || (_health != null && _health <= 0))
		{
			return;
		}
		
		if (_enabledSet)
		{
			playSoundEffect("shield_enemy_show");
		}
		
		_enabledSet = true;
		_enabled = true;
		
		_sensor.setEnabled(true);
		
		// Delayed enabling
		_presentation.stop();
		_presentation.startEx("show", [], false, 0, getPresentationCallback("idle"));
		
		if (isIndestructible() && _parent.hasProperty("noticeEMP"))
		{
			addProperty("noticeEMP");
			_parent.removeProperty("noticeEMP");
		}
	}
	
	function disableShield(p_playHideAnimation = true)
	{
		if (_enabledSet && _enabled == false)
		{
			return;
		}
		
		if (_enabledSet)
		{
			playSoundEffect("shield_enemy_hide");
		}
		
		_enabledSet = true;
		_enabled = false;
		
		_sensor.setEnabled(false);
		
		if (p_playHideAnimation)
		{
			_presentation.start("hide", [], true, 5);
		}
		else
		{
			_presentation.stop();
		}
		
		// Check if we need to restore the noticeEMP property now that the shields are down
		if (hasProperty("noticeEMP"))
		{
			_parent.addProperty("noticeEMP");
			removeProperty("noticeEMP");
		}
	}
}
