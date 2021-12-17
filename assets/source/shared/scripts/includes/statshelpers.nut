tt_include("includes/permhelpers");

// Used for result screens
enum StatsTypes
{
	CampaignStats,
	MissionStats,
	CampaignScore,
	MissionScore,
	CampaignTimes,
	MissionTimes
}

::_perm_table.stats =
{
	hacked_enemies                = 0,
	collected_specials            = 0,
	killed_cockroaches            = 0,
	plundered_healbots            = 0,
	killed_enemies_underwater     = 0,
	completed_singlelife_missions = 0,
	challenge_gold                = 0,
	challenge_silver              = 0,
	challenge_bronze              = 0,
	unlocked_achievements         = 0
}

// Register stats; do this at compile time
foreach (key, stats in ::_perm_table.stats)
{
	::Stats.addStatDefinition(key, StatType_Int);
}

function Stats::load()
{
	restore();
	
	// Just to make sure; set the int stats again; apparently sometimes Steam fails to handle them properly
	foreach (key, stats in ::_perm_table.stats)
	{
		::Stats.setIntStat(key, ::_perm_table.stats[key]);
	}
	::Stats.storeStats();
}

function Stats::restore()
{
	local stats = ::getRegistry().getPersistent("stats");
	if (stats != null)
	{
		::_perm_table.stats = stats;
	}
	
	local unlockedAchievements = ::getRegistry().getPersistent("achievements");
	local count = 0;
	if (unlockedAchievements != null)
	{
		count = unlockedAchievements.len();
		foreach (achievement, unlockdate in unlockedAchievements)
		{
			::echo("[UNLOCKED ACHIEVEMENT] " + achievement);
			::Stats.setAchievementUnlockedStatus(achievement, true);
		}
	}
	if (stats != null && count != stats["unlocked_achievements"])
	{
		::Stats.setStat("unlocked_achievements", count);
	}
}

function Stats::reset()
{
	foreach (key, stats in ::_perm_table.stats)
	{
		::_perm_table.stats[key] = 0;
		::Stats.setIntStat(key, 0);
	}
	::Stats.storeStats();
	::getRegistry().erasePersistent("stats");
	::getRegistry().erasePersistent(::PickupSpecial.c_registryKey);
	::getRegistry().erasePersistent(::Cockroach.c_registryKey);
	// Also reset the shown notifications
	::getRegistry().erasePersistent("notifications");
}

function Stats::setStat(p_stat, p_value)
{
	::Stats.submitTelemetryEvent("set_stat", p_stat + " -> " + ::niceStringFromObject(p_value));
	
	::_perm_table.stats[p_stat] = p_value;
	::Stats.setIntStat(p_stat, p_value);
	::Stats.storeStats();
	::getRegistry().setPersistent("stats", ::_perm_table.stats);
}

function Stats::incrementStat(p_stat)
{
	::Stats.setStat(p_stat, ++::_perm_table.stats[p_stat]);
}

::g_telemetryID      <- null;
::g_telemetryEnabled <- false;

function Stats::initTelemetry()
{
	::g_telemetryID = ::getRegistry().getPersistent("telemetry_id");
	if (::g_telemetryID == null)
	{
		// Make an ID
		::g_telemetryID = ::rnd().tostring();
		::getRegistry().setPersistent("telemetry_id", ::g_telemetryID);
	}
	
	if (::getRegistry().existsPersistent("telemetry_enabled"))
	{
		::g_telemetryEnabled = ::getRegistry().getPersistent("telemetry_enabled") 
	}
	
	::echo("Telemetry ID: '" + ::g_telemetryID + "'");
	::echo("Telemetry Enabled: " + ::g_telemetryEnabled);
}

::Stats.initTelemetry();

function Stats::setTelemetryEnabled(p_enabled)
{
	::g_telemetryEnabled = true;
	::Stats.submitTelemetryEvent("set_telemetry_enabled", p_enabled);
	
	::g_telemetryEnabled = p_enabled;
	::getRegistry().setPersistent("telemetry_enabled", p_enabled);
}

function Stats::isTelemetryEnabled()
{
	return ::g_telemetryEnabled;
}

function Stats::submitTelemetryEvent(p_type, p_details = null)
{
	// MR: Disabled since GDPR
	return;
	
	if (::g_telemetryEnabled == false)
	{
		return;
	}
	
	::echo("*** SENDING TELEMETRY EVENT: " + p_type.tostring() + " (" + ::niceStringFromObject(p_details) + ") ***");
	
	local playMode = "";
	local missionID = "";
	local checkpointID = "";
	local gamemodeID = "";
	local isEasyModeEnabled = true;
	if (::ProgressMgr.hasInstance())
	{
		playMode = ::ProgressMgr.getPlayMode();
		missionID = playMode != PlayMode.BattleArena ? ::Level.getMissionID() : ::Level.getName();
		checkpointID = ::ProgressMgr.hasLastCheckPoint() ? ::ProgressMgr.getLastCheckPoint().id.tostring() : "";
		gamemodeID = ::ProgressMgr.getGameMode().tostring();
		isEasyModeEnabled = ::ProgressMgr.isEasyModeEnabled();
	}
	
	local player = ::getFirstEntityByTag("PlayerBot");
	local position = "";
	if (player != null)
	{
		local pos = player.getPosition();
		position = pos.x.tointeger() + ", " + pos.y.tointeger();
	}
	
	local platform = ::getCurrentPlatform();
	switch (platform)
	{
	case Platform_WIN: platform = "win"; break;
	case Platform_LNX: platform = "lnx"; break;
	case Platform_MAC: platform = "mac"; break;
	default:
		::tt_panic("Invalid platform '" + platform + "'");
		platform = "xxx";
	}
	
	local region = ::getRegion();
	::sendGetRequest("www.rivethegame.com", "/stats/index.php",
	[
		"type", p_type.tostring(),
		"checkpoint_id", checkpointID,
		"mission_id", missionID,
		"level_id", ::Level.getName(),
		"gamemode_id", gamemodeID,
		"playmode_id", playMode.tostring(),
		"player_id", ::g_telemetryID,
		"platform", ::isSteamBuild() ? platform + "/steam" : platform,
		"easymode", isEasyModeEnabled ? "1" : "0",
		"details", p_details == null ? "" : p_details.tostring(),
		"position", position,
		"test", ::isTestBuild() ? "1" : "0",
		"version", ::getVersion(),
		"region", region.len() > 0 ? region.toupper() : ""
	]);
}
