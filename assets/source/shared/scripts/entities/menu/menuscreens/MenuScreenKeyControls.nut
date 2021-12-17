include_entity("menu/MenuScreen");

class MenuScreenKeyControls extends MenuScreen
</
	placeable      = Placeable_Hidden
	collisionRect  = [ 0.0, 0.0, 2.0, 1.0 ]  // center X, center Y, width, height
/>
{
	static c_actions = ["up", "down", "left", "right", "jump", "hack", 
	                    "select_weapon_1", "select_weapon_2", "select_weapon_3", "select_weapon_4"];
	_action = null;
	
	function isCancelKey(p_key)
	{
		return p_key == Key_Escape;
	}
	
	function isAllowedKey(p_key)
	{
		return p_key != Key_Escape && p_key != Key_Enter &&
		       p_key != Key_Left && p_key != Key_Right && p_key != Key_Up && p_key != Key_Down;
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onConfigureAction(p_action)
	{
		enterKeyListeningMode(p_action);
	}
	
	function onKeyboardDown(p_key)
	{
		local bindings = ::getKeyBindingsTable();
		if (isCancelKey(p_key))
		{
			local key = (_action in bindings) ? bindings[_action] : null;
			exitKeyListeningMode(key);
			return;
		}
		else if (isAllowedKey(p_key) == false)
		{
			return;
		}
		
		if (_action in bindings)
		{
			bindings[_action] = p_key;
		}
		else
		{
			bindings[_action] <- p_key;
		}
		::setKeyBindingsTable(bindings);
		exitKeyListeningMode(p_key);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Inherited Methods
	
	function create()
	{
		base.create();
		
		createTitle("MENU_CUSTOM_KEY_CONTROLS");
		createFaceButtons(["back"]);
		
		local buttons = [];
		foreach (action in c_actions)
		{
			buttons.push(["btn_" + action, "GUIButton", ["", action], onConfigureAction]);
		}
		createPanelButtons(::Vector2(0.0, ::MainMenu.c_buttonsYOffset), ::MainMenu.c_buttonsSpacing, 0, buttons);
		updateActionButtons();
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods
	
	function updateActionButtons()
	{
		local bindings = ::getKeyBindingsTable();
		foreach (action in c_actions)
		{
			local key = (action in bindings) ? ::getKeyName(bindings[action]).toupper() : "";
			updateActionButtonText(action, key);
		}
	}
	
	function updateActionButtonText(p_action, p_text)
	{
		_elements["btn_" + p_action]._label.setTextLocalizedAndFormatted("MENU_KEYBOARD_" + p_action.toupper(), [p_text]);
	}
	
	function enterKeyListeningMode(p_action)
	{
		_action = p_action;
		updateActionButtonText(p_action, "_");
		::addKeyboardBlockingListeningEntity(this, InputPriority.GUI);
		_elements["btn_" + _action]._presentation.start("selected", ["edit"], false, 0);
		_mouseEnabled = false;
	}
	
	function exitKeyListeningMode(p_key)
	{
		::removeKeyboardListeningEntity(this);
		updateActionButtonText(_action, p_key != null ? ::getKeyName(p_key).toupper() : "");
		_elements["btn_" + _action]._presentation.start("selected", [], false, 0);
		_action = null;
		_mouseEnabled = true;
	}
}
