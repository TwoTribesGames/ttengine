function Level::fadeOutAndLoad(p_level)
{
	::createLevelFade(p_level);
}

function Level::prepareForExit(p_spawnPointID)
{
	// Notify game that a levelexit occurs
	signalExit();
	
	::SpawnSettings.clear();
	if (p_spawnPointID != null)
	{
		::SpawnSettings.set("SpawnPointID", p_spawnPointID);
	}
	
	local player = ::getFirstEntityByTag("PlayerBot");
	if (player != null)
	{
		player.customCallback("onLevelExit");
	}
}

function Level::getAllCampaignMissionIDs()
{
	local ids = getAllMissionIDs();
	return ids.filter( @(index, val) ::startswith(val, "ch_") == false );
}

function Level::getAllChallengeMissionIDs()
{
	local ids = getAllMissionIDs();
	return ids.filter( @(index, val) ::startswith(val, "ch_") );
}

function Level::getAllBattleArenaMissionIDs()
{
	local names = ::getLevelNames();
	return names.filter( @(index, val) ::startswith(val, "ba_") );
}

function Level::isBattleArena(p_levelID)
{
	return ::startswith(p_levelID, "ba_");
}

function Level::isChallenge(p_missionID)
{
	return ::startswith(p_missionID, "ch_");
}
