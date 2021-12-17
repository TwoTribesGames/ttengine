::g_gameOverMessageCount <-
{
	// Entities / Environment
	AIRMINEBOT         = 5,
	ASTEROIDENEMY      = 5,
	BAT                = 5,
	BASETURRET         = 0, // Dont show any specific message because it is an hidden entity
	BLASTWAVEDEBRIS    = 4,
	BUMPERENEMY        = 5,
	CAMERAONRAILS      = 5,
	CRUSHBOX           = 5,
	DEATHRAY           = 5,
	DESTRUCTIBLEOBJECTKAMIKAZE = 2,
	DLL                = 2,
	DROPSHIP           = 5,
	ELECTRICDEATH      = 5,
	ENDBOSS            = 10,
	EXPLOSIONTRIGGER   = 0,
	INVISIBLEDAMAGER   = 0,
	HEALTHBOT          = 2,
	KAMIKAZEENEMY      = 10,
	LAVA               = 10,
	MINE               = 6,
	PISTON             = 5,
	PLAYERBOT          = 4,
	PROTOENEMY         = 0,
	SEAMINE            = 5,
	SHREDDER           = 5,
	SLICER             = 6,
	SQUID              = 5,
	TETROMINO          = 5,
	TRAIN              = 11,
	TURRET             = 5,
	UNDERWATERROTORENEMY = 5,
	WALKERTURRET       = 5,
	WASPENEMY          = 5,
	WATERWALKERENEMY   = 5,
	WRECKINGBALL       = 5,
	
	// Weapons
	ENEMYBULLET        = 5,
	EXPLOSION          = 11,
	FLAME              = 5,
	LASERGUN           = 11,
	
	// Generic message count
	GENERIC_COMMON     = 10,
	GENERIC_RARE       = 76
}

// Use this table for redirecting specific entities to a single one
// Please note that ::g_gameOverMessageCount table has higher priority
::g_gameOverMessageConversionTable <-
{
	ASTEROIDSMALL      = "ASTEROIDENEMY",
	ASTEROIDMEDIUM     = "ASTEROIDENEMY",
	ASTEROIDBIG        = "ASTEROIDENEMY",
	BUMPERENEMYBOSS    = "BUMPERENEMY",
	DROPSHIPBIG        = "DROPSHIP",
	DROPSHIPMEGA       = "DROPSHIP",
	DROPSHIPBOSS       = "DROPSHIP",
	ELEVATOR           = "CRUSHBOX",
	ENDBOSSDEATHRAY    = "DEATHRAY",
	ENDBOSSDEBRIS      = "LAVA",
	ENDBOSSTRAIN       = "TRAIN",
	LAVAWAVE           = "LAVA",
	POWERSOURCE        = "ELECTRICDEATH",
	TETROMINO2X1       = "TETROMINO",
	TETROMINO1X2       = "TETROMINO",
	TETROMINO1X1       = "TETROMINO",
	TRAINGATE          = "ELECTRICDEATH",
	TRAINRIDETRAIN     = "TRAIN",
	TRAINTRACK         = "ELECTRICDEATH",
	TRIGGERHEALTHDECREASE = "PISTON",
	WALKERTURRETBIG    = "WALKERTURRET",
	WALLMINEBOT        = "MINE",
	LASERTURRET        = "LASERGUN"
}


function getGameOverMessageID(p_killer)
{
	// Determine message
	if (::Level.isChallenge(::Level.getMissionID()))
	{
		return "GAME_OVER_CHALLENGE";
	}
	
	local messageID = "GAME_OVER_";
	local isString = typeof(p_killer) == "string";
	
	if (isString == false && ::isValidEntity(p_killer) == false)
	{
		::tt_warning("Killer is invalid; no specific game over message possible!");
		
		local count = ::g_gameOverMessageCount["GENERIC_COMMON"];
		messageID += "GENERIC_COMMON_" + ::rnd_minmax(1, count);
		return messageID;
	}
	
	// Check if killer has _shooter member; if it has, we need to check both messages
	local killerID = isString ? p_killer : p_killer.getType().toupper();
	local shooterID = null;
	if ("_shooter" in p_killer && ::isValidEntity(p_killer._shooter))
	{
		shooterID = p_killer._shooter.getType().toupper();
	}
	
	local killerMessageCount  = 0;
	local shooterMessageCount = 0;
	
	if (killerID in ::g_gameOverMessageCount)
	{
		killerMessageCount = ::g_gameOverMessageCount[killerID];
	}
	else
	{
		// Check in conversion table
		if (killerID in ::g_gameOverMessageConversionTable)
		{
			killerID           = ::g_gameOverMessageConversionTable[killerID];
			killerMessageCount = ::g_gameOverMessageCount[killerID];
		}
		else
		{
			::tt_panic("NIELS: '" + killerID + "' can have a specific game over message! Or perhaps add it to the conversion table?");
		}
	}
	
	if (shooterID != null)
	{
		if (shooterID in ::g_gameOverMessageCount)
		{
			shooterMessageCount = ::g_gameOverMessageCount[shooterID];
		}
		else
		{
			// Check in conversion table
			if (shooterID in ::g_gameOverMessageConversionTable)
			{
				shooterID           = ::g_gameOverMessageConversionTable[shooterID];
				shooterMessageCount = ::g_gameOverMessageCount[shooterID];
			}
			else
			{
				::tt_panic("NIELS: '" + shooterID + "' can have a specific game over message! Or perhaps add it to the conversion table?");
			}
		}
	}
	
	// Common and rare are two standard groups
	local totalGroups = 2 + (killerMessageCount > 0 ? 1 : 0) + (shooterMessageCount > 0 ? 1 : 0);
	
	local r = ::frnd();
	local threshold = 1.0 / totalGroups;
	
	local killerMessageThreshold  = killerMessageCount > 0 ? threshold : 0.0;
	local shooterMessageThreshold = killerMessageThreshold  + (shooterMessageCount > 0 ? threshold : 0.0);
	local genericCommonThreshold  = shooterMessageThreshold + threshold;
	
	// Kill messages
	if (r < killerMessageThreshold)
	{
		local id = ::rnd_minmax(1, killerMessageCount);
		return messageID + killerID + "_" + id;
	}
	// Shooter messages
	else if (r < shooterMessageThreshold)
	{
		local id = ::rnd_minmax(1, shooterMessageCount);
		return messageID + shooterID + "_" + id;
	}
	// Generic common messages
	else if (r < genericCommonThreshold)
	{
		local id = ::rnd_minmax(1, ::g_gameOverMessageCount["GENERIC_COMMON"]);
		return messageID + "GENERIC_COMMON_" + id;
	}
	
	// Else Generic rare messages
	local id = ::rnd_minmax(1, ::g_gameOverMessageCount["GENERIC_RARE"]);
	return messageID + "GENERIC_RARE_" + id;
}
