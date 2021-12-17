include_entity("rewind/enemies/BaseScourEnemy");
include_entity("rewind/conversation/ConversationMgr");
include_entity("rewind/PlayerBot");
include_entity("triggers/Trigger");

class DLL extends BaseScourEnemy
</
	editorImage         = "editor.dll"
	libraryImage        = "editor.library.dll"
	movementset         = "Static"
	placeable           = Placeable_Everyone
	collisionRect       = [ 0.0, 1.0, 1.85, 1.85 ]
	pathFindAgentRadius = 0.666
	stickToEntity       = true
	group               = "01. Enemies"
/>
{
	</
		type           = "bool"
		group          = "Appearance"
		order          = 0.0
	/>
	hasShield = false;
	
	</
		type   = "string"
		choice = ["Random", "left", "right"]
		order  = 1
	/>
	orientation = "right";
	
	</
		type           = "entity"
		filter         = ::getTriggerTypes()
		referenceColor = ReferenceColors.Enable
		description    = "When set, shield will be enabled/disabled."
		group          = "Appearance"
		order          = 0.1
	/>
	toggleShieldSignal = null;
	
	</
		type           = "entity"
		description    = "When set, the DLL will follow this entity"
		group          = "Behavior"
		order          = 1.0
	/>
	followEntity = null;
	
	</
		type           = "entity"
		filter         = ::getTriggerTypes()
		referenceColor = ReferenceColors.Enable
		description    = "When set, the DLL will go home"
		group          = "Behavior"
		order          = 1.1
	/>
	goHomeSignal = null;
	
	</
		type           = "entity"
		filter         = ::getTriggerTypes()
		referenceColor = ReferenceColors.Enable
		description    = "When set, the DLL will go scour"
		group          = "Behavior"
		order          = 1.2
	/>
	goScourSignal = null;
	
	</
		type           = "entity"
		filter         = ::getTriggerTypes()
		referenceColor = ReferenceColors.Enable
		description    = "When set, the DLL will go to its waypoint"
		group          = "Behavior"
		order          = 1.3
	/>
	goWaypointSignal = null;
	
	positionCulling = false;
	
	// Constants
	static c_maxHealth         = 35;
	static c_mass              = 2.0;
	static c_score             = 250;
	static c_dazedByEMPTimeout = 2.0;
	static c_pickupDropCount   = 0;
	static c_waypointThrust    = 38;
	static c_energy            = 10;
	static c_playerScourOffset = ::Vector2(0, 5);
	static c_minScourDelay     = 1.0;
	static c_maxScourDelay     = 5.0;
	
	_followMoveSettings       = null;
	_turnMoveSettings         = null;
	_proximitySensor          = null;
	_currentEmotion           = null;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		base.onInit();
		
		addProperty("healedByHealthBot");
		addProperty("hiddenForBumperEnemies")
		removeProperty("touchDamage");
		removeProperty("attractHomingMissiles");
		removeProperty("noticeEMP");
		
		setPositionCullingEnabled(false);
		
		_scourMoveSettings    = configureMovement(c_mass, 4.0, 68.0, 4.0, 2.0);
		_waypointMoveSettings = configureMovement(c_mass, 4.0, c_waypointThrust, 1.5, 0.5);
		_goHomeMoveSettings   = configureMovement(c_mass, ::frnd_minmax(4.0, 5.0), ::frnd_minmax(80.0, 90.0), 2.5, 1.0);
		_followMoveSettings   = configureMovement(c_mass, 8.0, 240.0, 15.0, 3.0);
		_turnMoveSettings     = configureMovement(c_mass, 8.0, 0.0, 0.01, 0.01);
		
		_presentation = createPresentationObject("presentation/" + getType().tolower());
		_presentation.setAffectedByMovement(true);
		
		setEmotion("neutral");
		
		if (orientation == "Random")
		{
			setForwardAsLeft(brnd());
		}
		else
		{
			setForwardFromString(this, orientation);
		}
		
		if (toggleShieldSignal != null)
		{
			toggleShieldSignal.addChildTrigger(this);
		}
		
		if (goHomeSignal != null)
		{
			goHomeSignal.addChildTrigger(this);
		}
		
		if (goScourSignal != null)
		{
			goScourSignal.addChildTrigger(this);
		}
		
		if (goWaypointSignal != null)
		{
			goWaypointSignal.addChildTrigger(this);
		}
		
		_proximitySensor = addTouchSensor(::CircleShape(0, 12));
		_proximitySensor.setExitCallback("onProximityExit");
		_proximitySensor.setEnabled(false);
	}
	
	function onSpawn()
	{
		base.onSpawn();
		
		if (hasShield == false)
		{
			_shield.disableShield(false);
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onEnabled()
	{
		base.onEnabled();
		
		if (followEntity != null)
		{
			pushState("FollowEntity");
		}
	}
	
	function onDisabled()
	{
		base.onDisabled();
	}
	
	function onPresentationObjectCallback(p_object, p_name)
	{
		if (p_name == "end_anim")
		{
			// restart idle
			_presentation.start("idle", [], false, 0);
		}
	}
	
	function onConversationCallback(p_name, p_value)
	{
		switch (p_name)
		{
		case "setEmotion":      setEmotion(p_value);     break;
		case "goHome":          goHome();                break;
		case "goScour":         startScouring();         break;
		case "goWaypoint":      goWaypoint();            break;
		case "enableShield":    _shield.enableShield();  break;
		case "disableShield":   _shield.disableShield(); break;
		case "selfDestruct":    ::killEntity(this);      break;
		case "enableDoor":      enableDoor();            break;
		
		default:
			::tt_panic("Unhandled conversation callback '" + p_name + "'");
			break;
		}
	}
	
	function onDamage(p_healthbar, p_damage)
	{
		local damager = ("_shooter" in p_healthbar._lastDamager) ? 
			p_healthbar._lastDamager._shooter : p_healthbar._lastDamager;
		
		if (damager instanceof ::PlayerBot)
		{
			local emotion = p_healthbar.getNormalizedHealth() < 0.35 ? "scared" : "angry";
			setTemporaryEmotion(emotion, p_damage);
		}
	}
	
	function onHealthBarEmpty(p_healthBar, p_killer)
	{
		base.onHealthBarEmpty(p_healthBar, p_killer);
		
		if (hasProperty("dieQuietly") == false)
		{
			p_killer = ("_shooter" in p_killer) ? p_killer._shooter : p_killer;
			
			if (p_killer instanceof ::PlayerBot)
			{
				::ProgressMgr.incrementDLLKillCount();
			}
		}
	}
	
	function onDie()
	{
		base.onDie();
		
		if (hasProperty("dieQuietly") == false)
		{
			::Stats.submitTelemetryEvent("dll_died", getID());
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Inherited Methods
	
	function getPlayerScourOffset(p_player)
	{
		local playerPos = p_player.getCenterPosition();
		return playerPos + c_playerScourOffset + 
			(playerPos.x > getCenterPosition().x ? ::Vector2(-5, 0) : ::Vector2(5, 0));
	}
	
	function createShield()
	{
		return addChild("EnemyShield", getCenterOffset(),
			{
				_type  = ShieldType.Indestructible,
				_range = 3.0,
				_angle = 45
			}
		);
	}
	
	function _triggerEnter(p_entity, p_parent)
	{
		if (p_parent == null)
		{
			::tt_panic("_triggerEnter should have parent != null");
			return;
		}
		
		if (p_parent.equals(toggleShieldSignal))
		{
			_shield.isEnabled() ? _shield.disableShield() : _shield.enableShield();
			return;
		}
		
		if (p_parent.equals(goHomeSignal))
		{
			goHome();
			return;
		}
		
		if (p_parent.equals(goScourSignal))
		{
			startScouring();
			return;
		}
		
		if (p_parent.equals(goWaypointSignal))
		{
			goWaypoint();
			return;
		}
		
		// Default behavior
		base._triggerEnter(p_entity, p_parent);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods
	
	function goWaypoint()
	{
		followEntity = null;
		setState("");
		updateState();
		setWaypoint(firstWaypoint);
		startScouring();
	}
	
	function enableDoor()
	{
		stopMovement();
		_presentation.start("idle", ["enable_door"], false, 1);
	}
	
	function setEmotion(p_emotion)
	{
		if (_currentEmotion != p_emotion)
		{
			stopTimer("resetTemporaryEmotion");
			_currentEmotion = p_emotion;
			_presentation.clearTags();
			_presentation.addTag(p_emotion);
			
			// Instantly start working animation
			if (p_emotion == "working")
			{
				_presentation.start("idle", [], false, 0);
			}
		}
	}
	
	function resetTemporaryEmotion()
	{
		local emotion = _currentEmotion;
		_currentEmotion = null;
		setEmotion(emotion);
	}
	
	function setTemporaryEmotion(p_emotion, p_duration)
	{
		_presentation.clearTags();
		_presentation.addTag(p_emotion);
		_presentation.stop();
		
		// restart idle
		_presentation.start("idle", [], false, 0);
		
		local threshold = p_emotion == "scared" ? 0.7 : 0.5;
		
		// Perhaps say something
		if (::ConversationMgr.isPlaying() == false && ::frnd() < threshold)
		{
			::ConversationMgr.queueConversation(
			{
				textID   = "DLL_HIT_" + p_emotion.toupper() + "_" + ::rnd_minmax(1, 8) + "_DL",
				actor    = this.weakref(),
				delay    = 0.0,
				duration = 2.0,
				emotion  = null,
				trigger  = null,
				callback = null
			});
		}
		
		startCallbackTimer("resetTemporaryEmotion", p_duration);
	}
}

class DLL_State.FollowEntity
{
	function onEnterState()
	{
		_proximitySensor.setTarget(followEntity);
		_proximitySensor.setEnabled(true);
		
		customCallback("onMoveToFollowEntity");
	}
	
	function onExitState()
	{
		stopTimer("onCheckDirection");
		_proximitySensor.setEnabled(false);
		_proximitySensor.setTarget(null);
	}
	
	function onMoveToFollowEntity()
	{
		if (::isValidEntity(followEntity) == false)
		{
			followEntity = null;
			popState();
			return false;
		}
		
		local xoffset = brnd() ? -5.0 : 5.0;
		startPathMovementToEntity(followEntity, ::Vector2(xoffset, 5.0), _followMoveSettings);
		return true;
	}
	
	function onMovementEnded(p_direction)
	{
		startCallbackTimer("onCheckDirection", 0.2);
	}
	
	function onMovementFailed(p_direction, p_moveName)
	{
		// Retry in a bit
		startCallbackTimer("onMoveToFollowEntity", 0.5);
	}
	
	function onPathMovementFailed(p_closestPoint)
	{
		// Retry in a bit
		startCallbackTimer("onMoveToFollowEntity", 0.5);
	}
	
	function onCheckDirection()
	{
		if (::isValidEntity(followEntity) == false)
		{
			followEntity = null;
			popState();
			return false;
		}
		
		if (::isEntityFacingPosition(this, followEntity.getCenterPosition()) == false)
		{
			// start a very small movement in the correct direction to let DLL face the followEntity
			stopTimer("onCheckDirection");
			local diff = followEntity.getCenterPosition().x > getCenterPosition().x ? 0.01 : -0.01;
			startPathMovementToPosition(getCenterPosition() + ::Vector2(diff, 0), _turnMoveSettings);
			return;
		}
		
		startCallbackTimer("onCheckDirection", 0.2);
	}
	
	function onProximityExit(p_entity, p_sensor)
	{
		stopTimer("onCheckDirection");
		
		customCallback("onMoveToFollowEntity");
	}
	
	function onDisabled()
	{
		popState();
		
		base.onDisabled();
	}
}
DLL.setattributes("positionCulling", null);
