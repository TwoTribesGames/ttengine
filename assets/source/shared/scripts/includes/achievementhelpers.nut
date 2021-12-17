::achievementTable <-
{
	// id is used for PS4 logic; don’t change unless you also change the order in the trophy file (trp)!
	// sortid is used for display order
	collectionista        = { order =  0, hidden = false, id =  0, locid = "NEW_ACHIEVEMENT_3_29"},
	lock_and_load         = { order =  1, hidden = false, id = 25, locid = "NEW_ACHIEVEMENT_3_5"},
	scavenger             = { order =  2, hidden = false, id =  5, locid = "NEW_ACHIEVEMENT_1_12"},
	how_could_you         = { order =  3, hidden = false, id =  8, locid = "NEW_ACHIEVEMENT_1_16"},
	the_one               = { order =  4, hidden = false, id = 15, locid = "NEW_ACHIEVEMENT_1_25"},
	copilot               = { order =  5, hidden = false, id = 45, locid = "NEW_ACHIEVEMENT_3_30"},
	breaking_and_entering = { order =  6, hidden = false, id = 23, locid = "NEW_ACHIEVEMENT_3_3"},
	spray_and_pray        = { order =  7, hidden = true,  id = 18, locid = "NEW_ACHIEVEMENT_1_28"},
	peace                 = { order =  8, hidden = false, id = 29, locid = "NEW_ACHIEVEMENT_3_11"},
	heartless             = { order =  9, hidden = false, id = 16, locid = "NEW_ACHIEVEMENT_1_26"},
	farmer                = { order = 10, hidden = false, id = 34, locid = "NEW_ACHIEVEMENT_3_17"},
	greenpeace            = { order = 11, hidden = false, id = 32, locid = "NEW_ACHIEVEMENT_3_15"},
	copilot_alive         = { order = 12, hidden = false, id = 46, locid = "NEW_ACHIEVEMENT_3_31"},
	moby_dick             = { order = 13, hidden = false, id =  3, locid = "NEW_ACHIEVEMENT_1_10"},
	follow_the_leader     = { order = 14, hidden = false, id = 37, locid = "NEW_ACHIEVEMENT_3_20"},
	inked_out             = { order = 15, hidden = false, id = 12, locid = "NEW_ACHIEVEMENT_1_22"},
	choo_choo             = { order = 16, hidden = false, id = 27, locid = "NEW_ACHIEVEMENT_3_9"},
	sleeping_with_fishes  = { order = 17, hidden = true,  id = 26, locid = "NEW_ACHIEVEMENT_3_8"},
	minesweeper           = { order = 18, hidden = false, id = 11, locid = "NEW_ACHIEVEMENT_1_20"},
	// l-ife (see below)
	access_denied         = { order = 20, hidden = false, id = 22, locid = "NEW_ACHIEVEMENT_3_2"},
	mint_condition        = { order = 21, hidden = false, id = 38, locid = "NEW_ACHIEVEMENT_3_21"},
	eye_of_the_tiger      = { order = 22, hidden = false, id =  4, locid = "NEW_ACHIEVEMENT_1_11"},
	got_your_back         = { order = 23, hidden = false, id = 21, locid = "NEW_ACHIEVEMENT_3_0"},
	turncoat              = { order = 24, hidden = false, id = 17, locid = "NEW_ACHIEVEMENT_1_27"},
	the_big_cleanup       = { order = 25, hidden = false, id = 30, locid = "NEW_ACHIEVEMENT_3_12"},
	hide_and_reap         = { order = 26, hidden = false, id = 20, locid = "NEW_ACHIEVEMENT_1_31"},
	squeeky_clean         = { order = 27, hidden = true,  id = 31, locid = "NEW_ACHIEVEMENT_3_13"},
	friendly_waters       = { order = 28, hidden = false, id = 36, locid = "NEW_ACHIEVEMENT_3_19"},
	pest_control          = { order = 29, hidden = false, id = 35, locid = "NEW_ACHIEVEMENT_3_18"},
	smashed_and_slivered  = { order = 30, hidden = false, id = 13, locid = "NEW_ACHIEVEMENT_1_23"},
	alt_f4                = { order = 31, hidden = false, id = 14, locid = "NEW_ACHIEVEMENT_1_24"},
	purge                 = { order = 32, hidden = false, id =  1, locid = "NEW_ACHIEVEMENT_1_6"},
	unblockable           = { order = 33, hidden = false, id =  2, locid = "NEW_ACHIEVEMENT_1_9"},
	houdini               = { order = 34, hidden = false, id = 19, locid = "NEW_ACHIEVEMENT_1_30"},
	bronze_medal          = { order = 35, hidden = false, id = 44, locid = "NEW_ACHIEVEMENT_3_28"},
	silver_medal          = { order = 36, hidden = false, id = 43, locid = "NEW_ACHIEVEMENT_3_27"},
	gold_medal            = { order = 37, hidden = false, id = 42, locid = "NEW_ACHIEVEMENT_3_26"},
	single_purchase       = { order = 38, hidden = false, id = 39, locid = "NEW_ACHIEVEMENT_3_22"},
	new_sheriff           = { order = 39, hidden = false, id =  7, locid = "NEW_ACHIEVEMENT_1_14"},
	godmode               = { order = 40, hidden = false, id = 28, locid = "NEW_ACHIEVEMENT_3_10"},
	big_saver             = { order = 41, hidden = false, id =  6, locid = "NEW_ACHIEVEMENT_1_13"},
	motherloot            = { order = 42, hidden = false, id = 40, locid = "NEW_ACHIEVEMENT_3_23"},
	best_friends          = { order = 43, hidden = false, id = 33, locid = "NEW_ACHIEVEMENT_3_16"},
	you_can_do_it         = { order = 44, hidden = false, id =  9, locid = "NEW_ACHIEVEMENT_1_18"},
	are_you_nuts          = { order = 45, hidden = false, id = 10, locid = "NEW_ACHIEVEMENT_1_19"},
	guybrush              = { order = 46, hidden = false, id = 47, locid = "NEW_ACHIEVEMENT_3_32"},
	willy                 = { order = 47, hidden = false, id = 41, locid = "NEW_ACHIEVEMENT_3_24"}
};

