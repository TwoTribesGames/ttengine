include_entity("menu/IngameMenu");

enum IngamePostMissionSelectionType
{
	MainMenu,
	Replay,
	Next
}

class IngamePostMissionSelection extends IngameMenu
</
	placeable      = Placeable_Hidden
	collisionRect  = [ 0.0, 1.0, 2.0, 2.0 ]  // center X, center Y, width, height
/>
{
	static c_colorGradingTexture  = "ingamemenu";     // Required
	static c_musicTrack           = "ingamemenu";     // Optional
	
	_isPlayerDead         = false;
	_isSingleLifeCampaign = false;
	_nextMissionID        = null;
	_selectedType         = null;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		local ids = ::Level.getAllCampaignMissionIDs();
		local missionID = ::Level.getMissionID();
		for (local i = 0; i < ids.len()-1; ++i)
		{
			if (ids[i] == missionID)
			{
				_nextMissionID = ids[i+1];
				break;
			}
		}
		
		base.onInit();
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onFadeEnded(p_fade, p_animation)
	{
		closeAll();
		switch (_selectedType)
		{
		case IngamePostMissionSelectionType.MainMenu:
			::Stats.submitTelemetryEvent("quit_to_main_menu");
			::ProgressMgr.startMainMenu();
			break;
			
		case IngamePostMissionSelectionType.Replay:
			if (_isSingleLifeCampaign)
			{
				::Stats.submitTelemetryEvent("restart_campaign");
				::ProgressMgr.resetCampaign();
				::ProgressMgr.startCampaign();
			}
			else
			{
				::Stats.submitTelemetryEvent("restart_mission");
				::ProgressMgr.restartMission();
			}
			break;
			
		case IngamePostMissionSelectionType.Next:
			::Stats.submitTelemetryEvent("next_mission");
			::ProgressMgr.startMission(_nextMissionID);
			break;
			
		default:
			::tt_panic("Unhandled type '" + _selectedType + "'");
			break;
		}
	}
	
	function onMainMenuSelected()
	{
		setExitSequence(IngamePostMissionSelectionType.MainMenu);
	}
	
	function onReplaySelected()
	{
		setExitSequence(IngamePostMissionSelectionType.Replay);
	}
	
	function onNextMissionSelected()
	{
		setExitSequence(IngamePostMissionSelectionType.Next);
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
		
		createTitle("MENU_POST_MISSION_HEADER");
		
		_isSingleLifeCampaign = ::ProgressMgr.getGameMode() == GameMode.SingleLife &&
		                        ::ProgressMgr.getPlayMode() == PlayMode.Campaign;
		
		local missionCompleted = _activator instanceof ::PlayerBot;
		
		if (_isSingleLifeCampaign)
		{
			createPanelButtons(::Vector2(0.0, 0.04), 0.06, 0,
				[
					["btn_restart",  "GUIButton", ["MENU_REPLAY_CAMPAIGN"], onReplaySelected],
					["btn_mainmenu", "GUIButton", ["MENU_MAIN"],            onMainMenuSelected]
				]
			);
		}
		else if (missionCompleted && ::ProgressMgr.isMissionUnlocked(_nextMissionID))
		{
			createPanelButtons(::Vector2(0.0, 0.04), 0.06, 0,
				[
					["btn_next",     "GUIButton", ["MENU_NEXT_MISSION"],   onNextMissionSelected],
					["btn_restart",  "GUIButton", ["MENU_REPLAY_MISSION"], onReplaySelected],
					["btn_mainmenu", "GUIButton", ["MENU_MAIN"],           onMainMenuSelected]
				]
			);
		}
		else
		{
			createPanelButtons(::Vector2(0.0, 0.04), 0.06, 0,
				[
					["btn_restart",  "GUIButton", ["MENU_REPLAY_MISSION"], onReplaySelected],
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
