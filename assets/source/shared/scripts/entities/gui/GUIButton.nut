include_entity("gui/GUIElement");

class GUIButton extends GUIElement
</
	placeable      = Placeable_Hidden
	collisionRect  = [ 0.0, 0.0, 1.085, 0.05 ]
/>
{
	// Constants
	static c_presentationFile  = "guibutton";
	static c_textFont          = GlyphSetID_Header;
	static c_textAlignment     = HorizontalAlignment_Center;
	static c_textHasDropShadow = false;
	static c_textLabelWidth    = 20.0;
	static c_textLabelHeight   = 1.0;
	static c_isSelectable      = true;
	static c_canBeFocusedOn    = true;
	
	// Creation params
	_content          = null;
	_delegate         = null;
	_caller           = null;
	
	// Internals
	_isPressed        = false;
	_label            = null;
	_args             = null;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		base.onInit();
		
		_label = addTextLabel("", c_textLabelWidth, c_textLabelHeight, c_textFont);
		_label.setHorizontalAlignment(HorizontalAlignment_Center);
		_label.setVerticalAlignment(VerticalAlignment_Center);
		_label.setColor(ColorRGBA(255,255,255,255));
		if (c_textHasDropShadow)
		{
			local offset = ::Vector2(-0.1 / 34, -0.1 / 34); // / 34 = 1080 (height) / 32 (0.1 was tweaked in worldspace where 1 unit is 32 pixels)
			
			_label.addDropShadow(offset, ::ColorRGBA(0,0,0,128));
		}
		_presentation.addTextLabel(_label, ::Vector2(0, 0));
		
		updateLabel();
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onPressed()
	{
		if (isEnabled() == false)
		{
			::Audio.playGlobalSoundEffect("Effects", "menu_select_cannot");
		}
		
		base.onPressed();
	}
	
	function onCursorPressed()
	{
		setPressed(true);
		base.onCursorPressed();
	}
	
	function onCursorReleased(p_duration)
	{
		setPressed(false);
		base.onCursorReleased(p_duration);
	}
	
	function onCursorReleasedOnEntity(p_duration)
	{
		base.onCursorReleasedOnEntity(p_duration);
		
		// call button delegate
		if (_delegate != null)
		{
			_delegate.acall(_args);
		}
		
		setIsNew(_id, false);
		
		::Audio.playGlobalSoundEffect("Effects", "menu_next");
		::setRumble(RumbleStrength_Low, 0.0);
	}
	
	function onCursorFocus(p_cursor)
	{
		base.onCursorFocus(p_cursor);
		
		_animName = "selected";
		startAnimation();
	}
	
	function onLostCursorFocus(p_cursor)
	{
		base.onLostCursorFocus(p_cursor);
		
		_animName = "idle";
		startAnimation();
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// 'Static' Methods
	
	function setIsNew(p_id, p_isNew)
	{
		if (p_id == null)
		{
			::tt_panic("p_id should not be null");
			return;
		}
		
		local table = ::getRegistry().getPersistent("new_buttons");
		if (table == null)
		{
			table = {};
		}
		
		local hasID = (p_id in table);
		if (hasID && p_isNew == false)
		{
			delete table[p_id];
			::getRegistry().setPersistent("new_buttons", table);
		}
		else if (hasID == false && p_isNew)
		{
			table[p_id] <- true;
			::getRegistry().setPersistent("new_buttons", table);
		}
	}
	
	function isNew(p_id)
	{
		if (p_id == null)
		{
			::tt_panic("p_id should not be null");
			return false;
		}
		
		local table = ::getRegistry().getPersistent("new_buttons");
		if (table == null)
		{
			return false;
		}
		return (p_id in table);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods
	
	function processContent(p_content)
	{
		if (p_content != null)
		{
			_updateContent(p_content);
		}
	}
	
	function setPressed(p_pressed)
	{
		_isPressed = p_pressed;
		if (p_pressed)
		{
			_animName = "pressed";
			startAnimation();
		}
		else
		{
			_animName = "idle";
			startAnimation(["released"]);
		}
	}
	
	function setVisible(p_visible)
	{
		base.setVisible(p_visible);
		
		_label.setNumberOfVisibleCharacters(p_visible ? -1 : 0);
		
		if (p_visible)
		{
			updateLabel();
		}
	}
	
	function updateLabel()
	{
		processContent(_content);
		
		if (isNew(_id))
		{
			_label.setTextUTF8(_label.getTextUTF8() + " " + ::getLocalizedStringUTF8("MENU_BUTTON_NEW"));
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// 'Protected' Methods
	
	function _updateContent(p_content)
	{
		if (typeof(p_content) != "array" || p_content.len() == 0 ||
		   (typeof(p_content[0]) != "string" && typeof(p_content[0]) != "array"))
		{
			::tt_panic("Content should be an array with first element a textid string or array and optional " +
			           "remaining elements parameters for the callback.\n" +
			           "Content Type is: " + typeof(p_content) + "\n" +
			           "Content is:\n" + ::niceStringFromObject(p_content));
			_label.setText("#ERROR");
			return;
		}
		
		if (typeof(p_content[0]) == "string")
		{
			if (p_content[0].len() > 0)
			{
				_label.setTextLocalized(p_content[0]);
			}
		}
		else
		{
			// Array of two strings; first string is the textid, second string is the variable argument (non-localized)
			if (p_content[0].len() == 2)
			{
				if (typeof(p_content[0][0]) == "string" && typeof(p_content[0][1]) == "string")
				{
					_label.setTextLocalizedAndFormatted(p_content[0][0], [p_content[0][1]]);
				}
				else
				{
					::tt_panic("If first element is array, the first string should be the textid " + 
					           "and the second (and optional third) string should be the variable argument (non-localized).\n" +
					           "Content Type is: " + typeof(p_content[0]) + "\n" +
					           "Content is:\n" + ::niceStringFromObject(p_content[0]));
					_label.setText("#ERROR");
					return;
				}
			}
			else if (p_content[0].len() == 3)
			{
				if (typeof(p_content[0][0]) == "string" && typeof(p_content[0][1]) == "string" && typeof(p_content[0][2]) == "string")
				{
					_label.setTextLocalizedAndFormatted(p_content[0][0], [p_content[0][1], p_content[0][2]]);
				}
				else
				{
					::tt_panic("If first element is array, the first string should be the textid " + 
					           "and the second AND third string should be the variable argument (non-localized).\n" +
					           "Content Type is: " + typeof(p_content[0]) + "\n" +
					           "Content is:\n" + ::niceStringFromObject(p_content[0]));
					_label.setText("#ERROR");
					return;
				}
			}
		}
		
		if (_delegate == null)
		{
			return;
		}
		else if (_caller == null)
		{
			::tt_panic("_caller cannot be null when _delegate is set. " +
			           "Otherwise the delegate doesn't know which instance should receive the callback.");
			return;
		}
		
		_args = [_caller];
		
		// First index always contains the text id
		for (local i= 1; i < p_content.len(); ++i)
		{
			_args.push(p_content[i]);
		}
	}
}
