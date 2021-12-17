include_entity("menu/IngameMenu");

class SpecialPickupScreen extends IngameMenu
</
	placeable      = Placeable_Hidden
	collisionRect  = [ 0.0, 0.5, 1.0, 1.0 ]  // center X, center Y, width, height
/>
{
	static c_colorGradingTexture = "ingamemenu";        // Required
	static c_musicTrack          = "ingamemenu_happy";  // Optional
	
	_specialID  = null;
	_triggers   = null;
	_activator  = null;
	_allowClose = false;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		_triggers = {};
		
		// Delayed, so don't call base
		startCallbackTimer("onShow", 5.0);
		
		// Create triggers
		createTrigger("ConversationTrigger",
		{
			conversation  = "special_" + _specialID.tolower(),
			stickToEntity = _activator
		});
		
		createTrigger("DisableControlsTrigger",
		{
			disableMoveControls    = true,
			disableAimControls     = true,
			disableHackControls    = true,
			disableWeapons         = true,
			stickToEntity          = _activator
		});
		
		createTrigger("CameraFOVTrigger",
		{
			SetFOVOnEnter  = true,
			FOVEnter       = 50,
			SetFOVOnExit   = true,
			FOVExit        = ::Camera.getTargetFOV(),
			easingDuration = 3.0,
			stickToEntity  = _activator
		});
		
		createTrigger("CameraOffsetTrigger",
		{
			offsetYEnter   = 3,
			setOnExit      = true,
			offsetYExit    = 0,
			easingDuration = 3.0,
			stickToEntity  = _activator
		});
		
		::Audio.duckVolume(DuckingPreset.SpecialPickup);
	}
	
	function onSpawn()
	{
		// Delayed, so don't call base
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Script Item Handling
	
	function onScriptItemProcessed(p_id)
	{
		local prefix = getScriptItemPrefix();
		if (p_id == "start")
		{
			local window = createWindow("video", ::Vector2(0.45, -0.01),
			{
				_titleID = prefix + "VIDEO_HEADER",
				_width   = 0.4,
				_height  = 0.36,
				_zOffset = 0.26,
				_parent = this
			});
			window.createPresentation("video", "specialpickupscreen", ::Vector2(0, -0.025), HudAlignment.Center, [_specialID.tolower(), "pc"]); // FIX AFTER SONY APPROVAL ::getPlatformString()]);
			
			startScriptItemTimer("showLog", 0.5);
		}
		else if (p_id == "showLog")
		{
			local window = createWindow("log", ::Vector2(-0.22, -0.05),
			{
				_titleID = "RESULTSCREEN_LOG",
				_width   = 0.85,
				_height  = 0.35,
				_zOffset = 0.24,
				_parent = this
			});
			window.createTypedTextArea("log", prefix + "LOG",::Vector2(0, 0), ::TextColors.Light, 2.8, 0.3);
			
			startScriptItemTimer("closeAllowed", 1.0);
		}
		else if (p_id == "closeAllowed")
		{
			_allowClose = true;
		}
		else if (p_id == "log")
		{
			if (hasElement("facebutton_exit") == false)
			{
				createFaceButtons(["exit"]);
			}
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onShow()
	{
		::Audio.unduckVolume(DuckingPreset.SpecialPickup);
		
		base.onInit();
		base.onSpawn();
	}
	
	function onButtonMenuPressed()
	{
	}
	
	function onButtonCancelPressed()
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
		if (_allowClose == false)
		{
			return;
		}
		
		if (instantlyShowAllWindowElements())
		{
			close();
			return;
		}
		else
		{
			if (hasElement("facebutton_exit") == false)
			{
				createFaceButtons(["exit"]);
			}
		}
	}
	
	function onDie()
	{
		base.onDie();
		
		deleteAllTriggers();
		
		::ProgressMgr.setLastCheckPoint("specialpickup");
		::ProgressMgr.storeCheckPoint();
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods
	
	function getScriptItemPrefix()
	{
		// FIX AFTER SONY APPROVAL ::getPlatformString()]);
		return "SPECIALPICKUPSCREEN_" + _specialID.toupper() + "_STEAM_";
	}
	
	function createTrigger(p_type, p_props)
	{
		if (p_type in _triggers)
		{
			::tt_panic("Already have a trigger with type '" + p_type + "'");
			return;
		}
		
		local trigger = ::spawnEntity(p_type, getPosition() + ::Vector2(0, 5), p_props);
		_triggers[p_type] <- trigger;
		return trigger;
	}
	
	function deleteTrigger(p_type)
	{
		if (p_type in _triggers)
		{
			::killEntity(_triggers[p_type]);
			delete _triggers[p_type];
		}
	}
	
	function deleteAllTriggers()
	{
		// Different easing duration when exiting
		_triggers["CameraFOVTrigger"].easingDuration    = 1.0;
		_triggers["CameraOffsetTrigger"].easingDuration = 1.0;
		
		foreach (trigger in _triggers)
		{
			::killEntity(trigger);
		}
		_triggers.clear();
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Inherited Methods
	
	function create()
	{
		base.create();
		
		createTitle("PAUSEMENU_TITLE");
		
		startScriptItemTimer("start", 0.0);
	}
}
