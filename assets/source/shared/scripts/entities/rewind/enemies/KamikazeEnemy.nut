include_entity("rewind/enemies/BaseScourEnemy");

class KamikazeEnemy extends BaseScourEnemy
</
	editorImage         = "editor.enemykamikaze"
	libraryImage        = "editor.library.enemykamikaze"
	movementset         = "Static"
	placeable           = Placeable_Everyone
	collisionRect       = [ 0.0, 0.5, 1.85, 1.85 ]
	pathFindAgentRadius = 0.666
	pathCrowdSeparation = true
	group               = "01. Enemies"
/>
{
	</
		type        = "bool"
		order       = 1
		description = "Enable to have super fast moving Kamikazes with a large sight range"
	/>
	fastMode = false;
	
	// Constants
	static c_maxHealth                  = 5;
	static c_mass                       = 3.0;
	static c_score                      = 100;
	static c_randomSpeedAngle           = [-15, 15];
	static c_randomSpeedDelay           = [0.1, 0.25];
	static c_randomSpeedMultiplier      = 1.0;
	static c_dazedByEMPTimeout          = 2.5;
	static c_pickupDropCount            = 8;
	static c_isAffectedByVirus          = true;
	static c_waypointThrust             = 120;
	static c_energy                     = 5;
	static c_decreaseVirusHealthTimeout = 0.5;
	static c_decreaseVirusHealthAmount  = 0.1;
	static c_healthMultiplierWhenHacked = 3.0;
	
	_sightSensor  = null;
	_moveSettings = null;
	
	// The hunted entity
	_targetEntity = null;
	
	// The particle effect for when the player is spotted
	_spottedEffect = null;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		base.onInit();
		
		local sightRange = fastMode ? 200 : 35;
		
		if (fastMode)
		{
			_moveSettings         = configureMovement(c_mass, 9.0, ::frnd_minmax(590.0, 700.0), 2.5, 0.15, "fly");
			_waypointMoveSettings = configureMovement(c_mass, 9.0, c_waypointThrust * 2.5, 2.5, 0.15, "fly");
		}
		else
		{
			_moveSettings         = configureMovement(c_mass, ::frnd_minmax(8.0, 9.0), ::frnd_minmax(220.0, 260.0), -1, -1, "fly");
			_waypointMoveSettings = configureMovement(c_mass, ::frnd_minmax(8.0, 9.0), c_waypointThrust * 2.0, 0.5, 1.0, "fly");
		}
		_moveSettings.setCollisionIsMovementFailure(false);
		_waypointMoveSettings.setCollisionIsMovementFailure(false);
		
		_scourMoveSettings  = configureMovement(c_mass, ::frnd_minmax(4.0, 5.0), ::frnd_minmax(40.0, 50.0), 2.5, 1.0, "fly");
		_goHomeMoveSettings = configureMovement(c_mass, ::frnd_minmax(4.0, 5.0), ::frnd_minmax(80.0, 90.0), 2.5, 1.0, "fly");
		
		// Add width of entity to sightrange
		sightRange += getCollisionRect().getWidth();
		
		local player = ::getFirstEntityByTag("PlayerBot");
		if (player != null)
		{
			_sightSensor = addSightSensor(::CircleShape(0, sightRange), player);
			_sightSensor.setEnterCallback("onSightEnter");
			_sightSensor.setDelay(::frnd_minmax(0.1, 0.25));
			_sightSensor.setStopOnWaterPool(true); // He won't see you while you are under water!
		}
		
		_presentation = createPresentationObject("presentation/"+ getType().tolower());
		_presentation.setAffectedByMovement(true);
		
		local rect = getCollisionRect();
		setTouchShape(::CircleShape(0, rect.getWidth() / 2), rect.getPosition());
		
		addProperty("touchDamage", 0);
		addProperty("healedByHealthBot");
		addProperty("shotgunKnockbackScale", 68);
		removeProperty("reduceSpeedWhenEnteringWater");
	}
	
	function onSpawn()
	{
		base.onSpawn();
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onTimer(p_name)
	{
		base.onTimer(p_name);
		
		if (p_name == "decreaseVirusHealth")
		{
			decreaseVirusHealth(c_decreaseVirusHealthAmount);
			startTimer("decreaseVirusHealth", c_decreaseVirusHealthTimeout);
		}
	}
	
	function onEnabled()
	{
		base.onEnabled();
		
		// HACK: Make sure the scour state is set (don't do this when in hacked state; causes weird bugs)
		if (containsVirus() == false)
		{
			updateState();
		}
		
		if (_sightSensor != null)
		{
			_sightSensor.setSuspended(false);
		}
		
		if (_targetEntity != null)
		{
			pushState("MoveToTarget");
		}
	}
	
	function onDisabled()
	{
		base.onDisabled();
		
		if (_sightSensor != null)
		{
			_sightSensor.setSuspended(true);
			_sightSensor.removeAllSensedEntities();
		}
	}
	
	function onDazedEnter()
	{
		base.onDazedEnter();
		
		// There should be no more target after dazed
		_targetEntity = null;
	}
	
	_zeroGravityField     = null;
	_soundEffectCue       = null;
	_scourRestoreSettings = null;
	function onVirusUploaded(p_entity)
	{
		base.onVirusUploaded(p_entity);
		
		if (_zeroGravityField == null)
		{
			//Spawn zero gravity field
			_zeroGravityField = ::spawnEntity("ZeroGravityGenerator", getCenterPosition(),
				{
					displayGenerator = false,
					stickToEntity = this
				}
			);
		}
		_moveSettings.setThrust(220);
		_moveSettings.setDrag(8);
		_moveSettings.setCollisionDrag(8);
		startTimer("decreaseVirusHealth", c_decreaseVirusHealthTimeout);
		stopTimer("onRandomSpeed");
		_scourRestoreSettings = getScourSettings();
		removeProperty("touchDamage");
		addProperty("weaponGroup", ::WeaponGroup.Player);
		
		// Immediately move to the entity that uploaded the virus
		_targetEntity = p_entity.weakref();
		pushState("MoveToTarget");
	}
	
	function onVirusRemoved()
	{
		base.onVirusRemoved();
		
		::killEntity(this);
	}
	
	function onVirusHealthEmpty()
	{
		::killEntity(this);
	}
	
	function onDie()
	{
		if (_soundEffectCue != null)
		{
			_soundEffectCue.stop();
			_soundEffectCue = null;
		}
		
		if (_spottedEffect != null)
		{
			_spottedEffect.stop(false);
			_spottedEffect = null;
		}
		
		::killEntity(_zeroGravityField);
		
		base.onDie();
	}
	
	function onTouchDamageDealt(p_source)
	{
		if (isEnabled() && containsVirus() == false)
		{
			removeProperty("touchDamage");
			addProperty("noRandomPickupsOnDie");
			createExplosion(getCenterPosition(), getCollisionRect().getWidth() + 2, this);
			customCallback("onExplode");
		}
	}
	
	function onSightEnter(p_entity, p_sensor)
	{
		_targetEntity = p_entity.weakref();
		
		pushState("MoveToTarget");
	}
	
	function onBulletHit(p_bullet)
	{
		base.onBulletHit(p_bullet);
		
		if (isEnabled() == false)
		{
			return;
		}
		
		setSpeed(getSpeed() * 0.9);
		
		if (p_bullet._shooter instanceof ::PlayerBot)
		{
			_targetEntity = p_bullet._shooter.weakref();
			
			pushState("MoveToTarget");
		}
	}
	
	function onExplosionHit(p_explosion)
	{
		if (p_explosion._shooter instanceof ::KamikazeEnemy)
		{
			return;
		}
		
		base.onExplosionHit(p_explosion);
		
		if (isEnabled() == false)
		{
			return;
		}
		
		setSpeed(getSpeed() * 0.7);
		
		if (p_explosion.isFromPlayer())
		{
			_targetEntity = p_explosion._shooter.weakref();
			
			pushState("MoveToTarget");
		}
	}
	
	function onLaserHit(p_laser)
	{
		base.onLaserHit(p_laser);
		
		if (isEnabled() == false)
		{
			return;
		}
		
		setSpeed(getSpeed() * 0.9);
		
		if (p_laser._parent instanceof ::PlayerBot)
		{
			_targetEntity = p_laser._parent.weakref();
			
			pushState("MoveToTarget");
		}
	}
	
	function onRandomSpeed()
	{
		if (containsVirus())
		{
			return;
		}
		
		local speed = getSpeed();
		local angle = ::getAngleFromVector(speed);
		angle += ::frnd_minmax(c_randomSpeedAngle[0], c_randomSpeedAngle[1]);
		setSpeed(::getVectorFromAngle(angle) * speed.length() * c_randomSpeedMultiplier);
		
		startCallbackTimer("onRandomSpeed", ::frnd_minmax(c_randomSpeedDelay[0], c_randomSpeedDelay[1]));
	}
}


