class GUICursor extends EntityBase
</
	placeable      = Placeable_Hidden
	collisionRect  = [ 0.0, 0.0, 1.0, 1.0 ]
/>
{
	_pressedElement = null;
	_currentElement = null;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		registerEntityByTag("GUICursor");
		
		disableTileRegistration();
		setPositionCullingEnabled(false);
		local elem = _currentElement;
		_currentElement = null;
		focusOn(elem);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onDie()
	{
		if (_currentElement != null) 
		{
			_currentElement.onLostCursorFocus(this);
		}
	}
	
	function onButtonLeftPressed()
	{
		releasePressedElement();
		if (_currentElement != null && _currentElement.onMoveLeft() == false)
		{
			focusOn(_currentElement._previousHorizontalElement, true);
		}
	}
	
	function onButtonRightPressed()
	{
		releasePressedElement();
		if (_currentElement != null && _currentElement.onMoveRight() == false)
		{
			focusOn(_currentElement._nextHorizontalElement, true);
		}
	}
	
	function onButtonUpPressed()
	{
		releasePressedElement();
		if (_currentElement != null && _currentElement.onMoveUp() == false)
		{
			focusOn(_currentElement._previousVerticalElement, true);
		}
	}
	
	function onButtonDownPressed()
	{
		releasePressedElement();
		if (_currentElement != null && _currentElement.onMoveDown() == false)
		{
			focusOn(_currentElement._nextVerticalElement, true);
		}
	}
	
	function onButtonAcceptPressed()
	{
		onPointerPressed(_currentElement);
	}
	
	function onButtonAcceptReleased(p_duration)
	{
		onPointerReleased(_currentElement, p_duration, true);
	}
	
	function onPointerPressed(p_element)
	{
		_pressedElement = p_element;
		if (_pressedElement != null)
		{
			_pressedElement.onPressed();
		}
	}
	
	function onPointerReleased(p_element, p_duration, p_releasedOnEntity)
	{
		if (_pressedElement == null)
		{
			// No element was pressed (or pressed has been reset)
			return;
		}
		::assert(p_element == _pressedElement, "p_element should be _pressedElement");
		if (p_releasedOnEntity == false)
		{
			// Element was pressed but mouse moved off it
			releasePressedElement();
			refocusOnCurrentElement();
			return;
		}
		
		if (_pressedElement.equals(_currentElement) == false && _pressedElement.c_canBeFocusedOn)
		{
			focusOn(_pressedElement);
		}
		else
		{
			refocusOnCurrentElement();
		}
		
		_pressedElement.onReleased(p_duration, true);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods
	
	function releasePressedElement()
	{
		if (_pressedElement != null)
		{
			_pressedElement.onReleased(0.0, false);
			_pressedElement = null;
		}
	}
	
	function getFocusElement()
	{
		return _currentElement;
	}
	
	function refocusOnCurrentElement()
	{
		local elem = _currentElement;
		_currentElement = null;
		focusOn(elem);
	}
	
	function focusOn(p_entity, p_playSoundEffect = false)
	{
		if (p_entity != null && p_entity.equals(_currentElement) == false)
		{
			::assert(p_entity instanceof ::GUIElement, "GUICursors can only focus on entities that derive from GUIElement (trying to focus on a " + p_entity.getType() + ")");
			if (_currentElement != null)
			{
				// Do not call onCursorReleasedOnEntity because that'll trigger the _currentElements action
				_currentElement.onCursorReleased(0.03);
				_currentElement.onLostCursorFocus(this);
			}
			
			if (p_playSoundEffect)
			{
				::Audio.playGlobalSoundEffect("Effects", "menu_highlight");
				::setRumble(RumbleStrength_Low, 0.0);
			}
			p_entity.onCursorFocus(this);
			_currentElement = p_entity;
		}
	}
}
