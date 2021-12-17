class RewindEntity extends EntityBase
</
	placeable = Placeable_Hidden
/>
{
	</
		type  = "bool"
		order = 99999998
		group = "Misc."
	/>
	positionCulling = true;
	
	_deathEvent     = null;
	_children       = null;
	_stateStack     = null;
	
	function onInvalidProperties(p_properties)
	{
		// Default behavior is to remove non-existent properties
		removeNonexistentProperties(this, p_properties);
		
		return p_properties;
	}
	
	function onInit()
	{
		// FIXME: Perhaps add another entity that is actually part of the world; RewindEntity is also used for
		// entities that you cannot interact with
		//addProperty("hasCorpse", ::CorpseParams());
		
		// Set detection and touch defaults
		/*	
		setDefaultLightDetectionPoints();
		setDefaultSightDetectionPoints();
		*/
		setDefaultTouchShape();
		
		setPositionCullingEnabled(positionCulling);
	}
	
	function onSpawn()
	{
		initStickToEntity();
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Culling
	
	function onCulled()
	{
	}
	
	function onUnculled()
	{
		// Process the states that were set just before being culled
		updateState();
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Update
	
	function update(p_deltaTime)
	{
		if (_children != null && _children.len() > 0)
		{
			foreach (handle, child in _children)
			{
				if (child.isSuspended() == false) child.childUpdate(p_deltaTime);
			}
		}
		
		updateStickToEntity();
		updateState();
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Parenting
	
	function subscribeDeathEvent(p_caller, p_callback)
	{
		if (_deathEvent == null)
		{
			_deathEvent = ::EventPublisher("deathevent");
		}
		_deathEvent.subscribe(p_caller, p_callback);
	}
	
	function unsubscribeDeathEvent(p_caller, p_callback)
	{
		_deathEvent.unsubscribe(p_caller, p_callback);
		if (_deathEvent.len() == 0)
		{
			_deathEvent = null;
		}
	}
	
	function addChild(p_type, p_offset = ::Vector2(0, 0), p_spawnProperties = {})
	{
		p_spawnProperties._parent <- this.weakref();
		
		local child = ::spawnEntity(p_type, getPosition() + p_offset, p_spawnProperties);
		
		subscribeDeathEvent(child, "onParentDied");
		child.setPositionCullingParent(this);
		child.setParentEntityWithOffset(this, p_offset);
		
		local handle = child.getHandleValue();
		if (_children == null)
		{
			_children = {};
		}
		_children[handle] <- child.weakref();
		
		return child.weakref();
	}
	
	function removeChild(p_child)
	{
		local handle = p_child.getHandleValue();
		if (handle in _children)
		{
			delete _children[handle];
		}
		
		if (_children.len() == 0)
		{
			_children = null;
		}
		unsubscribeDeathEvent(p_child, "onParentDied");
	}
	
	function callOnChildren(p_funcName, p_params = [])
	{
		if (_children == null || _children.len() == 0)
		{
			return;
		}
		
		if (typeof(p_params) != "array")
		{
			p_params = [p_params];
		}
		local args = clone p_params;
		
		args.insert(0, p_funcName);
		args.insert(0, null); // to make room for the child entity
		
		foreach (handle, child in _children)
		{
			if (child.isSuspended() == false)
			{
				args[0] = child;
				customCallback.acall(args);
			}
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// States
	
	function hasState(p_name)
	{
		return _stateStack != null && _stateStack.exists(p_name);
	}
	
	function setState(p_name, p_priority = StatePriority.Normal)
	{
		if (_stateStack == null)
		{
			_stateStack = ::StateStack();
		}
		_stateStack.set(p_name, p_priority);
	}
	
	function pushState(p_name, p_priority = StatePriority.Normal)
	{
		if (_stateStack == null)
		{
			_stateStack = ::StateStack();
		}
		_stateStack.push(p_name, p_priority);
	}
	
	function popState(p_priority = StatePriority.Normal)
	{
		if (_stateStack == null)
		{
			::tt_panic("No statestack, cannot popState");
			return;
		}
		_stateStack.pop(p_priority);
	}
	
	function replaceState(p_name, p_priority = StatePriority.Normal)
	{
		if (_stateStack == null)
		{
			::tt_panic("No statestack, cannot replaceState");
			return;
		}
		_stateStack.replace(p_name, p_priority);
	}
	
	function updateState()
	{
		if (_stateStack != null && _stateStack.process(this))
		{
			//echo(this + " new state " + _stateStack.top());
			// new state; set it!
			base.setState(_stateStack.top() == null ? "" : _stateStack.top());
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Animation helpers
	
	function onPresentationObjectCanceled(p_object, p_name)
	{
		// Generate a general callback for presentation objects that stopped for any reason
		customCallback("onPresentationObjectStopped", p_object, p_name);
	}
	
	function onPresentationObjectEnded(p_object, p_name)
	{
		// Generate a general callback for presentation objects that stopped for any reason
		customCallback("onPresentationObjectStopped", p_object, p_name);
	}
	
	// implement this too, so we can safely call base.onPresentationObjectStopped
	function onPresentationObjectStopped(p_object, p_name)
	{
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Property helpers
	
	// adds all properties to prevent entity from being killed by something
	function addInvincibilityProperties(p_removeCorpse = true)
	{
		removeProperty("noticeExplosions");
		removeProperty("noticeBullets");
		removeProperty("noticeCrush");
		removeProperty("noticeEMP");
		removeProperty("noticeShredders");
		removeProperty("attractHomingMissiles");
		removeProperty("noticeTrainTracks");
		addProperty("hiddenForBumperEnemies");
		addProperty("ignoreLasers");
		
		if (p_removeCorpse)
		{
			removeProperty("hasCorpse");
		}
	}
	
	function removeInvincibilityProperties()
	{
		addProperty("noticeExplosions");
		addProperty("noticeBullets");
		addProperty("noticeEMP");
		addProperty("noticeCrush");
		addProperty("noticeShredders");
		addProperty("attractHomingMissiles");
		addProperty("noticeTrainTracks");
		removeProperty("hiddenForBumperEnemies");
		removeProperty("ignoreLasers");
	}
	
	// these callbacks are called from the entity helper functions that alter the detection of entities
	function onMadeBackgroundEntity()
	{
		addInvincibilityProperties();
	}
	
	function onRemovedFromWorld()
	{
		removeProperty("hasCorpse");
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Sensor helpers
	
	function setDefaultSightDetectionPoints()
	{
		setSightDetectionPoints([::Vector2(0, 0)]);
	}
	
	function setDefaultLightDetectionPoints()
	{
		setLightDetectionPoints([::Vector2(0, 0)]);
	}
	
	function setDefaultTouchShape()
	{
		local rect = getCollisionRect();
		setTouchShape(::BoxShape(rect.getWidth() - 0.01, rect.getHeight() - 0.01), rect.getPosition());
	}
	
	function setCollisionRectWithVectorRect(p_rect)
	{
		base.setCollisionRectWithVectorRect(p_rect);
		setDefaultSightDetectionPoints();
		setDefaultTouchShape();
	}
	
	function addCrushSensor(p_crushDamage = null, p_crushIfContained = true)
	{
		local rect = getCollisionRect();
		local crushRect = null;
		
		if (p_crushIfContained)
		{
			crushRect = ::VectorRect(::Vector2(0, -0.2), rect.getWidth() + 1, rect.getHeight() + 0.4);
		}
		else
		{
			crushRect = ::VectorRect(::Vector2(0, 0), rect.getWidth() - 1, rect.getHeight() - 1.0);
		}
		return addCrushSensorEx(crushRect, p_crushDamage, p_crushIfContained);
	}
	
	function addCrushSensorEx(p_rect, p_crushDamage = null, p_crushIfContained = true)
	{
		local shape = ::BoxShape(p_rect.getWidth(), p_rect.getHeight());
		if (p_crushIfContained)
		{
			shape.doContains();
		}
		local crushSensor = addTouchSensor(shape, null, p_rect.getPosition());
		crushSensor.setEnterCallback("onCrushEnter");
		crushSensor.setFilterCallback("onCrushFilter");
		
		// If set, this crush sensor will not kill this entity but will inflict p_crushDamage on it instead
		if (p_crushDamage != null)
		{
			addProperty("crushDamage", p_crushDamage);
		}
		
		return crushSensor;
	}
	
	function setDetectableByTouchOnly()
	{
		setDetectableByTouch(true);
		setDetectableBySight(false);
		setDetectableByLight(false);
		
		removeAllSightDetectionPoints();
		removeAllLightDetectionPoints();
	}
	
	function setDetectableBySightOnly()
	{
		setDetectableByTouch(false);
		setDetectableBySight(true);
		setDetectableByLight(false);
		
		removeAllLightDetectionPoints();
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Direction helpers because this function is used quite often
	
	function getForwardDir()
	{
		return getDirectionFromLocalDir(LocalDir_Forward);
	}
	
	function getBackDir()
	{
		return getDirectionFromLocalDir(LocalDir_Back);
	}
	
	function flipForwardDir()
	{
		setForwardAsLeft(isForwardLeft() == false);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Movement helpers
	
	// Stops movement but lets the current move "step" finish
	function softStopMovement()
	{
		startMovement(Direction_None, 0);
	}
	
	function startPathMovementToEntity(p_entity, p_offset, p_moveSettings)
	{
		if (::isValidEntity(p_entity))
		{
			startPathMovement(p_offset, p_moveSettings, p_entity);
			return;
		}
		
		::tt_panic(this + " is trying to path move to entity but the entity '" + p_entity + "' is invalid");
	}
	
	function startPathMovementToPosition(p_position, p_moveSettings)
	{
		return startPathMovement(p_position, p_moveSettings, null);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Shared functionality for deaths
	
	function onHealthBarEmpty(p_healthBar, p_killer)
	{
		addProperty("killer", p_killer);
		::killEntity(this);
	}
	
	function onBulletHit(p_bullet)
	{
		if (hasProperty("noticeBullets"))
		{
			callOnChildren("onBulletHit", p_bullet);
		}
	}
	
	function onFlameHit(p_flame)
	{
		if (hasProperty("noticeFlames"))
		{
			callOnChildren("onFlameHit", p_flame);
		}
	}
	
	function onExplode()
	{
		if (hasProperty("hasCorpse"))
		{
			local prop = getProperty("hasCorpse");
			prop.addPresentationTags("burned");
		}
		::killEntity(this);
	}
	
	function onExplosionHit(p_explosion)
	{
		if (hasProperty("noticeExplosions"))
		{
			callOnChildren("onExplosionHit", p_explosion);
		}
	}
	
	function onLaserHit(p_laser)
	{
		if (hasProperty("ignoreLasers") == false)
		{
			callOnChildren("onLaserHit", p_laser);
		}
	}
	
	_isInWater = false;
	function isInWater()
	{
		return _isInWater;
	}
	
	function onWaterTouchEnter()
	{
	}
	
	function onWaterTouchExit()
	{
	}
	
	function onWaterEnclosedEnter()
	{
		_isInWater = true;
		callOnChildren("onWaterEnter");
	}
	
	function onWaterEnclosedExit()
	{
		_isInWater = false;
		callOnChildren("onWaterExit");
	}
	
	function onLavaTouchEnter()
	{
	}
	
	function onLavafallTouchEnter()
	{
	}
	
	function onCrushed(p_crusher)
	{
		//FIXME: remove leftover from TT2? Rewind still has crushing (pistons)
		//::g_CrushAchievement.addCompletedEntity(this);
		if (p_crusher.hasProperty("crushDamage"))
		{
			if ("_healthBar" in this && _healthBar != null)
			{
				_healthBar.doDamage(p_crusher.getProperty("crushDamage"), p_crusher);
			}
			return;
		}
		
		if (hasProperty("hasCorpse"))
		{
			local prop = getProperty("hasCorpse");
			prop.addPresentationTags("crushed");
		}
		addProperty("killer", p_crusher);
		::killEntity(this);
	}
	
	function onTrainTrackTouch(p_trainTrack)
	{
		addProperty("killer", p_trainTrack);
		::killEntity(this);
	}
	
	function onShredderTouch(p_shredder)
	{
		addProperty("killer", p_shredder);
		::killEntity(this);
	}
	
	function onCrushFilter(p_entity)
	{
		return p_entity.hasProperty("noticeCrush");
	}
	
	function onCrushEnter(p_entity, p_sensor)
	{
		p_entity.customCallback("onCrushed", this);
	}
	
	function onPreSpawnSectionKill()
	{
		removeProperty("hasCorpse");
		addProperty("dieQuietly");
	}
	
	function onDie()
	{
		if (_deathEvent != null)
		{
			_deathEvent.publish(this);
		}
		
		// spawn corpse 
		local prop = getProperty("hasCorpse");
		if (prop != null && hasProperty("dieQuietly") == false)
		{
			// clone the spawn properties 
			local spawnProperties = clone prop.spawnProperties;
			
			if (prop.overrideCorpseEntity == false)
			{
				// add the presentation filename automatically if it was left empty
				if (("_presentationFile" in spawnProperties) == false)
				{
					spawnProperties["_presentationFile"] <- getType().tolower();
				}
			}
			
			if (prop.copyOrientation)
			{
				spawnProperties["_isForwardLeft"] <- isForwardLeft();
			}
			if (prop.copyFloorDirection)
			{
				spawnProperties["_floorDir"] <- getFloorDirection();
			}
			
			// make sure the collision rects match
			spawnProperties["_collisionRect"] <- getCollisionRect();
			
			// copy the culling parameter too
			spawnProperties["positionCulling"] <- positionCulling;
			
			// Copy the presentation layer from the main presentation (if _presentation exists)
			// FIXME: This silently assumes that ALL RewindEntities use _presentation as their main
			// presentation. This might not be the case.
			if ("_presentation" in this && _presentation != null)
			{
				spawnProperties["_presentationLayer"] <- _presentation.getLayer();
			}
			
			// spawn the corpse
			::spawnEntity(prop.corpseEntity, getPosition(), spawnProperties);
		}
	}
	
	function onPresentationObjectCallback(p_object, p_name)
	{
		// Default fallback for removing presentations using callbacks
		if (p_name == "remove")
		{
			p_object.stop();
			destroyPresentationObject(p_object);
		}
		else if (p_name == "kill_parent")
		{
			::killEntity(this);
		}
		else if (p_name == "kill_parent_quietly")
		{
			addProperty("dieQuietly");
			::killEntity(this);
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Fluids
	
	function enableDefaultWaterInteraction()
	{
		local settings = addFluidSettings(FluidType_Water);
		settings.setWaveGenerationEnabled(true);
		
		local particles = settings.getSurfaceParticleSettings();
		particles.setEnabled(true);
		particles.setTriggerFile("particles/water_surface.trigger");
		settings.setSurfaceParticleSettings(particles);
		
		local splashParticles = settings.getUnderFallParticleSettings();
		splashParticles.setEnabled(true);
		splashParticles.setTriggerFile("particles/water_underfall.trigger");
		settings.setUnderFallParticleSettings(splashParticles);
		
		local size = getCollisionRect().getWidth();
		
		local enterCue            = "entity_water_enter_medium";
		local exitCue             = "entity_water_exit_medium";
		//local inFluidCue          = "entity_water_idle_medium";
		
		if (size <= 1)
		{
			enterCue            = "entity_water_enter_medium";
			exitCue             = "entity_water_exit_small";
			//inFluidCue          = "entity_water_idle_small";
		}
		else if (size >= 3)
		{
			enterCue            = "entity_water_enter_big";
			exitCue             = "entity_water_exit_big";
			//inFluidCue          = "entity_water_idle_big";
		}
		
		settings.setEnterFluidSoundCue(enterCue);
		settings.setExitFluidSoundCue(exitCue);
		settings.setEnterFluidParticleEffect("particles/water_splash_enter");
		settings.setExitFluidParticleEffect("particles/water_splash_exit");
		//settings.setInFluidPoolSoundCue(inFluidCue);
		settings.setUnderFluidFallSoundCue("entity_waterfall_under");
	}
	
	function enableDefaultLavaInteraction()
	{
		local settings = addFluidSettings(FluidType_Lava);
		settings.setWaveGenerationEnabled(true);
		
		local particles = settings.getSurfaceParticleSettings();
		particles.setEnabled(true);
		particles.setTriggerFile("particles/lava_surface.trigger");
		settings.setSurfaceParticleSettings(particles);
		
		local splashParticles = settings.getUnderFallParticleSettings();
		splashParticles.setEnabled(true);
		splashParticles.setTriggerFile("particles/lava_underfall.trigger");
		settings.setUnderFallParticleSettings(splashParticles);
		
		local size = getCollisionRect().getWidth();
		
		local enterCue            = "entity_lava_enter_medium";
		local exitCue             = "entity_lava_exit_medium";
		//local inFluidCue          = "entity_water_idle_medium";
		
		if (size <= 1)
		{
			enterCue            = "entity_lava_enter_medium";
			exitCue             = "entity_lava_exit_small";
			//inFluidCue          = "entity_water_idle_small";
		}
		else if (size >= 3)
		{
			enterCue            = "entity_lava_enter_big";
			exitCue             = "entity_lava_exit_big";
			//inFluidCue          = "entity_water_idle_big";
		}
		
		settings.setEnterFluidSoundCue(enterCue);
		settings.setExitFluidSoundCue(exitCue);
		settings.setEnterFluidParticleEffect("particles/lava_splash_enter");
		settings.setExitFluidParticleEffect("particles/lava_splash_exit");
		//settings.setUnderFluidFallSoundCue("entity_lavafall_under");
	}
}
