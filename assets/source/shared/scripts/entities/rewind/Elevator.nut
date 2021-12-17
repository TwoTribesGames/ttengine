include_entity("rewind/RewindEntity");

// FIXME: NEEDS BIG CLEANUP; THIS CLASS IS A MESS

class Elevator extends RewindEntity
</ 
	editorImage    = "editor.elevator"
	libraryImage   = "editor.library.elevator"
	placeable      = Placeable_Everyone
	movementset    = "Elevator"
	collisionRect  = [ 0.0, 1.0, 2.0, 2.0 ]  // note the large collision rect use to make sure it keeps empty space for the entities in the elevator
	sizeShapeColor = Colors.CrushBox
	sizeShapeFromEntityCenter = false
//	group = "Rewind"
/>
{
	</
		type           = "entity"
		filter         = ["Waypoint"]
		referenceColor = ReferenceColors.Waypoint
		order = 0
	/>
	firstStop = null;
	_destinationWaypoint = null;
	_currentWaypoint = null;
	
	</
		type = "integer"
		min  = 1
		max  = 300
		order = 1
	/>
	width = 3;
	
	</
		type = "integer"
		min  = 1
		max  = 300
		order = 1
	/>
	height = 3;
	
	</
		type = "string"
		choice = ["c","s", "p"]
		order = 4
	/>
	material = "s";
	
	</
		type = "float"
		min = 0
		max = 100
		order = 5
	/>
	xSpeed = 5;
	
	</
		type = "float"
		min = 0
		max = 100
		order = 5.1
	/>
	ySpeed = 3;
	
	</
		type = "float"
		min = 0
		max = 100
		order = 5.5
	/>
	advanceDelay = 0.5;
	
	_moveDelay   = 0.5;
	
	</
		type        = "bool"
		description = "Should the advanceDelay set in waypoints be used to override this elevators advanceDelay?"
		order = 98
	/>
	useWaypointAdvanceDelay = false;
	
	</
		type        = "bool"
		description = "Should the elevator reverse its direction at the end of its path?"
		order = 99
	/>
	reversible = false;
	_reversed  = false;
	
	</
		type  = "bool"
		order = 100
	/>
	enabled = false;
	
	</
		type  = "bool"
		order = 100
	/>
	crushEntities = false;
	
	</
		type = "string"
		choice = getPresentationNames()
		order = 7
	/>
	presentationFile = "door_airlock";
	
	</
		type = "bool"
		order = 8
		description = "Should anims defined in the the movement be used, if false only 'idle' will be played"
	/>
	useMomevementSetAnims = true;
	
	_crushSensor  = null;
	
	_lightRadius  = 4;
	
	function onInit()
	{
		// Initialize the collision rect first
		local offset = ::Vector2(0, height * 0.5);
		setCollisionRect(offset, width, height);
		
		base.onInit();
		
		setDetectableByTouchOnly();
		addInvincibilityProperties();
		addProperty("weaponGroup", ::WeaponGroup.Neutral);
		
		setEntityCollisionTiles(createCollisionTileString(width, height, material));
		setLightBlocking(true);
		
		createMovementController(false);
		setCanBePushed(false);
		setFloorDirection(Direction_None);
		
		setSpeedFactor(::Vector2(xSpeed, ySpeed));
		
		if (presentationFile != null)
		{
			local presentation = createPresentationObject("presentation/" + presentationFile);
			if (useMomevementSetAnims)
			{
				presentation.setAffectedByMovement(true);
			}
			presentation.addCustomValue("width", width);
			presentation.addCustomValue("height", height);
		}
		if (crushEntities)
		{
			addCrushSensor(null, false);
		}
		
		if (advanceDelay == 0) advanceDelay = 0.01;
		// TODO automatically create a waypoint at the start position?
	}
	
	function onSpawn()
	{
		base.onSpawn();
		
		startAllPresentationObjects("idle", []);
		
		setNextStop(firstStop);
		
		setEnabled(enabled);
	}
	
	function setEnabled(p_enabled)
	{
		enabled = p_enabled;
		
		if (p_enabled)
		{
			moveToNextStop();
		}
		else
		{
			stopAllTimers();
			stopMovement();
		}
	}
	
	function moveToNextStop()
	{
		if (_destinationWaypoint != null)
		{
			local customScale = _destinationWaypoint.customScale;
			setSpeedFactor(::Vector2(xSpeed * customScale, ySpeed * customScale));
			
			local dir  = getDirectionToPos(_destinationWaypoint.getPosition())
			local dist = getDistanceToPos(_destinationWaypoint.getPosition(), dir);
			
			if (startMovement(dir, dist) == false)
			{
				softStopMovement();
				startTimer("start_moving", _moveDelay);
			}
			else
			{
				startAllPresentationObjects("active", []);
			}
		}
	}
	
	// provide an override argument to override the automatic selection of the next waypoint
	function setNextStop(p_override = null)
	{
		local nextWaypoint = p_override != null ? p_override : getNextWaypoint();
		
		if (nextWaypoint != null)
		{
			_destinationWaypoint = nextWaypoint;
			//DebugView.setColor(255, 255, 255, 255);
			//DebugView.drawLineBetween(this, _destinationWaypoint, 1.5);
		}
		else if (reversible)
		{
			_reversed = _reversed == false;
			_destinationWaypoint = getNextWaypoint();
		}
	}
	
	function getNextWaypoint()
	{
		return _destinationWaypoint != null ? _destinationWaypoint.getNextWaypoint() : null;
	}
	
	function getDirectionToPos(p_pos)
	{
		local diff = p_pos - getPosition();
		
		if (diff.x > 0)
		{
			return Direction_Right;
		}
		else if (diff.x < 0)
		{
			return Direction_Left;
		}
		else if (diff.y < 0)
		{
			return Direction_Down;
		}
		else if (diff.y > 0)
		{
			return Direction_Up;
		}
		
		return Direction_None;
	}
	
	// returns the number of tiles in p_direction to p_pos
	function getDistanceToPos(p_pos, p_direction)
	{
		local mask = ::getVectorFromDirection(p_direction);
		local diff = p_pos - getPosition();
		
		// float comparison, not sure if this'll hold
		if (approximately(mask.x, 0))
		{
			return fabs(diff.y);
		}
		else
		{
			return fabs(diff.x);
		}
	}
	
	function onTimer(p_name)
	{
		if (p_name == "start_moving")
		{
			moveToNextStop();
		}
	}
	
	function onMovementFailed(p_direction, p_moveName)
	{
		startAllPresentationObjects("stop", []);
		
		// TODO if we haven't moved from our (last?) checkpoint stop trying to move
		// i.e. only try to keep moving when the elevator is on its way
	}
	
	function onMovementEnded(p_direction)
	{
		startAllPresentationObjects("stop", []);
		// Martijn: disabled this check because in some situations the 
		// elevator overshoots its destination, and then it simply stops.
		//if (getPosition().approximately(_destinationWaypoint.getPosition()))
		{
			if (::isValidEntity(_currentWaypoint) && _currentWaypoint.autoAdvance == false)
			{
				_currentWaypoint._enableEvent.unsubscribe(this, "onWaypointEnabled");
			}
			_currentWaypoint = _destinationWaypoint;
			
			_moveDelay = (useWaypointAdvanceDelay) ? _currentWaypoint.advanceDelay : advanceDelay;
			
			if (_currentWaypoint.autoAdvance)
			{
				startTimer("start_moving", _moveDelay);
			}
			else
			{
				_currentWaypoint._enableEvent.subscribe(this, "onWaypointEnabled");
			}
			
			setNextStop();
		}
	}
	
	function onWaypointEnabled(p_waypoint, p_enabled)
	{//::echo("onWaypointEnabled", p_waypoint, p_enabled, this);
		if (p_enabled)
		{
			startTimer("start_moving", _moveDelay);
		}
	}
}
