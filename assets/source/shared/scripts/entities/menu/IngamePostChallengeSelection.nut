include_entity("menu/IngameMenu");

enum IngamePostChallengeSelectionType
{
	MainMenu,
	StartChallenge
};

class IngamePostChallengeSelection extends IngameMenu
</
	placeable      = Placeable_Hidden
	collisionRect  = [ 0.0, 1.0, 2.0, 2.0 ]  // center X, center Y, width, height
/>
{
	static c_colorGradingTexture  = "ingamemenu";     // Required
	static c_musicTrack           = "ingamemenu";     // Optional
	
	_selectedType = null;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onFadeEnded(p_fade, p_animation)
	{
		closeAll();
		switch (_selectedType)
		{
		case IngamePostChallengeSelectionType.MainMenu:
			::Stats.submitTelemetryEvent("quit_to_main_menu");
			::ProgressMgr.startMainMenu();
			break;
			
		case IngamePostChallengeSelectionType.StartChallenge:
			::ProgressMgr.startChallenge(::Level.getMissionID());
			break;
			
		default:
			::tt_panic("Unhandled type '" + _selectedType + "'");
			break;
		}
	}
	
	function onMainMenuSelected()
	{
		setExitSequence(IngamePostChallengeSelectionType.MainMenu);
	}
	
	function onStartChallengeSelected()
	{
		setExitSequence(IngamePostChallengeSelectionType.StartChallenge);
	}
	
	function onButtonMenuPressed()
	{
	}
	
	function onButtonCancelPressed()
	{
	}
	
	function onDie()
	{
		base.onDie();
		
		// This screen is launched during the fade; set volume back to 0.0 after closing screen
		::Audio.setTVOverallVolume(0.0, 0.0);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Inherited Methods
	
	function create()
	{
		base.create();
		
		// This screen is launched during the fade and the fade sets all volumes to 0.0
		::Audio.setTVOverallVolume(100.0, 0.0);
		
		createTitle("MENU_POST_CHALLENGE_HEADER");
		
		local medal = ::ProgressMgr.getChallengeMedal(::Level.getMissionID());
		
		if (medal < ChallengeMedal.Gold)
		{
			local nextMedalName = ::ProgressMgr.getChallengeMedalName(++medal);
			createPanelButtons(::Vector2(0.0, 0.04), 0.06, 0,
				[
					["btn_challenge",  "GUIButton", ["MENU_START_CHALLENGE_" + nextMedalName.toupper()], onStartChallengeSelected],
					["btn_mainmenu", "GUIButton", ["MENU_MAIN"],            onMainMenuSelected]
				]
			);
		}
		else
		{
			createPanelButtons(::Vector2(0.0, 0.04), 0.06, 0,
				[
					["btn_restart",  "GUIButton", ["MENU_REPLAY_CHALLENGE"], onStartChallengeSelected],
					["btn_mainmenu", "GUIButton", ["MENU_MAIN"],           onMainMenuSelected]
				]
			);
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods
	
	function setExitSequence(p_type)
	{
		_selectedType = p_type;
		createPresentation("static_end", "hud_static");
		::createPersistentFade(this, "transparent_to_opaque");
	}
}
