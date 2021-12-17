include_entity("menu/MenuScreen");

class MenuScreenControllerSettings extends MenuScreen
</
	placeable      = Placeable_Hidden
	collisionRect  = [ 0.0, 0.0, 2.0, 1.0 ]  // center X, center Y, width, height
/>
{
	_gamePadScheme = null;
	_rumbleEnabled = null;

	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn

	function onInit()
	{
		base.onInit();

		registerEntityByTag("MenuScreenControllerSettings");
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks

	function onButtonFaceUpPressed()
	{
		::showControllerApplet();
	}

	function onTimer(p_name)
	{
		base.onTimer(p_name);

		if (p_name == "rumble")
		{
			::setRumble(RumbleStrength_Medium, 0.0);
		}
	}

	function onRumbleSelected(p_key, p_value)
	{
		::GUIToggleButton.onSetOptionsData(p_key, p_value);
		if (p_value)
		{
			// Add small delay to allow for code update to enable the rumble
			startTimer("rumble", 0.1);
		}
	}

	function onSwitchControlsSelected(p_value)
	{
		::GUIToggleButton.onSetOptionsData(::OptionsKeys.GamePadScheme, p_value);
		updateControls();
	}

	function onSwitchModeChanged(p_enabled)
	{
		pushScreen("MenuScreenSwitchMode");
	}

	function onDie()
	{
		base.onDie();

		if (_rumbleEnabled != ::OptionsData.get(::OptionsKeys.Rumble))
		{
			::Stats.submitTelemetryEvent("set_rumble_enabled", ::OptionsData.get(::OptionsKeys.Rumble));
		}

		if (_gamePadScheme != ::OptionsData.get(::OptionsKeys.GamePadScheme))
		{
			::Stats.submitTelemetryEvent("set_gamepad_scheme", ::getGamepadControlSchemeString());
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Inherited Methods

	function create()
	{
		base.create();

		createTitle("MENU_CONTROLLER_SETTINGS");

		_rumbleEnabled = ::OptionsData.get(::OptionsKeys.Rumble);
		_gamePadScheme = ::OptionsData.get(::OptionsKeys.GamePadScheme);

		createFaceButtons(["back"]);

		local buttons = [];

		buttons.push(["lst_controller", "GUIListButton", getControllerItems(), onSwitchControlsSelected]);
		buttons.push(::GUIToggleButton.makeButtonForOptionsKeyEx(::OptionsKeys.Rumble, onRumbleSelected));

		createPanelButtons(::Vector2(0.0, ::MainMenu.c_buttonsYOffset), ::MainMenu.c_buttonsSpacing, 0, buttons);
		updateControls();
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods

	function getControllerItems()
	{
		local tempItems = [];

		tempItems.push(["MENU_GAMEPADSCHEME_A1", GamepadControlScheme_A1]);
		tempItems.push(["MENU_GAMEPADSCHEME_A2", GamepadControlScheme_A2]);
		tempItems.push(["MENU_GAMEPADSCHEME_B1", GamepadControlScheme_B1]);
		tempItems.push(["MENU_GAMEPADSCHEME_B2", GamepadControlScheme_B2]);

		return ::GUIListButton.getScrolledItems(tempItems, ::OptionsData.get(::OptionsKeys.GamePadScheme));
	}

	function updateControls()
	{
		removeLabelNoFade("primary");
		removeLabelNoFade("secondary");
		removeLabelNoFade("switch");
		removeLabelNoFade("jump");
		removeLabelNoFade("hack_lb");
		removeLabelNoFade("hack_rb");
		removeLabelNoFade("move");
		removeLabelNoFade("aim");
		removeLabelNoFade("secondary_lb");
		removeLabelNoFade("secondary_rb");
		removeLabelNoFade("move_label");
		removeLabelNoFade("attack_label");
		removePresentationNoFade("controller");

		local offset = -0.025;

		local tags = [::getControllerTypeString(), ::getPlatformString()];
		createPresentation("controller", "menuscreencontrollersettings", ::Vector2(0, 0), HudAlignment.Center, tags);

		// Controls shared by both schemes
		createText("move",    "MENU_CONTROLLER_MOVE",             ::Vector2(-0.28, offset - 0.33), ::TextColors.Light, HorizontalAlignment_Right);
		createText("switch",  "MENU_CONTROLLER_SWITCH_SECONDARY", ::Vector2( 0.28, offset - 0.16), ::TextColors.Light, HorizontalAlignment_Left);

		// Scheme specific controls
		switch (::OptionsData.get(::OptionsKeys.GamePadScheme))
		{
		case GamepadControlScheme_A1:
			createText("hack_lb",   "MENU_CONTROLLER_HACK",      ::Vector2(-0.28, offset - 0.07), ::TextColors.Light, HorizontalAlignment_Right);
			createText("jump",      "MENU_CONTROLLER_JUMP",      ::Vector2(-0.28, offset - 0.0),  ::TextColors.Light, HorizontalAlignment_Right);
			createText("secondary", "MENU_CONTROLLER_SECONDARY", ::Vector2(0.28,  offset - 0.0),  ::TextColors.Light, HorizontalAlignment_Left);
			createText("hack_rb",   "MENU_CONTROLLER_HACK",      ::Vector2(0.28,  offset - 0.07), ::TextColors.Light, HorizontalAlignment_Left);
			createText("aim",       "MENU_CONTROLLER_AIM_FIRE",  ::Vector2(0.28,  offset - 0.33), ::TextColors.Light, HorizontalAlignment_Left);
			break;

		case GamepadControlScheme_A2:
			createText("hack_lb",   "MENU_CONTROLLER_HACK",      ::Vector2(-0.28, offset - 0.0), ::TextColors.Light, HorizontalAlignment_Right);
			createText("jump",      "MENU_CONTROLLER_JUMP",      ::Vector2(-0.28, offset - 0.07),  ::TextColors.Light, HorizontalAlignment_Right);
			createText("secondary", "MENU_CONTROLLER_SECONDARY", ::Vector2(0.28,  offset - 0.0),  ::TextColors.Light, HorizontalAlignment_Left);
			createText("hack_rb",   "MENU_CONTROLLER_HACK",      ::Vector2(0.28,  offset - 0.07), ::TextColors.Light, HorizontalAlignment_Left);
			createText("aim",       "MENU_CONTROLLER_AIM_FIRE",  ::Vector2(0.28,  offset - 0.33), ::TextColors.Light, HorizontalAlignment_Left);
			break;

		case GamepadControlScheme_B1:
			createText("hack_lb",   "MENU_CONTROLLER_HACK",      ::Vector2(-0.28, offset - 0.07), ::TextColors.Light, HorizontalAlignment_Right);
			createText("jump",      "MENU_CONTROLLER_JUMP",      ::Vector2(-0.28, offset - 0.0),  ::TextColors.Light, HorizontalAlignment_Right);
			createText("primary",   "MENU_CONTROLLER_PRIMARY",   ::Vector2(0.28,  offset - 0.0),  ::TextColors.Light, HorizontalAlignment_Left);
			createText("secondary", "MENU_CONTROLLER_SECONDARY", ::Vector2(0.28,  offset - 0.07), ::TextColors.Light, HorizontalAlignment_Left);
			createText("aim",       "MENU_CONTROLLER_AIM",       ::Vector2(0.28,  offset - 0.33), ::TextColors.Light, HorizontalAlignment_Left);
			break;

		case GamepadControlScheme_B2:
			createText("hack_lb",   "MENU_CONTROLLER_HACK",      ::Vector2(-0.28, offset - 0.0), ::TextColors.Light, HorizontalAlignment_Right);
			createText("jump",      "MENU_CONTROLLER_JUMP",      ::Vector2(-0.28, offset - 0.07),  ::TextColors.Light, HorizontalAlignment_Right);
			createText("primary",   "MENU_CONTROLLER_PRIMARY",   ::Vector2(0.28,  offset - 0.0),  ::TextColors.Light, HorizontalAlignment_Left);
			createText("secondary", "MENU_CONTROLLER_SECONDARY", ::Vector2(0.28,  offset - 0.07), ::TextColors.Light, HorizontalAlignment_Left);
			createText("aim",       "MENU_CONTROLLER_AIM",       ::Vector2(0.28,  offset - 0.33), ::TextColors.Light, HorizontalAlignment_Left);
			break;
		}
	}
}
::registerClassForSetPlayerCallbacks(MenuScreenControllerSettings);
