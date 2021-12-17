include_entity("rewind/enemies/BaseEnemy");

class Cockroach extends BaseEnemy
</
	editorImage    = "editor.cockroach"
	libraryImage   = "editor.library.cockroach"
	placeable      = Placeable_Everyone
	movementset    = "StaticIdle"
	collisionRect  = [ 0.0, 0.5, 1.0, 1.0 ]
	group          = "01. Enemies"
/>
{
	</
		type        = "string"
		choice      = ["Detect", "left", "right", "up", "down"]
		description = "Choose a specific direction to override the auto detection of the floor direction"
		group       = "StickTo"
	/>
	stickToCollision = "Detect";
	
	</
		type   = "string"
		choice = ["Random", "left", "right"]
		order  = 1
	/>
	orientation = "Random";
	
	// Constants
	static c_registryKey      = "killed_cockroaches";
	static c_maxHealth        = 1;
	static c_score            = 0;
	static c_rumbleStrength   = null;
	
	_minWalkDistance   = 3;
	_maxWalkDistance   = 8;
	
	_isFalling = false;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit/onSpawn
	
	function onCreate(p_id)
	{
		if (::Level.isChallenge(::Level.getMissionID()))
		{
			return true;
		}
		
		return isDisposed(getUniqueID(p_id)) == false;
	}
	
	function onInit()
	{
		base.onInit();
		
		setUpdateSurvey(true);
		setCanBeCarried(true);
		
		removeProperty("attractHomingMissiles");
		removeProperty("noticeVirusUploader");
		removeProperty("noticeFlames");
		removeProperty("opensPipeGates");
		addProperty("noRandomPickupsOnDie");
		addProperty("noCameraShakeOnDie");
		
		registerEntityByTag("Cockroach");
		
		_presentation = createPresentationObject("presentation/cockroach");
		_presentation.setAffectedByMovement(true);
		
		if (stickToCollision == "Detect")
		{
			setFloorDirection(::getTouchingSolid(this, Direction_Down)); // Fall back to down when it's not touching anything
		}
		else
		{
			local direction = getDirectionFromName(stickToCollision);
			if (isValidDirection(direction) == false) direction = Direction_Down;
			setFloorDirection(direction);
		}
		
		if (orientation == "Random")
		{
			setForwardAsLeft(brnd());
		}
		else
		{
			setForwardFromString(this, orientation);
		}
		
		setMovementSet("Cockroach");
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onEnabled()
	{
		base.onEnabled();
		
		setState("Walking");
	}
	
	function onDisabled()
	{
		base.onDisabled();
		
		setState("");
	}
	
	function onFall(p_moveName)
	{
		_isFalling = true;
	}
	
	function onLand(p_moveName)
	{
		_isFalling = false;
	}
	
	function onLavaTouchEnter()
	{
	}
	
	function onProgressRestored(p_id)
	{
		if (::Level.isChallenge(::Level.getMissionID()))
		{
			return;
		}
		
		if (isDisposed(getUniqueID(getID())))
		{
			// Already killed
			addProperty("dieQuietly");
			::killEntity(this);
		}
	}
	
	function onDie()
	{
		base.onDie();
		
		if (::Level.isChallenge(::Level.getMissionID()))
		{
			return;
		}
		
		if (hasProperty("dieQuietly") == false)
		{
			local table = ::getRegistry().getPersistent(c_registryKey);
			if (table == null)
			{
				table = {};
			}
			
			local id = getUniqueID(getID());
			if ((id in table) == false)
			{
				table[id] <- true;
				::getRegistry().setPersistent(c_registryKey, table);
				::Stats.incrementStat("killed_cockroaches");
			}
			else
			{
				::tt_panic("Cockroach '" + id + "' was already in table");
			}
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods
	
	function getUniqueID(p_id)
	{
		return ::Level.getName() + p_id;
	}
	
	function isDisposed(p_id)
	{
		local table = ::getRegistry().getPersistent(c_registryKey);
		if (table == null)
		{
			return false;
		}
		return (p_id in table);
	}
	
	function reverseDirection()
	{
		stopTimer("moveforward");
		flipForwardDir();
		startMoveForwardTimer();
	}
	
	function startMoveForwardTimer()
	{
		startTimer("moveforward", ::frnd_minmax(0.5, 0.75));
	}
	
	function moveForward()
	{
		if (_isFalling == false)
		{
			local moveStarted = startMovement(getForwardDir(), ::frnd_minmax(_maxWalkDistance, _minWalkDistance));
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
class Cockroach_State.Walking
{
	function onEnterState()
	{
		moveForward();
	}
	
	function onExitState()
	{
		stopTimer("moveforward");
	}
	
	function onMovementEnded(p_direction)
	{
		startMoveForwardTimer();
	}
	
	// Use movement failed to detect whether we hit a wall
	function onMovementFailed(p_direction, p_moveName)
	{
		if (getLocalDirFromDirection(p_direction) == LocalDir_Forward)
		{
			reverseDirection();
		}
	}
	
	function onPresentationObjectStopped(p_object, p_name)
	{
		base.onPresentationObjectStopped(p_object, p_name);
		
		if (p_name == "turn")
		{
			moveForward();
		}
	}
	
	function onLand(p_moveName)
	{
		base.onLand(p_moveName);
		startMoveForwardTimer();
	}
	
	function onTimer(p_name)
	{
		//base.onTimer(p_name);
		
		if (p_name == "moveforward")
		{
			moveForward();
		}
	}
}