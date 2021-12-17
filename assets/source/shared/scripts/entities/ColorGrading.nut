class ColorGrading extends EntityBase
</
	placeable      = Placeable_Hidden
	collisionRect  = [ 0.0, 0.0, 0.1, 0.1 ]
/>
{
	// Internal members
	_stack     = null;
	_currentID = null;
	
	function onInit()
	{
		_stack     = [];
		_currentID = 0;
		::removeEntityFromWorld(this);
	}
	
	// Public members and methods
	function addInternal(p_textureName, p_duration, p_priority)
	{
		local texture = ::getColorGradingTexture(p_textureName);
		local activeID  = (_stack.len() == 0) ? null : _stack.top()[0];
		_currentID++;
		_stack.push([_currentID, texture, p_priority]);
		
		// Sort based on priority (which is in [2]) and id (which is in [0])
		_stack.sort(@(a, b) a[2] == b[2] ? a[0] <=> b[0] : a[2] <=> b[2]);
		
		if (_stack.top()[0] != activeID)
		{
			::setDefaultColorGrading(texture, p_duration);
		}
		
		// Return texture id
		return _currentID;
	}
	
	function addTimedInternal(p_textureName, p_fadeInDuration, p_duration, p_fadeOutDuration)
	{
		if ((p_fadeInDuration + p_duration + p_fadeOutDuration) <= 0)
		{
			::tt_panic("Tried to start a colorgrading effect with a duration of 0 (zero, noppes, nada!)");
			return;
		}
		
		::ColorGrading.add(p_textureName, p_fadeInDuration);
		local duration = p_fadeInDuration + p_duration;
		if (duration <= 0.0) duration = 0.001;
		
		startTimer("" + _currentID + "_" + p_fadeOutDuration, duration); // Duration is the time between fade in and fade out
	}
	
	function removeInternal(p_id, p_duration)
	{
		if (p_id == null)
		{
			return;
		}
		
		if (_stack.len() == 0)
		{
			::tt_panic("::ColorGrading.reset() called while the stack is empty");
			return;
		}
		
		// Check if this colorgrading is the last one
		if (_stack.top()[0] == p_id)
		{
			_stack.pop();
			
			if (_stack.len() == 0)
			{
				// last element popped; reset color grading
				::setDefaultColorGrading(null, p_duration);
				return;
			}
			
			// Do color grading to
			local texture = _stack.top()[1];
			::setDefaultColorGrading(texture, p_duration);
			return;
		}
		
		// Not last one; simply remove it from the 'stack'
		local idx = 0;
		foreach(elem in _stack)
		{
			if (elem[0] == p_id)
			{
				_stack.remove(idx);
				return;
			}
			idx++;
		}
		
		::tt_panic("Couldn't find id '" + p_id + "' in ColorGrading stack");
	}
	
	function onTimer(p_name)
	{
		local idx = p_name.find("_");
		local id = p_name.slice(0, idx).tointeger();
		local fadeoutTime = p_name.slice(idx + 1).tofloat();
		
		::ColorGrading.remove(id, fadeoutTime);
	}
	
	function getInstance()
	{
		if (hasInstance() == false)
		{
			::g_colorGradingInstance = ::spawnEntity("ColorGrading", ::Vector2(0, 0));
		}
		return ::g_colorGradingInstance;
	}
	
	function hasInstance()
	{
		return ::g_colorGradingInstance != null;
	}
}

function ColorGrading::add(p_textureName, p_duration, p_priority = ColorGradingPriority.Normal)
{
	return ::ColorGrading.getInstance().addInternal(p_textureName, p_duration, p_priority);
}

function ColorGrading::remove(p_id, p_duration = 0.0)
{
	::ColorGrading.getInstance().removeInternal(p_id, p_duration);
}

function ColorGrading::oneShot(p_textureName, p_fadeInDuration, p_duration, p_fadeOutDuration)
{
	::ColorGrading.getInstance().addTimedInternal(p_textureName, p_fadeInDuration, p_duration, p_fadeOutDuration);
}

function ColorGrading::reset()
{
	::setDefaultColorGrading(null, 0.0);
	if (hasInstance())
	{
		::killEntity(::g_colorGradingInstance);
		::g_colorGradingInstance = null;
	}
	::ColorGrading.getInstance();
}

::g_colorGradingInstance <- null;
