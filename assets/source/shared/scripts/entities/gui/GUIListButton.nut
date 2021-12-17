include_entity("gui/GUIButton");

enum SideButtonState
{
	Left,
	Right,
	None
}

class GUIListButton extends GUIButton
</
	placeable      = Placeable_Hidden
	collisionRect  = [ 0.0, 0.0, 1.085, 0.05 ]
/>
{
	static c_presentationFile  = "guilistbutton";
	static c_sideButtonWidth   = 1.085 / 2.0;
	
	_index = 0;
	_sideButtonState = SideButtonState.None;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		base.onInit();
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onCursorPressed()
	{
		_sideButtonState = getSideButtonState();
		if (_sideButtonState == SideButtonState.Left)
		{
			_presentation.removeTag("right");
			_presentation.addTag("left");
			base.onCursorPressed();
		}
		else if (_sideButtonState == SideButtonState.Right)
		{
			_presentation.removeTag("left");
			_presentation.addTag("right");
			base.onCursorPressed();
		}
		// else don't do anything
	}
	
	function onCursorReleased(p_duration)
	{
		_presentation.removeTag("left");
		_presentation.removeTag("right");
	}
	
	function onCursorReleasedOnEntity(p_duration)
	{
		_presentation.removeTag("left");
		_presentation.removeTag("right");
		if (_sideButtonState == getSideButtonState())
		{
			// Same enter and exit; switch based on side button state
			if (_sideButtonState == SideButtonState.Left)
			{
				onMoveLeft();
			}
			else if (_sideButtonState == SideButtonState.Right)
			{
				onMoveRight();
			}
		}
	}
	
	function onMoveLeft()
	{
		if (isEnabled() == false)
		{
			return false;
		}
		
		moveLeft();
		select();
		
		::Audio.playGlobalSoundEffect("Effects", "menu_switch");
		::setRumble(RumbleStrength_Low, 0.0);
		return true; // Button is handled
	}
	
	function onMoveRight()
	{
		if (isEnabled() == false)
		{
			return false;
		}
		
		moveRight();
		select();
		
		::Audio.playGlobalSoundEffect("Effects", "menu_switch");
		::setRumble(RumbleStrength_Low, 0.0);
		return true; // Button is handled
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods
	
	function getSideButtonState()
	{
		local cursor = ::getFirstEntityByTag("GUICursor");
		if (cursor == null)
		{
			return SideButtonState.None;
		}
		
		local rect = getCollisionRectWorldSpace();
		local cursorPos = cursor.getPosition();
		if (rect.containsPoint(cursorPos))
		{
			local sideX = (rect.getPosition().x - (rect.getWidth() / 2.0)) + c_sideButtonWidth;
			if (cursorPos.x < sideX)
			{
				return SideButtonState.Left;
			}
			else if (cursorPos.x > -sideX)
			{
				return SideButtonState.Right;
			}
		}
		
		return SideButtonState.None;
	}
	
	function moveLeft()
	{
		--_index;
		if (_index < 0) _index = _content.len() - 1;
		_updateContent(_content[_index]);
	}
	
	function moveRight()
	{
		++_index;
		if (_index >= _content.len()) _index = 0;
		_updateContent(_content[_index]);
	}
	
	function select()
	{
		// call button delegate
		if (_delegate != null)
		{
			_delegate.acall(_args);
		}
	}
	
	function getScrolledItems(p_items, p_selection)
	{
		local foundIdx = 0;
		for (local i = 0; i < p_items.len(); ++i)
		{
			if (p_items[i][1] == p_selection)
			{
				foundIdx = i;
				break;
			}
		}
		
		local i = foundIdx;
		local items = [];
		do
		{
			items.push(p_items[i]);
			++i;
			if (i >= p_items.len()) i = 0;
		} while (i != foundIdx);
		
		return items;
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Inherited Methods
	
	function processContent(p_content)
	{
		// For GUIListButtons the content should be an array of arrays
		if (typeof(p_content) != "array" )
		{
			::tt_panic("GUIListButton expects content to be of type 'array', not '" + typeof(p_content) + "'");
			return;
		}
		else if (p_content.len() < 1)
		{
			::tt_panic("GUIListButton expects content to have at least 1 element, not " + p_content.len());
			return;
		}
		_updateContent(p_content[_index]);
	}
}
