tt_include("includes/permhelpers");
tt_include("includes/statshelpers");

enum ChallengeMedal
{
	None    = 0,
	Bronze  = 1,
	Silver  = 2,
	Gold    = 3,
	Count   = 4,
	Invalid = 3
};

enum ChallengeLockedStatus
{
	LockedAndWaiting = 0,
	LockedByPrevious = 1,
	Unlocked         = 2,
	Invalid          = 3
};

enum GameMode
{
	Normal     = 0,
	SpeedRun   = 1,
	Brutal     = 2,
	SingleLife = 3,
	Count      = 4,
	Invalid    = 5
};

enum PlayMode
{
	Campaign     = 0,
	Mission      = 1,
	Level        = 2,
	BattleArena  = 3,
	Challenge    = 4,
	Count        = 5,
	Invalid      = 6
};

class ProgressMgr
{
	static c_registryKey  = "progressData";
	static c_version      = 38;
	static c_maxLaps      = 10;
	static c_singleLifeLapPercentages =
	[
		0.52, 0.99, 1.86, 2.01,                                                  // 01_asteroidfield
		3.02, 4.98, 6.10, 8.64, 9.97, 10.07, 11.13,                              // 02_greenhouse
		12.43, 13.37, 14.09, 15.07, 16.62, 17.95, 18.32,                         // 03_steelworks
		19.90, 21.09, 23.05, 24.61, 25.93, 27.34,                                // 04_trainride_stationary
		28.59, 29.92, 31.25, 32.58, 33.91, 35.24, 36.57, 37.91, 39.72, 41.23,    // 05_waterworks_north
		42.56, 43.89, 45.22, 46.55, 47.88, 49.21, 50.54, 51.87,                  // 06_gradius
		53.21, 54.53, 55.86,                                                     // 07_trainride
		56.19, 57.52, 58.85, 60.18, 62.21, 63.54, 64.17, 65.23, 66.12,           // 08_unstable_south
		66.85, 67.17, 67.83, 68.16, 69.49, 71.12, 71.82,                         // 09_steelworks_lavarise
		72.35, 74.48, 75.81, 77.14, 78.47, 79.82, 81.13, 82.46, 83.79,           // 10_waterworks_south
		84.12, 85.45, 87.18, 88.51, 90.44, 91.77, 92.16, 93.43, 94.12,           // 11_unstable_north
		95.26, 96.13, 96.88, 97.59,                                              // 12_steelworks_north
		98.16, 99.31, 100.00                                                     // 13_steelworks_destruction
	];

	static c_singleLifeMissionStartIndices =
	[
		0,  // 01_asteroidfield
		4,  // 02_greenhouse
		11, // 03_steelworks
		18, // 04_trainride_stationary
		24, // 05_waterworks_north
		34, // 06_gradius
		42, // 07_trainride
		45, // 08_unstable_south
		54, // 09_steelworks_lavarise
		61, // 10_waterworks_south
		70, // 11_unstable_north
		79, // 12_steelworks_north
		83  // 13_steelworks_destruction
	];

	static c_singleLifeMissionEndOffset =
	[
		3, // 01_asteroidfield
		5, // 02_greenhouse
		6, // 03_steelworks
		5, // 04_trainride_stationary
		9, // 05_waterworks_north
		6, // 06_gradius
		2, // 07_trainride
		8, // 08_unstable_south
		6, // 09_steelworks_lavarise
		7, // 10_waterworks_south
		3, // 11_unstable_north
		3, // 12_steelworks_north
		2  // 13_steelworks_destruction
	];

	// Internals
	_progressData             = null; // Local copy of progressdata to save round trip to registry for each and every get
	_tempCheckPointCount      = 0;
	_fireStartMissionCallback = false;

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Singleton Instance Management

	function getInstance()
	{
		if (::_perm_table.progressMgr == null)
		{
			::_perm_table.progressMgr = ProgressMgr();
		}
		return ::_perm_table.progressMgr;
	}

	function hasInstance()
	{
		return ::_perm_table.progressMgr != null;
	}

