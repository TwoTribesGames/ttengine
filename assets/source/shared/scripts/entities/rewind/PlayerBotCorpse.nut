include_entity("Corpse");

class PlayerBotCorpse extends Corpse
</
	placeable      = Placeable_Hidden
	movementset    = "Static"
/>
{
	_isPermanent    = true;
	
	_restartEnabled = false;
	_messageID      = null;
	_stats          = null;
	_fade           = null;
	_scoreContainer = null;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		base.onInit();
		
		addProperty("openDoors");
		::removeEntityFromWorld(this);
		_scoreContainer._parent = this.weakref();
		
		if (::ProgressMgr.getGameMode() == GameMode.SingleLife)
		{
			::ProgressMgr.signalPlayerKilled();
		}
	}
	
	function onSpawn()
	{
		base.onSpawn();
		
		::addButtonInputListeningEntity(this, InputPriority.Normal);
		
		::Camera.setScrollingEnabled(false);
		::Camera.setFollowEntity(this);
		
		setRumbleEnabled(false);
		
		//gets deleted as the hud is recreated on respawn
		local gameoverTextLabel = ::Hud.createTextElement(this,
		{
			locID               = _messageID,
			presentation        = "hud_gameover"
			width               = 0.85,
			height              = 0.1,
			glyphset            = GlyphSetID_Header,
			hudalignment        = HudAlignment.Center,
			position            = ::Vector2(0.0, 0.0),
			color               = ColorRGBA(55,55,55,255),
			horizontalAlignment = HorizontalAlignment_Center,
			verticalAlignment   = VerticalAlignment_Center,
			autostart           = true,
			layer               = HudLayer.Death,
		});
		gameoverTextLabel.presentation.start("gameover", [], false, 0);
		
		local tipID = ::rnd_minmax(1, 19);
		local buttonpromptTextLabel = ::Hud.createTextElement(this,
		{
			locID               = "GAME_OVER_TIPS_" + tipID.tostring(),
			presentation        = "hud_gameover",
			width               = 1.5,
			height              = 0.1,
			glyphset            = GlyphSetID_Text,
			hudalignment        = HudAlignment.Center,
			position            = ::Vector2(0.0, 0.0),
			color               = ColorRGBA(255,255,255,255),
			horizontalAlignment = HorizontalAlignment_Center,
			verticalAlignment   = VerticalAlignment_Bottom,
			autostart           = true,
			layer               = HudLayer.Death,
		});
		buttonpromptTextLabel.presentation.start("buttonprompt", [], false, 0);
		
		startTimer("enable_restart", 0.3);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onButtonRespawnPressed()
	{
		restart();
	}
	
	function onFadeEnded(p_fade, p_animation)
	{
		if (p_fade.equals(_fade))
		{
			::killEntity(this);
			::ProgressMgr.signalPlayerKilled();
		}
	}
	
	function onTimer(p_name)
	{
		if (p_name == "enable_restart")
		{
			_restartEnabled = true;
		}
		else if (p_name == "create_fade")
		{
			fadeOutAndSignalExit();
		}
	}
	
	function onProgressRestored(p_id)
	{
		if (p_id != "editor" && p_id != "recorder")
		{
			// don't provide a parent for the fade because we're not interested in when it ended (yet?)
			createFade(null, "transparent_to_opaque");
		}
	}
	
	function onDie()
	{
		::removeButtonInputListeningEntity(this);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods
	
	function fadeOutAndSignalExit()
	{
		if (_fade == null)
		{
			_fade = createPersistentFade(this, "transparent_to_opaque").weakref();
		}
	}
	
	function restart()
	{
		if (_restartEnabled)
		{
			// Make sure a potential bullettime instance is gone before we do the fade
			BulletTime.remove();
			_restartEnabled = false;
			
			if (::ProgressMgr.getGameMode() == GameMode.SingleLife)
			{
				_scoreContainer.onEndMission(::Level.getMissionID());
			}
			else
			{
				local playMode = ::ProgressMgr.getPlayMode();
				if (playMode == PlayMode.BattleArena || playMode == PlayMode.Challenge)
				{
					local id = playMode == PlayMode.BattleArena ? ::Level.getName() : ::Level.getMissionID();
					_scoreContainer.onEndMission(id);
					startTimer("create_fade", 0.05); // start a bit later so menu has a chance to be created first
				}
				else
				{
					fadeOutAndSignalExit();
				}
			}
		}
	}
	
	function getActualStats()
	{
		return _stats;
	}
}
PauseMenu.makePauseMenuOpener(PlayerBotCorpse);
