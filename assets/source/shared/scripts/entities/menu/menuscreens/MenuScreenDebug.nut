include_entity("menu/MenuScreen");

class MenuScreenDebug extends MenuScreen
</
	placeable      = Placeable_Hidden
	collisionRect  = [ 0.0, 0.0, 2.0, 1.0 ]  // center X, center Y, width, height
/>
{
	static c_items =
	[
		// name, price
		["homingmissile" , 999],
		["shotgun"       , 999],
		["grenade"       , 999],
		["emp"           , 999],
		["health_level_1", 720],
		["health_level_2", 1337],
		["magnet_level_1", 720],
		["magnet_level_2", 1337]
	];

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks

	function onButtonDemoResetPressed()
	{
		close();
	}

	function onUnlockModes()
	{
		local mgr = ::ProgressMgr.getInstance();
		mgr._progressData.progress[GameMode.SpeedRun].isUnlocked = true;
		mgr._progressData.progress[GameMode.SingleLife].isUnlocked = true;
		mgr._progressData.areChallengesUnlocked = true;
		::GUIButton.setIsNew("btn_mode_speedrun", true);
		::GUIButton.setIsNew("btn_mode_singlelife", true);
		::GUIButton.setIsNew("btn_challenges", true);
		::GUIButton.setIsNew("btn_battle_arenas", true);
		::GUIButton.setIsNew("btn_missions", true);
		mgr._sync();
		evaluate();
	}

	function onUnlockMissions()
	{
		local mgr = ::ProgressMgr.getInstance();
		mgr._progressData.completedMissions = clone ::Level.getAllCampaignMissionIDs();
		mgr._sync();
		evaluate();
	}

	function onResetAchievements()
	{
		::Stats.resetAll();
		::Stats.reset();
	}

	function onMaxLoot()
	{
		if (isPlayer())
		{
			_activator._energyContainer.setEnergy(10007);
			evaluate();
		}
	}

	function onLockItems()
	{
		if (isPlayer())
		{
			::ProgressMgr.resetPurchasedItems();
			_activator._secondaryWeapon.lockAll();
			_activator._healthBar._upgradeLevel = -1;
			_activator._healthBar.upgrade();
			_activator._energyContainer._upgradeLevel = -1;
			_activator._energyContainer.upgrade();
			evaluate();
		}
	}

	function onUnlockItems()
	{
		if (isPlayer())
		{
			local items = [];
			foreach (item in c_items)
			{
				items.push(item[0]);
			}
			::ProgressMgr.setPurchasedItems(items);
			_activator._secondaryWeapon.unlock("emp");
			_activator._secondaryWeapon.unlock("shotgun");
			_activator._secondaryWeapon.unlock("grenade");
			_activator._secondaryWeapon.unlock("homingmissile");
			_activator._healthBar._upgradeLevel = 1;
			_activator._healthBar.upgrade();
			_activator._energyContainer._upgradeLevel = 1;
			_activator._energyContainer.upgrade();
			evaluate();
		}
	}

	function onGodModeEnabled(p_enabled)
	{
		if (isPlayer())
		{
			_activator.onButtonDebugCheatPressed();
		}
	}

	function onDamage()
	{
		if (isPlayer())
		{
			_activator._healthBar.doDamage(5, _activator);
		}
	}

	function onEasyModeEnabled(p_enabled)
	{
		::ProgressMgr.setEasyMode(p_enabled);
	}

	function onResetEasyModeOffered()
	{
		local mgr = ::ProgressMgr.getInstance();
		mgr._progressData.lastCheckPoints[mgr._progressData.gameMode].deathCount = 0;
		mgr._progressData.isEasyModeOffered = false;
		mgr._sync();
		evaluate();
	}

	function onResetSpecials()
	{
		::getRegistry().erasePersistent(::PickupSpecial.c_registryKey);
	}

	function onCrash()
	{
		close();
		::simulateCrash();
	}

	function onToggleFpsCounter()
	{
		::toggleFpsCounter();
	}

	function onLockChallenges()
	{
		local mgr = ::ProgressMgr.getInstance();
		mgr._progressData.challenges = {};
		::ProgressMgr.initChallenges(mgr._progressData.challenges);
		mgr._sync();

		// Reset the stats as well
		::_perm_table.stats.challenge_gold   = 0;
		::_perm_table.stats.challenge_silver = 0;
		::_perm_table.stats.challenge_bronze = 0;
		::getRegistry().setPersistent("stats", ::_perm_table.stats);
		::Stats.load();
		evaluate();
	}

	function onUnlockChallenges()
	{
		local mgr = ::ProgressMgr.getInstance();

		foreach (challenge in mgr._progressData.challenges)
		{
			challenge.medal = ChallengeMedal.None;
			challenge.completedDate = "19781006";
		}
		mgr._sync();

		// Reset the stats as well
		::_perm_table.stats.challenge_gold   = 0;
		::_perm_table.stats.challenge_silver = 0;
		::_perm_table.stats.challenge_bronze = mgr._progressData.challenges.len();
		::getRegistry().setPersistent("stats", ::_perm_table.stats);
		::Stats.load();
		evaluate();
	}

	function onPrevDebugPoint()
	{
		_activator.customCallback("onButtonDebugPreviousSpawnPointPressed");
	}

	function onNextDebugPoint()
	{
		_activator.customCallback("onButtonDebugNextSpawnPointPressed");
	}

	function onGotoEnd()
	{
		_activator.customCallback("onButtonDebugLastSpawnPointPressed");
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Inherited Methods

	function create()
	{
		base.create();

		createTitle("MENU_DEBUG");
		createFaceButtons(["back"]);

		local buttons = [];

		if (isPlayer())
		{
			local secondaries = _activator._secondaryWeapon._unlockedTypes.len();
			buttons.push(["btn_lock_items", "GUIButton", ["MENU_DEBUG_LOCK_ALL_ITEMS"], onLockItems]);
			buttons.push(["btn_unlock_items", "GUIButton", ["MENU_DEBUG_UNLOCK_ALL_ITEMS"], onUnlockItems]);
			buttons.push(::GUIToggleButton.makeButton("debug_godmode", onGodModeEnabled, getRegistry().get("godmode")));
			buttons.push(["btn_maxloot", "GUIButton", ["MENU_DEBUG_MAXLOOT"], onMaxLoot]);
			buttons.push(["btn_damage", "GUIButton", ["MENU_DEBUG_DAMAGE"], onDamage]);
			buttons.push(["btn_prev_debug_point", "GUIButton", ["MENU_DEBUG_PREV_DEBUG_POINT"], onPrevDebugPoint]);
			buttons.push(["btn_next_debug_point", "GUIButton", ["MENU_DEBUG_NEXT_DEBUG_POINT"], onNextDebugPoint]);
			buttons.push(["btn_end_level", "GUIButton", ["MENU_DEBUG_GOTO_END"], onGotoEnd]);
			buttons.push(["btn_reset_easymode_offered", "GUIButton", ["MENU_DEBUG_RESET_EASYMODE_OFFERED"], onResetEasyModeOffered]);
		}
		else
		{
			buttons.push(["btn_unlock_modes", "GUIButton", ["MENU_DEBUG_UNLOCK_ALL_MODES"], onUnlockModes]);
			buttons.push(["btn_unlock_missions", "GUIButton", ["MENU_DEBUG_UNLOCK_ALL_MISSIONS"], onUnlockMissions]);
		}
		buttons.push(["btn_lock_challenges", "GUIButton", ["MENU_DEBUG_LOCK_CHALLENGES"], onLockChallenges]);
		buttons.push(["btn_unlock_challenges", "GUIButton", ["MENU_DEBUG_UNLOCK_CHALLENGES"], onUnlockChallenges]);
		buttons.push(::GUIToggleButton.makeButton("debug_easymode", onEasyModeEnabled, ::ProgressMgr.isEasyModeEnabled()));
		buttons.push(["btn_reset_achievements", "GUIButton", ["MENU_DEBUG_RESET_ACHIEVEMENTS"], onResetAchievements]);
		buttons.push(["btn_reset_specials", "GUIButton", ["MENU_DEBUG_RESET_SPECIALS"], onResetSpecials]);
		buttons.push(["btn_crash", "GUIButton", ["MENU_DEBUG_CRASH"], onCrash]);
		buttons.push(["btn_toggle_fps_counter", "GUIButton", ["MENU_DEBUG_TOGGLE_FPS_COUNTER"], onToggleFpsCounter]);

		if (isPlayer())
		{
			// Ingame offset
			createPanelButtons(::Vector2(0.0, 0.40), ::MainMenu.c_buttonsSpacing / 1.5, 0, buttons);
		}
		else
		{
			createPanelButtons(::Vector2(0.0, ::MainMenu.c_buttonsYOffset), ::MainMenu.c_buttonsSpacing, 0, buttons);
		}

		evaluate();
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods

	function isPlayer()
	{
		return ::isValidAndInitializedEntity(_activator) && (_activator instanceof ::PlayerBot);
	}

	function evaluate()
	{
		local mgr = ::ProgressMgr.getInstance();
		if (_activator instanceof ::PlayerBot)
		{
			local purchasedItems = ::ProgressMgr.getPurchasedItems();
			_elements["btn_lock_items"].setEnabled(purchasedItems.len() > 0);
			_elements["btn_unlock_items"].setEnabled(purchasedItems.len() < c_items.len());
			_elements["btn_reset_easymode_offered"].setEnabled(mgr._progressData.isEasyModeOffered);
		}
		else
		{
			_elements["btn_unlock_missions"].setEnabled(mgr._progressData.completedMissions.len() < ::Level.getAllCampaignMissionIDs().len());

			local unlock = mgr._progressData.progress[GameMode.SpeedRun].isUnlocked &&
			               mgr._progressData.progress[GameMode.SingleLife].isUnlocked;
			_elements["btn_unlock_modes"].setEnabled(unlock == false);
		}

		local unlockedChallenges = ::ProgressMgr.getNumberOfUnlockedChallenges();
		_elements["btn_unlock_challenges"].setEnabled(unlockedChallenges < ::ProgressMgr.getNumberOfChallenges());
		_elements["btn_lock_challenges"].setEnabled(unlockedChallenges > 1);
	}
}
