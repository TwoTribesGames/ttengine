class PatternMovement
{
	_currentPattern = null;
	_targetEntity   = null;
	_time           = 0.0;
	_isActive       = false;
	_direction      = null;
	_previousResult = null;
	
	constructor(p_targetEntity)
	{
		_currentPattern = null;
		_targetEntity   = p_targetEntity.weakref();
		_time           = 0.0;
		_isActive       = false;
		_direction      = null;
		_previousResult = null;
	}
	
	// Generators
	generate =
	{
		sin_x   = function(t, scale, speed) { speed *= 2 * ::PI; return ::Vector2(::sin(t * speed) * scale * speed, 0.0); }
		sin_y   = function(t, scale, speed) { speed *= 2 * ::PI; return ::Vector2(0.0, ::sin(t * speed) * scale * speed); }
		cos_x   = function(t, scale, speed) { speed *= 2 * ::PI; return ::Vector2(::cos(t * speed) * scale * speed, 0.0); }
		cos_y   = function(t, scale, speed) { speed *= 2 * ::PI; return ::Vector2(0.0, ::cos(t * speed) * scale * speed); }
		circle  = function(t, scale, speed) { speed *= 2 * ::PI; return ::Vector2(::sin(t * speed) * scale * speed, ::cos(-t * speed) * scale * speed); }
	}
	
	function startMovement(p_pattern, p_direction = ::Vector2(0, 0), p_time = 0.0)
	{
		if (_targetEntity == null)
		{
			return;
		}
		
		_targetEntity.stopMovement();
		_previousResult = ::Vector2(0, 0);
		_currentPattern = p_pattern;
		_direction      = p_direction;
		_time           = p_time;
		_isActive       = true;
	}
	
	function setPattern(p_pattern)
	{
		_currentPattern = p_pattern;
	}
	
	function setTime(p_time)
	{
		_time = p_time;
	}
	
	function setDirection(p_direction)
	{
		_direction = p_direction;
	}
	
	function getDirection()
	{
		return _direction;
	}
	
	function stopMovement()
	{
		if (_targetEntity == null)
		{
			return;
		}
		_targetEntity.setSpeed(::Vector2(0, 0));
		
		_currentPattern = null;
		_isActive       = false;
	}
	
	function pauseMovement()
	{
		_isActive = false;
	}
	
	function resumeMovement()
	{
		_isActive = true;
	}
	
	function getDelta()
	{
		
		_isActive = false;
	}
	
	function update(p_deltaTime)
	{
		if (_targetEntity == null || _isActive == false)
		{
			return;
		}
		
		_time += p_deltaTime;
		local speed = _currentPattern == null ? ::Vector2(0, 0) :
			_currentPattern.call(_targetEntity, _time, _previousResult);
		_targetEntity.setSpeed(speed + _direction);
		_previousResult = speed;
	}
}