// Special case <3
::achievementTable["l-ife"] <- { order = 19, hidden = false, id = 24, locid = "NEW_ACHIEVEMENT_3_4"};

foreach (key, achievement in ::achievementTable)
{
	::Stats.addAchievement(key, achievement.id);
}

// Add progress achievements
::Stats.addIntProgressAchievementRequirement("unlocked_achievements", "collectionista", ::achievementTable.len()-1);
::Stats.addIntProgressAchievementRequirement("killed_cockroaches", "how_could_you", 300);
::Stats.addIntProgressAchievementRequirement("hacked_enemies", "the_one", 50);
::Stats.addIntProgressAchievementRequirement("collected_specials", "scavenger", 5);
::Stats.addIntProgressAchievementRequirement("plundered_healbots", "heartless", 10);
::Stats.addIntProgressAchievementRequirement("completed_singlelife_missions", "you_can_do_it", 13);
::Stats.addIntProgressAchievementRequirement("killed_enemies_underwater", "hide_and_reap", 25);
::Stats.addIntProgressAchievementRequirement("challenge_gold", "gold_medal", 18);
::Stats.addIntProgressAchievementRequirement("challenge_silver", "silver_medal", 18);
::Stats.addIntProgressAchievementRequirement("challenge_bronze", "bronze_medal", 18);


function onAchievementUnlocked(p_name, p_id)
{
	::Stats.submitTelemetryEvent("achievement", p_name);
	local unlockedAchievements = ::getRegistry().getPersistent("achievements");
	if (unlockedAchievements == null)
	{
		unlockedAchievements = {};
	}
	
	if ((p_name in unlockedAchievements) == false)
	{
		unlockedAchievements[p_name] <- ::getDateString() + ::getTimeString();
		::getRegistry().setPersistent("achievements", unlockedAchievements);
		::spawnEntity("AchievementToaster", ::Vector2(0, 0), { _name = p_name });
		::Stats.incrementStat("unlocked_achievements");
	}
	else
	{
		::tt_panic("onAchievementUnlocked called, yet '" + p_name + "' is part of the unlockedAchievements in registry");
	}
}

function onAchievementsReset()
{
	::getRegistry().erasePersistent("achievements");
}
