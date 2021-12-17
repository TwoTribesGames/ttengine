include_entity("rewind/EntityChild");

class ScoreContainer extends EntityChild
</
	placeable      = Placeable_Hidden
	movementset    = "Static"
	collisionRect  = [ 0.0, 0.0, 0.1, 0.1 ]
	group          = "Rewind"
/>
{
	static c_statsTable =
	{
		time = 0,
		score = 0,
		kills = 0,
		killed = 0,
		killStreak = 0,
		energy = 0,
		multikill = 0,
		primaryFired = 0,
		secondaryFired = 0,
		suicides = 0,
		healthLost = 0,
		hitCount = 0,
		accuracy = 0,
		cockroaches = 0
	};

	// Constants
	static c_multiplierTimeout = 5.0;    // in seconds
	static c_multikillTimeout  = 0.08;   // in seconds
	static c_multikillBonus    = 250;    // Added score = c_multikillBonus * kills * kills
	static c_multikillMaxBonus = 30000; // Maximum added bonus
	static c_maxMultiplier     = 20;
	static c_maxMultikills     = 7;

	// Store / Restore Constants
	static c_internalsStoredOnLevelExit      = ["_gameStats", "_currentKillStreak", "_multiplier", "_lapTimes", "_lapIndex", "_restartMissionStats"];
	static c_internalsStoredOnMissionRestart = ["_gameStats.campaign.time",
	                                            "_gameStats.campaign.killed",
	                                            "_gameStats.campaign.suicides",
	                                            "_gameStats.campaign.healthLost"];
	static c_internalsStoredOnDeath          = ["_gameStats.campaign.time", "_gameStats.mission.time",
	                                            "_gameStats.campaign.killed", "_gameStats.mission.killed",
	                                            "_gameStats.campaign.suicides", "_gameStats.mission.suicides",
	                                            "_gameStats.campaign.healthLost", "_gameStats.mission.healthLost"];
	static c_internalsStoredForUpdate        = ["_gameStats", "_restartMissionStats", "_currentKillStreak", "_multiplier", "_lapTimes", "_lapIndex"];

	_displayScore    = 0; // Score that is displayed
	_multiplier      = 1;

	_gameStats           = null;
	_restartMissionStats = null;
	_resultScreenStats   = null;

	//Text labels
	_scoreLabel        = null;
	_multiplierLabel   = null;
	_multikillLabel    = null;

	_currentKillStreak = 0;
	_currentMultikill  = 0;
	_lapIndex          = -1;
	_lapTimes          = null;
	_resultScreen      = null;

	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn

	function onInit()
	{
		_lapTimes = [];

		if (_gameStats == null)
		{
			_gameStats =
			{
				campaign = clone c_statsTable,
				mission  = clone c_statsTable
			}
		}

		local gameMode = ::ProgressMgr.getGameMode();
		local playMode = ::ProgressMgr.getPlayMode();
		if (playMode == PlayMode.Campaign && _restartMissionStats == null)
		{
			_restartMissionStats =
			{
				missionid = null,
				campaign  = clone c_statsTable,
				mission   = clone c_statsTable
			}
		}

		if (playMode == PlayMode.Mission && gameMode == GameMode.SingleLife)
		{
			// Find first lapindex for this mission
			_lapIndex = ::ProgressMgr.getSingleLifeMissionStartIndex(::Level.getMissionID());
		}

		base.onInit();

		_scoreLabel = ::Hud.createTextElement(this,
		{
			locID                = "",
			width                = 0.30,
			height               = 0.05,
			glyphset             = GlyphSetID_Text,
			hudalignment         = HudAlignment.Left,
			position             = ::Vector2(0.066, 0.40),
			color                = ColorRGBA(255,255,255,115),
			horizontalAlignment  = HorizontalAlignment_Left,
			autostart            = true,
			layer                = HudLayer.Normal | HudLayer.VirusUploadMode | HudLayer.Death
		});

		// Multiplier
		if (gameMode != GameMode.SpeedRun)
		{
			_multiplierLabel = ::Hud.createTextElement(this,
			{
				locID                = "",
				width                = 0.18,
				height               = 0.05,
				glyphset             = GlyphSetID_Text,
				hudalignment         = HudAlignment.Left,
				position             = ::Vector2(0.066, 0.365),
				color                = ColorRGBA(255,255,255,230),
				presentation         = "hud_score_multiplier",
				horizontalAlignment  = HorizontalAlignment_Left,
				scale                = 0.9,
				autostart            = false,
				layer                = HudLayer.Normal | HudLayer.VirusUploadMode | HudLayer.Death
			});
			_multiplierLabel.presentation.addCustomValue("multiplierMaxTimer" , c_multiplierTimeout);

			// Killstreak
			_multikillLabel = ::Hud.createTextElement(this,
			{
				locID                = "",
				width                = 0.35,
				height               = 0.08,
				glyphset             = GlyphSetID_Text,
				hudalignment         = HudAlignment.Left,
				position             = ::Vector2(0.066, 0.35),
				color                = ColorRGBA(255,255,255,255),
				presentation         = "hud_score_multikill",
				horizontalAlignment  = HorizontalAlignment_Left,
				scale                = 1.3,
				autostart            = false,
				layer                = HudLayer.Normal | HudLayer.VirusUploadMode | HudLayer.Death
			});
		}

		updateMultiplier();
		updateScoreLabel();

		_displayScore = _gameStats.campaign.score;
		if (_multiplier > 1)
		{
			startCallbackTimer("stopMultiplier", c_multiplierTimeout);
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks

	function onReloadRequested()
	{
		::_perm_table.bulletHitCount = 0; // reset the global here
	}

	function onLevelExit()
	{
		stopMultikill();

		base.onLevelExit();
	}

	function onStartMission(p_missionID)
	{
		base.onStartMission(p_missionID);

		stopMultiplier();
		stopMultikill();

		::_perm_table.bulletHitCount = 0; // reset the global here

		_gameStats.mission = clone c_statsTable;

		if (::ProgressMgr.getPlayMode() == PlayMode.Campaign)
		{
			::null_assert(_restartMissionStats);
			if (_restartMissionStats.missionid != p_missionID)
			{
				// Started a new mission; sync
				_restartMissionStats.missionid = p_missionID;
				_restartMissionStats.campaign  = _gameStats.campaign;
			}
			else
			{
				// Restarted a mission, replace values with the ones that were saved earlier
				_gameStats.campaign = _restartMissionStats.campaign;
			}
		}
		else
		{
			_gameStats.campaign.score = _gameStats.mission.score;
		}
		_lapTimes.clear();
		_displayScore = _gameStats.campaign.score;
	}

	function getActualStats()
	{
		local gameStats =
		{
			campaign = clone _gameStats.campaign,
			mission  = clone _gameStats.mission
		};

		gameStats.mission.hitCount = ::_perm_table.bulletHitCount;
		gameStats.campaign.hitCount += ::_perm_table.bulletHitCount;

		gameStats.mission.accuracy  = gameStats.mission.primaryFired > 0 ?
		                              gameStats.mission.hitCount / gameStats.mission.primaryFired.tofloat() : 1.0;
		gameStats.campaign.accuracy = gameStats.campaign.primaryFired > 0 ?
		                              gameStats.campaign.hitCount / gameStats.campaign.primaryFired.tofloat() : 1.0;

		local cockroaches = getCockroachCount();
		gameStats.mission.cockroaches = cockroaches;
		gameStats.campaign.cockroaches = cockroaches;

		return gameStats;
	}

	function onProgressRestored(p_id)
	{
		// Since deathcount is always reset when restoring from menu, we can simply check
		// if player died if deathCount > 0
		base.onProgressRestored(p_id);

		if (::ProgressMgr.getLastCheckPointDeathCount() > 0)
		{
			stopMultiplier();
			_gameStats.mission.score -= 20000;
			_gameStats.campaign.score -= 20000;
			if (_gameStats.mission.score < 0) _gameStats.mission.score = 0;
			if (_gameStats.campaign.score < 0) _gameStats.campaign.score = 0;
		}
	}

	function onDie()
	{
		++_gameStats.mission.killed;
		++_gameStats.campaign.killed;

		base.onDie();

		::Hud.destroyTextElement(_scoreLabel);
		::Hud.destroyTextElement(_multiplierLabel);
		::Hud.destroyTextElement(_multikillLabel);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods

	function getCockroachCount()
	{
		return ::getEntitiesByTag("Cockroach").len();
	}

	function updateMultiplier()
	{
		if (_multiplierLabel == null)
		{
			return;
		}

		if (_multiplier > 1)
		{
			_multiplierLabel.label.setText(_multiplier + "x");
			_multiplierLabel.presentation.stop();
			_multiplierLabel.presentation.start("", [], false, 0);
		}
		else
		{
			_multiplierLabel.label.setText("");
			_multiplierLabel.presentation.stop();
		}
	}

	function updateScoreLabel()
	{
		local result = (::ProgressMgr.getGameMode() == GameMode.SpeedRun) ?
			::formatTime(_gameStats.campaign.time) : ::formatScore(_displayScore);

		_scoreLabel.label.setText(result);
	}

	function addKillScore(p_score)
	{
		++_gameStats.campaign.kills;
		++_gameStats.mission.kills;

		if (::Game.isFarmingEnabled())
		{
			return 0;
		}

		local addedScore = ::ceil(p_score * _multiplier);
		_gameStats.campaign.score += addedScore;
		_gameStats.mission.score += addedScore;

		++_currentMultikill;
		++_currentKillStreak;
		if (_currentKillStreak > _gameStats.campaign.killStreak)
		{
			_gameStats.campaign.killStreak = _currentKillStreak;
		}
		if (_currentKillStreak > _gameStats.mission.killStreak)
		{
			_gameStats.mission.killStreak = _currentKillStreak;
		}
		if (_currentMultikill > _gameStats.campaign.multikill)
		{
			_gameStats.campaign.multikill = _currentMultikill;
		}
		if (_currentMultikill > _gameStats.mission.multikill)
		{
			_gameStats.mission.multikill = _currentMultikill;
		}

		if (_multiplier < c_maxMultiplier)
		{
			++_multiplier;
		}
		startCallbackTimer("stopMultiplier", c_multiplierTimeout);
		startCallbackTimer("stopMultikill", c_multikillTimeout);

		updateMultiplier();

		return addedScore;
	}

	function addEnergyScore(p_energy)
	{
		_gameStats.mission.energy += p_energy;
		_gameStats.campaign.energy += p_energy;

		local addedScore = ::ceil(p_energy * 20.0 * _multiplier);
		_gameStats.campaign.score += addedScore;
		_gameStats.mission.score += addedScore;
		return addedScore;
	}

	function addPrimaryFired()
	{
		++_gameStats.campaign.primaryFired;
		++_gameStats.mission.primaryFired;
	}

	function addSecondaryFired()
	{
		++_gameStats.campaign.secondaryFired;
		++_gameStats.mission.secondaryFired;
	}

	function addSuicide()
	{
		++_gameStats.campaign.suicides;
		++_gameStats.mission.suicides;
	}

	function addHealthLost(p_amount)
	{
		stopMultiplier();
		_gameStats.campaign.healthLost += p_amount;
		_gameStats.mission.healthLost += p_amount;
	}

	function stopMultikill()
	{
		if (_currentMultikill > 1)
		{
			local extraScore = ::min(c_multikillBonus * _currentMultikill * _currentMultikill, c_multikillMaxBonus);
			_gameStats.campaign.score += extraScore;
			_gameStats.mission.score += extraScore;

			if (_multikillLabel != null)
			{
				local id = _currentMultikill < c_maxMultikills ? _currentMultikill : c_maxMultikills;
				_multikillLabel.label.setTextLocalizedAndFormatted("SCORE_MULTIKILL_" + id, [::formatScore(extraScore)]);
				_multikillLabel.presentation.stop();
				_multikillLabel.presentation.start("", ["multikill_" + id], false, 0);
			}
		}
		_currentMultikill = 0;
	}

	function stopMultiplier()
	{
		_multiplier        = 1;
		_currentKillStreak = 0;

		updateMultiplier();
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Child Update

	function childUpdate(p_deltaTime)
	{
		if (::isValidEntity(_resultScreen))
		{
			return;
		}

		_gameStats.campaign.time += p_deltaTime;
		_gameStats.mission.time += p_deltaTime;

		if (::ProgressMgr.getGameMode() != GameMode.SpeedRun && _gameStats.campaign.score != _displayScore)
		{
			// 'Rolling' score
			if (_gameStats.campaign.score > _displayScore)
			{
				_displayScore += 60000 * p_deltaTime;
				if (_displayScore > _gameStats.campaign.score)
				{
					_displayScore = _gameStats.campaign.score;
				}
			}
			else
			{
				_displayScore -= 30000 * p_deltaTime;
				if (_displayScore < _gameStats.campaign.score)
				{
					_displayScore = _gameStats.campaign.score;
				}
			}
		}

		updateScoreLabel();
	}
}
