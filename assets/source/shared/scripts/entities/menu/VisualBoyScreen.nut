include_entity("menu/MenuConsoleWindow");
include_entity("menu/IngameMenu");

class VisualBoyScreen extends MenuScreen
</
	placeable      = Placeable_Hidden
	collisionRect  = [ 0.0, 1.0, 2.0, 2.0 ]  // center X, center Y, width, height
/>
{
	static c_rom                  = "roms/ThreeTribes_8409.gba";
	static c_emulatorVolume       = 0.2;
	static c_emulatorFilter       = "Stretch 4x";
	
	static c_yOffset              = 0.17;
	static c_voiceOverEndFadeTime = 1.0;
	static c_scriptItemPrefix     = "s#";
	
	_currentSubtitleIdx = 0;
	_subtitles          = null;
	
	_voiceOverEffect    = null;
	_logWindow          = null;
	_consoleWindow      = null;
	_controlsWindow     = null;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		// Init this before the onInit
		_subtitles = [];
		
		base.onInit();
		
		if (::g_menuInstance != null && ::g_menuInstance._playingMusicTrack != null)
		{
			::g_menuInstance._playingMusicTrack.stop();
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Script Item Handling
	
	function onScriptItemProcessed(p_id)
	{
		if (hasElement("facebutton_rom_back"))
		{
			return;
		}
		local prefix = "VISUALBOY_";
		if (p_id == "start")
		{
			createPresentation("bootscreen", "campaignstartscreen");
			createPresentation("loadingbar", "campaignstartscreen");
			::Audio.playGlobalSoundEffect("Effects", "intro_loading");
			addSubtitles([
				[0.0, "SECRET_PICKUP_2_RS"]
			]);
		}
		else if (p_id == "SECRET_PICKUP_2_RS_done")
		{
			createFaceButtons(["skip"]);
			_logWindow = createWindow("log", ::Vector2(0.0, 0.0),
			{
				_titleID = "RESULTSCREEN_LOG",
				_width   = 0.95,
				_height  = 0.55,
				_zOffset = 0.08,
				_parent = this
			});			
			_logWindow.createTypedTextArea("log", "VISUALBOY_LOG", ::Vector2(0.0, 0.0), ::TextColors.Light, 1.0, 1.0);
		}
		else if (p_id == "log")
		{
			if (hasElement("facebutton_skip"))
			{
				removeElement("facebutton_skip");
			}
			createFaceButtons(["next"]);
		}		
		else if (p_id == "openConsole")
		{
			if (hasElement("facebutton_next"))
			{
				removeElement("facebutton_next");
			}
			createFaceButtons(["skip"]);
			
			_consoleWindow = createConsoleWindow("console", ::Vector2(-0.14, -0.1125),
			{
				_titleID   = "CONSOLE_HEADER",
				_width   = 0.85,
				_zOffset = 0.1
				_lineCount = 12,
				_parent    = this
			});
			_consoleWindow.addCommands([
				// [startdelay, command type, locsheet id, enddelay]
				[1.0, ConsoleCommandType.Command, prefix + "CONSOLE_COMMAND_A", 0.0],
				[0.2, ConsoleCommandType.Result,  prefix + "CONSOLE_RESULT_A" , 0.3],
				[0.0, ConsoleCommandType.Result,  null,                         0.0],
				[1.0, ConsoleCommandType.Command, prefix + "CONSOLE_COMMAND_B", 0.0],
				[0.2, ConsoleCommandType.Result,  prefix + "CONSOLE_RESULT_B" , 0.3],
				[0.0, ConsoleCommandType.Result,  null,                         0.0]
			]);
		}
		else if (p_id == (prefix + "CONSOLE_RESULT_B"))
		{
			addSubtitles([
				[1.0, "08_BLOCKINGCAP_RS"]
			]);
		}
		else if (p_id == "08_BLOCKINGCAP_RS_done")
		{
			_consoleWindow.addCommands([
				// [startdelay, command type, locsheet id, enddelay]
				[1.0, ConsoleCommandType.Command, prefix + "CONSOLE_COMMAND_C", 0.0],
				[0.2, ConsoleCommandType.Result,  prefix + "CONSOLE_RESULT_C" , 0.3],
				[0.0, ConsoleCommandType.Result,  null,                         0.0]
			]);
		}
		else if (p_id == (prefix + "CONSOLE_RESULT_C"))
		{
			addSubtitles([
				[1.0, "01_UH_A_RS"]
			]);
		}
		else if (p_id == "01_UH_A_RS_done")
		{
			_consoleWindow.addCommands([
				// [startdelay, command type, locsheet id, enddelay]
				[0.5, ConsoleCommandType.Command, prefix + "CONSOLE_COMMAND_D", 0.0],
				[0.2, ConsoleCommandType.Result,  prefix + "CONSOLE_RESULT_D" , 0.3],
				[0.0, ConsoleCommandType.Result,  null,                         1.0]
			]);
		}
		else if (p_id == (prefix + "CONSOLE_RESULT_D"))
		{
			addSubtitles([
				[1.0, "SWEET_RS"]
			]);
			startScriptItemTimer("showControls", 1.5);
			startScriptItemTimer("showVideo", 2.0);
		}
		else if (p_id == "showControls")
		{
			_controlsWindow = createWindow("controls", ::Vector2(0.57, 0.18),
			{
				_titleID = prefix + "CONTROLS_HEADER",
				_width   = 0.45,
				_height  = 0.20,
				_zOffset = 0.29,
				_parent = this
			});
			local textID = prefix + "CONTROLS_" + ::getPlatformString().toupper();
			_controlsWindow.createDelayedTextArea(0.0, "usage", textID,::Vector2(0, 0), ::TextColors.Light, 2.8, 0.3);
		}
		else if (p_id == "showVideo")
		{
			local window = createWindow("video", ::Vector2(-0.15, 0.0),
			{
				_titleID = prefix + "VIDEO_HEADER",
				_width   = 1.0,
				_height  = 0.666,
				_zOffset = 0.25,
				_parent = this
			});
			window.createPresentation("video", "visualboyscreen", ::Vector2(0.0, -0.025));
			startScriptItemTimer("loadROM", 0.1);
		}
		else if (p_id == "loadROM")
		{
			onButtonFaceUpPressed();
			
			if (hasElement("facebutton_skip"))
			{
				removeElement("facebutton_skip");
			}
			createFaceButtons(["rom_back"]);
		}
	}
	
	function startScriptItemTimer(p_name, p_delay)
	{
		startTimer(c_scriptItemPrefix + p_name, p_delay);
	}
	
	function stopScriptItemTimer(p_name)
	{
		stopTimer(c_scriptItemPrefix + p_name);
	}	
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onConsoleWindowResultShown(p_window, p_id)
	{
		// Forward to a single point of entrance
		customCallback("onScriptItemProcessed", p_id);
	}
	
	function onConsoleWindowCommandTyped(p_window, p_id)
	{
		// Forward to a single point of entrance
		customCallback("onScriptItemProcessed", p_id);
	}
	
	function onWindowTextLabelTyped(p_window, p_id)
	{
		// Forward to a single point of entrance
		// Note the ID is the element ID, not the text id
		customCallback("onScriptItemProcessed", p_id);
	}
	
	function onDelayedPresentationShown(p_id)
	{
		// Forward to a single point of entrance
		customCallback("onScriptItemProcessed", p_id);
	}
	
	function onDie()
	{
		::VisualBoy.unload();
		
		if (::g_menuInstance != null && ::g_menuInstance._playingMusicTrack != null)
		{
			::g_menuInstance._playingMusicTrack.play();
		}		
		base.onDie();
	}
	
	function onButtonAcceptPressed()
	{
		if (_controlsWindow != null)
		{
			return;
		}
		
		// skipping console
		if (_consoleWindow != null)
		{
			trace();
			stopAllTimers();
			_consoleWindow.stopAllTimers();
			if (_controlsWindow == null)
			{
				startScriptItemTimer("showControls", 0.0);
			}
			startScriptItemTimer("showVideo", 0.5);
		}
		else if (_logWindow != null)
		{
			if (hasElement("facebutton_next"))
			{
				startScriptItemTimer("openConsole", 0.1);
			}
			else
			{
				_logWindow.instantlyShowAllElements();
				_logWindow.stopAllTimers();
			}
		}
	}
	
	function onButtonCancelPressed()
	{
		// No exit possible
	}
	
	function onButtonMenuPressed()
	{
		::VisualBoy.unload();
		
		base.onButtonMenuPressed();
	}
	
	function onButtonFaceUpPressed()
	{
		::VisualBoy.unload();
		::VisualBoy.load(c_rom);
		::VisualBoy.setSoundVolume(c_emulatorVolume);
		::VisualBoy.setFilter(c_emulatorFilter);
	}
	
	function onSubtitleShown(p_id)
	{
		_voiceOverEffect = ::Audio.playGlobalSoundEffect("VoiceOver", p_id);
		if (_voiceOverEffect.isPlaying() == false)
		{
			_voiceOverEffect = null;
			::tt_warning("Invalid subtitle voiceover '" + p_id + "'");
			startTimer("nextSubtitle", 2.0);
		}
		
		// Forward to a single point of entrance
		customCallback("onScriptItemProcessed", p_id);
	}
	
	function onSubtitleFinished(p_id)
	{
		// Forward to a single point of entrance
		customCallback("onScriptItemProcessed", p_id + "_done");
	}	
	
	function onTimer(p_name)
	{
		if (p_name.len() > 2)
		{
			local action = p_name.slice(0, 2);
			local target = p_name.slice(2);
			
			switch (action)
			{
			case c_scriptItemPrefix:
				customCallback("onScriptItemProcessed", target);
				return;
			
			default:
				// Don't do anything
				break;
			}
		}
		
		base.onTimer(p_name);
		
		if (p_name == "nextSubtitle")
		{
			local id = _subtitles[_currentSubtitleIdx][1];
			customCallback("onSubtitleFinished", id);
			removeLabel("sub_" + id);
			++_currentSubtitleIdx;
			if (_currentSubtitleIdx < _subtitles.len())
			{
				createSubtitle();
			}
		}
		else if (p_name == "showSubtitle")
		{
			local id = _subtitles[_currentSubtitleIdx][1];
			local label = createLabel("sub_" + id, id, ::Vector2(0, -0.38), ::TextColors.White, 1.1, 0.1, GlyphSetID_Text,
			                          HorizontalAlignment_Center,
			                          VerticalAlignment_Center, ["subtitle"]);
			label.presentation.setCustomZOffset(0.40); // Cannot use 0.49 in Fullscreen?!
			label.presentation.setCustomUniformScale(1.25);
			customCallback("onSubtitleShown", id);
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Inherited Methods
	
	function close()
	{
		if (_voiceOverEffect != null)
		{
			_voiceOverEffect.fadeAndStop("Volume", 0.0, c_voiceOverEndFadeTime);
			_voiceOverEffect = null;
		}
		base.close();
	}
	
	function create()
	{
		customCallback("onScriptItemProcessed", "start");
		base.create();
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Subtitle Methods
	
	function createSubtitle()
	{
		local delay = _subtitles[_currentSubtitleIdx][0];
		startTimer("showSubtitle", delay);
	}
	
	function addSubtitles(p_subtitles)
	{
		_subtitles.extend(p_subtitles);
		if (_voiceOverEffect == null)
		{
			createSubtitle();
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Update
	
	function update(p_deltaTime)
	{
		base.update(p_deltaTime);
		
		if (_voiceOverEffect != null && _voiceOverEffect.isPlaying() == false)
		{
			// Subtitle VO finished; move to next
			_voiceOverEffect = null;
			onTimer("nextSubtitle");
		}
	}
}
