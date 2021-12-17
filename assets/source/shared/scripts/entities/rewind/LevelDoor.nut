include_entity("rewind/RewindEntity");

class LevelDoor extends RewindEntity
</
	editorImage    = "editor.leveldoor"
	libraryImage   = "editor.library.leveldoor"
	placeable      = Placeable_Everyone
	collisionRect  = [ 0.0, 0.0, 8.0, 8.0 ]
	movementset    = "LevelDoor"
	displayName    = "LevelDoor"
//	group          = "Rewind"
	sizeShapeColor = Colors.CrushBox
	sizeShapeFromEntityCenter = false
/>
{
	</
		type        = "integer"
		min         = 1
		max         = 6
		order       = 0.0
		group       = "Appearance"
		autoGetSet  = true
	/>
	width = 3;
	halfWidth = null;
	
	</
		type        = "integer"
		min         = 1
		max         = 6
		order       = 0.1
		group       = "Appearance"
		autoGetSet  = true
	/>
	height = 3;
	halfHeight = null;
	
	</
		type        = "string"
		choice      = ["left", "right"]
		order       = 0.2
		group       = "Appearance"
	/>
	orientation = "left";
	
	</
		type        = "string"
		choice      = ["Opened", "Opening", "Closed", "Closing"]
		order       = 0.3
		group       = "Appearance"
	/>
	startState = "Closed";
	
	</
		type        = "string"
		choice      = ["up", "down"]
		order       = 1.0
		group       = "Movement"
	/>
	moveDirection = "up";
	
	</
		type        = "integer"
		min         = 1
		max         = 6
		order       = 3
		autoGetSet  = true
		order       = 1.1
		group       = "Movement"
	/>
	moveDistance = 3;
	
	</
		type           = "bool"
		order          = 100
		group          = "Misc."
	/>
	enabled = true;
	_enabledSet = false;
	
	</
		type           = "entity"
		filter         = ::getTriggerTypes()
		group          = "Misc."
		order          = 101
		referenceColor = ReferenceColors.Enable
		description    = "When set, this entity will be enabled when an incoming signal fires."
	/>
	enableSignal = null;
	
	// Internal members
	_closedPosition   = null;
	_openedPosition   = null;
	_submergedInWater = false;
	
	_openSensor     = null;
	_crushSensor    = null;
	
	_activators     = null;
	_presentation   = null;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		base.onInit();
		
		// FIXME: Added this because it only works before the leveldoor has moved, otherwise it's never in water for some reason.
		setUpdateSurvey(true);
		if (hasSurveyResult(SurveyResult_SubmergedInWater))
		{
			_submergedInWater = true;
		}
		
		if(orientation == "left")
		{
			flipForwardDir();
		}
		
		if(moveDirection == "up") // Collin's note: bad way to make sure we only spawn one.
		{
			addShoeboxInclude("levels/shoebox_includes/leveldoor_" + orientation, 
				getShoeboxSpaceFromWorldPos(::Vector2(getPosition().x + 0.5, getPosition().y - 2)),
				0, 0, 1);
		}
		
		addInvincibilityProperties();
		addProperty("weaponGroup", ::WeaponGroup.Neutral);
		
		setDetectableByTouchOnly();
		
		halfWidth  = width / 2.0;
		halfHeight = height / 2.0;
		setCollisionRect(::Vector2(0, 0.5 * height), width, height);
		setEntityCollisionTiles(createCollisionTileString(width, height, "p"));
		setLightBlocking(true);
		
		addCrushSensor();
		
		_closedPosition = getPosition();
		_activators = ::EntityCollection();
		
		//loadPresentationFile("presentation/leveldoor_" + moveDirection);
		_presentation = createPresentationObject("presentation/leveldoor_" + moveDirection);
		_presentation.setAffectedByMovement(true);
		
		// Overwrite string value of moveDirection with corresponding enum value
		moveDirection = getDirectionFromName(moveDirection);
		
		switch (moveDirection)
		{
		case Direction_Up  : 
			_openedPosition = ::Vector2(_closedPosition.x, _closedPosition.y + moveDistance);
			_openSensor     = addTouchSensorWorldSpace(::BoxShape(15, 6), null, _closedPosition );
			break;
			
		case Direction_Down: 
			_openedPosition = ::Vector2(_closedPosition.x, _closedPosition.y - moveDistance);
			_openSensor     = addTouchSensorWorldSpace(
				BoxShape(15, 6), null, ::Vector2(_closedPosition.x, _closedPosition.y + moveDistance)
			);
			break;
			
		default:
			::tt_panic("Unhandled door type '" + moveDirection + "'");
			break;
		}
		
		_openSensor.setEnterCallback ("onDoorEnter" );
		_openSensor.setExitCallback  ("onDoorExit"  );
		_openSensor.setFilterCallback("onDoorFilter");
		
		if (enableSignal != null)
		{
			enableSignal.addChildTrigger(this);
		}
	}
	
	function onSpawn()
	{
		base.onSpawn();
		
		local topX    = (_closedPosition.x - halfWidth ).tointeger();
		local topY    = (_closedPosition.y + height - 1).tointeger();
		local bottomX = topX + width - 1;
		local bottomY = topY - height + 1;
		
		_presentation.start("disabled", [], false, 0);
		setFloorDirection(Direction_None);
		
		switch (moveDirection)
		{
		case Direction_Up: 
			topY    += moveDistance;
			bottomY += moveDistance;
			break;
			
		case Direction_Down: 
			topY    -= moveDistance;
			bottomY -= moveDistance;
			break;
			
		default:
			::tt_panic("Unhandled moveDirection '" + moveDirection + "'");
			break;
		}
		
		for (local x = topX; x <= bottomX; x++)
		{
			for (local y = topY; y >= bottomY; y--)
			{
				local tilePos = ::Vector2(x, y);
				setThemeType(tilePos, ThemeType_UseLevelDefault);
			}
		}
		
		if (startState == "Closing" || startState == "Opened")
		{
			setPosition(_openedPosition);
		}
		setState(startState);
		
		setEnabled(enabled);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onDoorFilter(p_entity)
	{
		return p_entity.hasProperty("openDoors");
	}
	
	function onDoorEnter(p_entity, p_sensor)
	{
		addActivator(p_entity);
	}
	
	function onDoorExit(p_entity, p_sensor)
	{
		if(_activators.containsEntity(p_entity))
		{
			removeActivator(p_entity);
		}
	}
	
	function addActivator(p_entity)
	{
		customCallback("onActivatorAdded", p_entity, true);
	}
	
	function removeActivator(p_entity)
	{
		customCallback("onActivatorRemoved", p_entity, true);
	}
	
	// keep track of the entities that activated the Door
	function onActivatorAdded(p_entity, p_value)
	{
		if (_activators.isEmpty())
		{
			customCallback("onFirstActivatorAdded", p_entity);
		}
		_activators.addEntity(p_entity);
	}
	
	function onActivatorRemoved(p_entity, p_value)
	{
		_activators.removeEntity(p_entity);
		
		if (_activators.isEmpty())
		{
			customCallback("onActivatorsDepleted");
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Inherited methods
	
	function _triggerEnter(p_entity, p_parent)
	{
		if (p_parent == null)
		{
			::tt_panic("_triggerEnter should have parent != null");
			return;
		}
		
		// Default behavior
		setEnabled(true);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods
	
	function setEnabled(p_enabled)
	{
		if (_enabledSet && p_enabled == enabled)
		{
			return;
		}
		
		_enabledSet = true;
		enabled = p_enabled;
		
		_openSensor.setEnabled(p_enabled);
		if (p_enabled)
		{
			_presentation.start("enabled", [], false, 0);
		}
		else
		{
			_presentation.start("disabled", [], false, 0);
		}
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
class LevelDoor_State.Opened
{
	function onEnterState()
	{
		setPosition(_openedPosition);
	}
	
	function onActivatorsDepleted()
	{
		setState("Closing");
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//

class LevelDoor_State.Opening
{
	function onEnterState()
	{
		stopMovement();
		
		local delta = fabs(_openedPosition.y - getPosition().y) - 0.1;
		if (delta > 0)
		{
			if (startMovement(moveDirection, delta))
			{
				playSoundEffect(_submergedInWater ? "door_open_submerged" : "door_open");
			}
		}
	}
	
	function onExitState()
	{
	}
	
	function onActivatorsDepleted()
	{
		setState("Closing");
	}
	
	function onMovementEnded(p_direction)
	{
		//FIXME: Replace with proper sound effect when created by SP
		//playSoundEffect("toki_door_opened");
		setState("Opened");
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
class LevelDoor_State.Closed
{
	function onEnterState()
	{
		setPosition(_closedPosition);
	}
	
	function onFirstActivatorAdded(p_entity)
	{
		setState("Opening");
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
class LevelDoor_State.Closing
{
	function onEnterState()
	{
		stopMovement();
		
		local delta = fabs(getPosition().y - _closedPosition.y) - 0.1;
		if (delta > 0)
		{
			local opposingDirection = Direction_Down;
			switch (moveDirection)
			{
			case Direction_Up  : opposingDirection = Direction_Down; break;
			case Direction_Down: opposingDirection = Direction_Up;   break;
			default:
				::tt_panic("Unhandled moveDirection '" + moveDirection + "'");
				break;
			}
			
			if (startMovement(opposingDirection, delta))
			{
				playSoundEffect(_submergedInWater ? "door_close_submerged" : "door_close");
			}
		}
	}
	
	function onExitState()
	{
	}
	
	function onFirstActivatorAdded(p_entity)
	{
		setState("Opening");
	}
	
	function onMovementEnded(p_direction)
	{
		setState("Closed");
		::Camera.shakeScreen(0.05, getPosition(), CameraShakeFlag.PositionAndFOV);
		::setRumble(RumbleStrength_Low, 0.0);
	}
	
	function onMovementFailed(p_direction, p_moveName)
	{
		local delta = ::fabs(getPosition().y - _openedPosition.y) - 0.1;
		
		if (delta > 0)
		{
			::Camera.shakeScreen(0.05, getPosition(), CameraShakeFlag.PositionAndFOV);
			::setRumble(RumbleStrength_Low, 0.0);
		}
	}
}
Trigger.makeTriggerTarget(LevelDoor);
