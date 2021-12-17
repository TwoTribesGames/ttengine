include_entity("rewind/RewindEntity");

class CameraOnRails extends RewindEntity
</
	editorImage         = "editor.cameraonrails"
	libraryImage        = "editor.library.cameraonrails"
	placeable           = Placeable_Everyone
	movementset         = "Static"
	collisionRect       = [ 0.0, 1.0, 2.0, 2.0 ]
	group               = "05. Camera"
	stickToEntity       = false
/>
{
	</
		type           = "entity"
		filter         = ["Waypoint"]
		order          = 1
		referenceColor = ReferenceColors.Waypoint
	/>
	firstWaypoint = null;
	
	</
		type  = "bool"
		order = 2
	/>
	startAtPlayerPosition = true;
	
	</
		type  = "bool"
		order = 3
	/>
	autoMovePlayer = true;
	
	</
		type  = "bool"
		order = 100
	/>
	enabled = true;
	
	</
		type = "float"
		min = 0
		max = 80
		order = 4
	/>
	thrust = 8.5;
	
	_edgeRadius               = 0.03; // in screenspace
	_bounceSpeed              = 2.0;
	_outsideScreenDamage      = 0.5;
	_outsideScreenDamageDelay = 0.2; // in seconds
	
	// Internal values. Don't modify
	_waypoint           = null;
	_moveSettings       = null;
	_outsideScreenTimer = 0.0;
	
	function onInit()
	{
		base.onInit();
		
		if (firstWaypoint == null)
		{
			editorWarning("firstWaypoint needs to be set");
			return;
		}
		
		_moveSettings = PhysicsSettings(0.5, 1.0, 0.0, 5, 1.0);
		_moveSettings.setCollisionWithSolid(false);
		startMovementInDirection(::Vector2(0, 0), _moveSettings);
		
		::removeEntityFromWorld(this);
	}
	
	function onSpawn()
	{
		base.onSpawn();
		
		if (enabled)
		{
			// Hack to make sure the initial setEnabled doesn't do an early exit
			enabled = false;
			setEnabled(true);
		}
	}
	
	_previousFollowedEntity = null;
	function setEnabled(p_enabled)
	{
		if (p_enabled == enabled)
		{
			return;
		}
		
		enabled = p_enabled;
		if (enabled)
		{
			_previousFollowedEntity = ::Camera.getPrimaryFollowEntity();
			::Camera.setPrimaryFollowEntity(this);
			
			local playerBot = ::getFirstEntityByTag("PlayerBot");
			if (playerBot != null)
			{
				if (autoMovePlayer)
				{
					playerBot.setReferenceSpeedEntity(this);
				}
				
				if (startAtPlayerPosition)
				{
					setPosition(playerBot.getCenterPosition());
				}
			}
			
			if (firstWaypoint != null)
			{
				_waypoint  = firstWaypoint;
				moveToWayPoint();
			}
		}
		else
		{
			stopMovement();
			::Camera.setPrimaryFollowEntity(_previousFollowedEntity);
			local playerBot = ::getFirstEntityByTag("PlayerBot");
			if (playerBot != null)
			{
				playerBot.setReferenceSpeedEntity(null);
			}
		}
	}
	
	function update(p_deltaTime)
	{
		if (enabled == false)
		{
			return;
		}
		
		local playerBot = ::getFirstEntityByTag("PlayerBot");
		if (playerBot == null)
		{
			return;
		}
		
		local newSpeed = playerBot.getSpeed();
		
		// Check if player bumps into walls and account for that by providing equal but opposite speed
		{
			local localSpeed = getSpeed();
			local touchingWall = ::getTouchingSolid(playerBot);
			if (touchingWall == Direction_Right)
			{
				newSpeed.x = ::min(-localSpeed.x, newSpeed.x);
			}
			else if (touchingWall == Direction_Left)
			{
				newSpeed.x = ::max(-localSpeed.x, newSpeed.x);
			}
		
			if (touchingWall == Direction_Up)
			{
				newSpeed.y = ::min(-localSpeed.y, newSpeed.y);
			}
			else if (touchingWall == Direction_Down)
			{
				newSpeed.y = ::max(-localSpeed.y, newSpeed.y);
			}
			
			playerBot.setSpeed(newSpeed);
		}
		
		// Do boundary logic
		// Translate to screenspace
		local screenRect = ::Camera.getScreenSpaceRect();
		local screenPos = ::Camera.worldToScreen(playerBot.getCenterPosition());
		
		if (screenRect.containsCircle(screenPos, _edgeRadius) == false)
		{
			local currentRotation = ::Camera.getCurrentRotation();
			
			// Compensate for rotation
			newSpeed = newSpeed.rotate(currentRotation);
			
			// X boundaries
			if (screenPos.x - _edgeRadius < screenRect.getLeft())
			{
				newSpeed.x = ::max(_bounceSpeed, newSpeed.x);
			}
			else if (screenPos.x + _edgeRadius > screenRect.getRight())
			{
				newSpeed.x = ::min(-_bounceSpeed, newSpeed.x);
			}
			
			// Y boundaries
			if (screenPos.y + _edgeRadius > screenRect.getTop())
			{
				newSpeed.y = ::min(-_bounceSpeed, newSpeed.y);
			}
			else if (screenPos.y - _edgeRadius < screenRect.getBottom())
			{
				newSpeed.y = ::max(_bounceSpeed, newSpeed.y);
			}
			// Rotate back
			newSpeed = newSpeed.rotate(-currentRotation);
			
			playerBot.setSpeed(newSpeed);
		}
		
		local outsideScreen = screenRect.intersectsCircle(screenPos, _edgeRadius) == false;
		if (outsideScreen)
		{
			_outsideScreenTimer += p_deltaTime;
		}
		else
		{
			_outsideScreenTimer = 0.0;
		}
		
		if (_outsideScreenTimer >= _outsideScreenDamageDelay)
		{
			playerBot._healthBar.doDamage(_outsideScreenDamage * 60 * p_deltaTime, this);
		}
	}
	
	function moveToWayPoint()
	{
		local scaledThrust = thrust * _waypoint.customScale;
		if (scaledThrust < 0.0)
		{
			_waypoint.editorWarning("Cannot use negative customScale for CameraOnRails");
			scaledThrust = thrust;
		}
		_moveSettings.setThrust(scaledThrust);
		startMovementToEntityEx(_waypoint, _waypoint.getCenterOffset(), _moveSettings);
	}
	
	function onMovementEnded(p_direction)
	{
		if (_waypoint.getNextWaypoint() != null)
		{
			if (_waypoint.autoAdvance)
			{
				_waypoint = _waypoint.getNextWaypoint();
				moveToWayPoint();
			}
			else
			{
				_waypoint._enableEvent.subscribe(this, "onWaypointEnabled");
			}
		}
		else
		{
			setEnabled(false);
		}
	}
	
	function onWaypointEnabled(p_waypoint, p_enabled)
	{
		if (_waypoint.equals(p_waypoint))
		{
			_waypoint = _waypoint.getNextWaypoint();
			moveToWayPoint();
		}
	}
}
