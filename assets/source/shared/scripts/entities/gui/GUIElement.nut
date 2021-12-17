class GUIElement extends EntityBase
</
	placeable      = Placeable_Hidden
	collisionRect  = [ 0.0, 0.0, 6.0, 4.0 ]
/>
{
	static c_presentationFile  = null;
	static c_isSelectable      = false;
	static c_canBeFocusedOn    = false;
	
	_presentation              = null;
	_animName                  = "idle";
	_enabled                   = true;
	_enabledSet                = false;
	_visible                   = true;
	_nextHorizontalElement     = null;
	_previousHorizontalElement = null;
	_nextVerticalElement       = null;
	_previousVerticalElement   = null;
	_width                     = null;
	_height                    = null;
	_parentMenu                = null;
	_id                        = null;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		makeScreenSpaceEntity();
		createPresentation();
		
		setEnabled(_enabled);
	}
	
	function onSpawn()
	{
		startAnimation(["appear"], 1);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	// Hover state
	function onHoverEnter()
	{
		if (_presentation != null)
		{
			_presentation.addTag("hover");
			startAnimation();
		}
	}
	
	function onHoverExit()
	{
		if (_presentation != null)
		{
			_presentation.removeTag("hover");
			startAnimation();
		}
	}
	
	// base pressed handling for pointer or cursor or whatever press, returns true if handled
	function onPressed()
	{
		if (isEnabled())
		{
			onCursorPressed();
			return true;
		}
		
		return false;
	}
	
	// base pressed handling for pointer or cursor or whatever release, returns true if handled
	function onReleased(p_duration, p_isOnEntity)
	{
		if (isEnabled())
		{
			if (p_isOnEntity)
			{
				onCursorReleasedOnEntity(p_duration);
			}
			else
			{
				onCursorReleased(p_duration);
			}
			
			return true;
		}
		
		return false;
	}
	
	// base move left handling for pointer or cursor or whatever, returns true if handled
	function onMoveLeft()
	{
		return false;
	}
	
	// base move right handling for pointer or cursor or whatever, returns true if handled
	function onMoveRight()
	{
		return false;
	}
	
	// base move up handling for pointer or cursor or whatever, returns true if handled
	function onMoveUp()
	{
		return false;
	}
	
	// base move down handling for pointer or cursor or whatever, returns true if handled
	function onMoveDown()
	{
		return false;
	}
	
	// Callbacks to override, these get called when the element is enabled, "hot" etc.
	function onCursorPressed()
	{
	}
	
	function onCursorReleased(p_duration)
	{
	}
	
	function onCursorReleasedOnEntity(p_duration)
	{
	}
	
	function onCursorFocus(p_cursor)
	{
		if (_parentMenu != null)
		{
			_parentMenu.customCallback("onCursorFocus", this);
		}
	}
	
	function onLostCursorFocus(p_cursor)
	{
		if (_parentMenu != null)
		{
			_parentMenu.customCallback("onLostCursorFocus", this);
		}
	}
	
	function onRefocus()
	{
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods
	
	function createPresentation()
	{
		if (c_presentationFile != null)
		{
			_presentation = createPresentationObjectInLayer("presentation/menu/elements/" + c_presentationFile, ParticleLayer_Hud);
			if (_parentMenu != null)
			{
				_parentMenu.addPresentationTags(_presentation);
			}
			else
			{
				::tt_panic(this + " should have parent menu");
			}
		}
	}
	
	function setVisible(p_visible)
	{
		_presentation.stop();
		if (p_visible)
		{
			startAnimation(["appear"], 1);
		}
		else
		{
			_presentation.stop();
		}
		_visible = p_visible;
	}
	
	function setEnabled(p_enabled)
	{
		if (_enabledSet && p_enabled == _enabled)
		{
			return;
		}
		
		_enabled = p_enabled;
		if (p_enabled)
		{
			_presentation.removeTag("disabled");
		}
		else
		{
			_presentation.addTag("disabled");
		}
		
		if (_enabledSet)
		{
			restartAnimation();
		}
		_enabledSet = true;
	}
	
	function isEnabled()
	{
		return _enabled;
	}
	
	function startAnimation(p_tags = [], p_priority = 1)
	{
		_presentation.stop();
		_presentation.start(_animName, p_tags, false, p_priority);
	}
	
	function restartAnimation(p_tags = [], p_priority = 1)
	{
		if (_presentation.getPriority() <= p_priority)
		{
			_presentation.stop();
		}
		startAnimation(p_tags);
	}
}
