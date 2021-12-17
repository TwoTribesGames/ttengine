class ForceField extends EntityBase
</
	editorImage    = "editor.forcefield"
	libraryImage   = "editor.library.forcefield"
	placeable      = Placeable_Developer
	collisionRect  = [ 0.0, 1.0, 2.0, 2.0 ]
	sizeShapeColor = Colors.ForceField
	displayName    = "Force Field"
	stickToEntity  = true
/>
{
	</
		type        = "string"
		choice      = ["circle", "rectangle"]
		order       = 0.0
		group       = "Appearance"
	/>
	type = "rectangle";
	
	</
		type        = "integer"
		min         = 1
		max         = 300
		order       = 0.1
		group       = "Appearance"
		conditional = "type == rectangle"
	/>
	width = 2;
	
	</
		type        = "integer"
		min         = 1
		max         = 250
		order       = 0.2
		group       = "Appearance"
		conditional = "type == rectangle"
	/>
	height = 2;
	
	</
		type        = "integer"
		min         = 1
		max         = 100
		order       = 0.3
		group       = "Appearance"
		conditional = "type == circle"
	/>
	radius = 4;
	
	</
		type        = "string"
		choice      = ["water", "wind", "nothing"]
		order       = 0.3
		group       = "Appearance"
	/>
	visual = "water";
	
	</
		type        = "integer"
		min         = -180
		max         = 180
		order       = 1.0
		group       = "Force Settings"
	/>
	rotation = 0;
	
	</
		type        = "integer"
		min         = 1
		max         = 1000
		order       = 1.1
		group       = "Force Settings"
	/>
	force = 50;
	
	</
		type        = "bool"
		order       = 2.0
		group       = "Player Settings"
	/>
	affectsPlayer = true;
	
	</
		type        = "bool"
		order       = 2.1
		group       = "Player Settings"
		conditional = "affectsPlayer == true"
	/>
	disableMoveControls = false;
	
	</
		type        = "bool"
		order       = 2.2
		group       = "Player Settings"
		conditional = "affectsPlayer == true"
	/>
	disableAimControls = false;
	
	</
		type        = "bool"
		order       = 2.3
		group       = "Player Settings"
		conditional = "affectsPlayer == true"
	/>
	resetBoostCount = false;
	
	</
		type        = "bool"
		order       = 3.0
		group       = "Enemy Settings"
	/>
	affectsEnemies = false;
	
	</
		type           = "bool"
		order          = 100
		group          = "Misc."
	/>
	enabled = true;
	
	// Internals
	_enabledSet       = false;
	_sensor           = null;
	_effect           = null;
	_sound            = null;
	_affectedEntities = {}; // global table
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		switch (type)
		{
		case "circle":    _sensor = addTouchSensor(::CircleShape(0, radius)); break;
		case "rectangle": _sensor = addTouchSensor(::BoxShape(width, height)); break;
		default:
			::tt_panic("Unhandled fieldType '" + fieldType + "'");
			::killEntity(this);
			return;
		}
		
		::removeEntityFromWorld(this);
		_sensor.setDefaultEnterAndExitCallback();
		_sensor.setFilterCallback("onTouchFilter");
		initStickToEntity();
		setEnabled(enabled);
	}
	
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onTouchEnter(p_entity, p_sensor)
	{
		// Store entity and its current external force
		local id = p_entity.getHandleValue();
		local forceVector = ::getVectorFromAngle(rotation) * force;
		if ((id in _affectedEntities) == false)
		{
			// Store original force
			_affectedEntities[id] <- {};
			_affectedEntities[id]["original"] <- p_entity.getExternalForce();
		}
		_affectedEntities[id][getHandleValue()] <- forceVector; // add force
		
		p_entity.setExternalForce(getCumulativeForce(id));
		
		if (p_entity instanceof ::PlayerBot)
		{
			// Sound cue name is corresponding with visual
			if (visual != "nothing")
			{
				_sound = playSoundEffect("forcefield_" + visual);
			}
			
			if (disableMoveControls)
			{
				p_entity.setMoveControlsEnabled(false);
			}
			
			if (disableAimControls)
			{
				p_entity.setAimControlsEnabled(false);
			}
			
			if (resetBoostCount)
			{
				p_entity.resetBoostCount();
			}
		}
	}
	
	function onTouchExit(p_entity, p_sensor)
	{
		local id = p_entity.getHandleValue();
		if (id in _affectedEntities)
		{
			// Remove field from entities stack
			delete _affectedEntities[id][getHandleValue()];
			
			p_entity.setExternalForce(getCumulativeForce(id));
			
			if (_affectedEntities[id].len() == 1)
			{
				// At last entry; remove the entity entirely
				delete _affectedEntities[id];
			}
		}
		
		if (p_entity instanceof ::PlayerBot)
		{
			if (_sound != null)
			{
				_sound.stop();
				_sound = null;
				
				// Play one shot
				playSoundEffect("forcefield_" + visual + "_end");
			}
			
			if (disableMoveControls)
			{
				p_entity.setMoveControlsEnabled(true);
			}
			
			if (disableAimControls)
			{
				p_entity.setAimControlsEnabled(true);
			}
		}
	}
	
	function onTouchFilter(p_entity)
	{
		if (p_entity.hasProperty("noticeForceFields") == false || p_entity.getMass() == null)
		{
			return false;
		}
		
		if (p_entity instanceof ::PlayerBot && affectsPlayer == false)
		{
			return false;
		}
		
		if (p_entity instanceof ::BaseEnemy && affectsEnemies == false)
		{
			return false;
		}
		
		return true;
	}
	
	function onProgressRestored(p_id)
	{
		setVisualEnabled(enabled);
	}
	
	function onDie()
	{
		// Simulate onTouchExits to ensure cleanup
		_sensor.removeAllSensedEntitiesWithCallbacks();
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Game CallbacksW
	
	function onGameInitialized()
	{
		_affectedEntities.clear();
	}
	
	function onGameUnserialized()
	{
	}
	
	function onGameReloaded()
	{
		_affectedEntities.clear();
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods
	
	function getCumulativeForce(p_id)
	{
		local result = ::Vector2(0, 0);
		foreach (elem in _affectedEntities[p_id])
		{
			result += elem;
		}
		return result;
	}
	
	function setEnabled(p_enabled)
	{
		if (_enabledSet && p_enabled == enabled)
		{
			return;
		}
		
		_enabledSet = true;
		enabled = p_enabled;
		setVisualEnabled(enabled);
		_sensor.setEnabled(enabled);
		
		if (enabled == false)
		{
			_sensor.removeAllSensedEntities();
		}
	}
	
	function setVisualEnabled(p_enabled)
	{
		if (p_enabled && visual != null && visual != "nothing")
		{
			local direction = ::getVectorFromAngle(rotation);
			local offset    = -direction * 2.0;
			local speed     = direction* 7.0;
			
			_effect = spawnParticleContinuous("particles/forcefield_" + visual, offset, true, 0, false, ParticleLayer_UseLayerFromParticleEffect, 1.0);
			
			switch (type)
			{
			case "rectangle": 
				_effect.setEmitterProperties(0, { area_type = "rectangle", rect_width = width, rect_height = height + 2.0, velocity_x = speed.x * 1.75, velocity_y = speed.y * 3.0, particles = (width * height) });
				_effect.setEmitterProperties(1, { area_type = "rectangle", rect_width = width, rect_height = height + 2.0, velocity_x = speed.x * 2.5, velocity_y = speed.y * 5.0, particles = (width * height) });
				_effect.setEmitterProperties(2, { area_type = "rectangle", rect_width = width, rect_height = height + 2.0, velocity_x = speed.x * 1.75, velocity_y = speed.y * 3.0, particles = (width * height) });
				break;
			
			case "circle":
				_effect.setEmitterProperties(0, { area_type = "circle", radius = radius, velocity_x = speed.x, velocity_y = speed.y, particles = radius * radius * ::PI });
				_effect.setEmitterProperties(1, { area_type = "circle", radius = radius, velocity_x = speed.x, velocity_y = speed.y, particles = radius * radius * ::PI });
				_effect.setEmitterProperties(2, { area_type = "circle", radius = radius, velocity_x = speed.x, velocity_y = speed.y, particles = radius * radius * ::PI });
				break;
			
			default:
				::tt_panic("Unhandled trigger type '" + type + "'");
				break;
			}
		}
		else
		{
			if (_effect != null)
			{
				// Soft stop
				_effect.stop(true);
			}
			_effect = null;
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Update
	
	function update(p_deltaTime)
	{
		updateStickToEntity();
	}
}
::registerClassForGameCallbacks(ForceField);
