include_entity("menu/SpiderStatusMenu");

class PauseMenu extends SpiderStatusMenu
</
	placeable      = Placeable_Hidden
	collisionRect  = [ 0.0, 0.0, 2.0, 1.0 ]  // center X, center Y, width, height
/>
{
	static c_colorGradingTexture = "ingamemenu";     // Required
	static c_musicTrack          = "ingamemenu";     // Optional
	static c_yOffset             = -0.055;

	_isSingleLifeCampaign = false;

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks

	function onDebugSelected()
	{
		closeAll(true);
		pushScreen("MenuScreenDebug");
	}

	function onExitSelected()
	{
		close();
	}

	function onMainMenuSelected(p_arg1 = null, p_arg2 = null)
	{
		::Stats.submitTelemetryEvent("quit_to_main_menu");
		::ProgressMgr.startMainMenu();
	}

	function onSettingsSelected()
	{
		pushScreen("MenuScreenSettings", { _ingame = true });
	}

	function onAchievementsSelected()
	{
		pushScreen("MenuScreenAchievements");
	}

	function onRestartSelected()
	{
		if (_isSingleLifeCampaign)
		{
			::Stats.submitTelemetryEvent("restart_campaign");
			::ProgressMgr.resetCampaign();
			::ProgressMgr.startCampaign();
		}
		else
		{
			switch (::ProgressMgr.getPlayMode())
			{
			case PlayMode.Challenge:
				::ProgressMgr.startChallenge(::Level.getMissionID());
				break;

			case PlayMode.BattleArena:
				::ProgressMgr.startBattleArena(::Level.getName());
				break;

			default:
				::Stats.submitTelemetryEvent("restart_mission");
				::ProgressMgr.restartMission();
				break;
			}
		}
	}

	function onButtonFaceUpPressed()
	{
	}

	function onButtonFaceLeftPressed()
	{
		local stats = _activator.getActualStats();
		pushScreen("ResultScreenMissionStatistics", { _gameStats = stats, _resultScreenStats = null });
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Inherited Methods

	function createMusic()
	{
		if (c_musicTrack != null)
		{
			local trackName = c_musicTrack + (_isPlayerDead ? "_gameover" : "");
			_playingMusicTrack = ::createMusicTrack(trackName);
			_playingMusicTrack.play();
		}
	}

	function create()
	{
		base.create();

		_isSingleLifeCampaign = false;
		local postfix = "MISSION";
		local gameMode = ::ProgressMgr.getGameMode();
		local playMode = ::ProgressMgr.getPlayMode();
		if (gameMode == GameMode.SingleLife && playMode == PlayMode.Campaign)
		{
			postfix = "CAMPAIGN";
			_isSingleLifeCampaign = true;
		}
		else if (gameMode == GameMode.Invalid)
		{
			switch (playMode)
			{
			case PlayMode.BattleArena: postfix = "BATTLEARENA"; break
			case PlayMode.Challenge:   postfix = "CHALLENGE"; break
			default:
				::tt_panic("Unhandled playmode '" + playMode + "'");
				break;
			}
		}

		local buttons =
		[
			["btn_continue", "GUIButton", ["PAUSEMENU_CONTINUE"], onExitSelected],
			["btn_restart",  "GUIConfirmationWithFadeButton",
				["PAUSEMENU_RESTART_" + postfix],
				onRestartSelected],
			["btn_achievements", "GUIButton", ["MENU_ACHIEVEMENTS"], onAchievementsSelected],
		];

		buttons.push(["btn_settings", "GUIButton", ["PAUSEMENU_SETTINGS"], onSettingsSelected]);

		if (playMode == PlayMode.Campaign)
		{
			buttons.push(["btn_mainmenu", "GUIConfirmationWithFadeButton", ["PAUSEMENU_MAIN_MENU", "_PROGRESS", ::Vector2(0.0, -0.38)], onMainMenuSelected]);
		}
		else
		{
			buttons.push(["btn_mainmenu", "GUIConfirmationWithFadeButton", ["PAUSEMENU_MAIN_MENU"], onMainMenuSelected]);
		}

		// FIXME: Remove from final builds
		if (::isTestBuild())
		{
			buttons.push(["btn_debug", "GUIButton", ["MENU_DEBUG"], onDebugSelected]);
		}

		local yoffset = 0.10 + ((buttons.len() / 2) * 0.06);
		createPanelButtons(::Vector2(0.0, yoffset), 0.06, 0, buttons);

		buttons = ["statistics"];
		createFaceButtons(buttons);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods

	function makePauseMenuOpener(p_class)
	{
		// Adds the onButtonMenuPressed callback to a class
		p_class.onButtonMenuPressed <- function ()
		{
			if (::g_activeFade == null)
			{
				::MenuScreen.pushScreen("PauseMenu", { _activator = this.weakref() });
			}
		};
	}
}
