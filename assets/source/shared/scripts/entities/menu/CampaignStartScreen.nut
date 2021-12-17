include_entity("menu/MenuConsoleWindow");
include_entity("menu/IngameMenu");

class CampaignStartScreen extends IngameMenu
</
	placeable      = Placeable_Hidden
	collisionRect  = [ 0.0, 1.0, 2.0, 2.0 ]  // center X, center Y, width, height
/>
{
	static c_colorGradingTexture  = "ingamemenu"; // Required
	static c_musicTrack           = null;         // Optional
	static c_displayStaticAtStart = false;
	static c_yOffset              = 0.17;

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
			::Audio.playGlobalSoundEffect("Effects", "intro_asteroid_begin");
			::Audio.playGlobalSoundEffect("Effects", "intro_asteroid_atmo");
			::Audio.playGlobalSoundEffect("Effects", "intro_noise_alarm");
			//::Audio.playGlobalSoundEffect("Effects", "introsequence_asteroidfield");
			addSubtitles([
				// [delay, locID]
				[1.5, "01_WAKE_A_RS"],
				[0.5, "01_WAKE_C_RS"],
				[1.75, "01_BOOT_A_RS"],
				[2.25, "01_CHECKUP_A_RS"],
				[0.3, "01_CHECKUP_B_RS"],
				[0.3, "01_CHECKUP_C_RS"],
				[6.5, "01_SELFIE_RS"],
				[2.5, "01_SPACEROCK_A_RS"]
			]);
			createPresentation("static", "hud_static");
			::setRumble(RumbleStrength_High, 0.0);
			createDelayedPresentation(0.0, "alarm", "campaignstartscreen_alarm");
			removePresentation("static", 8.5);
			startScriptItemTimer("rumble_medium", 8.75);
			createDelayedPresentation(8.5, "bootscreen", "campaignstartscreen");
			createDelayedPresentation(8.5, "loadingbar", "campaignstartscreen");
			startScriptItemTimer("loadingbarSound", 8.5);
			createDelayedPresentation(8.6, "flash", "campaignstartscreen_flash");
			createDelayedPresentation(8.6, "asteroid_loading", "campaignstartscreen_asteroid_loading");
			removePresentation("bootscreen", 12.5);
			removePresentation("loadingbar", 12.5);
			startScriptItemTimer("showInfo", 12.75);
			startScriptItemTimer("telemetrySound", 13.52);
			startScriptItemTimer("showConsoleWindow", 22);
			startScriptItemTimer("showSelfieCamera", 27.0);
			startScriptItemTimer("showAsteroidCamera", 32);
		}
		else if (p_id == "rumble_medium")
		{
			::setRumble(RumbleStrength_Medium, 0.0);
		}
		else if (p_id == "bootscreen")
		{
			// Create labels
			createTitle("PAUSEMENU_TITLE");
		}
		else if (p_id == "showInfo")
		{
			::Audio.playGlobalSoundEffect("Effects", "intro_telemetry");

			// Create all info categories
			local tags = ["singleplayer"];
			createDelayedPresentation(0.0, "spiderbot", "pausemenu", ::Vector2(0, c_yOffset), HudAlignment.Center, tags);
			createDelayedText        (0.1, "health_header",    "PAUSEMENU_HEALTH_HEADER",    ::Vector2(-0.27, -0.065 + c_yOffset), ::TextColors.Light, HorizontalAlignment_Right);
			createDelayedText        (0.2, "health",           "CAMPAIGNSTART_HEALTH",       ::Vector2(-0.27, -0.105 + c_yOffset), ::TextColors.Dark,  HorizontalAlignment_Right);
			createDelayedText        (0.3, "magnet_header",    "PAUSEMENU_MAGNET_HEADER",    ::Vector2(-0.32, -0.162 + c_yOffset), ::TextColors.Light, HorizontalAlignment_Right);
			createDelayedText        (0.4, "magnet",           "CAMPAIGNSTART_MAGNET",       ::Vector2(-0.32, -0.202 + c_yOffset), ::TextColors.Dark,  HorizontalAlignment_Right);
			createDelayedText        (0.5, "fuel_header",      "PAUSEMENU_FUEL_HEADER",      ::Vector2(-0.34, -0.260 + c_yOffset), ::TextColors.Light, HorizontalAlignment_Right);
			createDelayedPresentation(0.6, "fuel", "campaignstartscreen", ::Vector2(-0.45, -0.3 + c_yOffset));
			createDelayedText        (0.7, "primary_header",   "PAUSEMENU_PRIMARY_HEADER",   ::Vector2( 0.27, -0.065 + c_yOffset), ::TextColors.Light, HorizontalAlignment_Left);
			createDelayedText        (0.8, "primary",          "CAMPAIGNSTART_PRIMARY",      ::Vector2( 0.27, -0.105 + c_yOffset), ::TextColors.Dark,  HorizontalAlignment_Left);
			createDelayedText        (0.9 "secondary_header", "PAUSEMENU_SECONDARY_HEADER", ::Vector2( 0.32, -0.162 + c_yOffset), ::TextColors.Light, HorizontalAlignment_Left);
			createDelayedText        (1.0, "secondary",        "CAMPAIGNSTART_SECONDARY",    ::Vector2( 0.32, -0.202 + c_yOffset), ::TextColors.Dark,  HorizontalAlignment_Left);
			createDelayedText        (1.1, "hackbeam_header",  "PAUSEMENU_HACKBEAM_HEADER",  ::Vector2( 0.34, -0.260 + c_yOffset), ::TextColors.Light, HorizontalAlignment_Left);
			createDelayedText        (1.2, "hackbeam",         "CAMPAIGNSTART_HACKBEAM",     ::Vector2( 0.34, -0.305 + c_yOffset), ::TextColors.Dark,  HorizontalAlignment_Left);
		}
		else if (p_id == "telemetrySound")
		{
			::Audio.playGlobalSoundEffect("Effects", "intro_telemetry");
		}
		else if (p_id == "showConsoleWindow")
		{
			local window = createConsoleWindow("console", ::Vector2(-0.23, 0.075),
			{
				_titleID   = "CONSOLE_HEADER",
				_width   = 0.9,
				_zOffset = 0.2
				_lineCount = 7,
				_parent    = this
			});

			window.addCommands([
				// [startdelay, command type, locsheet id, enddelay]
				[1.0, ConsoleCommandType.Command, "01_CONSOLE_LOGINCOMMAND",       0.0],
				[0.2, ConsoleCommandType.Result,  "01_CONSOLE_LOGINRESULT",        0.3],
				[0.0, ConsoleCommandType.Result,  null,                            0.0],
				[0.5, ConsoleCommandType.Command, "01_CONSOLE_CAMERACOMMAND",      0.0],
				[0.2, ConsoleCommandType.Result,  "01_CONSOLE_CAMERARESULT",       0.3],
				[0.0, ConsoleCommandType.Result,  null,                            0.0],
				[3.5, ConsoleCommandType.Command, "01_CONSOLE_VIEWCOMMAND",        0.0],
				[0.2, ConsoleCommandType.Result,  "01_CONSOLE_VIEWRESULT",         0.3],
				[0.0, ConsoleCommandType.Result,  null,                            0.0],
				[4.0, ConsoleCommandType.Command, "01_CONSOLE_SCROLLINGCOMMAND",   0.0],
				[0.2, ConsoleCommandType.Result,  "01_CONSOLE_SCROLLINGRESULT",    0.0],
				[0.0, ConsoleCommandType.Result,  null,                            0.0],
			]);
		}
		else if (p_id == "loadingbarSound")
		{
			::Audio.playGlobalSoundEffect("Effects", "intro_loading");
		}
		else if (p_id == "showSelfieCamera")
		{
			local window = createWindow("video", ::Vector2(0.42, -0.05),
			{
				_titleID = "VIRUSPICKUPSCREEN_VIDEOFEED",
				_width   = 0.4,
				_height  = 0.36,
				_zOffset = 0.25,
			_parent = this
			});
			_windows["video"].createPresentation("video", "viruspickupscreen", ::Vector2(0, -0.025));
			_windows["video"]._presentations["video"].start("appear", ["roughshot"], false, 0);
		}
		else if (p_id == "showAsteroidCamera")
		{
			_windows["video"]._presentations["video"].start("appear", ["asteroid_camera"], false, 0);
		}
		else if (p_id == "01_CONSOLE_SCROLLINGRESULT")
		{
			startScriptItemTimer("close", 1.25);
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
		// Don't call base. Custom background here
		customCallback("onScriptItemProcessed", "start");

		createPresentation("grid", "hud_grid");
		createPresentation("scanlines", "hud_scanlines");
		createPresentation("reflection_light", "hud_reflection_light");
	}
}
