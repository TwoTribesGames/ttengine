include_entity("menu/IngameMenu");

class CampaignEndScreen extends IngameMenu
</
	placeable      = Placeable_Hidden
	collisionRect  = [ 0.0, 0.5, 1.0, 1.0 ]  // center X, center Y, width, height
/>
{
	static c_colorGradingTexture = "ingamemenu"; // Required
	static c_musicTrack          = null;         // Optional
	static c_yOffset             = 0.17;

	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn

	function onInit()
	{
		base.onInit();

		stopTimer("handleAudio");
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Script Item Handling

	function onScriptItemProcessed(p_id)
	{
		if (p_id == "start")
		{
			::Audio.playGlobalSoundEffect("Effects", "endsequence_ambience_cockpit");
			startScriptItemTimer("powerdownSound", 20.0);
			startScriptItemTimer("cryoSound", 30.0);
			startScriptItemTimer("logoutSound", 38.0);

			// Create labels
			createTitle("PAUSEMENU_TITLE");

			startScriptItemTimer("showConsole", 1.0);

			// Create all info categories
			local timeOffset = 0.0;
			local tags = ["singleplayer"];
			createPresentation("spiderbot", "pausemenu", ::Vector2(0, c_yOffset), HudAlignment.Center, tags);
			createText        ("health_header",    "PAUSEMENU_HEALTH_HEADER",    ::Vector2(-0.27, -0.065 + c_yOffset), ::TextColors.Light, HorizontalAlignment_Right);
			createText        ("health",           "CAMPAIGNSTART_HEALTH",       ::Vector2(-0.27, -0.105 + c_yOffset), ::TextColors.Dark,  HorizontalAlignment_Right);
			createText        ("magnet_header",    "PAUSEMENU_MAGNET_HEADER",    ::Vector2(-0.32, -0.162 + c_yOffset), ::TextColors.Light, HorizontalAlignment_Right);
			createText        ("magnet",           "PAUSEMENU_PRIMARY_DISABLED",       ::Vector2(-0.32, -0.202 + c_yOffset), ::TextColors.Dark,  HorizontalAlignment_Right);
			createText        ("fuel_header",      "PAUSEMENU_FUEL_HEADER",      ::Vector2(-0.34, -0.260 + c_yOffset), ::TextColors.Light, HorizontalAlignment_Right);
			createPresentation("fuel", "campaignendscreen", ::Vector2(-0.45, -0.3 + c_yOffset));
			createText        ("primary_header",   "PAUSEMENU_PRIMARY_HEADER",   ::Vector2( 0.27, -0.065 + c_yOffset), ::TextColors.Light, HorizontalAlignment_Left);
			createText        ("primary",          "PAUSEMENU_PRIMARY_DISABLED",      ::Vector2( 0.27, -0.105 + c_yOffset), ::TextColors.Dark,  HorizontalAlignment_Left);
			createText        ("secondary_header", "PAUSEMENU_SECONDARY_HEADER", ::Vector2( 0.32, -0.162 + c_yOffset), ::TextColors.Light, HorizontalAlignment_Left);
			createText        ("secondary",        "PAUSEMENU_PRIMARY_DISABLED",    ::Vector2( 0.32, -0.202 + c_yOffset), ::TextColors.Dark,  HorizontalAlignment_Left);
			createText        ("hackbeam_header",  "PAUSEMENU_HACKBEAM_HEADER",  ::Vector2( 0.34, -0.260 + c_yOffset), ::TextColors.Light, HorizontalAlignment_Left);
			createText        ("hackbeam",         "PAUSEMENU_PRIMARY_DISABLED",     ::Vector2( 0.34, -0.305 + c_yOffset), ::TextColors.Dark,  HorizontalAlignment_Left);

			addSubtitles([
				[2.0, "13_TERMINAL_A_RS"],
				[0.5, "13_TERMINAL_B_RS"],
				[1.5, "13_TERMINAL_C_RS"],
				[1.0, "13_TERMINAL_F_RS"]
			]);
		}
		else if (p_id == "powerdownSound")
		{
			::Audio.playGlobalSoundEffect("Effects", "endsequence_powerdown");
		}
		else if (p_id == "cryoSound")
		{
			::Audio.playGlobalSoundEffect("Effects", "endsequence_cryo_start");
		}
		else if (p_id == "logoutSound")
		{
			::Audio.playGlobalSoundEffect("Effects", "endsequence_logout_succesful");
		}
		else if (p_id == "showConsole")
		{
			local window = createConsoleWindow("console", ::Vector2(-0.15, 0.075),
			{
				_titleID   = "CONSOLE_HEADER",
				_width     = 0.7,
				_zOffset   = 0.2
				_lineCount = 7,
				_parent    = this
			});

			window.addCommands([
				// [startdelay, command type, locsheet id, enddelay]
				[18.0, ConsoleCommandType.Command, "13_CONSOLE_SHUTDOWNCOMMAND",       0.0],
				[0.2, ConsoleCommandType.Result,  "13_CONSOLE_SHUTDOWNRESULT",        0.0],
				[0.0, ConsoleCommandType.Result,  null,                     0.0],
				[9.0, ConsoleCommandType.Command, "13_CONSOLE_CRYOCOMMAND", 0.0],
				[0.2, ConsoleCommandType.Result,  "13_CONSOLE_CRYORESULT",  0.0],
				[0.0, ConsoleCommandType.Result,  null,                            0.0],
				[6.45, ConsoleCommandType.Command, "13_CONSOLE_LOGOUTCOMMAND",      0.0],
				[0.2, ConsoleCommandType.Result,  "13_CONSOLE_LOGOUTRESULT",       0.0],
				[0.0, ConsoleCommandType.Result,  null,                            0.0],
				[160.0, ConsoleCommandType.Command, "13_CONSOLE_SHUTDOWNCOMMAND",       0.0],
			]);
		}
		else if (p_id == "13_CONSOLE_SHUTDOWNRESULT")
		{
			addSubtitles([
				[2.0, "13_TERMINAL_D_RS"],
				[1.0, "13_TERMINAL_E_RS"]
			]);
		}
		else if (p_id == "13_CONSOLE_CRYORESULT")
		{
			createPresentation("cryo", "hud_cryo", ::Vector2(0, 0));

			addSubtitles([
				[1.5, "13_HANSOLO_RS"],
			]);
		}
		else if (p_id == "13_CONSOLE_LOGOUTRESULT")
		{
			addSubtitles([
				[1.5, "13_SUCCESS_RS"]
			]);
		}
		else if (p_id == "13_SUCCESS_RS_done")
		{
			addSubtitles([
				[2.0, "13_GOODBYE_A_RS"],
				[0.5, "13_GOODBYE_B_RS"]
			]);
		}
		else if (p_id == "13_GOODBYE_B_RS_done")
		{
			createDelayedTypedText(7.0, "2000_years_later", "13_LATER", ::Vector2( 0.0, 0.0), ::TextColors.Light, HorizontalAlignment_Center);
			createDelayedPresentation(2.0, "fade_to_black", "campaignendscreen", ::Vector2(0.0, 0.0));
			startScriptItemTimer("close", 11.0);
		}
		else if (p_id == "close")
		{
			close();
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks

	function onDelayedLabelShown(p_id)
	{
	}

	function onButtonMenuPressed()
	{
	}

	function onButtonUpPressed()
	{
	}

	function onButtonDownPressed()
	{
	}

	function onButtonAcceptPressed()
	{
	}

	function onButtonCancelPressed()
	{
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Inherited Methods

	function create()
	{
		if (c_displayStaticAtStart)
		{
			createPresentation("static_start", "hud_static");
			removePresentation("static_start");
		}

		// Don't call base. Custom background here
		customCallback("onScriptItemProcessed", "start");

		createPresentation("grid", "hud_grid");
		createPresentation("scanlines", "hud_scanlines");
		createPresentation("reflection_light", "hud_reflection_light");
	}
}
