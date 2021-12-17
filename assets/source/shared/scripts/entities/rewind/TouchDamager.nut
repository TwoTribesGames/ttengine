include_entity("rewind/EntityChild");

class TouchDamager extends EntityChild
</
	placeable      = Placeable_Hidden
	movementset    = "Static"
	collisionRect  = [ 0.0, 0.0, 0.1, 0.1 ]
	group          = "Rewind"
/>
{
	// Constants 
	static c_sensorRecheckTime = 0.25;
	
	_touchDamageSensor = null;
	_healthBar = null;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		base.onInit();
		
		local parentRect = _parent.getCollisionRect();
		
		_touchDamageSensor = addTouchSensor(::BoxShape(parentRect.getWidth(), parentRect.getHeight()));
		_touchDamageSensor.setEnterCallback("onTouchDamageEnter");
		_touchDamageSensor.setFilterCallback("onTouchDamageFilter");
		
		startCallbackTimer("recheck", c_sensorRecheckTime);
	}
	
	function onTouchDamageFilter(p_entity)
	{
		return (p_entity instanceof ::RewindEntity) && p_entity.hasProperty("touchDamage");
	}
	
	function onTouchDamageEnter(p_entity, p_sensor)
	{
		// Save this because onTouchDamageDealt might kill p_entity; causing the properties to get lost
		local hasTouchDamage = p_entity.hasProperty("touchDamage");
		local touchDamageAmount = 0;
		if (hasTouchDamage)
		{
			touchDamageAmount = p_entity.getProperty("touchDamage");
			if (p_entity.isEnabled() == false && touchDamageAmount > 1)
			{
				touchDamageAmount = 1;
			}
		}
		p_entity.customCallback("onTouchDamageDealt", _parent);
		
		// parent might have altered the touchDamage; fetch again
		if (p_entity.hasProperty("touchDamage") == false)
		{
			return;
		}
		
		touchDamageAmount = p_entity.getProperty("touchDamage");
		if (p_entity.isEnabled() == false && touchDamageAmount > 1)
		{
			touchDamageAmount = 1;
		}
		
		if (_healthBar != null)
		{
			_healthBar.doTouchDamage(touchDamageAmount, p_entity);
		}
		else
		{
			_parent.customCallback("onHealthBarEmpty", null, this);
		}
		
		_parent.customCallback("onTouchDamage", p_entity, touchDamageAmount);
	}
	
	function onParentDied(p_parent)
	{
		base.onParentDied(p_parent);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods
	
	function recheck()
	{
		_touchDamageSensor.removeAllSensedEntities();
		startCallbackTimer("recheck", c_sensorRecheckTime);
	}
}
