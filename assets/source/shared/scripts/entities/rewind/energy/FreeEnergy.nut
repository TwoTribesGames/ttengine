//include_entity("rewind/RewindEntity");

class FreeEnergy extends EntityBase
</
	editorImage    = "editor.freeenergy"
	libraryImage   = "editor.library.freeenergy"
	placeable      = Placeable_Hidden
	movementset    = "Collectible"
	collisionRect  = [ 0.0, 0.0, 0.5, 0.5 ]  // center X, center Y, width, height
/>
{
	// Constants
	static c_mass         = 0.5;
	static c_energyAmount = 1;
	
	_targetEntity        = null;
	_moveSettings        = null;
	_moveSettingsZeroG   = null;
	_moveSettingsCollect = null;
	_particleEffect      = null;
	_isInWater           = false;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		setUpdateSurvey(true);
		setPositionCullingEnabled(false);
		setCanBeCarried(true);
		
		setDetectableBySight(false);
		setDetectableByTouch(true);
		setDetectableByLight(false);
		
		addProperty("noticeShredders");
		addProperty("noticeTrainTracks");
		addProperty("noticeCrush");
		removeProperty("noticeForceFields");
		
		local rect = getCollisionRect();
		setTouchShape(::CircleShape(0, rect.getWidth() / 2.0), rect.getPosition());
		setSightDetectionPoints([::Vector2(0, 0)]);
		
		setCanBePushed(true);
		
		_moveSettings      = PhysicsSettings(c_mass, 1.0, 10, 0.0, -1.0);
		_moveSettings.setCollisionIsMovementFailure(true);
		_moveSettingsZeroG = PhysicsSettings(c_mass, 0.5, 0, 0.0, -1.0);
		_moveSettings.setCollisionIsMovementFailure(true);
		
		_moveSettingsCollect = PhysicsSettings(c_mass, 8, 300, 0.1, 1.0);
		_moveSettingsCollect.setCollisionWithSolid(false);
		
		_moveSettings.setExtraForce(::Vector2(0, -70));
		_moveSettings.setBouncyness(0.4);
		_moveSettings.setCollisionDrag(5.0);
		_moveSettingsZeroG.setBouncyness(0.9);
	}
	
	function onSpawn()
	{
		_particleEffect = spawnParticleContinuous("particles/nutsnbolts_effect", ::Vector2(0, 0), true, 0, false, ParticleLayer_UseLayerFromParticleEffect, 1.0);
		
		local direction = getRandomDirectionVector();
		startMovementInDirection(direction, _moveSettings);
		local speed = direction * ::frnd_minmax(20, 30);
		speed += ::Vector2(0, ::frnd_minmax(15, 20)); // account for gravity FIXME: Account for zero G
		setSpeed(speed);
		startTimer("enableSight", 0.25);
		startTimer("killme", ::frnd_minmax(8, 9));
		
		registerZeroGravityAffectedEntity(this);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	
	function onMovementEnded(p_direction)
	{
		collect();
	}
	
	function onTimer(p_name)
	{
		if (p_name == "force")
		{
			_moveSettingsCollect.setExtraForce(::Vector2(0, 0));
			startMovementToEntityEx(_targetEntity, _targetEntity.getCenterOffset(), _moveSettingsCollect);
		}
		else if (p_name == "enableSight")
		{
			setDetectableBySight(true);
		}
		else if (p_name == "fallback")
		{
			collect();
		}
		else if (p_name == "boost")
		{
			if (_targetEntity != null)
			{
				_moveSettingsCollect.setThrust(500);
				startMovementToEntityEx(_targetEntity, _targetEntity.getCenterOffset(), _moveSettingsCollect);
			}
		}
		else if (p_name == "killme")
		{
			::killEntity(this);
		}
	}
	
	function onPresentationObjectEnded(p_object, p_name)
	{
		if (p_name == "collected")
		{
			::killEntity(this);
		}
	}
	
	function onCrushed(p_crusher)
	{
		::killEntity(this);
	}
	
	function onDie()
	{
		if (_particleEffect != null)
		{
			_particleEffect.stop(false);
			_particleEffect = null;
		}
		unregisterZeroGravityAffectedEntity(this);
		
		if (_targetEntity != null && _targetEntity._parent != null)
		{
			_targetEntity._parent.unsubscribeDeathEvent(this, "onTargetDied");
		}
	}
	
	function onLavafallEnclosedEnter()
	{
		spawnParticleOneShot("particles/lavafall_splash_bullet", getCenterPosition(), false, 0, true, ParticleLayer_UseLayerFromParticleEffect, 1.0);
		::killEntity(this);
	}
	
	function onLavaEnclosedEnter()
	{
		spawnParticleOneShot("particles/lava_splash_bullet", getCenterPosition(), false, 0, true, ParticleLayer_UseLayerFromParticleEffect, 1.0);
		::killEntity(this);
	}
	
	function onWaterEnclosedEnter()
	{
		_isInWater = true;
		// HACK: Reused onZeroGravityEnter
		setSpeed(getSpeed() / 4.0);
		onZeroGravityEnter(null);
	}
	
	function onWaterEnclosedExit()
	{
		_isInWater = false;
		// HACK: Reused onZeroGravityEnter
		onZeroGravityExit(null);
	}
	
	function onZeroGravityEnter(p_source)
	{
		// Only do this if not already collecting
		if (_targetEntity == null)
		{
			startMovementInDirection(getSpeed().normalize(), _moveSettingsZeroG);
		}
	}
	
	function onZeroGravityExit(p_source)
	{
		// Only do this if not already collecting
		if (_targetEntity == null)
		{
			startMovementInDirection(::Vector2(0, 0), _moveSettings);
		}
	}
	
	function onShredderTouch(p_shredder)
	{
		::killEntity(this);
	}
	
	function onTargetDied(p_target)
	{
		_targetEntity = null;
		stopTimer("fallback");
		stopTimer("force");
		
		if (::isInZeroGravity(this) || _isInWater)
		{
			onZeroGravityEnter(null);
		}
		else
		{
			onZeroGravityExit(null);
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods
	
	function getMass()
	{
		return c_mass;
	}
	
	function startCollection(p_entity)
	{
		if (::isValidEntity(_targetEntity) == false)
		{
			_particleEffect = spawnParticleContinuous("particles/nutsnbolts_collecttrail", ::Vector2(0, 0), true, 0, false, ParticleLayer_UseLayerFromParticleEffect, 1.0);
			playSoundEffect("collectible_collect");
			_targetEntity = p_entity.weakref();
			
			_moveSettingsCollect.setExtraForce(::Vector2(::frnd_minmax(-500, 500), ::frnd_minmax(0, 275)));
			startMovementToEntityEx(_targetEntity, _targetEntity.getCenterOffset(), _moveSettingsCollect);
			unregisterZeroGravityAffectedEntity(this);
			setDetectableByTouch(false);
			stopTimer("killme");
			startTimer("fallback", 10.0);
			startTimer("force", 0.1);
			startTimer("boost", ::frnd_minmax(2.0, 3.0));
			
			if (_targetEntity._parent != null)
			{
				_targetEntity._parent.subscribeDeathEvent(this, "onTargetDied");
			}
		}
	}
	
	function collect()
	{
		if (::isValidEntity(_targetEntity))
		{
			_targetEntity.customCallback("onCollectedEnergy", c_energyAmount);
			
			// Spawn particle effect in worldspace because entity will die immediately after
			spawnParticleOneShot("particles/nutsnbolts_collected", getPosition(), false, 0, true, ParticleLayer_UseLayerFromParticleEffect, 2.0);
			playSoundEffect("collectible_collected");
			
			::killEntity(this);
		}
		
		stopTimer("fallback");
		stopTimer("force");
	}
}