	function constructor()
	{
		_progressData = ::getRegistry().getPersistent(c_registryKey);
		::echo("## Running RIVE on platform: '" + ::getPlatformString() + "'");
		::echo("## Loaded progress version: " + (_progressData != null ? _progressData.version : "<none>"));
		::echo("## Current progress version: " + c_version);

		::echo("## Loaded checkpoints", ::CheckPointMgr.getAllIDs());

		// Handle stats update before loading the stats
		if (_progressData != null && _progressData.version != c_version)
		{
			::handleUpdateStats(_progressData.version);
		}

		// Load stats; do this before updating the savedata
		::Stats.load();

		// Handle update
		if (_progressData != null && _progressData.version != c_version)
		{
			_progressData = ::handleUpdate(_progressData);
			_sync();
		}

		// If progress data is still null; reset it
		if (_progressData == null)
		{
			_reset(false);
		}

		if (_progressData.language != null)
		{
			::setLanguage(_progressData.language);
		}

		::null_assert(_progressData);

		::echo("## Completed Missions", _progressData.completedMissions);

		if (::isTestBuild())
		{
			if (::isInLevelMode())
			{
				local isBattleArena = ::Level.isBattleArena(::Level.getName());
				// Dont use setPlayMode(); here as the singleton has not been assigned yet
				_progressData.gameMode = isBattleArena ? GameMode.Invalid : GameMode.Normal;
				_progressData.playMode = isBattleArena ? PlayMode.BattleArena : PlayMode.Level;
				_sync();
			}
			else if (::isInMissionMode())
			{
				local isChallenge = ::Level.isChallenge(::Level.getMissionID());
				// Dont use setPlayMode(); here as the singleton has not been assigned yet
				_progressData.gameMode = isChallenge ? GameMode.Invalid : GameMode.Normal;
				_progressData.playMode = isChallenge ? PlayMode.Challenge : PlayMode.Mission;
				_sync();
			}
			::echo("## Starting in PlayMode." + getPlayModeName(_progressData.playMode));
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Public Methods ('static')

	function restore()
	{
		local mgr = ::ProgressMgr.getInstance();
		mgr._progressData = ::getRegistry().getPersistent(c_registryKey);
		// Also sync the stats
		::Stats.restore();
	}

	function reset(p_showIntroduction = true)
	{
		local mgr = ::ProgressMgr.getInstance();
		mgr._reset(p_showIntroduction);
	}

	function hasCleanProgressData()
	{
		local mgr = ::ProgressMgr.getInstance();
		if (mgr._progressData == null)
		{
			::tt_panic("ProgressMgr not initialized");
			return false;
		}
		return mgr._progressData.isClean;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Language helpers

	function setLanguage(p_language)
	{
		local mgr = ::ProgressMgr.getInstance();
		if (mgr._progressData == null)
		{
			::tt_panic("ProgressMgr not initialized");
			return;
		}

		if (p_language == null || p_language == "default")
		{
			::setLanguageToPlatformDefault();
			mgr._progressData.language = null;
		}
		else
		{
			::setLanguage(p_language);
			mgr._progressData.language = p_language;
		}
		mgr._sync();
	}

	function getLanguage()
	{
		local mgr = ::ProgressMgr.getInstance();
		if (mgr._progressData == null)
		{
			::tt_panic("ProgressMgr not initialized");
			return;
		}
		return mgr._progressData.language == null ? "default" : mgr._progressData.language;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Mission / Campaign helpers

	function startMission(p_missionID)
	{
		local mgr = ::ProgressMgr.getInstance();
		if (mgr._progressData.playMode == PlayMode.Campaign)
		{
			::Level.prepareForExit(null);
		}
		else
		{
			::EntityChild.erasePermanentSettings(mgr._progressData.gameMode, PlayMode.Mission);
			::getRegistry().clear();
		}
		::Stats.submitTelemetryEvent("mission_start", p_missionID);
		::Level.startMissionID(p_missionID);
	}

	function startBattleArena(p_missionID)
	{
		setGameMode(GameMode.Invalid);
		setPlayMode(PlayMode.BattleArena);

		::Stats.submitTelemetryEvent("battlearena_start", p_missionID);
		::Level.setMissionID("*");
		::Level.load(p_missionID); // The mission id, is in fact a level
	}

	function startChallenge(p_missionID)
	{
		local mgr = ::ProgressMgr.getInstance();
		if (mgr._progressData == null)
		{
			::tt_panic("ProgressMgr not initialized");
			return;
		}

		setGameMode(GameMode.Invalid);
		setPlayMode(PlayMode.Challenge);

		local nextMedal = getChallengeNextMedal(p_missionID);
		local name = getChallengeMedalName(nextMedal);
		::Stats.submitTelemetryEvent("challenge_" + name + "_started", p_missionID);
		::Level.startMissionID(p_missionID);
	}

	function isAtEndOfCampaign()
	{
		local mgr = ::ProgressMgr.getInstance();
		if (mgr._progressData == null)
		{
			::tt_panic("ProgressMgr not initialized");
			return false;
		}

		return mgr._progressData.playMode == PlayMode.Campaign &&
		       ::Level.getMissionID() == "13_steelworks_destruction";
	}

	function endChallenge(p_medal)
	{
		local player = ::getFirstEntityByTag("PlayerBot");
		if (::isValidEntity(player) == false)
		{
			// There needs to be a valid player
			return;
		}

		local missionID = ::Level.getMissionID();
		local name = getChallengeMedalName(p_medal);
		local newMedal = setChallengeMedal(missionID, p_medal);
		::Stats.submitTelemetryEvent("challenge_" + name + "_completed", missionID);

		// Reset the active mission
		local mgr = ::ProgressMgr.getInstance();
		mgr._progressData.activeMission = null;
		mgr._sync();

		player.customCallback("onEndChallenge", missionID, p_medal, newMedal);
	}

	function endMission()
	{
		local player = ::getFirstEntityByTag("PlayerBot");
		local missionID = ::Level.getMissionID();
		::Stats.submitTelemetryEvent("mission_end", missionID);

		if (::isValidEntity(player))
		{
			player.customCallback("onEndMission", missionID);
		}

		local mgr = ::ProgressMgr.getInstance();
		local playMode = mgr._progressData.playMode;
		local gameMode = mgr._progressData.gameMode;
		local allIDs = ::Level.getAllCampaignMissionIDs();
		if (playMode == PlayMode.Campaign)
		{
			if (mgr._progressData.completedMissions.find(missionID) == null)
			{
				mgr._progressData.completedMissions.push(missionID);
			}
		}

		local atEndOfCampaign = isAtEndOfCampaign();

		// Check for no DLL killed by you achievement
		if (atEndOfCampaign && mgr._progressData.progress[gameMode].killedDLLs == 0)
		{
			::Stats.unlockAchievement("best_friends");
			::Stats.storeAchievements();
		}

		// Check for single-life achievements
		if (gameMode == GameMode.SingleLife)
		{
			if (atEndOfCampaign)
			{
				::Stats.unlockAchievement("are_you_nuts");
				::Stats.storeAchievements();
			}

			if (mgr._progressData.singleLifeCompletedMissions.find(missionID) == null)
			{
				mgr._progressData.singleLifeCompletedMissions.push(missionID);

				::Stats.setStat("completed_singlelife_missions", mgr._progressData.singleLifeCompletedMissions.len());
			}
		}

		mgr._progressData.activeMission = null;
		mgr._sync();

		setLastCheckPoint("end_mission");
		storeCheckPoint();
	}

	function restartMission()
	{
		::Stats.submitTelemetryEvent("mission_restart");
		if (getPlayMode() == PlayMode.Level)
		{
			::CheckPointMgr.restore("level");
			return;
		}

		local mgr = ::ProgressMgr.getInstance();
		if (::Level.getName() != "menu")
		{
			mgr._setActiveMission(::Level.getMissionID());
		}
		if (mgr._progressData.playMode == PlayMode.Campaign)
		{
			local id = mgr._getCheckPointID();

			if (::CheckPointMgr.hasID(id + "mission_start"))
			{
				local player = ::getFirstEntityByTag("PlayerBot");
				if (player != null)
				{
					player.customCallback("onMissionRestart");
				}
				::CheckPointMgr.clearID(id);
				::CheckPointMgr.restore(id + "mission_start");
				return;
			}
			else
			{
				::tt_panic("restartMission called for campaign, without having checkpoint '" + id + "mission_start'");
			}
		}
		else if (mgr._progressData.playMode == PlayMode.BattleArena ||
		         mgr._progressData.playMode == PlayMode.Challenge)
		{
			local player = ::getFirstEntityByTag("PlayerBot");
			if (player != null)
			{
				player.customCallback("onMissionRestart");
			}
			::CheckPointMgr.restore(mgr._getCheckPointID());
			return;
		}
		startMission(::Level.getMissionID());
	}

	function getActiveMissionData()
	{
		local mgr = ::ProgressMgr.getInstance();
		return mgr._progressData.activeMission;
	}

	function hasActiveMission()
	{
		local mgr = ::ProgressMgr.getInstance();
		return mgr._progressData.activeMission != null;
	}

	function startCampaign()
	{
		setPlayMode(PlayMode.Campaign);
		::Stats.submitTelemetryEvent("campaign_start");
		::EntityChild.erasePermanentSettings(getGameMode(), PlayMode.Campaign);
		::getRegistry().clear();
		// Don't use startMission() as that stores temp. values that shouldn't be around anymore
		local missionID = "01_asteroidfield";
		::Stats.submitTelemetryEvent("mission_start", missionID);
		::Level.startMissionID(missionID);
	}

	function canContinueCampaign(p_gameMode)
	{
		// Check for save data
		if (::CheckPointMgr.hasID("campaign_" + p_gameMode) || ::CheckPointMgr.hasID("campaign_" + p_gameMode + "mission_start"))
		{
			return true;
		}

		// Save data not present; check if game was updated
		local mgr = ::ProgressMgr.getInstance();
		return mgr._progressData.canContinueCampaignAfterUpdate[p_gameMode];
	}

	function continueCampaign()
	{
		setPlayMode(PlayMode.Campaign);

		setLastCheckPoint("continue_campaign");
		local mgr = ::ProgressMgr.getInstance();
		local id = mgr._getCheckPointID();
		if (::CheckPointMgr.hasID(id) || ::CheckPointMgr.hasID(id + "mission_start"))
		{
			::Stats.submitTelemetryEvent("campaign_continue");
			restoreCheckPoint();
			return;
		}

		// else no valid checkpoint; this can only happen after updating, so restore from update
		local missionID = mgr._progressData.progress[mgr._progressData.gameMode].lastMissionID;
		if (missionID == null)
		{
			::tt_panic("lastMissionID is null. No point to return to. Unfortunately campaign has to reset.");
			::Stats.submitTelemetryEvent("restart_campaign_after_update_error");
			::ProgressMgr.resetCampaign();
			::ProgressMgr.startCampaign();
			return;
		}

		::Stats.submitTelemetryEvent("campaign_continue_after_update", missionID);
		::Level.startMissionID(missionID);
	}

	function resetCampaign()
	{
		::Stats.submitTelemetryEvent("campaign_reset");
		local gameMode = getGameMode();
		local campaignID = "campaign_" + gameMode;

		// Delete all checkpoints for current campaign
		local ids = ::CheckPointMgr.getAllIDs();
		foreach (id in ids)
		{
			if (::stringStartsWith(id, campaignID))
			{
				::CheckPointMgr.clearID(id);
			}
		}

		// Reset current scores
		local mgr = ::ProgressMgr.getInstance();
		if (mgr._progressData == null)
		{
			::tt_panic("ProgressMgr not initialized");
			return;
		}

		// Reset gamemode specifics
		{
			mgr._progressData.lastCheckPoints[gameMode] = { id = null, deathCount = 0 };
			mgr._progressData.progress[gameMode].killedDLLs = 0;
			mgr._progressData.progress[gameMode].lastMissionID = null;
			mgr._progressData.canContinueCampaignAfterUpdate[gameMode] = false;
		}

		mgr._progressData.progress[gameMode].playedEasyMode = false;
		mgr._progressData.progress[gameMode].easyMode = false;

		if (gameMode != GameMode.SpeedRun)
		{
			foreach (missionData in mgr._progressData.missions)
			{
				missionData[gameMode].campaignScore = 0;
			}
		}
		else
		{
			foreach (missionData in mgr._progressData.missions)
			{
				missionData[gameMode].campaignLaps = [];
			}
		}

		mgr._sync();
	}

	function startMainMenu()
	{
		// Should be in fade already at this point; no need to fade again
		saveGameStateToDisk(true); // ensure to wait for thread to finish saving!
		setPlayMode(PlayMode.Invalid);
		::Level.setMissionID("*");
		::Level.load("menu");
	}

	function incrementDLLKillCount()
	{
		local mgr = ::ProgressMgr.getInstance();
		if (mgr._progressData == null)
		{
			::tt_panic("ProgressMgr not initialized");
			return "Invalid";
		}

		if (mgr._progressData.playMode != PlayMode.Campaign)
		{
			return;
		}

		++mgr._progressData.progress[mgr._progressData.gameMode].killedDLLs;
		mgr._sync();
	}

	function getLastCheckPointDeathCount()
	{
		local mgr = ::ProgressMgr.getInstance();
		if (mgr._progressData == null)
		{
			::tt_panic("ProgressMgr not initialized");
			return 0;
		}

		if (mgr._progressData.gameMode < GameMode.Count)
		{
			return mgr._progressData.lastCheckPoints[mgr._progressData.gameMode].deathCount;
		}
		return 0;
	}

	function incrementLastCheckPointDeathCount()
	{
		local mgr = ::ProgressMgr.getInstance();
		local id = mgr._getCheckPointID();
		if (::CheckPointMgr.hasID(id) == false &&
		    ::CheckPointMgr.hasID(id + "mission_start") == false)
		{
			::tt_panic("No checkpoint id '" + id + "' to increment");
			return;
		}
		++mgr._progressData.lastCheckPoints[mgr._progressData.gameMode].deathCount;
		mgr._sync();
	}

	function setLastCheckPoint(p_id)
	{
		local mgr = ::ProgressMgr.getInstance();
		if (mgr._progressData == null)
		{
			::tt_panic("ProgressMgr not initialized");
			return "Invalid";
		}

		if (mgr._progressData.gameMode < GameMode.Count)
		{
			local table = mgr._progressData.lastCheckPoints[mgr._progressData.gameMode];
			table.id         = p_id;
			table.deathCount = 0;
			if (mgr._progressData.gameMode == GameMode.Normal && mgr._progressData.isEasyModeOffered != null)
			{
				mgr._progressData.isEasyModeOffered = false;
			}
			mgr._sync();
		}
	}

	function hasLastCheckPoint()
	{
		local mgr = ::ProgressMgr.getInstance();
		if (mgr._progressData == null)
		{
			return false;
		}

		if (mgr._progressData.gameMode < GameMode.Count)
		{
			return mgr._progressData.lastCheckPoints[mgr._progressData.gameMode].id != null;
		}
		return false;
	}

	function getLastCheckPoint()
	{
		local mgr = ::ProgressMgr.getInstance();
		if (mgr._progressData == null)
		{
			::tt_panic("ProgressMgr not initialized");
			return "Invalid";
		}

		return mgr._progressData.lastCheckPoints[mgr._progressData.gameMode];
	}

	function getLastMissionID()
	{
		local mgr = ::ProgressMgr.getInstance();
		if (mgr._progressData == null)
		{
			::tt_panic("ProgressMgr not initialized");
			return null;
		}
		return mgr._progressData.progress[mgr._progressData.gameMode].lastMissionID;
	}

	function signalPlayerKilled()
	{
		local playMode = getPlayMode();
		if (playMode == PlayMode.BattleArena || playMode == PlayMode.Challenge)
		{
			restoreCheckPoint();
			return;
		}

		local mode = getGameMode();
		switch (mode)
		{
		case GameMode.SingleLife:
			if (playMode == PlayMode.Campaign)
			{
				// Delete all checkpoints for current campaign
				local campaignID = "campaign_" + mode;
				local ids = ::CheckPointMgr.getAllIDs();
				foreach (id in ids)
				{
					if (::stringStartsWith(id, campaignID))
					{
						::CheckPointMgr.clearID(id);
					}
				}
			}
			break;

		default:
			incrementLastCheckPointDeathCount();
			restoreCheckPoint();
			break;
		}
	}

	function getGameMode()
	{
		local mgr = ::ProgressMgr.getInstance();
		if (mgr._progressData == null)
		{
			::tt_panic("ProgressMgr not initialized");
			return "Invalid";
		}
		return mgr._progressData.gameMode;
	}

	function getGameModeName(p_gameMode)
	{
		switch (p_gameMode)
		{
		case GameMode.Normal:     return "Normal";
		case GameMode.SpeedRun:   return "SpeedRun";
		case GameMode.Brutal:     return "Brutal";
		case GameMode.SingleLife: return "SingleLife";
		default:
			::tt_panic("Unhandled gamemode '" + p_gameMode + "'");
			break;
		}
		return "Invalid"
	}

	function setGameMode(p_gameMode)
	{
		local mgr = ::ProgressMgr.getInstance();
		if (mgr._progressData == null)
		{
			::tt_panic("ProgressMgr not initialized");
			return;
		}
		// Store in registry
		mgr._progressData.gameMode = p_gameMode;
		mgr._sync();
	}

	function isGameModeUnlocked(p_gameMode)
	{
		local mgr = ::ProgressMgr.getInstance();
		if (mgr._progressData == null)
		{
			::tt_panic("ProgressMgr not initialized");
			return false;
		}
		return mgr._progressData.progress[p_gameMode].isUnlocked;
	}

	function unlockGameMode(p_gameMode)
	{
		local mgr = ::ProgressMgr.getInstance();
		if (mgr._progressData == null)
		{
			::tt_panic("ProgressMgr not initialized");
			return false;
		}
		mgr._progressData.progress[p_gameMode].isUnlocked = true;
		mgr._sync();
	}

	function getPlayMode()
	{
		local mgr = ::ProgressMgr.getInstance();
		if (mgr._progressData == null)
		{
			::tt_panic("ProgressMgr not initialized");
			return PlayMode.Campaign;
		}
		return mgr._progressData.playMode;
	}

	function getPlayModeName(p_playMode)
	{
		switch (p_playMode)
		{
		case PlayMode.Campaign:     return "Campaign";
		case PlayMode.Mission:      return "Mission";
		case PlayMode.Level:        return "Level";
		case PlayMode.BattleArena:  return "BattleArena";
		case PlayMode.Challenge:    return "Challenge";
		case PlayMode.Invalid:      return "Invalid";
		default:
			::tt_panic("Unhandled playmode '" + p_playMode + "'");
			break;
		}
		return "Unhandled";
	}

	function setPlayMode(p_playMode)
	{
		local mgr = ::ProgressMgr.getInstance();
		if (mgr._progressData == null)
		{
			::tt_panic("ProgressMgr not initialized");
			return;
		}
		// Store in registry
		mgr._progressData.playMode = p_playMode;
		mgr._sync();
	}

	function areChallengesUnlocked()
	{
		local mgr = ::ProgressMgr.getInstance();
		if (mgr._progressData == null)
		{
			::tt_panic("ProgressMgr not initialized");
			return false;
		}
		return mgr._progressData.areChallengesUnlocked;
	}

	function unlockChallenges()
	{
		local mgr = ::ProgressMgr.getInstance();
		if (mgr._progressData == null)
		{
			::tt_panic("ProgressMgr not initialized");
			return;
		}
		mgr._progressData.areChallengesUnlocked = true;
		mgr._sync();
	}

	function getChallengeMedalName(p_medal)
	{
		switch (p_medal)
		{
		case ChallengeMedal.None:   return "none";
		case ChallengeMedal.Bronze: return "bronze";
		case ChallengeMedal.Silver: return "silver";
		case ChallengeMedal.Gold:   return "gold";
		default:
			::tt_panic("Unhandled challenge medal: '" + p_medal + "'");
			break;
		}
		return "none";
	}

	function getChallengeMedalFromName(p_name)
	{
		for (local i = 0; i < ChallengeMedal.Count; ++i)
		{
			if (getChallengeMedalName(i) == p_name)
			{
				return i;
			}
		}

		::tt_panic("Unhandled challenge medal name: '" + p_name + "'");
		return ChallengeMedal.Invalid;
	}

	function isMissionUnlocked(p_missionID)
	{
		if (p_missionID == null)
		{
			return false;
		}

		local mgr = ::ProgressMgr.getInstance();
		if (mgr._progressData == null)
		{
			return false;
		}
		return mgr._progressData.completedMissions.find(p_missionID) != null;
	}

	function getNumberOfUnlockedMissions()
	{
		local mgr = ::ProgressMgr.getInstance();
		if (mgr._progressData == null)
		{
			return false;
		}
		return mgr._progressData.completedMissions.len();
	}

	function getChallengeMedal(p_missionID)
	{
		if (p_missionID == null)
		{
			return ChallengeMedal.None;
		}

		local mgr = ::ProgressMgr.getInstance();
		if (mgr._progressData == null)
		{
			return false;
		}

		if (p_missionID in mgr._progressData.challenges)
		{
			return mgr._progressData.challenges[p_missionID].medal;
		}
		return ChallengeMedal.None;
	}

	function getChallengeNextMedal(p_missionID)
	{
		local medal = getChallengeMedal(p_missionID);
		return (medal < ChallengeMedal.Gold) ? medal + 1 : medal;
	}

	function setChallengeMedal(p_missionID, p_medal)
	{
		if (p_missionID == null || p_medal < ChallengeMedal.Bronze || p_medal > ChallengeMedal.Gold)
		{
			return false;
		}

		local mgr = ::ProgressMgr.getInstance();
		if (mgr._progressData == null)
		{
			return false;
		}

		local table = mgr._progressData.challenges[p_missionID];
		if (table.completedDate == null)
		{
			table.completedDate = ::getDateString();
		}

		if (table.medal < p_medal)
		{
			mgr._progressData.challenges[p_missionID].medal = p_medal;
			mgr._sync();

			// Reevaluate this type of medal
			local totalMedals = 0;
			foreach (challenge in mgr._progressData.challenges)
			{
				if (challenge.medal >= p_medal)
				{
					++totalMedals;
				}
			}

			local stat = "challenge_" + getChallengeMedalName(p_medal);
			::Stats.setStat(stat, totalMedals);

			// New medal
			return true;
		}

		mgr._sync();
		return false;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Unlocked weapons helpers

	function unlockPurchasedItem(p_item)
	{
		local mgr = ::ProgressMgr.getInstance();
		if (mgr._progressData == null)
		{
			return;
		}

		if (mgr._progressData.purchasedItems.find(p_item) == null)
		{
			mgr._progressData.purchasedItems.push(p_item);
			mgr._sync();
		}
	}

	function setPurchasedItems(p_items)
	{
		local mgr = ::ProgressMgr.getInstance();
		if (mgr._progressData == null)
		{
			return;
		}

		mgr._progressData.purchasedItems = p_items;
		mgr._sync();
	}

	function getPurchasedItems()
	{
		local mgr = ::ProgressMgr.getInstance();
		if (mgr._progressData == null)
		{
			return [];
		}

		return mgr._progressData.purchasedItems;
	}

	function resetPurchasedItems()
	{
		local mgr = ::ProgressMgr.getInstance();
		if (mgr._progressData == null)
		{
			return;
		}

		mgr._progressData.purchasedItems.clear();
		mgr._sync();
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// SingleLife helpers

	function getSingleLifeCampaignPercentageFromIndex(p_index)
	{
		// Died before first lap
		if (p_index < 0)
		{
			return 0.0;
		}

		if (p_index >= c_singleLifeLapPercentages.len())
		{
			::tt_panic("p_index out of bounds: " + p_index);
			return 0.0;
		}
		return c_singleLifeLapPercentages[p_index] / 100.0;
	}

	function getSingleLifeMissionPercentageFromIndex(p_missionID, p_index)
	{
		local requestedMissionNumber = (p_missionID.slice(0, 2).tointeger())-1;
		if (p_index >= c_singleLifeLapPercentages.len())
		{
			::tt_panic("p_index out of bounds: " + p_index);
			return 0.0;
		}

		// Find matching mission index
		local missionNumber = 0;
		local startIndex = 0;
		for (local i = c_singleLifeMissionStartIndices.len()-1; i >= 0; --i)
		{
			if (p_index >= c_singleLifeMissionStartIndices[i])
			{
				startIndex = c_singleLifeMissionStartIndices[i];
				missionNumber = i;
				break;
			}
		}

		if (missionNumber != requestedMissionNumber)
		{
			if (missionNumber > requestedMissionNumber)
			{
				::tt_panic("missionNumber > requestedMissionNumber, this shouldn't happen");
			}
			return 0.0;
		}

		// Found index, now normalize percentage based on mission end index of this mission
		local endIndex          = startIndex + c_singleLifeMissionEndOffset[missionNumber];

		if (p_index < startIndex || p_index > endIndex)
		{
			// Died before first lap or in intermission (shouldn't happen)
			return 0.0;
		}

		local startPercentage   = startIndex > 0 ? c_singleLifeLapPercentages[startIndex-1] : 0.0;
		local endPercentage     = c_singleLifeLapPercentages[endIndex];
		local currentPercentage = c_singleLifeLapPercentages[p_index];

		return (currentPercentage - startPercentage) / (endPercentage - startPercentage);
	}

	function getSingleLifeMissionStartIndex(p_missionID)
	{
		local missionNumber = (p_missionID.slice(0, 2).tointeger())-1;
		return c_singleLifeMissionStartIndices[missionNumber]-1;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Introduction

	function isIntroductionDisplayed()
	{
		local mgr = ::ProgressMgr.getInstance();
		if (mgr._progressData == null)
		{
			return false;
		}
		return mgr._progressData.isIntroductionDisplayed;
	}

	function setIsIntroductionDisplayed(p_isDisplayed)
	{
		local mgr = ::ProgressMgr.getInstance();
		if (mgr._progressData == null)
		{
			::tt_panic("ProgressMgr not initialized");
			return;
		}
		mgr._progressData.isIntroductionDisplayed = p_isDisplayed;
		mgr._sync();
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Easy Mode

	function setIsEasyModeOffered(p_isOffered)
	{
		local mgr = ::ProgressMgr.getInstance();
		if (mgr._progressData == null)
		{
			::tt_panic("ProgressMgr not initialized");
			return;
		}
		// Use null to indicate no offer should be made anymore
		mgr._progressData.isEasyModeOffered = p_isOffered;
		mgr._sync();
	}

	function shouldOfferEasyMode()
	{
		local mgr = ::ProgressMgr.getInstance();
		if (mgr._progressData == null)
		{
			return false;
		}
		return mgr._progressData.gameMode == GameMode.Normal &&
		       mgr._progressData.playMode == PlayMode.Campaign &&
		       mgr._progressData.isEasyModeOffered == false &&
		       mgr._progressData.progress[GameMode.Normal].easyMode == false && // Not currently in easymode already
		       mgr._progressData.lastCheckPoints[GameMode.Normal].deathCount >= 14; // +1 is the actual deathcount
	}

	function setEasyMode(p_enabled)
	{
		local mgr = ::ProgressMgr.getInstance();
		if (mgr._progressData == null)
		{
			::tt_panic("ProgressMgr not initialized");
			return;
		}

		if (mgr._progressData.gameMode >= GameMode.Count)
		{
			if (p_enabled)
			{
				::tt_panic("Cannot enable easyMode in GameMode '" + mgr._progressData.gameMode + "'");
				return;
			}
		}
		else
		{
			// Set the global flag
			switch (mgr._progressData.playMode)
			{
			case PlayMode.Campaign:
				mgr._progressData.progress[mgr._progressData.gameMode].easyMode = p_enabled;
				break;
			case PlayMode.Mission:
				mgr._progressData.missionEasyMode = p_enabled;
				break;
			default:
				::tt_warning("Unhandled playmode '" + mgr._progressData.playMode + "'");
				break;
			}
		}

		// Signal to player (when ingame)
		local player = ::getFirstEntityByTag("PlayerBot");
		if (player != null)
		{
			player.customCallback("onEasyModeEnabled", p_enabled);

			// And set the flag for the current mission (once set, it cannot be reverted, only through reset campaign)
			if (p_enabled)
			{
				if (mgr._progressData.playMode == PlayMode.Campaign)
				{
					mgr._progressData.progress[mgr._progressData.gameMode].playedEasyMode = true;
				}
				mgr._progressData.activeMissionPlayedEasyMode = true;
			}
		}
		mgr._sync();
	}

	function isEasyModeEnabled()
	{
		local mgr = ::ProgressMgr.getInstance();
		if (mgr._progressData == null)
		{
			::tt_panic("ProgressMgr not initialized");
			return false;
		}

		if (mgr._progressData.gameMode >= GameMode.Count)
		{
			return false;
		}

		switch (mgr._progressData.playMode)
		{
		case PlayMode.Campaign:
			return mgr._progressData.progress[mgr._progressData.gameMode].easyMode;
		case PlayMode.Mission:
			return mgr._progressData.missionEasyMode;
		default:
			break;
		}

		return false;
	}

	function hasPlayedEasyModeInCampaign()
	{
		local mgr = ::ProgressMgr.getInstance();
		if (mgr._progressData == null)
		{
			::tt_panic("ProgressMgr not initialized");
			return false;
		}

		if (mgr._progressData.gameMode >= GameMode.Count)
		{
			::tt_panic("hasPlayedEasyModeInCampaign used invalid gamemode '" + mgr._progressData.gameMode + "'");
			return false;
		}

		if (mgr._progressData.playMode != PlayMode.Campaign)
		{
			::tt_panic("hasPlayedEasyModeInCampaign only works for PlayMode.Campaign");
			return false;
		}

		return mgr._progressData.progress[mgr._progressData.gameMode].playedEasyMode;
	}

	function hasPlayedEasyModeInActiveMission()
	{
		local mgr = ::ProgressMgr.getInstance();
		if (mgr._progressData == null)
		{
			::tt_panic("ProgressMgr not initialized");
			return false;
		}

		if (mgr._progressData.gameMode >= GameMode.Count)
		{
			::tt_panic("hasPlayedEasyModeInActiveMission used invalid gamemode '" + mgr._progressData.gameMode + "'");
			return false;
		}

		return mgr._progressData.activeMissionPlayedEasyMode;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Laptimes Methods

	function getFastestCampaignTime()
	{
		local mgr = ::ProgressMgr.getInstance();
		if (mgr._progressData == null)
		{
			::tt_panic("ProgressMgr not initialized");
			return null;
		}
		return mgr._progressData.progress[GameMode.SpeedRun].fastestTime;
	}

	function setCampaignTime(p_time)
	{
		local mgr = ::ProgressMgr.getInstance();
		if (mgr._progressData == null)
		{
			::tt_panic("ProgressMgr not initialized");
			return false;
		}
		local gameMode = getGameMode();
		if (gameMode != GameMode.SpeedRun)
		{
			::tt_panic("setCampaignTime should only be used in SpeedRun mode");
			return false;
		}

		local table = mgr._progressData.progress[GameMode.SpeedRun];
		if (table.fastestTime == null || p_time < table.fastestTime)
		{
			table.fastestTime = p_time;
			::echo("[SpeedRun] New fastest campaign time: " + ::formatTime(p_time));
			mgr._sync();
			return true;
		}

		::echo("[SpeedRun] No new fastest campaign time: " + ::formatTime(p_time));
		mgr._sync();
		return false;
	}

	function getMissionTimeForCampaign(p_missionID)
	{
		local mgr = ::ProgressMgr.getInstance();
		if (mgr._progressData == null)
		{
			::tt_panic("ProgressMgr not initialized");
			return null;
		}
		return mgr._progressData.missions[p_missionID][GameMode.SpeedRun].campaignTime;
	}

	function getFastestMissionTime(p_missionID)
	{
		local mgr = ::ProgressMgr.getInstance();
		if (mgr._progressData == null)
		{
			::tt_panic("ProgressMgr not initialized");
			return null;
		}
		return mgr._progressData.missions[p_missionID][GameMode.SpeedRun].fastestTime;
	}

	function getMissionLapTimeForCampaign(p_missionID, p_lapIndex)
	{
		if (p_lapIndex < 0 || p_lapIndex >= c_maxLaps)
		{
			::tt_panic("Lap index '" + p_lapIndex + "' should be in range [0, " + c_maxLaps + ")");
			return null;
		}

		local mgr = ::ProgressMgr.getInstance();
		if (mgr._progressData == null)
		{
			::tt_panic("ProgressMgr not initialized");
			return null;
		}
		local table = mgr._progressData.missions[p_missionID][GameMode.SpeedRun].campaignLaps;
		if (p_lapIndex < table.len())
		{
			return table[p_lapIndex];
		}
		return null;
	}

	function getFastestLapTime(p_missionID, p_lapIndex)
	{
		if (p_lapIndex < 0 || p_lapIndex >= c_maxLaps)
		{
			::tt_panic("Lap index '" + p_lapIndex + "' should be in range [0, " + c_maxLaps + ")");
			return null;
		}

		local mgr = ::ProgressMgr.getInstance();
		if (mgr._progressData == null)
		{
			::tt_panic("ProgressMgr not initialized");
			return null;
		}
		local table = mgr._progressData.missions[p_missionID][GameMode.SpeedRun].fastestLaps;
		if (p_lapIndex < table.len())
		{
			return table[p_lapIndex];
		}
		return null;
	}

	function getMissionLapTimesForCampaign(p_missionID)
	{
		local mgr = ::ProgressMgr.getInstance();
		if (mgr._progressData == null)
		{
			::tt_panic("ProgressMgr not initialized");
			return null;
		}
		return mgr._progressData.missions[p_missionID][GameMode.SpeedRun].campaignLaps;
	}

	function getFastestLapTimes(p_missionID)
	{
		local mgr = ::ProgressMgr.getInstance();
		if (mgr._progressData == null)
		{
			::tt_panic("ProgressMgr not initialized");
			return null;
		}
		return mgr._progressData.missions[p_missionID][GameMode.SpeedRun].fastestLaps;
	}

	// Returns true if it's the fastest time
	function setLapTimes(p_missionID, p_lapTimes)
	{
		if (p_lapTimes.len() == 0)
		{
			::tt_panic("Cannot submit empty laptimes");
			return false;
		}

		if (p_lapTimes.len() > c_maxLaps)
		{
			::tt_panic("Lap times size '" + p_lapTimes.len() + "' should be in range [0, " + c_maxLaps + ")");
			return false;
		}

		local mgr = ::ProgressMgr.getInstance();
		if (mgr._progressData == null)
		{
			::tt_panic("ProgressMgr not initialized");
			return false;
		}

		local currentTime = p_lapTimes[p_lapTimes.len()-1];

		// First check if we need to set the campaign times as well
		local table = mgr._progressData.missions[p_missionID][GameMode.SpeedRun];
		if (mgr._progressData.playMode == PlayMode.Campaign)
		{
			table.campaignLaps = clone p_lapTimes;
			table.campaignTime = currentTime;
		}

		// Now check for fastest time
		if (table.fastestTime == null || currentTime < table.fastestTime)
		{
			table.fastestLaps = clone p_lapTimes;
			table.fastestTime = currentTime;
			::echo("[SpeedRun:" + p_missionID + "] New fastest mission time: " + table.fastestTime);
			mgr._sync();
			return true;
		}
		else
		{
			::echo("[SpeedRun:" + p_missionID + "] Set campaign mission time: " + currentTime);
		}

		mgr._sync();
		return false;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// (High)Score Methods

	function getMissionScoreForCampaign(p_missionID)
	{
		local gameMode = getGameMode();
		if (gameMode == GameMode.SpeedRun)
		{
			::tt_panic("getScore should cannot be used in SpeedRun mode");
			return null;
		}

		local mgr = ::ProgressMgr.getInstance();
		if (mgr._progressData == null)
		{
			::tt_panic("ProgressMgr not initialized");
			return null;
		}
		return mgr._progressData.missions[p_missionID][gameMode].campaignScore;
	}

	function getMissionHighScore(p_missionID)
	{
		local gameMode = getGameMode();
		if (gameMode == GameMode.SpeedRun)
		{
			::tt_panic("getMissionHighScore should cannot be used in SpeedRun mode");
			return null;
		}

		local mgr = ::ProgressMgr.getInstance();
		if (mgr._progressData == null)
		{
			::tt_panic("ProgressMgr not initialized");
			return null;
		}
		return mgr._progressData.missions[p_missionID][gameMode].highScore;
	}

	function setMissionScore(p_missionID, p_score)
	{
		local gameMode = getGameMode();
		if (gameMode == GameMode.SpeedRun)
		{
			::tt_panic("setScore cannot be used in SpeedRun mode");
			return false;
		}

		local mgr = ::ProgressMgr.getInstance();
		if (mgr._progressData == null)
		{
			::tt_panic("ProgressMgr not initialized");
			return false;
		}

		p_score = p_score.tointeger();
		// First check if we need to set the campaign score as well
		local table = mgr._progressData.missions[p_missionID][gameMode];
		if (mgr._progressData.playMode == PlayMode.Campaign)
		{
			::echo("[" + gameMode + ":" + p_missionID + "] Set campaign score for this mission: " + p_score);
			table.campaignScore = p_score;
		}

		if (table.highScore == null || p_score > table.highScore)
		{
			table.highScore = p_score;
			::echo("[" + gameMode + ":" + p_missionID + "] New mission highscore: " + p_score);
			mgr._sync();
			return true;
		}

		mgr._sync();
		return false;
	}

	function getCampaignHighScore()
	{
		local gameMode = getGameMode();
		if (gameMode == GameMode.SpeedRun)
		{
			::tt_panic("getMissionHighScore should cannot be used in SpeedRun mode");
			return null;
		}

		local mgr = ::ProgressMgr.getInstance();
		if (mgr._progressData == null)
		{
			::tt_panic("ProgressMgr not initialized");
			return null;
		}
		return mgr._progressData.progress[gameMode].highScore;
	}

	function setCampaignScore(p_score)
	{
		local gameMode = getGameMode();
		if (gameMode == GameMode.SpeedRun)
		{
			::tt_panic("setScore cannot be used in SpeedRun mode");
			return false;
		}

		local mgr = ::ProgressMgr.getInstance();
		if (mgr._progressData == null)
		{
			::tt_panic("ProgressMgr not initialized");
			return false;
		}

		p_score = p_score.tointeger();
		local table = mgr._progressData.progress[gameMode];
		if (table.highScore == null || p_score > table.highScore)
		{
			table.highScore = p_score;
			::echo("[" + gameMode + "] New campaign highscore: " + p_score);
			mgr._sync();
			return true;
		}
		::echo("[" + gameMode + "] No new campaign highscore: " + p_score);
		mgr._sync();
		return false;
	}

	function getBattleArenaHighScore(p_missionID)
	{
		local playMode = getPlayMode();
		if (playMode != PlayMode.BattleArena)
		{
			::tt_panic("getBattleArenaHighScore should only be used in battle arena missions");
			return null;
		}

		local mgr = ::ProgressMgr.getInstance();
		if (mgr._progressData == null)
		{
			::tt_panic("ProgressMgr not initialized");
			return null;
		}

		return mgr._progressData.battleArenas[p_missionID].highScore;
	}

	function setBattleArenaScore(p_missionID, p_score)
	{
		local playMode = getPlayMode();
		if (playMode != PlayMode.BattleArena)
		{
			::tt_panic("setBattleArenaScore should only be used in battle arena missions");
			return null;
		}

		local mgr = ::ProgressMgr.getInstance();
		if (mgr._progressData == null)
		{
			::tt_panic("ProgressMgr not initialized");
			return false;
		}

		p_score = p_score.tointeger();
		// First check if we need to set the campaign score as well
		local table = mgr._progressData.battleArenas[p_missionID];
		if (table.highScore == null || p_score > table.highScore)
		{
			table.highScore = p_score;
			::echo("[BattleArena:" + p_missionID + "] New highscore: " + p_score);
			mgr._sync();
			return true;
		}

		mgr._sync();
		return false;
	}

	function getChallengeLockedStatus(p_missionID)
	{
		local mgr = ::ProgressMgr.getInstance();
		if (mgr._progressData == null)
		{
			::tt_panic("ProgressMgr not initialized");
			return ChallengeLockedStatus.Invalid;
		}

		local missionIndex = p_missionID.slice(3, 5).tointeger();

		// First challenge or if there was an attempt; challenge is always unlocked
		if (missionIndex == 1)
		{
			return ChallengeLockedStatus.Unlocked;
		}

		// Find previous mission
		local prevMissionIndex = missionIndex - 1;
		foreach (key, value in mgr._progressData.challenges)
		{
			local idx = key.slice(3, 5).tointeger();
			if (prevMissionIndex == idx)
			{
				if (value.completedDate == null)
				{
					// Previous challenge not finished yet
					return ChallengeLockedStatus.LockedByPrevious;
				}

				// Next challenge is unlocked if previous one was completed at minimum bronze
				// and is at least a day old
				local now = ::getDateString();
				return value.completedDate < now ?
					ChallengeLockedStatus.Unlocked : ChallengeLockedStatus.LockedAndWaiting;
			}
		}
		return ChallengeLockedStatus.Invalid;
	}

	function getNumberOfChallenges()
	{
		local mgr = ::ProgressMgr.getInstance();
		if (mgr._progressData == null)
		{
			::tt_panic("ProgressMgr not initialized");
			return 0;
		}

		return mgr._progressData.challenges.len();
	}

	function getNumberOfUnlockedChallenges()
	{
		local mgr = ::ProgressMgr.getInstance();
		if (mgr._progressData == null)
		{
			::tt_panic("ProgressMgr not initialized");
			return 0;
		}

		local count = 0;
		local sortedChallenges = [];
		foreach (key, value in mgr._progressData.challenges)
		{
			local idx = key.slice(3, 5).tointeger();
			sortedChallenges.push([idx, value.completedDate]);
		}
		sortedChallenges.sort(@(a, b) a[0] <=> b[0]);

		local now = ::getDateString();
		local prevAttempt = null;
		foreach (challenge in sortedChallenges)
		{
			if (challenge[1] != null)
			{
				prevAttempt = challenge[1];
				++count;
			}
			else
			{
				if (prevAttempt == null)
				{
					// First challenge is always unlocked
					return 1;
				}

				return prevAttempt < now ? ::min(sortedChallenges.len(), count + 1) : count;
			}
		}
		return count;
	}

	function getChallengeHighScore(p_missionID)
	{
		local playMode = getPlayMode();
		if (playMode != PlayMode.Challenge)
		{
			::tt_panic("getChallengeHighScore should only be used in challenge missions");
			return null;
		}

		local mgr = ::ProgressMgr.getInstance();
		if (mgr._progressData == null)
		{
			::tt_panic("ProgressMgr not initialized");
			return null;
		}

		return mgr._progressData.challenges[p_missionID].highScore;
	}

	function setChallengeScore(p_missionID, p_score)
	{
		local playMode = getPlayMode();
		if (playMode != PlayMode.Challenge)
		{
			::tt_panic("setChallengeScore should only be used in challenge missions");
			return null;
		}

		local mgr = ::ProgressMgr.getInstance();
		if (mgr._progressData == null)
		{
			::tt_panic("ProgressMgr not initialized");
			return false;
		}

		p_score = p_score.tointeger();
		local table = mgr._progressData.challenges[p_missionID];
		if (table.highScore == null || p_score > table.highScore)
		{
			table.highScore = p_score;
			::echo("[BattleArena:" + p_missionID + "] New highscore: " + p_score);
			mgr._sync();
			return true;
		}

		mgr._sync();
		return false;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Savedata Related Methods

	function saveGameStateToDisk(p_waitForThreadToFinish)
	{
		_clearNonCampaignCheckPoints();
		::storeGameState(p_waitForThreadToFinish);
		::Stats.storeStats();
	}

	function storeCheckPoint()
	{
		local mgr = ::ProgressMgr.getInstance();
		storeCheckPointEx(mgr._getCheckPointID());
	}

	function storeCheckPointEx(p_id)
	{
		local mgr = ::ProgressMgr.getInstance();
		mgr._clearTempCheckPoints();
		::CheckPointMgr.store(p_id);
	}

	function restoreCheckPoint()
	{
		local mgr = ::ProgressMgr.getInstance();
		local id = mgr._getCheckPointID();
		if (::CheckPointMgr.hasID(id))
		{
			::CheckPointMgr.restore(id);
		}
		else
		{
			// No immediate checkpoint found; restart mission
			restartMission();
		}
	}

	function queueCheckPoint()
	{
		local mgr = ::ProgressMgr.getInstance();

		// Part of two stage save
		local id = "~" + mgr._tempCheckPointCount;

		::CheckPointMgr.store(id);
		++mgr._tempCheckPointCount;
		return id;
	}

	function commitCheckPoint(p_id)
	{
		local mgr = ::ProgressMgr.getInstance();
		if (::CheckPointMgr.hasID(p_id))
		{
			// Delete all older temp checkpoints
			_clearTempCheckPoints(mgr._getTimeStampFromID(p_id));

			// Now 'officially' store it
			local correctID = mgr._getCheckPointID();
			::CheckPointMgr.renameID(p_id, correctID);
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks

	function onGamePaused()
	{
		local gameMode = getGameMode();
		if (getPlayMode() == PlayMode.Campaign &&
		   (gameMode == GameMode.SingleLife || gameMode == GameMode.SpeedRun))
		{
			// Player should NOT be dead
			local player = ::getFirstEntityByTag("PlayerBot");
			if (::isValidAndInitializedEntity(player))
			{
				// Save a checkpoint here
				setLastCheckPoint("pause");
				storeCheckPoint();
			}
		}
	}

	function onPlayerReady(p_player)
	{
		setLastCheckPoint("playerready");

		local mgr = ::ProgressMgr.getInstance();
		local id = mgr._getCheckPointID();

		local shouldFireUpdateCallback =
			mgr._progressData.playMode == PlayMode.Campaign &&
			::CheckPointMgr.hasID(id) == false &&
			::CheckPointMgr.hasID(id + "mission_start") == false &&
			mgr._progressData.canContinueCampaignAfterUpdate[mgr._progressData.gameMode];

		if (mgr._progressData.playMode == PlayMode.Level)
		{
			storeCheckPointEx(id);
			return;
		}

		// At start of mission
		if (mgr._fireStartMissionCallback)
		{
			if (shouldFireUpdateCallback)
			{
				p_player.customCallback("onRestoreFromUpdate");
			}
			p_player.customCallback("onStartMission", ::Level.getMissionID());
			mgr._fireStartMissionCallback = false;
			if (mgr._progressData.playMode == PlayMode.Campaign)
			{
				storeCheckPointEx(id + "mission_start");
				return;
			}
		}

		storeCheckPointEx(id);
	}

	function initChallenges(p_challengeTable)
	{
		foreach (missionID in ::Level.getAllChallengeMissionIDs())
		{
			if ((missionID in p_challengeTable) == false)
			{
				p_challengeTable[missionID] <-
					{ medal = ChallengeMedal.None, highScore = null, completedDate = null };
			}
		}
	}

	function initBattleArenas(p_battleArenaTable)
	{
		foreach (missionID in ::Level.getAllBattleArenaMissionIDs())
		{
			if ((missionID in p_battleArenaTable) == false)
			{
				p_battleArenaTable[missionID] <- { highScore = null };
			}
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Private helpers (work on the instance!)

	function _reset(p_showIntroduction)
	{
		::tt_warning("*** Resetting progess data ***");

		local initMissions = function(p_table)
		{
			foreach (mission in ::Level.getAllCampaignMissionIDs())
			{
				p_table[mission] <- [];
				local modes = p_table[mission];
				modes.resize(GameMode.Count);
				modes[GameMode.Normal]     = { campaignScore = null, highScore = null };
				modes[GameMode.SpeedRun]   = { campaignTime = null, campaignLaps = [], fastestTime = null, fastestLaps = [] };
				modes[GameMode.Brutal]     = { campaignScore = null, highScore = null };
				modes[GameMode.SingleLife] = { campaignScore = null, highScore = null };
			}
		}

		::Stats.reset();
		::CheckPointMgr.clearAll();
		::getRegistry().clear();

		// Store reset surviving options
		local isInitialBoot = _progressData == null;
		local lang = _progressData != null ? _progressData.language : null;
		local options = ::OptionsData.getRegistryData();
		local telemetryID = ::getRegistry().getPersistent("telemetry_id");
		local notificationID = ::getRegistry().getPersistent("show_notification");
		local achievements = ::getRegistry().getPersistent("achievements");

		::getRegistry().clearPersistent();
		::OptionsData.setRegistryData(options);
		::getRegistry().setPersistent("telemetry_id", telemetryID);
		::getRegistry().setPersistent("show_notification", notificationID);
		::getRegistry().setPersistent("achievements", achievements);

		_progressData =
		{
			version                        = c_version,
			isClean                        = true,
			gameMode                       = GameMode.Invalid,
			playMode                       = PlayMode.Invalid,
			language                       = lang,
			activeMission                  = null,
			activeMissionPlayedEasyMode    = false,
			isIntroductionDisplayed        = false,
			missionEasyMode                = false,
			isEasyModeOffered              = false,
			areChallengesUnlocked          = false,
			switchMode                     = false,
			completedMissions              = [],
			singleLifeCompletedMissions    = [],
			progress                       = [],
			lastCheckPoints                = [],
			canContinueCampaignAfterUpdate = [],
			purchasedItems                 = [],
			missions =
			{
			},
			battleArenas =
			{
			},
			challenges =
			{
			}
		}

		// Init the gamemodes
		local modes = _progressData.progress;
		modes.resize(GameMode.Count);
		modes[GameMode.Normal]     = { lastMissionID = null, isUnlocked = true,  highScore = null,   playedEasyMode = false, easyMode = false, killedDLLs = 0 };
		modes[GameMode.SpeedRun]   = { lastMissionID = null, isUnlocked = false, fastestTime = null, playedEasyMode = false, easyMode = false, killedDLLs = 0 };
		modes[GameMode.Brutal]     = { lastMissionID = null, isUnlocked = false, highScore = null,   playedEasyMode = false, easyMode = false, killedDLLs = 0 };
		modes[GameMode.SingleLife] = { lastMissionID = null, isUnlocked = false, highScore = null,   playedEasyMode = false, easyMode = false, killedDLLs = 0 };

		// Init the last checkpoints
		local checkpoints = _progressData.lastCheckPoints;
		checkpoints.resize(GameMode.Count);
		checkpoints[GameMode.Normal]     = { id = null, deathCount = 0 };
		checkpoints[GameMode.SpeedRun]   = { id = null, deathCount = 0 };
		checkpoints[GameMode.Brutal]     = { id = null, deathCount = 0 };
		checkpoints[GameMode.SingleLife] = { id = null, deathCount = 0 };

		// Init the update flags
		local canContinueCampaignAfterUpdate = _progressData.canContinueCampaignAfterUpdate;
		for (local i = 0; i < GameMode.Count; ++i)
		{
			canContinueCampaignAfterUpdate.push(false);
		}

		// Init the missions
		initMissions(_progressData.missions);

		// Init the challenges and battle arenas
		initChallenges(_progressData.challenges);

		initBattleArenas(_progressData.battleArenas);

		// Don't use _sync as that clears the isClean flag
		::getRegistry().setPersistent(c_registryKey, _progressData);

		saveGameStateToDisk(true); // ensure to wait for thread to finish saving!

		if (isInitialBoot)
		{
			::Stats.submitTelemetryEvent("initial_boot", ::getLanguage());
		}
		else
		{
			::Stats.submitTelemetryEvent("reset_all", ::getLanguage());
		}

		if (p_showIntroduction)
		{
			::Level.fadeOutAndLoad("area_introduction");
		}
	}

	function _startMissionCallback(p_missionID)
	{
		// Ensure all lingering checkpoints are removed
		_clearNonCampaignCheckPoints();

		// Clear the current campaign checkpoint
		::CheckPointMgr.clearID(_getCheckPointID());

		if (_setActiveMission(p_missionID) == false)
		{
			return "";
		}

		_fireStartMissionCallback = true;

		// Ensure last mission id is set
		if (_progressData.playMode == PlayMode.Campaign)
		{
			_progressData.progress[_progressData.gameMode].lastMissionID = p_missionID;
			_sync();
		}

		::echo("Starting mission '" + p_missionID + "' in level '" + _progressData.activeMission.start_level + "'");
		return _progressData.activeMission.start_level;
	}

	function _setActiveMission(p_missionID)
	{
		// Fetch mission data
		_progressData.activeMission = ::getJSONFromFile("missions/" + p_missionID + ".json");
		_progressData.activeMissionPlayedEasyMode = false;

		if (_progressData.activeMission == null)
		{
			::tt_panic("Unable to load mission data for mission ID '" + p_missionID + "'");
			return false;
		}

		_sync();
		return true;
	}

	function _clearTempCheckPoints(p_timestamp = null)
	{
		local ids = ::CheckPointMgr.getAllIDs();
		foreach (id in ids)
		{
			local timestamp = _getTimeStampFromID(id);
			if (timestamp != null && (p_timestamp == null || timestamp < p_timestamp))
			{
				::CheckPointMgr.clearID(id);
			}
		}
	}

	function _clearNonCampaignCheckPoints()
	{
		local ids = ::CheckPointMgr.getAllIDs();
		foreach (id in ids)
		{
			if (::stringStartsWith(id, "campaign") == false)
			{
				::CheckPointMgr.clearID(id);
			}
		}
	}

	function _getTimeStampFromID(p_id)
	{
		if (p_id == null || p_id.len() < 2 || p_id[0] != '~')
		{
			return null;
		}

		return p_id.slice(1).tointeger();
	}

	function _getCheckPointID()
	{
		switch (_progressData.playMode)
		{
		case PlayMode.Campaign:     return "campaign_" + _progressData.gameMode;
		case PlayMode.Mission:      return "mission";
		case PlayMode.Level:        return "level";
		case PlayMode.BattleArena:  return "battlearena";
		case PlayMode.Challenge:    return "challenge";
		default:
			::tt_panic("Unhandled playmode '" + _progressData.playMode + "'");
			break;
		}
		return "mission";
	}

	function _sync()
	{
		if (_progressData != null)
		{
			_progressData.isClean = false;
		}
		::getRegistry().setPersistent(c_registryKey, _progressData);
	}
}

// This callback is fired by the game and expects a level name to be returned
// depending on the mission ID
function onStartMission(p_missionID)
{
	local mgr = ::ProgressMgr.getInstance();
	return mgr._startMissionCallback(p_missionID);
}
