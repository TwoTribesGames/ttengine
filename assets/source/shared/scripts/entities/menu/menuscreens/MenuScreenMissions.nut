include_entity("menu/MenuScreen");

class MenuScreenMissions extends MenuScreen
</
	placeable      = Placeable_Hidden
	collisionRect  = [ 0.0, 0.0, 2.0, 1.0 ]  // center X, center Y, width, height
/>
{
	_easyMode        = true;
	_normalEasyMode  = true;
	_gameMode        = GameMode.Normal;
	_selectedMission = "01_asteroidfield";
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onButtonFaceUpPressed()
	{
	}
	
	function onGameModeSelected(p_gameMode)
	{
		_gameMode = p_gameMode;
		
		// switch back to last selected easy mode for GameMode.Normal
		local easyMode = _gameMode == GameMode.Normal ? _normalEasyMode : false;
		
		if (easyMode != _easyMode)
		{
			_elements["lst_easymode"].moveRight();
			_easyMode = easyMode;
		}
		
		_elements["lst_easymode"].setEnabled(_gameMode == GameMode.Normal);
	}
	
	function onMissionSelected(p_mission)
	{
		_selectedMission = p_mission;
	}
	
	function onStartMissionSelected()
	{
		unfocus(false); // disable input during fade
		::createFade(this, "transparent_to_opaque");
	}
	
	function onEasyModeChanged(p_easyMode)
	{
		if (_gameMode == GameMode.Normal)
		{
			_normalEasyMode = p_easyMode;
			::getRegistry().setPersistent("mission_normal_easy", _normalEasyMode);
		}
		_easyMode = p_easyMode;
	}
	
	function onFadeEnded(p_fade, p_animation)
	{
		closeAll();
		
		::ProgressMgr.setGameMode(_gameMode);
		::ProgressMgr.setPlayMode(PlayMode.Mission);
		::ProgressMgr.setEasyMode(_easyMode);
		::ProgressMgr.startMission(_selectedMission);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Inherited Methods
	
	function create()
	{
		base.create();
		
		createTitle("MENU_SELECT_MISSION");
		createFaceButtons(["back"]);
		
		createPanelButtons(::Vector2(0.0, 0.10), ::MainMenu.c_buttonsSpacing, 0,
		[
			["lst_missions", "GUIListButton", getUnlockedMissionItems(), onMissionSelected],
			["lst_gamemodes", "GUIListButton", getUnlockedGameModeItems(), onGameModeSelected],
			["lst_easymode", "GUIListButton",
				[
					["MENU_DIFFICULTY_EASY_HEADER", true],
					["MENU_DIFFICULTY_NORMAL_HEADER", false]
				], onEasyModeChanged],
			["btn_start", "GUIButton", ["MENU_START_MISSION"], onStartMissionSelected],
		]);
		
		// Hack to store the easy mode state for missions throughout the game
		_normalEasyMode = ::getRegistry().getPersistent("mission_normal_easy");
		if (_normalEasyMode == null)
		{
			_normalEasyMode = true;
		}
		else if (_normalEasyMode == false)
		{
			_elements["lst_easymode"].moveRight();
			_easyMode = false;
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods
	
	function getUnlockedGameModeItems()
	{
		local tempItems = [];
		
		for (local i = 0; i < GameMode.Count; ++i)
		{
			if (::ProgressMgr.isGameModeUnlocked(i))
			{
				local name = ::ProgressMgr.getGameModeName(i).toupper();
				tempItems.push(["MENU_" + name, i]);
			}
		}
		
		return tempItems;
	}
	
	function getUnlockedMissionItems()
	{
		local ids = ::Level.getAllCampaignMissionIDs();
		local items = [];
		
		foreach (id in ids)
		{
			if (::ProgressMgr.isMissionUnlocked(id))
			{
				items.push(["MISSION_NAME_" + id.toupper(), id]);
			}
		}
		return items;
	}
	
	function getAllMissionItems()
	{
		local ids = ::Level.getAllCampaignMissionIDs();
		local items = [];
		
		foreach (id in ids)
		{
			items.push(["MISSION_NAME_" + id.toupper(), id]);
		}
		return items;
	}
}
