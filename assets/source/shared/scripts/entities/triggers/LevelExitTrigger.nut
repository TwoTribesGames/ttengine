include_entity("rewind/conversation/ConversationMgr");
include_entity("triggers/Trigger");

enum LevelExitMode
{
	Normal,         // triggers a level exit
	SpawnPlayer,    // pushes player out of level exit
	BouncePlayer    // player returned, bounce it away again, going to spawnplayer afterwards
};

class LevelExitTrigger extends Trigger
</
	editorImage    = "editor.levelexittrigger"
	libraryImage   = "editor.library.levelexittrigger"
	placeable      = Placeable_Developer
	collisionRect  = [ 0.0, 1.0, 2.0, 2.0 ]  // center X, center Y, width, height
	displayName    = "Trigger - Level Exit"
	group          = "04.1 Action Triggers"
	sizeShapeColor = Colors.Exit
	stickToEntity  = true
/>
{
	// Only works with rectangles
	</
		type        = "string"
		order       = 0
		conditional = "type != rectangle" // HACK: To hide this from the editor, but still pass it as a property
	/>
	type = "rectangle";
	
	</
		type        = "string"
		choice      = ::getLevelNames()
		order       = 0.0
		group       = "Specific Settings"
	/>
	level = null;
	
	</
		type        = "integer"
		choice      = ::range(0, 20)
		description = "When multiple exits point to the same level you can use the level ID to control at which exit the player starts"
		order       = 0.1
		group       = "Specific Settings"
	/>
	levelID = null;
	
	</
		type        = "integer"
		choice      = [0, 90, 180, 270]
		group       = "Specific Settings"
		order       = 0.2
	/>
	rotation = 0;
	
	</
		type        = "string"
		choice      = ["normal", "bounce"]
		group       = "Specific Settings"
		order       = 0.3
	/>
	mode = "normal";
	
	</
		type        = "bool"
		group       = "Specific Settings"
		order       = 0.4
	/>
	useNoFollowZone = true;
	
	</
		type        = "bool"
		group       = "Specific Settings"
		order       = 0.5
	/>
	showLevelName = false;
	
	</
		type        = "bool"
		group       = "Specific Settings"
		order       = 0.6
	/>
	discardHackedEntity= false;
	
	once = true;
	
	// Constants
	static c_bounceMessageDelay    = 1.5;
	static c_bounceMessageDuration = 2.5;
	static c_bounceMessageCount    = 10; // Update localization sheet for this!
	static c_bounceDarknessTimeout = 0.5;
	
	_zoneSensor        = null;
	_playerMoveTrigger = null;
	_bouncedPlayer     = null;
	_bounceFade        = null;
	_mode              = LevelExitMode.Normal;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		base.onInit();
		
		if (level == null)
		{
			editorWarning("Must specify level");
			::killEntity(this);
			return;
		}
		
		if (mode == "bounce")
		{
			_mode = LevelExitMode.BouncePlayer;
		}
		
		registerEntityByTag("SpawnPoint");
	}
	
	function onSpawn()
	{
		base.onSpawn();
		
		// We need to create an touchsensor since there is currently no way to redirect
		// the triggerexit to this entity
		local player = ::getFirstEntityByTag("PlayerBot");
		if (player != null)
		{
			_zoneSensor = addTouchSensor(::BoxShape(width, height), player);
			_zoneSensor.setEnterCallback("onZoneEnter");
			_zoneSensor.setExitCallback("onZoneExit");
		}
		
		if (showLevelName)
		{
			DebugView.setColor(255, 255, 100, 100);
			DebugView.drawTextInWorld(getCenterPosition(), "Level exit to: \n" + level, 10000);
		}
		
		setEnabled(enabled);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onTouchEnter(p_entity, p_sensor)
	{
		if (_mode == LevelExitMode.BouncePlayer)
		{
			prepareForBounce(p_entity);
		}
		else
		{
			base.onTouchEnter(p_entity, p_sensor);
		}
	}
	
	function onZoneEnter(p_entity, p_sensor)
	{
		if (_mode == LevelExitMode.SpawnPlayer)
		{
			createPlayerMoveTrigger(p_entity, rotation);
		}
		
		if (useNoFollowZone)
		{
			local camPos  = ::Camera.getTargetPosition();
			local exitPos = getCenterPosition();
			local offset  = ::Vector2(0, 0);
			local halfHeight = (height / 2.0);
			local halfWidth  = (width / 2.0);
			
			// Use hardcoded value 3.0 for snap distances; seems to work fine. Don't lower it too much though
			// because if the player goes really fast, the camera might be a few tiles away at this point
			switch (rotation)
			{
			case   0: if (::fabs((exitPos.y - halfHeight) - camPos.y) > 3.0) offset.y = -halfHeight; break;
			case  90: if (::fabs((exitPos.x - halfWidth ) - camPos.x) > 3.0) offset.x = -halfWidth;  break;
			case 180: if (::fabs((exitPos.y + halfHeight) - camPos.y) > 3.0) offset.y =  halfHeight; break;
			case 270: if (::fabs((exitPos.x + halfWidth ) - camPos.x) > 3.0) offset.x =  halfWidth;  break;
			default:
				::tt_panic("Unhandled rotation '" + p_rotation + "'");
				break;
			}
			
			if (offset.lengthSquared() > 0.1)
			{
				::Camera.setPosition(camPos + offset, 0.0, EasingType_QuadraticInOut);
			}
			::Camera.resetFollowStack();
		}
	}
	
	function onZoneExit(p_entity, p_sensor)
	{
		if (_playerMoveTrigger != null)
		{
			::killEntity(_playerMoveTrigger);
			_playerMoveTrigger = null;
		}
		
		if (_mode == LevelExitMode.SpawnPlayer)
		{
			_mode = LevelExitMode.BouncePlayer;
			_triggerSensor.setEnabled(true);
		}
		
		if (useNoFollowZone)
		{
			::Camera.resetFollowStack();
			::Camera.setFollowEntityStacked(true);
		}
	}
	
	function onTriggerEnter(p_entity)
	{
		prepareForExit(p_entity);
	}
	
	function onFadeEnded(p_fade, p_animation)
	{
		if (_mode == LevelExitMode.Normal)
		{
			loadLevel();
		}
		else if (_mode == LevelExitMode.BouncePlayer && p_animation != "opaque_to_transparent")
		{
			// Spawn here so that the player immediately switches position when it is all black
			spawn(_bouncedPlayer);
			startCallbackTimer("bounce", c_bounceDarknessTimeout);
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Inherited methods
	
	function setEnabled(p_enabled)
	{
		// The trigger itself (the actual levelexit) should only be active when enabled
		// in Normal mode
		base.setEnabled(p_enabled && (_mode == LevelExitMode.Normal || _mode == LevelExitMode.BouncePlayer))
	}
	
	// Override this so our touch sensor is actually smaller
	function getTouchSensorShape()
	{
		local triggerWidth = width;
		local triggerHeight = height;
		
		// The trigger sensor itself resizes based on the exitdirection
		if (rotation == 90 || rotation == 270)
		{
			triggerWidth  = 2;
		}
		else
		{
			triggerHeight = 2;
		}
		local shape = ::BoxShape(triggerWidth, triggerHeight);
		if (triggerOnTouch == false)
		{
			shape.doContains();
		}
		return shape;
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods
	
	function spawn(p_entity)
	{
		// Spawn player at base of trigger (add 0.01 to for precision errors leading the player
		// to end up in the ground)
		if (::isInZeroGravity(p_entity) || rotation == 180 || rotation == 0)
		{
			p_entity.setPosition(getCenterPosition());
		}
		else
		{
			local pos = getCenterPosition();
			pos.y = pos.y.tointeger();
			// Search for first solid tile below trigger
			for (local y = 0; y < (height / 2.0) + 3; ++y, --pos.y)
			{
				if (::isSolid(::getCollisionTypeLevelOnly(pos)))
				{
					break;
				}
			}
			p_entity.setPosition(::Vector2(pos.x, pos.y + 1));
		}
		_mode = LevelExitMode.SpawnPlayer;
	}
	
	function prepareForBounce(p_entity)
	{
		createPlayerMoveTrigger(p_entity, rotation + 180);
		stopAllTimers();
		::ConversationMgr.clear();
		_bouncedPlayer = p_entity.weakref();
		_bounceFade = createPersistentFade(this, "transparent_to_opaque");
	}
	
	function bounce()
	{
		// Enforce recheck of sensors
		if (_zoneSensor != null)
		{
			_zoneSensor.removeAllSensedEntities();
		}
		startCallbackTimer("showBounceMessage", c_bounceMessageDelay);
		createFade(this, "opaque_to_transparent");
		::killEntity(_bounceFade);
	}
	
	function unlockGuybrush()
	{
		::Stats.unlockAchievement("guybrush");
	}
	
	function showBounceMessage()
	{
		if (_bouncedPlayer != null)
		{
			local textID = "BOUNCE_MESSAGE_" + ::rnd_minmax(1, c_bounceMessageCount) + "_RS";
			::ConversationMgr.queueConversation(
			{
				textID   = textID,
				actor    = _bouncedPlayer,
				delay    = 0.0,
				duration = c_bounceMessageDuration,
				emotion  = "neutral", // FIXME: Let it depends on textID context?
				trigger  = null,
				callback = null
			});
			
			if (textID == "BOUNCE_MESSAGE_7_RS")
			{
				startCallbackTimer("unlockGuybrush", 2.5);
			}
		}
	}
	
	function prepareForExit(p_entity)
	{
		p_entity.customCallback("onPrepareForLevelExit", this);
		createPlayerMoveTrigger(p_entity, rotation + 180);
		createFade(this, "transparent_to_opaque");
	}
	
	function createPlayerMoveTrigger(p_entity, p_rotation)
	{
		if (_playerMoveTrigger != null)
		{
			::killEntity(_playerMoveTrigger);
		}
		
		_playerMoveTrigger = ::spawnEntity("PlayerMoveTrigger", getPosition(),
		{
			width              = width,
			height             = height,
			rotation           = ::wrapAngle(p_rotation + 180), // Spawn in opposite direction
			force              = 1.0,
			disableAimControls = false
		}).weakref();
	}
	
	// returns the ID to compare the saved ID to
	function getTargetID()
	{
		return level + (levelID == null ? "" : levelID.tostring());
	}
	
	// returns the ID to save in the registry and check in the next level
	function getCurrentID()
	{
		return Level.getName() + (levelID == null ? "" : levelID.tostring());
	}
	
	function loadLevel()
	{
		::Level.prepareForExit(getCurrentID());
		::Level.load(level);
	}
}
LevelExitTrigger.setattributes("once", null);
LevelExitTrigger.setattributes("triggerFilter", null);
LevelExitTrigger.setattributes("filterAllEntitiesOfSameType", null);