class KamikazeEnemy_State.MoveToTarget
{
	function onEnterState()
	{
		if (::isValidEntity(_targetEntity) == false)
		{
			popState();
			return;
		}
		
		startCallbackTimer("onRandomSpeed", ::frnd_minmax(c_randomSpeedDelay[0], c_randomSpeedDelay[1]));
		
		// Don't cull when chasing
		setPositionCullingEnabled(false);
		
		_presentation.addTag("MoveToTarget");
		_spottedEffect = spawnParticleOneShot("particles/kamikaze_spot_playerbot", ::Vector2(0,0), true, 0, false, ParticleLayer_UseLayerFromParticleEffect, getCollisionRect().getWidth() * 0.66);
		playSoundEffect("kamikaze_armed");
		startPathMovementToEntity(_targetEntity, _targetEntity.getCenterOffset(), _moveSettings);
	}
	
	function onExitState()
	{
		// Restore position culling
		setPositionCullingEnabled(positionCulling);
		
		stopMovement();
		stopTimer("onRandomSpeed");
		// FIXME: Do not manually start presentation animations on onExitState
		_presentation.removeTag("MoveToTarget");
		startAllPresentationObjects("idle", []);
	}
	
	function onPathMovementFailed(p_direction)
	{
		popState();
	}
	
	function onMovementFailed(p_direction, p_moveName)
	{
		popState();
	}
	
	function onMovementEnded(p_direction)
	{
		if (::isValidEntity(_targetEntity))
		{
			startPathMovementToEntity(_targetEntity, ::Vector2(0.0, 0.0), _moveSettings);
		}
	}
	
	function onDisabled()
	{
		popState();
		base.onDisabled();
	}
	
	function onWaterEnclosedEnter()
	{
		popState();
	}
}
