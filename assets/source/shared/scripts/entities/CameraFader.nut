// A class to play screenspace fades
::g_activeFade <- null;

class CameraFader extends EntityBase
</
	placeable      = Placeable_Hidden
	collisionRect  = [ 0.0, 0.0, 0.1, 0.1 ]
/>
{
	_parent           = null;
	_delegate         = null;
	_presentation     = null;
	
	_tvOffset  = 0;
	_drcOffset = 0;
	
	_fadeAnimationName = "";
	_loadLevel         = null;
	
	_isInterruptible = false;
	_killAtEnd = true;
	
	function onInit()
	{
		::removeEntityFromWorld(this);
		makeScreenSpaceEntity();
		setCanBePaused(false);
		
		_presentation = createPresentationObjectInLayer("presentation/wipes", ParticleLayer_InFrontOfHud);
		_presentation.addCustomValue("tvOffset", _tvOffset);
		_presentation.addCustomValue("drcOffset", _drcOffset);
		_presentation.startEx(_fadeAnimationName, [], false, 0, getPresentationCallback("done"));
		
		if (::g_activeFade != null)
		{
			::killEntity(::g_activeFade);
			::g_activeFade = null;
		}
		
		::g_activeFade = this.weakref();
	}
	
	function setDelegate(p_delegate)
	{
		_delegate = p_delegate;
	}
	
	_fadeEnded = false;
	function endFade()
	{
		// don't trigger the end callbacks twice
		if (_fadeEnded) return;
		_fadeEnded = true;
		
		if (_parent != null)
		{
			_parent.customCallback("onFadeEnded", this, _fadeAnimationName);
		}
		if (_delegate != null)
		{
			if (_delegate.getinfos().parameters.len() == 2)
			{
				_delegate.call(_parent, this);
			}
			else
			{
				_delegate.call(_parent);
			}
		}
		
		if (_loadLevel != null)
		{
			::Level.load(_loadLevel);
		}
		
		if (_killAtEnd)
		{
			::killEntity(this);
		}
	}
	
	/* this never happens, but it might be something we need to catch someday
	function onPresentationObjectCanceled(p_object, p_name)
	{
		endFade();
	}
	*/
	
	function onPresentationObjectEnded(p_object, p_name)
	{
		endFade();
		if (_parent != null)
		{
			_parent.customCallback("onFadeRemoved", this);
		}
	}
	
	function onPresentationObjectCallback(p_object, p_name)
	{
		if (p_name == "fade_end")
		{
			endFade();
		}
	}
	
	function onProgressRestored(p_id)
	{
		if (_isInterruptible || _fadeEnded) ::killEntity(this);
	}
	
	function onDie()
	{
		::g_activeFade = null;
	}
}

// Creates a fade that stays on screen until the fade animation ended
function createFade(p_parent = null, p_fadeAnimationName = "transparent_to_opaque", p_drcOffset = 0, p_tvOffset = 0)
{
	return ::spawnEntity("CameraFader", ::Vector2(0,0), 
	{ 
		_fadeAnimationName = p_fadeAnimationName, 
		_parent           = (p_parent != null) ? p_parent.weakref() : null, 
		_drcOffset        = p_drcOffset,
		_tvOffset         = p_tvOffset
	});
}

// Creates a fade that stays on screen until client code destroys it
function createPersistentFade(p_parent = null, p_fadeAnimationName = "transparent_to_opaque", p_drcOffset = 0, p_tvOffset = 0)
{
	return ::spawnEntity("CameraFader", ::Vector2(0,0), 
	{ 
		_fadeAnimationName = p_fadeAnimationName, 
		_parent           = (p_parent != null) ? p_parent.weakref() : null, 
		_drcOffset        = p_drcOffset,
		_tvOffset         = p_tvOffset,
		_killAtEnd        = false
	});
}

function createLevelFade(p_level, p_parent = null)
{
	local fade = createFade(p_parent, "transparent_to_opaque", 0, 0);
	
	fade._loadLevel       = p_level;
	
	return fade;
}

