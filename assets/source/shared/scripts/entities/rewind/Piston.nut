include_entity("rewind/RewindEntity");

class Piston extends RewindEntity
</
	editorImage    = "editor.piston"
	libraryImage   = "editor.library.piston"
	placeable      = Placeable_Everyone
	movementset    = "Piston"
	collisionRect  = [ 0.0, 1.0, 2.0, 2.0 ]  // note the large collision rect use to make sure it keeps empty space for the entities in the elevator
	sizeShapeColor = Colors.CrushBox
	sizeShapeFromEntityCenter = false
	displayName    = "Piston"
/>
{
	</
		type        = "integer"
		min         = 0
		max         = 50
		group       = "Movement"
		order       = 0.0
		description = "Use 0 to move indefinately."
	/>
	numberOfMoves = 1;
	
	</
		type        = "integer"
		min         = 0
		max         = 50
		group       = "Movement"
		order       = 0.1
	/>
	distance = 0;
	
	</
		type        = "bool"
		group       = "Movement"
		order       = 0.2
	/>
	isOpened = false;
	
	</
		type        = "integer"
		min         = 1
		max         = 75
		group       = "Movement"
		order       = 0.3
	/>
	speedOpen = 50;
	
	</
		type        = "integer"
		min         = 1
		max         = 75
		group       = "Movement"
		order       = 0.4
	/>
	speedClose = 50;
	
	</
		type        = "float"
		min         = 0.0
		max         = 10
		group       = "Movement"
		order       = 0.5
	/>
	timeOpened = 0.0;
	
	</
		type        = "float"
		min         = 0.0
		max         = 10
		group       = "Movement"
		order       = 0.6
	/>
	timeClosed = 0.0;
	
	</
		type        = "integer"
		choice      = [0, 90, 180, 270]
		group       = "Appearance"
		order       = 1.0
	/>
	rotation = 0;
	
	</
		type        = "string"
		choice      = ["s", "p"]
		group       = "Appearance"
		order       = 1.1
	/>
	tileType = "s";
	
	</
		type        = "integer"
		min         = 3
		max         = 75
		group       = "Appearance"
		order       = 1.2
	/>
	width = 5;
	
	</
		type        = "integer"
		min         = 3
		max         = 75
		group       = "Appearance"
		order       = 1.3
	/>
	height = 5;
	
	presentation = "piston";
	
	</
		type        = "bool"
		group       = "Appearance"
		order       = 1.5
	/>
	hasCoverGraphic = true;
	
	</
		type        = "bool"
		group       = "Misc."
		order       = 2.0
	/>
	crushEntities = false;
	
	</
		type        = "bool"
		description = "If enabled, entities can be carried / pushed."
		group       = "Misc."
		order       = 2.1
	/>
	allowCarry = true;
	
	</
		type        = "bool"
		group       = "Misc."
		order       = 2.2
	/>
	enabled = false;
	
	</
		type           = "entity"
		filter         = ::getTriggerTypes()
		group          = "Misc."
		order          = 2.2
		referenceColor = ReferenceColors.Enable
		description    = "When set, this entity will be enabled when an incoming signal fires."
	/>
	enableSignal = null;
	
	positionCulling = false;
	
	// Internal members
	_isHorizontal   = null;
	_enabledSet     = false;
	_isPlaying      = false;
	_moveDirection  = Direction_None;
	_moveCount      = 0;
	_openedPosition = null;
	_closedPosition = null;
	_wasOpened      = null;
	_tileSensor     = null;
	_atEndPosition  = false;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		// Initialize the collision rect first
		{
			local offset = ::Vector2(0, height * 0.5);
			setCollisionRect(offset, width, height);
		}
		
		base.onInit();
		
		setCanBePushed(false);
		setDetectableByTouchOnly();
		setUpdateSurvey(false);
		addInvincibilityProperties();
		addProperty("weaponGroup", ::WeaponGroup.Neutral);
		
		_isHorizontal = (rotation == 90 || rotation == 270);
		
		setEntityCollisionTiles(createCollisionTileString(width, height, tileType));
		setLightBlocking(true);
		
		local presPostfix = null;
		switch (rotation)
		{
		case 0:   presPostfix = "up"; break;
		case 90:  presPostfix = "horizontal"; break;
		case 180: presPostfix = "down"; break;
		case 270: presPostfix = "horizontal"; break;
		default:
			::tt_panic("Unhandled rotation '" + p_rotation + "'");
			presPostfix = "";
		}
		
		local maxDistance    = _isHorizontal ? width : height;
		local targetDistance = distance == 0 ? maxDistance : distance;
		
		if (targetDistance > maxDistance)
		{
			editorWarning("distance (" + targetDistance + ") greater than max allowed distance (" + maxDistance + ")");
			targetDistance = maxDistance;
		}
		
		// Add crushsensor
		if (crushEntities)
		{
			addCrushSensor(null, false);
		}
		
		// Calculate _openedPosition
		{
			local offset = null;
			switch (rotation)
			{
			case 0:   offset = ::Vector2(0, targetDistance); break;
			case 90:  offset = ::Vector2(targetDistance, 0); break;
			case 180: offset = ::Vector2(0, -targetDistance); break;
			case 270: offset = ::Vector2(-targetDistance, 0); break;
			default:
				::tt_panic("Unhandled rotation '" + p_rotation + "'");
			}
			
			_closedPosition = getPosition();
			_openedPosition = _closedPosition + offset;
		}
		
		if (presentation != null)
		{
			local pres = createPresentationObject("presentation/" + presentation + "_" + presPostfix);
			pres.addCustomValue("width", width);
			pres.addCustomValue("height", height);
			
			if (::getCollisionTypeLevelOnly(_openedPosition) == CollisionType_Water_Still)
			{
				pres.addTag("submerged");
			}
			
			if (rotation == 270)
			{
				pres.flipX();
			}
		}
		
		if (hasCoverGraphic)
		{
			local name = "presentation/pistoncover";
			local offset = null;
			
			switch (rotation)
			{
			case 0:
				offset = ::Vector2(0, height / 2.0);
				break;
				
			case 90:
			case 270:
				offset = ::Vector2(width / 2.0, 0);
				break;
				
			case 180:
				offset = ::Vector2(0, -height / 2.0);
				break;
				
			default:
				::tt_panic("Unhandled rotation '" + p_rotation + "'");
			}
			
			local pres = createPresentationObjectInLayer("presentation/pistoncover_" + presPostfix, ParticleLayer_InFrontOfShoeboxZeroOne);
			pres.setCustomUniformScale(_isHorizontal ? height : width);
			pres.setCustomTranslation(offset);
			pres.setIsFollowingParent(false);
			
			if (rotation == 270)
			{
				pres.flipX();
			}
		}
		
		if (isOpened)
		{
			setPosition(_openedPosition);
		}
		
		if (enableSignal != null)
		{
			enableSignal.addChildTrigger(this);
		}
		
		// Create tilesensor (faster than using survey results)
		{
			switch (rotation)
			{
			case 0:   _tileSensor = addTileSensor(::BoxShape(width - 0.1, 1),  ::Vector2(0, height / 2.0 - 0.5)); break;
			case 90:  _tileSensor = addTileSensor(::BoxShape(1, height - 0.1), ::Vector2(width / 2.0 - 0.5, 0)); break;
			case 180: _tileSensor = addTileSensor(::BoxShape(width - 0.1, 1),  ::Vector2(0, -height / 2.0 + 0.5)); break;
			case 270: _tileSensor = addTileSensor(::BoxShape(1, height - 0.1), ::Vector2(-width / 2.0 + 0.5, 0)); break;
			default:
				::tt_panic("Unhandled rotation '" + p_rotation + "'");
			}
			_tileSensor.setIgnoreOwnCollision(true);
			_tileSensor.setEnabled(false);
		}
		
		_wasOpened = isOpened;
	}
	
	function onSpawn()
	{
		base.onSpawn();
		
		startAllPresentationObjects("idle", []);
		
		setEnabled(enabled);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onTileSensorSolidTouchEnter(p_sensor)
	{
		if (_atEndPosition == false)
		{
			customCallback("onMovementEnded");
		}
		
		local w = _isHorizontal ? 1.0 : width;
		local h = _isHorizontal ? height : 1.0;
		local pos = p_sensor.getWorldPosition();
		
		local effectFile = "particles/piston_impact_" + (_isHorizontal ? "vertical" : "horizontal");
		local effect = spawnParticleOneShot(effectFile, pos, false, 0, true, ParticleLayer_UseLayerFromParticleEffect, 1.0);
		
		::Camera.shakeScreen(0.01, pos, CameraShakeFlag.Default);
		
		// NOTE: this should be the cloud
		effect.setEmitterProperties(
			0,
			{
				rect_width  = w,
				rect_height = h,
				particles   = w * h
			});
		
		effect.setEmitterProperties(
			7,
			{
				rect_width  = w,
				rect_height = h,
				particles   = w * h
			});
		
		effect.setEmitterProperties(
			8,
			{
				rect_width  = w,
				rect_height = h,
				particles   = w * h
			});
		
		effect.setEmitterProperties(
			9,
			{
				rect_width  = w,
				rect_height = h,
				particles   = w * h
			});
	}
	
	function onTimer(p_name)
	{
		if (p_name == "move")
		{
			move();
		}
	}
	
	function onMovementStarted()
	{
		::setRumble(RumbleStrength_Low, ::getRumblePanning(RumbleStrength_Low, getCenterPosition()));
		
		// HACK: To make carrying possibly, we need to let to do the movement system its work even though
		// it will fail to finish this move if piston is in wall.
		if (allowCarry)
		{
			startMovement(_moveDirection, -1);
		}
		
		startAllPresentationObjects(isOpened ? "closing" : "opening", []);
	}
	
	function onMovementEnded()
	{
		if (allowCarry)
		{
			stopMovement();
		}
		isOpened = isOpened == false;
		
		local hasImpact = false;
		local impactOffset = null;
		
		startAllPresentationObjects(isOpened ? "opened" : "closed", []);
		
		++_moveCount;
		_moveDirection = Direction_None;
		
		// Check end condition
		if (numberOfMoves > 0 && _moveCount >= numberOfMoves)
		{
			// Don't use setEnabled(false) here because that code path will invoke a possible additional
			// move to let the piston return to its start position
			stop();
			enabled = false;
			
			// Reset this flag to allow for setEnabled(false) to result in a move
			_enabledSet = false;
		}
		else if (_isPlaying)
		{
			// Continue playing
			play();
		}
		
		// Force on exact position
		if (_atEndPosition)
		{
			setPosition(isOpened ? _openedPosition : _closedPosition);
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Inherited methods
	
	function _triggerEnter(p_entity, p_parent)
	{
		// Enabled trigger fired; enable this entity
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
		
		if (enabled)
		{
			_moveCount = 0;
			// Don't call play() here as we instantly want to move and not wait for timeouts
			_isPlaying = true;
			
			// Moving away from correct position
			if (_moveDirection != Direction_None && _wasOpened != isOpened)
			{
				isOpened = isOpened == false;
				move();
			}
			else
			{
				move();
			}
		}
		else
		{
			// At wrong position move back
			if (_moveDirection == Direction_None && _wasOpened != isOpened)
			{
				move();
			}
			// Moving away from correct position
			else if (_moveDirection != Direction_None && _wasOpened == isOpened)
			{
				isOpened = isOpened == false;
				move();
			}
			stop();
		}
	}
	
	function move()
	{
		switch (rotation)
		{
		case 0:   _moveDirection = isOpened ? Direction_Down  : Direction_Up;    break;
		case 90:  _moveDirection = isOpened ? Direction_Left  : Direction_Right; break;
		case 180: _moveDirection = isOpened ? Direction_Up    : Direction_Down;  break;
		case 270: _moveDirection = isOpened ? Direction_Right : Direction_Left;  break;
		default:
			::tt_panic("Unhandled rotation '" + p_rotation + "'");
			_moveDirection = Direction_None;
		}
		
		if (_moveDirection != Direction_None)
		{
			customCallback("onMovementStarted");
		}
	}
	
	function play()
	{
		_isPlaying = true;
		if (isOpened)
		{
			if (timeOpened > 0.0)
			{
				startTimer("move", timeOpened);
			}
			else
			{
				move();
			}
		}
		else
		{
			if (timeClosed > 0.0)
			{
				startTimer("move", timeClosed);
			}
			else
			{
				move();
			}
		}
	}
	
	function stop()
	{
		_isPlaying = false;
		stopAllTimers();
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Update
	
	function update(p_deltaTime)
	{
		base.update(p_deltaTime);
		
		// We cannot use the movement system for this, as pistons require to move through collision
		// which is unsupported by the system. Hence the reason to handle movement manually.
		_atEndPosition = false;
		if (_moveDirection != Direction_None)
		{
			// update position
			local v              = (isOpened ? speedClose : speedOpen) * p_deltaTime;
			local delta          = null;
			local pos            = getPosition();
			local targetPosition = isOpened ? _closedPosition : _openedPosition;
			
			switch (_moveDirection)
			{
			case Direction_Up:    delta = ::Vector2(0,  v); _atEndPosition = (targetPosition.y - pos.y) < v; break;
			case Direction_Down:  delta = ::Vector2(0, -v); _atEndPosition = (pos.y - targetPosition.y) < v; break;
			case Direction_Left:  delta = ::Vector2(-v, 0); _atEndPosition = (pos.x - targetPosition.x) < v; break;
			case Direction_Right: delta = ::Vector2( v, 0); _atEndPosition = (targetPosition.x - pos.x) < v; break;
			default:
				::tt_panic("Unhandled direction '" + _moveDirection + "'");
				delta = ::Vector2(0, 0);
			}
			
			pos += delta;
			setPosition(pos);
			
			// Only enable tilesensor when opening and when sensor is not in potential wall anymore
			local length = _closedPosition.distanceTo(pos);
			_tileSensor.setEnabled(length > 1.5 && isOpened == false)
			
			if (_atEndPosition)
			{
				customCallback("onMovementEnded");
			}
		}
		else
		{
			_tileSensor.setEnabled(false);
		}
	}
}
Trigger.makeTriggerTarget(Piston);
