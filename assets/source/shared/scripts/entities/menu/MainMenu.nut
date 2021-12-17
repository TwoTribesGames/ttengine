include_entity("menu/Menu");

class MainMenu extends Menu
</
	placeable      = Placeable_Hidden
	collisionRect  = [ 0.0, 0.5, 1.0, 1.0 ]  // center X, center Y, width, height
/>
{
	static c_colorGradingTexture = "ingamemenu"; // Required
	static c_musicTrack          = "menuloop";   // Optional
	static c_buttonsYOffset      = 0.22;
	static c_buttonsSpacing      = 0.06;

	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn

	function onInit()
	{
		base.onInit();
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks

	function onTimer(p_name)
	{
		base.onTimer(p_name);

		if (p_name == "checkNotification")
		{
			local id = ::getNotification();
			if (id != null)
			{
				pushScreen("MenuScreenShowNotification", { _notification = id} );
			}
		}
	}

	function onButtonFaceUpPressed()
	{
	}

	function onButtonMenuPressed()
	{
		// No exit possible
	}

	function onButtonCancelPressed()
	{
		// No exit possible
	}

	function onAchievementsSelected()
	{
		pushScreen("MenuScreenAchievements");
	}

	function onVisualBoySelected()
	{
		pushScreen("VisualBoyScreen");
	}

	function onCampaignSelected()
	{
		::ProgressMgr.setGameMode(GameMode.Normal);
		::ProgressMgr.setPlayMode(PlayMode.Campaign);
		pushScreen("MenuScreenSelectDifficulty");
	}

	function onSettingsSelected()
	{
		pushScreen("MenuScreenSettings");
	}

	function onCreditsSelected()
	{
		closeAll();
		::Level.setMissionID("*");
		::Level.load("credits");
	}

	function onQuitSelected()
	{
		::quitGame()
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Inherited Methods

	function hide()
	{
		cleanupStatic();
		base.hide();
	}

	function refocus(p_focusElement, p_unhideVisuals)
	{
		base.refocus(p_focusElement, p_unhideVisuals);

		evaluate();
	}

	function create()
	{
		base.create();

		// Reset game specific modes here
		::ProgressMgr.setGameMode(GameMode.Invalid);

		// Create presentations
		createPresentation("reflection_body", "hud_reflection", ::Vector2(0, 0), HudAlignment.Center, ["body"]);
		createPresentation("reflection_head", "hud_reflection", ::Vector2(0, 0), HudAlignment.Center, ["head"]);
		createPresentation("reflection_toothbrush", "hud_reflection", ::Vector2(0, 0), HudAlignment.Center, ["toothbrush"]);

		createTitle("MENU_MAIN");
		local buttons = [];

		local selectedIndex = 0;
		buttons.push(["btn_campaign",  "GUIButton", ["MENU_CAMPAIGN"], onCampaignSelected]);
		buttons.push(["btn_achievements", "GUIButton", ["MENU_ACHIEVEMENTS"], onAchievementsSelected]);
		buttons.push(["btn_visualboy", "GUIButton", ["MENU_VISUALBOY"], onVisualBoySelected]);

		buttons.push(["btn_settings", "GUIButton", ["MENU_SETTINGS"], onSettingsSelected]);
		buttons.push(["btn_credits",  "GUIButton", ["MENU_CREDITS"],  onCreditsSelected]);

		if (::isPlayingOnPC())
		{
			buttons.push(["btn_quit", "GUIConfirmationWithFadeButton", ["MENU_QUIT"], onQuitSelected]);
		}
		local yoffset = -0.02 + ((buttons.len() / 2) * 0.06);
		createPanelButtons(::Vector2(0.0, yoffset), c_buttonsSpacing, selectedIndex, buttons);

		createPresentation("static_start", "hud_static");
		removePresentation("static_start", 0.25);

		evaluate();

		createMusic();

		// Cannot be done during creation, check it a little later
		startTimer("checkNotification", 0.03);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods

	function cleanupStatic()
	{
		if (hasPresentation("static_start"))
		{
			::Hud.destroyElement(_presentations["static_start"]);
			delete _presentations["static_start"];
			stopTimer(c_removeTimeoutPrefix + "static_start");
		}
	}

	function evaluate()
	{
	}
}
