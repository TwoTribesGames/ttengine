include_entity("menu/IngameMenu");

class SpiderStatusMenu extends IngameMenu
</
	placeable      = Placeable_Hidden
	collisionRect  = [ 0.0, 0.0, 2.0, 1.0 ]  // center X, center Y, width, height
/>
{
	static c_colorGradingTexture = "ingamemenu";     // Required
	static c_musicTrack          = "ingamemenu";     // Optional
	static c_yOffset             = 0.0;

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Inherited Methods

	function create()
	{
		base.create();

		// Create labels
		local playMode = ::ProgressMgr.getPlayMode();
		if (_isPlayerDead == false)
		{
			local locID = "";
			switch (playMode)
			{
			case PlayMode.BattleArena:
				locID = "BATTLEARENA_NAME_" + ::Level.getName().toupper();
				break;

			case PlayMode.Challenge:
				locID = "CHALLENGE_NAME_" + ::Level.getMissionID().toupper();
				break;

			default:
				locID = ::ProgressMgr.hasActiveMission() ?
					"MISSION_BRIEFING_" + ::Level.getMissionID().toupper() + "_HEADER" :
					"PAUSEMENU_MISSION_COMPLETED";
				break;
			}

			createTitle(locID);
		}
		else
		{
			createTextArea("guru_meditation", "PAUSEMENU_GURU_MEDITATION",
				::Vector2(0.0, 0.4125), ::TextColors.Light, 1.0, 0.1, HorizontalAlignment_Center);
		}

		// Create all info categories
		createText("health_header", "PAUSEMENU_HEALTH_HEADER",  ::Vector2(-0.27, -0.065 + c_yOffset), ::TextColors.Light, HorizontalAlignment_Right);
		createText("health",        null,                       ::Vector2(-0.27, -0.105 + c_yOffset), ::TextColors.Dark,  HorizontalAlignment_Right);
		createText("magnet_header", "PAUSEMENU_MAGNET_HEADER",  ::Vector2(-0.32, -0.162 + c_yOffset), ::TextColors.Light, HorizontalAlignment_Right);
		createText("magnet",        null,                       ::Vector2(-0.32, -0.202 + c_yOffset), ::TextColors.Dark,  HorizontalAlignment_Right);

		createText("primary_header",   "PAUSEMENU_PRIMARY_HEADER",   ::Vector2(0.27, -0.065 + c_yOffset), ::TextColors.Light, HorizontalAlignment_Left);
		createText("primary",          null,                         ::Vector2(0.27, -0.105 + c_yOffset), ::TextColors.Dark,  HorizontalAlignment_Left);
		createText("secondary_header", "PAUSEMENU_SECONDARY_HEADER", ::Vector2(0.32, -0.162 + c_yOffset), ::TextColors.Light, HorizontalAlignment_Left);
		createText("secondary",        null,                         ::Vector2(0.32, -0.202 + c_yOffset), ::TextColors.Dark,  HorizontalAlignment_Left);
		createText("hackbeam_header",  "PAUSEMENU_HACKBEAM_HEADER",  ::Vector2(0.34, -0.260 + c_yOffset), ::TextColors.Light, HorizontalAlignment_Left);
		createText("hackbeam",         null,                         ::Vector2(0.34, -0.300 + c_yOffset), ::TextColors.Dark,  HorizontalAlignment_Left);

		createText("player_status_header",  "PAUSEMENU_PLAYER_STATUS_HEADER", ::Vector2(0.0, -0.345 + c_yOffset), ::TextColors.Light);
		createText("player_status",         null,                               ::Vector2(0.0, -0.385 + c_yOffset), ::TextColors.Dark);

		// Fuel indicator (custom code)
		{
			createText("fuel_header",  "PAUSEMENU_FUEL_HEADER", ::Vector2(-0.34, -0.260 + c_yOffset), ::TextColors.Light, HorizontalAlignment_Right);
			local pres = createStoppedPresentation("fuel", "pausemenu_fuel", ::Vector2(-0.45, -0.3 + c_yOffset));

			// Convert mission id to number
			local missionID = ::Level.getMissionID();
			if (playMode != PlayMode.Challenge && playMode != PlayMode.BattleArena
			    && missionID.len() > 2)
			{
				local id = missionID.slice(0, 2).tointeger();
				if (id == 13 && ::ProgressMgr.hasActiveMission() == false)
				{
					// Hack for ending after mission 13
					id = 14;
				}
				pres.addCustomValue("frame", id-1);
			}
			else
			{
				pres.addCustomValue("frame", 0);
			}
			pres.start("fadein", [], false, 0);
		}

		// Update information
		if (_isPlayerDead == false)
		{
			updateUpgradeText("health", _activator._healthBar.getUpgradeLevel() + 1, 3);
			updateUpgradeText("magnet", _activator._energyContainer.getUpgradeLevel() + 1, 3);
			updatePrimaryText(_activator);
			updateSecondaryText(_activator);
			updateHackBeamText(_activator);
			_labels["player_status"].label.setTextLocalized("PAUSEMENU_PLAYER_STATUS_" + _activator.getStatus().toupper());
		}
		else
		{
			_labels["health"].label.setTextLocalized("PAUSEMENU_DISCONNECTED");
			_labels["magnet"].label.setTextLocalized("PAUSEMENU_DISCONNECTED");
			_labels["primary"].label.setTextLocalized("PAUSEMENU_DISCONNECTED");
			_labels["secondary"].label.setTextLocalized("PAUSEMENU_DISCONNECTED");
			_labels["hackbeam"].label.setTextLocalized("PAUSEMENU_DISCONNECTED");
			_labels["player_status"].label.setTextLocalized("PAUSEMENU_PLAYER_STATUS_KILLED");
			createText("fuel", "PAUSEMENU_DISCONNECTED", ::Vector2(-0.34, -0.305 + c_yOffset), ::TextColors.Light, HorizontalAlignment_Right);
		}
		createBotGraphic();
	}

	function refocus(p_focusElement, p_unhideVisuals)
	{
		base.refocus(p_focusElement, p_unhideVisuals);

		createBotGraphic();
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods

	function createBotGraphic()
	{
		removePresentationNoFade("spiderbot");
		local tags = [];
		tags.push("singleplayer");
		if (_isPlayerDead)
		{
			tags.push("dead");
		}
		createPresentation("spiderbot", "pausemenu", ::Vector2(0, c_yOffset), HudAlignment.Center, tags);
	}

	function updatePrimaryText(p_player)
	{
		if (p_player.weaponsEnabled() == false || p_player.aimControlsEnabled() == false)
		{
			_labels["primary"].label.setTextLocalized("PAUSEMENU_PRIMARY_DISABLED");
		}
		else if (p_player._lockAimAngle != null)
		{
			_labels["primary"].label.setTextLocalized("PAUSEMENU_PRIMARY_LOCKED");
		}
		else if (p_player._butterflyMode)
		{
			_labels["primary"].label.setTextLocalized("PAUSEMENU_PRIMARY_BUTTERFLIES");
		}
		else
		{
			_labels["primary"].label.setTextLocalized("PAUSEMENU_PRIMARY_NORMAL");
		}
	}

	function updateSecondaryText(p_player)
	{
		local numberOfSecondaries = p_player._secondaryWeapon._unlockedTypes.len();

		if (p_player.weaponsEnabled() == false || p_player.aimControlsEnabled() == false)
		{
			_labels["secondary"].label.setTextLocalized("PAUSEMENU_SECONDARY_DISABLED");
		}
		else if (numberOfSecondaries == 0)
		{
			_labels["secondary"].label.setTextLocalized("PAUSEMENU_SECONDARY_EMPTY");
		}
		else
		{
			_labels["secondary"].label.setTextLocalizedAndFormatted("PAUSEMENU_SECONDARY_NORMAL",
				[makeUpgradeText(numberOfSecondaries, 4)]);
		}
	}

	function updateHackBeamText(p_player)
	{
		local numberOfHacks = ::VirusHelpers.getTotalUnlockedVirusses() - 1;

		if (p_player.hackControlsEnabled() == false || p_player.aimControlsEnabled() == false)
		{
			_labels["hackbeam"].label.setTextLocalized("PAUSEMENU_HACKBEAM_DISABLED");
		}
		else if (numberOfHacks == 0)
		{
			_labels["hackbeam"].label.setTextLocalized("PAUSEMENU_HACKBEAM_EMPTY");
		}
		else
		{
			_labels["hackbeam"].label.setTextLocalizedAndFormatted("PAUSEMENU_HACKBEAM_NORMAL",
				[makeUpgradeText(numberOfHacks, 5)]);
		}
	}

	function updateUpgradeText(p_id, p_level, p_maxLevel)
	{
		_labels[p_id].label.setTextLocalizedAndFormatted(
			"PAUSEMENU_" + p_id.toupper() + "_" + p_level.tostring(), [makeUpgradeText(p_level, p_maxLevel)]);
	}

	function makeUpgradeText(p_level, p_maxLevel)
	{
		return "[" + p_level.tostring() + "/" + p_maxLevel.tostring() + "]";
	}
}
