include_entity("gui/GUIListButton");

class GUIToggleButton extends GUIListButton
</
	placeable      = Placeable_Hidden
	collisionRect  = [ 0.0, 0.0, 1.085, 0.05 ]
/>
{
	static c_presentationFile  = "guibutton";
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onMoveLeft()
	{
		return false; // Button is not handled
	}
	
	function onMoveRight()
	{
		return false; // Button is not handled
	}
	
	function onCursorReleasedOnEntity(p_duration)
	{
		moveRight();
		
		// call button delegate
		if (_delegate != null)
		{
			_delegate.acall(_args);
		}
		::Audio.playGlobalSoundEffect("Effects", "menu_switch");
		::setRumble(RumbleStrength_Low, 0.0);
	}
	
	function onSetOptionsData(p_key, p_value)
	{
		::OptionsData.set(p_key, p_value);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Inherited Methods
	
	function processContent(p_content)
	{
		// For GUIListButtons the content should be an array of arrays
		if (typeof(p_content) != "array" )
		{
			::tt_panic("GUIToggleButton expects content to be of type 'array', not '" + typeof(p_content) + "'");
			return;
		}
		else if (p_content.len() != 2)
		{
			::tt_panic("GUIToggleButton expects content to have 2 elements, not " + p_content.len());
			return;
		}
		_updateContent(p_content[_index]);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods
	
	function reevaluate(p_value)
	{
		if (_content[_index][1] != p_value)
		{
			moveRight();
		}
	}
	
	function makeButtonForOptionsKey(p_key)
	{
		return makeButtonForOptionsKeyEx(p_key, onSetOptionsData);
	}
	
	function makeButtonForOptionsKeyEx(p_key, p_callback, p_valueForOn = true, p_valueForOff = false,
	                                   p_postFixForOn = "ON", p_postFixForOff = "OFF")
	{
		local itemPrefix = "MENU_" + p_key.toupper() + "_";
		local value = ::OptionsData.get(p_key);
		local items = value == p_valueForOn ?
			[[itemPrefix + p_postFixForOn,  p_key, p_valueForOn],
			 [itemPrefix + p_postFixForOff, p_key, p_valueForOff]] :
			[[itemPrefix + p_postFixForOff, p_key, p_valueForOff],
			 [itemPrefix + p_postFixForOn,  p_key, p_valueForOn]];
		return ["btn_options_" + p_key.tolower(), "GUIToggleButton", items, p_callback];
	}
	
	function makeButton(p_name, p_callback, p_isOn = true)
	{
		local itemPrefix = "MENU_" + p_name.toupper() + "_";
		local items = p_isOn ?
			[[itemPrefix + "ON", true], [itemPrefix + "OFF", false]] :
			[[itemPrefix + "OFF", false], [itemPrefix + "ON", true]];
		return ["btn_" + p_name.tolower(), "GUIToggleButton", items, p_callback];
	}
}
