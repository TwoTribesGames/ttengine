include_entity("rewind/enemies/BaseEnemy");

class BaseScourEnemy extends BaseEnemy
</
	movementset         = "Static"
	placeable           = Placeable_Hidden
	collisionRect       = [ 0.0, 0.5, 1.0, 1.0 ]
	group               = "Rewind"
/>
{
	</
		type           = "entity"
		order          = 50
		filter         = ["Waypoint"]
		group          = "Scour"
		referenceColor = ReferenceColors.Waypoint
	/>
	firstWaypoint        = null;
	_currentWaypoint     = null;
	_destinationWaypoint = null;
	
	</
		type           = "entity_array"
		filter         = ["Area"]
		order          = 51
		group          = "Scour"
		referenceColor = ReferenceColors.Area
		description    = "The areas the bot will scour in. If waypoint is also set, the waypoints will be processed first."
	/>
	scourAreas = null;
	
	</
		type        = "bool"
		order       = 52
		group       = "Scour"
		description = "If enabled; this entity will also use the player as scour area."
	/>
	playerIsScourArea = false;
	
	</
		type           = "bool"
		group          = "Misc."
		order          = 102
		description    = "When set, this entity is aware of its surroundings such as water. Default is false for optimization purposes."
	/>
	updateSurvey = false;
	
	// Constants
	static c_minScourDelay     = 0.1;
	static c_maxScourDelay     = 0.5;
	static c_buoyancy          = 10.0;
	static c_waypointThrust    = 55.0;
	static c_playerScourOffset = ::Vector2(0, 0);
	
	_waypointMoveSettings      = null;
	_scourMoveSettings         = null;
	_goHomeMoveSettings        = null;
	
	_goingHome = true;
	
	_startPosition = null;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		base.onInit();
		
		setUpdateSurvey(updateSurvey);
		
		addProperty("reduceSpeedWhenEnteringWater");
		
		if (playerIsScourArea)
		{
			local playerBot = ::getFirstEntityByTag("PlayerBot");
			if (playerBot != null)
			{
				if (scourAreas == null)
				{
					scourAreas = [];
				}
				scourAreas.push(playerBot);
			}
		}
	}
	
	function onSpawn()
	{
		_startPosition = getCenterPosition();
		setWaypoint(firstWaypoint);
		
		// Do this last to make sure the _startPosition and waypoint are known when setting enabled state
		base.onSpawn();
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onEnabled()
	{
		base.onEnabled();
		
		startScouring();
	}
	
	function onDisabled()
	{
		base.onDisabled();
	}
	
	// These need to be here for base calls
	function onMovementEnded(p_direction)
	{
		//base.onMovementEnded(p_direction);
	}
	
	function onMovementFailed(p_direction, p_moveName)
	{
		//base.onMovementFailed(p_direction, p_moveName);
	}
	
	function onPathMovementFailed(p_direction)
	{
		//base.onPathMovementFailed(p_direction);
	}
	
	function onWaypointReached(p_waypoint)
	{
		if (p_waypoint.killWhenReached)
		{
			addProperty("dieQuietly");
			::killEntity(this);
		}
	}
	
	function onLavaTouchEnter()
	{
		base.onLavaTouchEnter();
		
		// FIXME: bounce off properly
		setSpeed(-getSpeed() / 2.0);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods
	
	function goHome()
	{
		resetScourSettings();
		startScouring();
	}
	
	function getWaypointThrust()
	{
		return c_waypointThrust;
	}
	
	function resetScourSettings()
	{
		setState("");
		updateState();
		_destinationWaypoint = null;
		_currentWaypoint = null;
		scourAreas = null;
	}
	
	function getScourSettings()
	{
		return { _destinationWaypoint = _destinationWaypoint, _currentWaypoint = _currentWaypoint,
		         scourAreas = scourAreas };
	}
	
	function setScourSettings(p_settings)
	{
		resetScourSettings();
		_destinationWaypoint = p_settings._destinationWaypoint;
		_currentWaypoint     = p_settings._currentWaypoint;
		scourAreas           = p_settings.scourAreas;
		startScouring();
	}
	
	function configureMovement(p_mass, p_drag, p_thrust, p_easeOut, p_moveEnd, p_moveAnim = "idle")
	{
		local settings = ::PhysicsSettings(p_mass, p_drag, p_thrust, p_easeOut, p_moveEnd);
		settings.setShouldTurn(true);
		settings.setTurnAnimationName("turn");
		settings.setPresentationAnimationName(p_moveAnim);
		return settings;
	}
	
	function startScouring()
	{
		if (isEnabled() == false)
		{
			return;
		}
		
		// Push low prio scour states and only if it doesn't already exists
		if (_destinationWaypoint != null)
		{
			if (hasState("FollowWaypoints") == false) pushState("FollowWaypoints", StatePriority.Low);
		}
		else if (scourAreas != null && scourAreas.len() > 0)
		{
			if (hasState("Scour") == false) pushState("Scour", StatePriority.Low);
		}
		else
		{
			if (hasState("MoveToStartPosition") == false) pushState("MoveToStartPosition", StatePriority.Low);
		}
	}
	
	function setWaypoint(p_waypoint)
	{
		_destinationWaypoint = p_waypoint;
		if (_destinationWaypoint == null)
		{
			if (scourAreas != null)
			{
				startScouring();
			}
			return;
		}
		
		local scaledThrust = getWaypointThrust() * p_waypoint.customScale;
		if (scaledThrust < 0.0)
		{
			p_waypoint.editorWarning("Cannot use negative customScale");
			scaledThrust = c_waypointThrust;
		}
		
		_waypointMoveSettings.setThrust(scaledThrust);
	}
	
	function moveToRandomScourPos()
	{
		// Clean up scourAreas as they might have become invalid
		if (scourAreas == null)
		{
			popState();
			return;
		}
		
		local rect = getCollisionRect();
		
		// Make sure we use a clone here!
		local cleanAreas = clone scourAreas;
		scourAreas.clear();
		foreach (area in cleanAreas)
		{
			if (::isValidEntity(area))
			{
				scourAreas.push(area);
			}
		}
		
		// All scourAreas became invalid
		if (scourAreas.len() == 0)
		{
			popState();
			return;
		}
		
		local area = randomArrayItem(scourAreas);
		local position = area instanceof ::PlayerBot ? 
			area.getCenterPosition() + c_playerScourOffset:
			area.getPointInArea(rect.getWidth() * 0.5, rect.getHeight() * 0.5);
		
		if (_goingHome && _goHomeMoveSettings != null)
		{
			startPathMovementToPosition(position, _goHomeMoveSettings);
		}
		else
		{
			startPathMovementToPosition(position, _scourMoveSettings);
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Update
	
	function update(p_deltaTime)
	{
		base.update(p_deltaTime);
		
		if (c_buoyancy > 0 && isInWater())
		{
			local speed = getSpeed();
			if (speed.y < 10.0);
			{
				speed.y += c_buoyancy * p_deltaTime;
				setSpeed(speed);
			}
		}
	}
}

class BaseScourEnemy_State.FollowWaypoints
{
	function onEnterState()
	{
		::assert(_waypointMoveSettings != null, "Trying to waypointFolow but _waypointMoveSettings aren't defined for " + this);
		if (hasTimer("onStartMoving") == false)
		{
			startCallbackTimer("onStartMoving", 0.1);
		}
		else
		{
			resumeTimer("onStartMoving");
		}
	}
	
	function onExitState()
	{
		suspendTimer("onStartMoving");
	}
	
	function onMovementEnded(p_direction)
	{
		base.onMovementEnded(p_direction);
		
		if (_destinationWaypoint.getCenterPosition().distanceTo(getCenterPosition()) >
		    _waypointMoveSettings.getMoveEndDistance())
		{
			startCallbackTimer("onStartMoving", 0.5);
			return;
		}
		
		if (::isValidEntity(_currentWaypoint) && _currentWaypoint.autoAdvance == false)
		{
			_currentWaypoint._enableEvent.unsubscribe(this, "onWaypointEnabled");
		}
		_currentWaypoint = _destinationWaypoint;
		
		local autoAdvance  = _currentWaypoint.autoAdvance;
		local advanceDelay = _currentWaypoint.advanceDelay;
		
		if (autoAdvance == false)
		{
			_currentWaypoint._enableEvent.subscribe(this, "onWaypointEnabled");
		}
		customCallback("onWaypointReached", _currentWaypoint);
		setWaypoint(_currentWaypoint.getNextWaypoint());
		
		if (autoAdvance)
		{
			startCallbackTimer("onStartMoving", advanceDelay);
		}
	}
	
	function onStartMoving()
	{
		if (_destinationWaypoint != null)
		{
			startPathMovementToEntity(_destinationWaypoint, _destinationWaypoint.getCenterOffset(), _waypointMoveSettings);
		}
	}
	
	function onWaypointEnabled(p_waypoint, p_enabled)
	{
		if (p_enabled)
		{
			customCallback("onStartMoving");
		}
	}
	
	function onMovementFailed(p_direction, p_moveName)
	{
		// just try again
		startCallbackTimer("onStartMoving", 0.5);
	}
	
	function onPathMovementFailed(p_closestPoint)
	{
		// just try again
		startCallbackTimer("onStartMoving", 0.5);
	}
	
	function onDisabled()
	{
		popState();
		base.onDisabled();
	}
}

class BaseScourEnemy_State.Scour
{
	function onEnterState()
	{
		::assert(_scourMoveSettings != null, "Trying to scour but _scourMoveSettings aren't defined for " + this);
		moveToRandomScourPos();
	}
	
	function onExitState()
	{
		stopTimer("new_scourpos");
		_goingHome = true;
	}
	
	function onMovementEnded(p_direction)
	{
		// make sure we keep trying to go home until it succeeds
		_goingHome = false;
		
		base.onMovementEnded(p_direction);
		
		startTimer("new_scourpos", ::frnd_minmax(c_minScourDelay, c_maxScourDelay));
	}
	
	function onMovementFailed(p_direction, p_moveName)
	{
		base.onMovementFailed(p_direction, p_moveName);
		
		startTimer("new_scourpos", ::frnd_minmax(c_minScourDelay, c_maxScourDelay));
	}
	
	function onPathMovementFailed(p_direction)
	{
		base.onPathMovementFailed(p_direction);
		
		startTimer("new_scourpos", ::frnd_minmax(c_minScourDelay, c_maxScourDelay));
	}
	
	function onTimer(p_name)
	{
		base.onTimer(p_name);
		
		if (p_name == "new_scourpos")
		{
			moveToRandomScourPos();
		}
	}
	
	function onDisabled()
	{
		popState();
		
		base.onDisabled();
	}
}

class BaseScourEnemy_State.MoveToStartPosition
{
	function onEnterState()
	{
		if (_goingHome && _goHomeMoveSettings != null)
		{
			startPathMovementToPosition(_startPosition, _goHomeMoveSettings);
		}
		else
		{
			startPathMovementToPosition(_startPosition, _scourMoveSettings);
		}
	}
	
	function onMovementEnded(p_direction)
	{
		base.onMovementEnded(p_direction)
		
		// make sure we keep trying to go home until it succeeds
		_goingHome = false;
	}
	
	function onExitState()
	{
	}
}
