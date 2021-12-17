include_entity("menu/Menu");

class IngameMenu extends Menu
</
	placeable      = Placeable_Hidden
	collisionRect  = [ 0.0, 0.5, 1.0, 1.0 ]  // center X, center Y, width, height
/>
{
	static c_colorGradingTexture  = null; // Required
	static c_musicTrack           = null; // Optional
	static c_displayStaticAtStart = true;
	static c_displayStaticAtEnd   = true;
	static c_voiceOverEndFadeTime = 1.0;
	static c_scriptItemPrefix     = "s#";
	static c_showCrackedScreenAtDeath = true;
	
	_isPlayerDead       = null;
	_subtitles          = null;
	_currentSubtitleIdx = 0;
	_voiceOverEffect    = null;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		// Notify activator that ingame menu is about to be opened.
		_activator.customCallback("onIngameMenuOpened", this);
		
		// Init this before the onInit
		_subtitles = [];
		
		if (_activator == null)
		{
			::tt_panic("IngameMenu activator should not be null");
			close();
			return;
		}
		
		if (_isPlayerDead == null)
		{
			_isPlayerDead = (_activator instanceof ::PlayerBotCorpse);
		}
		
		base.onInit();
		
		// Handle audio a bit later to make sure audio caused by queued game callbacks are also silenced.
		// Be sure not to make this value too small, otherwise the static sfx might be cut off
		startTimer("handleAudio", 0.2);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Script Functionality
	
	function onScriptItemProcessed(p_id)
	{
		// Implement this
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
	
	function onTimer(p_name)
	{
		if (p_name == "handleAudio")
		{
			::Audio.initVolumes();
			
			// And just to be sure to pause all music/sfx as some callbacks might have started new looping sounds.
			::MusicSource.pauseAll();
			::Audio.pauseCategory("Music");
			::Audio.pauseCategory("Effects");
			::Audio.pauseCategory("Ambient");
			createMusic();
			return;
		}
		
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
		else if (p_name == "closeBase")
		{
			base.close();
		}
	}
	
	function onDie()
	{
		base.onDie();
		
		// Notify activator that menu is closed.
		_activator.customCallback("onIngameMenuClosed", this);
		::Audio.restoreVolumes();
	}
	
	function onButtonMenuPressed()
	{
		close();
	}
	
	function onButtonCancelPressed()
	{
		close();
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
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Inherited Methods
	
	function close()
	{
		if (_voiceOverEffect != null)
		{
			_voiceOverEffect.fadeAndStop("Volume", 0.0, c_voiceOverEndFadeTime);
			_voiceOverEffect = null;
		}
		
		if (c_displayStaticAtEnd)
		{
			if (hasPresentation("static_end") == false)
			{
				createPresentation("static_end", "hud_static");
				startTimer("closeBase", 0.12);
			}
		}
		else
		{
			base.close();
		}
	}
	
	function closeAll(p_force = true)
	{
		cleanupStatic();
		base.closeAll(p_force);
	}
	
	function create()
	{
		base.create();
		
		if (_isPlayerDead && c_showCrackedScreenAtDeath)
		{
			createPresentation("cracked_screen", "hud_cracked_screen");
		}
		else
		{
			createPresentation("reflection_body", "hud_reflection", ::Vector2(0, 0), HudAlignment.Center, ["body"]);
			createPresentation("reflection_head", "hud_reflection", ::Vector2(0, 0), HudAlignment.Center, ["head"]);
			createPresentation("reflection_toothbrush", "hud_reflection", ::Vector2(0, 0), HudAlignment.Center, ["toothbrush"]);
		}
		
		if (c_displayStaticAtStart)
		{
			createPresentation("static_start", "hud_static");
			removePresentation("static_start");
		}
	}
	
	function getColorGradingTexture()
	{
		return _isPlayerDead ? c_colorGradingTexture + "_dead" : c_colorGradingTexture;
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods
	
	function cleanupStatic()
	{
		if (hasPresentation("static_start"))
		{
			::Hud.destroyElement(_presentations["static_start"]);
			delete _presentations["static_start"];
			stopTimer(c_removeTimeoutPrefix + "static_start");
		}
		
		if (hasPresentation("static_end"))
		{
			::Hud.destroyElement(_presentations["static_end"]);
			delete _presentations["static_end"];
			stopTimer(c_removeTimeoutPrefix + "static_end");
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Presentation Methods
	
	function hide()
	{
		cleanupStatic();
		base.hide();
	}
	
	function addPresentationTags(p_pres, p_id = null, p_extraTags = null)
	{
		base.addPresentationTags(p_pres, p_id, p_extraTags);
		
		p_pres.addTag(::Level.getMissionID());
		if (_isPlayerDead)
		{
			p_pres.addTag("dead");
		}
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
