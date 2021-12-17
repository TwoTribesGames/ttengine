enum ButterflyMode
{
	Heart,
	Normal
};

::g_registeredClassesForGameCallbacks <- [];
::g_registeredClassesForSetPlayerCountCallbacks <- [];
::g_gameHelperInstance <- null;

// Singleton
class Game
{
	_deltaTimeScale   = 1.0;
	_isPaused         = null;
	_isFarmingEnabled = false;
	_isInBackGround   = false;

	function init()
	{
		// Always reset instance
		::g_gameHelperInstance = null;
		getInstance();
	}

	function constructor()
	{
		_isPaused       = ::RefCountedBool(false);
		_deltaTimeScale = 1.0;
		::setFixedDeltaTimeScale(1.0);
	}

	function getInstance()
	{
		if (::g_gameHelperInstance == null)
		{
			::g_gameHelperInstance = ::Game();
		}
		return ::g_gameHelperInstance;
	}

	function pause()
	{
		local game = ::Game.getInstance();
		if (game._isPaused.set(true))
		{
			::ProgressMgr.onGamePaused();
			::stopRumble();
			::setFixedDeltaTimeScale(0.0);
			::MusicSource.pauseAll();
			::Audio.pauseCategory("Music");
			::Audio.pauseCategory("Effects");
			::Audio.pauseCategory("Ambient");
			::Audio.pauseCategory("VoiceOver");
		}
	}

	function unpause()
	{
		local game = ::Game.getInstance();
		if (game._isPaused.set(false))
		{
			::setFixedDeltaTimeScale(game._deltaTimeScale);
			::MusicSource.resumeAll();
			::Audio.resumeCategory("Music");
			::Audio.resumeCategory("Effects");
			::Audio.resumeCategory("Ambient");
			::Audio.resumeCategory("VoiceOver");
		}
	}

	function isInBackGround()
	{
		local game = ::Game.getInstance();
		return game._isInBackGround;
	}

	function setDeltaTimeScale(p_scale)
	{
		local game = ::Game.getInstance();
		game._deltaTimeScale = p_scale;
		if (game._isPaused.get() == false)
		{
			::setFixedDeltaTimeScale(p_scale);
		}
	}

	function isFarmingEnabled()
	{
		local game = ::Game.getInstance();
		return game._isFarmingEnabled;
	}

	function setFarmingEnabled(p_enabled, p_handleAchievement = true)
	{
		local game = ::Game.getInstance();
		if (game._isFarmingEnabled != p_enabled)
		{
			if (p_enabled && p_handleAchievement && ::isValidAndInitializedEntity(::getFirstEntityByTag("PlayerBot")))
			{
				::Stats.unlockAchievement("farmer");
				::Stats.storeAchievements();

			}
			game._isFarmingEnabled = p_enabled
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks

	function onAppEnteredBackground()
	{
		pause();
		_isInBackGround = true;
	}

	function onAppLeftBackground()
	{
		unpause();
		_isInBackGround = false;
	}
}

// If a class registers itself, it also received the onGameInitialized, onGameUnserialized and onGameReloaded callbacks
function registerClassForGameCallbacks(p_class)
{
	::g_registeredClassesForGameCallbacks.push(p_class);
}

function registerClassForSetPlayerCallbacks(p_class)
{
	::g_registeredClassesForSetPlayerCountCallbacks.push(p_class);
}

function hideElementsForPlatform()
{
}

// This callback called after initialization of the game (each level)
function onGameInitialized()
{
	hideElementsForPlatform();
	foreach (c in ::g_registeredClassesForGameCallbacks)
	{
		c.onGameInitialized();
	}
}

// This callback called after unserialization of the game
function onGameUnserialized()
{
	hideElementsForPlatform();
	foreach (c in ::g_registeredClassesForGameCallbacks)
	{
		c.onGameUnserialized();
	}
}

// This callback called after reloading of the game
function onGameReloaded()
{
	hideElementsForPlatform();
	foreach (c in ::g_registeredClassesForGameCallbacks)
	{
		c.onGameReloaded();
	}
}

function onAppEnteredBackground()
{
	local game = ::Game.getInstance();
	game.onAppEnteredBackground();
}

function onAppLeftBackground()
{
	local game = ::Game.getInstance();
	game.onAppLeftBackground();
}
