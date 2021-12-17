include_entity("rewind/pickups/PickupCollectEffect");
include_entity("rewind/RewindEntity");

class Pickup extends RewindEntity
</
	placeable      = Placeable_Hidden
	movementset    = "Static"
	collisionRect  = [0.0, 0.75, 1.5, 1.5] // center X, center Y, width, height
	workshopTags   = ["Collectibles"]
	stickToEntity  = true
/>
{
	// Constants
	static c_mass              = 0.5;
	static c_sensorRecheckTime = 0.1;
	static c_touchSensorRadius = 2.0;
	static c_callbackName      = null; // The callback that will be fired to the collecting entity
	static c_presentationName  = null;
	static c_rumbleStrength    = null;
	
	// Creation members
	_timeout                   = null; // If set, pickup will be removed after X amount of seconds
	
	// Internal members
	_touchSensor               = null;
	_presentation              = null;
	_moveSettings              = null;
	_moveSettingsZeroG         = null;
	_collectingEntity          = null;
	_isCollecting              = false;
	
	// Inherited members
	positionCulling            = false;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		base.onInit();
		
		setCanBeCarried(true);
		setUpdateSurvey(true);
		
		addProperty("noticeCrush");
		addProperty("noticeShredders");
		addProperty("noticeTrainTracks");
		removeProperty("noticeForceFields");
		addProperty("noticeDeathRays");
		addProperty("hasCorpse", ::CorpseParams("Corpse", { _presentationFile = c_presentationName, _rumbleStrength = c_rumbleStrength }));
		
		enableDefaultWaterInteraction();
		
		_presentation = createPresentationObject("presentation/" + c_presentationName);
		
		setCanBePushed(true);
		
		// Touch sensor
		local player = ::getFirstEntityByTag("PlayerBot");
		if (player != null)
		{
			_touchSensor = addTouchSensor(::CircleShape(0, c_touchSensorRadius), player, ::Vector2(0, (c_touchSensorRadius / 4)));
			_touchSensor.setEnterCallback("onTouchEnter");
		}
		
		// Movement settings
		_moveSettings      = PhysicsSettings(c_mass, 0.5, 0, 0.0, -1.0);
		_moveSettingsZeroG = PhysicsSettings(c_mass, 2, 0, 0.0, -1.0);
		
		_moveSettings.setExtraForce(::Vector2(0, -50));
		_moveSettings.setBouncyness(0.25);
		_moveSettingsZeroG.setBouncyness(0.25);
		_moveSettings.setCollisionDrag(5.0);
		
		// Add random to ensure not all pickups recheck at the exact same time
		startCallbackTimer("recheck", c_sensorRecheckTime + ::frnd_minmax(0.0, 1.0));
		
		if (_timeout != null)
		{
			startCallbackTimer("destroy", _timeout);
			local blinkTimeout = ::max(0, _timeout - 5.0);
			startCallbackTimer("blink", blinkTimeout);
		}
	}
	
	function onSpawn()
	{
		base.onSpawn();
		
		_presentation.start("idle", [], false, 0);
		
		registerZeroGravityAffectedEntity(this);
		
		if (::isInZeroGravity(this) == false && isInWater() == false)
		{
			startMovementInDirection(::Vector2(0, 0), _moveSettings);
		}
		
		// If we have a playerbot as parent we get collect it instantly
		if (stickToEntity instanceof ::PlayerBot)
		{
			// Pickup collected, don't show corpse anymore
			removeProperty("hasCorpse");
			
			stickToEntity.customCallback(c_callbackName, this);
			::killEntity(this);
		}
	}
	
	function onDropped(p_source)
	{
		if (p_source instanceof ::Dropship)
		{
			// Set position based on the location of the dropship's hatch in all size variations.
			local sourceHeight = p_source.getCollisionRect().getHeight();
			local sourcePosition = p_source.getCenterPosition();
			setPosition(::Vector2(sourcePosition.x, sourcePosition.y - (sourceHeight / 3)));
			
			local speedFactor = (sourceHeight / 7.0);
			
			setSpeed(p_source.getSpeed() + ::Vector2(0, -20 * speedFactor));
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onCrushed(p_crusher)
	{
		::killEntity(this);
	}
	
	function onScreenEnter()
	{
		if (hasTimer("destroy") == false)
		{
			// Once on screen, enable culling
			positionCulling = true;
			setPositionCullingEnabled(true);
		}
		removeProperty("dieQuietly");
	}
	
	function onScreenExit()
	{
		addProperty("dieQuietly");
	}
	
	function onTouchEnter(p_entity, p_sensor)
	{
		if (::isValidEntity(p_entity))
		{
			collect(p_entity);
		}
	}
	
	function onWaterEnclosedEnter()
	{
		base.onWaterEnclosedEnter();
		
		// HACK: Reused onZeroGravityEnter
		setSpeed(getSpeed() / 4.0);
		onZeroGravityEnter(null);
	}
	
	function onWaterEnclosedExit()
	{
		base.onWaterEnclosedExit();
		
		// HACK: Reused onZeroGravityEnter
		onZeroGravityExit(null);
	}
	
	function onLavaTouchEnter()
	{
		spawnParticleOneShot("particles/lava_splash_" + getType().tolower(), getCenterPosition(), false, 0, true, ParticleLayer_UseLayerFromParticleEffect, 1.0);
		::killEntity(this);
	}
	
	function onLavaWaveTouch(p_lavawave)
	{
		spawnParticleOneShot("particles/lava_splash_" + getType().tolower(), getCenterPosition(), false, 0, true, ParticleLayer_UseLayerFromParticleEffect, 1.0);
		::killEntity(this);
	}
	
	function onLavafallEnclosedEnter()
	{
		spawnParticleOneShot("particles/lava_splash_" + getType().tolower(), getCenterPosition(), false, 0, true, ParticleLayer_UseLayerFromParticleEffect, 1.0);
		::killEntity(this);
	}
	
	function onDeathRayEnter(p_ray)
	{
		spawnParticleOneShot("particles/lava_splash_"  + getType().tolower(), getCenterPosition(), false, 0, true, ParticleLayer_UseLayerFromParticleEffect, 1.0);
		::killEntity(this);
	}
	
	function onZeroGravityEnter(p_source)
	{
		startMovementInDirection(getSpeed().normalize(), _moveSettingsZeroG);
		
		if (_presentation != null)
		{
			_presentation.addTag("zerogravity");
			_presentation.start("idle", [], false, 0);
		}
	}
	
	function onZeroGravityExit(p_source)
	{
		startMovementInDirection(::Vector2(0, 0), _moveSettings);
		
		if (_presentation != null)
		{
			_presentation.removeTag("zerogravity");
			_presentation.start("idle", [], false, 0);
		}
	}
	
	function onDie()
	{
		base.onDie();
		
		unregisterZeroGravityAffectedEntity(this);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods
	
	function blink()
	{
		_presentation.start("blink", [], false, 1);
	}
	
	function destroy()
	{
		::killEntity(this);
	}
	
	function getMass()
	{
		return c_mass;
	}
	
	function recheck()
	{
		if (_touchSensor != null)
		{
			_touchSensor.removeAllSensedEntities();
			startCallbackTimer("recheck", c_sensorRecheckTime);
		}
	}
	
	function collect(p_entity)
	{
		if (::isValidEntity(p_entity) == false || _isCollecting)
		{
			return;
		}
		
		_isCollecting = true;
		
		::setRumble(RumbleStrength_Low, 0.0);
		
		// Pickup collected, don't show corpse anymore
		removeProperty("hasCorpse");
		
		// No more water interaction
		removeFluidSettings(FluidType_Water);
		
		// Make sure we don't get another touch anymore
		_touchSensor.setEnabled(false);
		
		stopMovement();
		
		p_entity.customCallback(c_callbackName, this);
		
		// Start high priority collected animation as it should not be overwritten by anything else
		_presentation.start("collected", [], false, 2);
	}
}
