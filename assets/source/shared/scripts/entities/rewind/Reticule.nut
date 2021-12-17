include_entity("rewind/EntityChild");

class Reticule extends EntityChild
</
	placeable      = Placeable_Hidden
	movementset    = "Static"
	collisionRect  = [ 0.0, 0.0, 1.1, 0.1 ]
/>
{
	static c_length = 30;
	
	_visible   = true;
	_shape     = null;
	_sensor    = null;
	_powerBeam = null;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		base.onInit();
		
		_shape = ::RayShape(::Vector2(0, 0));
		_sensor = addTouchSensor(_shape, null, ::Vector2(0, 0));
		_sensor.setFilterCallback("onFilter");
		_powerBeam = addPowerBeamGraphic(PowerBeamType_Sight, _sensor);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onProgressRestored(p_id)
	{
		// FIXME: Fix this lost shape thing!
		_sensor.setShape(_shape);
	}
	
	function onFilter(p_entity)
	{
		return p_entity instanceof ::BaseEnemy && p_entity.containsVirus() == false;
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods
	
	function setAngle(p_angle)
	{
		local rad = p_angle * ::degToRad;
		local dirX = ::sin(rad);
		local dirY = ::cos(rad);
		_shape.setOffsetEndPos(::Vector2(c_length * dirX, c_length * dirY));
		_sensor.setOffset(::Vector2(1.0 * dirX, 1.0 * dirY));
	}
	
	function setEnabled(p_enabled)
	{
		//echo("p_enabled " + p_enabled + " _enabled " + _enabled + " _visible " + _visible);
		if (_enabled == false && p_enabled && _visible)
		{
			_sensor.setEnabled(true);
		}
		else if (_enabled && p_enabled == false && _visible)
		{
			_sensor.setEnabled(false);
		}
		_enabled = p_enabled;
	}
	
	function show()
	{
		_visible = true;
	}
	
	function hide()
	{
		_visible = false;
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Update
	
	function childUpdate(p_deltaTime)
	{
		local showReticule = (::hasTwinstickControlScheme() == false ||
		                      ::getCurrentControllerType() == ControllerType_Keyboard) && _visible;
		
		if (_sensor.isEnabled() && showReticule == false)
		{
			_sensor.setEnabled(false);
		}
		else if (_sensor.isEnabled() == false && showReticule)
		{
			_sensor.setEnabled(true);
		}
	}
}
