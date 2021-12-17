//include_entity("rewind/Trigger");

class BulletTime extends EntityBase
</
	placeable      = Placeable_Hidden
	movementset    = "Static"
	collisionRect  = [ 0.0, 0.0, 1.0, 1.0 ]  // center X, center Y, width, height
	group          = "Rewind"
/>
{
	_startDelay      = 0.0;
	_slowdownTime    = 0.1;
	_bulletTime      = 0.3;
	_speedupTime     = 0.5;
	
	_endScale        = 0.1;
	_easingTypeStart = EasingType_QuadraticOut;
	_easingTypeEnd   = EasingType_QuadraticIn;
	
	_easingScale = null;
	
	_totalTime = 0;
	_isSpeedingUp = false;
	
	function onCreate(p_id)
	{
		// Overwrite possible old instance
		BulletTime.remove();
		return true;
	}
	
	function onInit()
	{
		_easingScale = EasingReal(1.0, _endScale, _slowdownTime, _easingTypeStart);
		_totalTime = _slowdownTime + _bulletTime + _speedupTime;
		::g_bulletTimeInstance = this;
		
		::removeEntityFromWorld(this);
		
		local player = ::getFirstEntityByTag("PlayerBot");
		if (player != null)
		{
			player.customCallback("onBulletTimeEnter");
		}
	}
	
	function onSpawn()
	{
	}
	
	_curTime = 0;
	function update(p_deltaTime)
	{
		if (_startDelay > 0)
		{
			_startDelay -= p_deltaTime;
			return;
		}
		_curTime += p_deltaTime;
		_easingScale.update(p_deltaTime);
		
		if (_curTime >= (_slowdownTime + _bulletTime) && _curTime < _totalTime && _isSpeedingUp == false)
		{
			// Speeding up
			_easingScale = EasingReal(_endScale, 1.0, _speedupTime, _easingTypeEnd);
			_isSpeedingUp = true;
		}
		else if (_curTime >= _totalTime) 
		{
			::killEntity(this);
			return;
		}
		
		::Game.setDeltaTimeScale(_easingScale.getValue());
	}
	
	function onDie()
	{
		::Game.setDeltaTimeScale(1.0);
		::g_bulletTimeInstance = null;
		local player = ::getFirstEntityByTag("PlayerBot");
		if (player != null)
		{
			player.customCallback("onBulletTimeExit");
		}
	}
}

function BulletTime::remove()
{
	if (::g_bulletTimeInstance != null)
	{
		::killEntity(::g_bulletTimeInstance);
	}
}

function BulletTime::isActive()
{
	return ::g_bulletTimeInstance != null;
}

::g_bulletTimeInstance <- null;
