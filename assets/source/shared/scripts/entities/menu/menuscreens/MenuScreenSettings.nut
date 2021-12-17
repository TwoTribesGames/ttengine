include_entity("menu/MenuScreen");

class MenuScreenSettings extends MenuScreen
</
	placeable      = Placeable_Hidden
	collisionRect  = [ 0.0, 0.0, 2.0, 1.0 ]  // center X, center Y, width, height
/>
{
	_isFullScreen       = true;
	_volumeChanged      = false;
	_ingame             = false;
	_screenShakeEnabled = null;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onMenuSelected(p_menuScreen)
	{
		pushScreen(p_menuScreen);
	}
	
	function onDebugSelected()
	{
		pushScreen("MenuScreenDebug");
	}
	
	function onCustomKeyControlsSelected()
	{
		pushScreen("MenuScreenKeyControls");
	}
	
	function onFullScreenChanged(p_enabled)
	{
		_isFullScreen = p_enabled;
		::Options.setFullScreen(_isFullScreen);
	}
	
	function onDeleteSaveDataSelected()
	{
		::ProgressMgr.reset();
	}
	
	function onDifficultySelected()
	{
		pushScreen("MenuScreenChangeDifficulty");
	}
	
	function onTelemetryChanged(p_enabled)
	{
		pushScreen("MenuScreenTelemetry");
	}
	
	function onDie()
	{
		base.onDie();
		
		if (_screenShakeEnabled != ::OptionsData.get(::OptionsKeys.ScreenShake))
		{
			::Stats.submitTelemetryEvent("set_screenshake_enabled", ::OptionsData.get(::OptionsKeys.ScreenShake));
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Inherited Methods
	
	function refocus(p_focusElement, p_unhideVisuals)
	{
		base.refocus(p_focusElement, p_unhideVisuals);
		
		if (_ingame)
		{
			if (hasElement("btn_easymode"))
			{
				_elements["btn_easymode"].reevaluate(::ProgressMgr.isEasyModeEnabled());
			}
		}
		else
		{
			if (hasElement("btn_telemetry"))
			{
				_elements["btn_telemetry"].reevaluate(::Stats.isTelemetryEnabled());
			}
		}
	}
	
	function create()
	{
		base.create();
		
		_screenShakeEnabled = ::OptionsData.get(::OptionsKeys.ScreenShake);
		
		createTitle(_ingame ? "PAUSEMENU_SETTINGS" : "MENU_SETTINGS");
		createFaceButtons(["back"]);
		
		local buttons = getPlatformSpecificButtons();
		
		// FIXME: Check if we can use this formula everywhere
		local yoffset = -0.02 + ((buttons.len() / 2) * 0.06);
		createPanelButtons(::Vector2(0.0, yoffset), ::MainMenu.c_buttonsSpacing, 0, buttons);
		
		if (_ingame == false)
		{
			// Check if delete save data should be selectable
			_elements["btn_delete_data"].setEnabled(::ProgressMgr.hasCleanProgressData() == false);
			
			// Version
			createText("version", null, ::Vector2(0, -0.38), ::TextColors.Dark, HorizontalAlignment_Center);
			_labels["version"].label.setText(::getVersionString());
			_labels["version"].presentation.setCustomUniformScale(0.75);
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods
	
	function getPlatformSpecificButtons()
	{
		local buttons = [];
		
		if (_ingame && ::ProgressMgr.getGameMode() == GameMode.Normal)
		{
			buttons.push(["btn_difficulty", "GUIButton", ["MENU_DIFFICULTY"], onDifficultySelected]);
		}
		
		buttons.push(["btn_controller", "GUIButton", ["MENU_CONTROLLER_SETTINGS", "MenuScreenControllerSettings"], onMenuSelected]);
		
		if (::isPlayingOnPC())
		{
			buttons.push(["btn_keys", "GUIButton", ["MENU_CUSTOM_KEY_CONTROLS"], onCustomKeyControlsSelected]);
		}
		
		buttons.push(["btn_volume", "GUIButton", ["MENU_VOLUME_SETTINGS", "MenuScreenVolumeSettings"], onMenuSelected]);
		
		if (::isPlayingOnPC())
		{
			buttons.push(["btn_resolution", "GUIButton", ["MENU_RESOLUTION_SETTINGS", "MenuScreenResolutionSettings"], onMenuSelected]);
		}
		
		buttons.push(::GUIToggleButton.makeButtonForOptionsKey(::OptionsKeys.ScreenShake));
		
		if (::isPlayingOnPC())
		{
			buttons.push(::GUIToggleButton.makeButton("FullScreen", onFullScreenChanged));
		}
		
		if (_ingame == false)
		{
			buttons.push(["btn_change_language", "GUIButton", ["MENU_LANGUAGE_SETTINGS", "MenuScreenLanguageSettings"], onMenuSelected]);
			// MR: Disabled since GDPR
			//buttons.push(::GUIToggleButton.makeButton("Telemetry", onTelemetryChanged, ::Stats.isTelemetryEnabled()));
			buttons.push(["btn_delete_data", "GUIConfirmationWithFadeButton", ["MENU_RESET_ALL_DATA"], onDeleteSaveDataSelected]);
		}
		
		// FIXME: Remove from final builds
		if (::isTestBuild())
		{
			buttons.push(["btn_debug", "GUIButton", ["MENU_DEBUG"], onDebugSelected]);
		}
		
		return buttons;
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Update
	
	function update(p_deltaTime)
	{
		base.update(p_deltaTime);
		
		if (_visible && ::isPlayingOnPC())
		{
			// Fullscreen can be toggeled using Alt-Enter so monitor its status
			local fullScreen = ::Options.isFullScreen();
			if (_isFullScreen != fullScreen)
			{
				_isFullScreen = fullScreen;
				_elements["btn_fullscreen"].moveRight();
			}
		}
	}
}
