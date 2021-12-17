class PatternMovementEntity extends EntityBase
</
	editorImage         = "editor.patternmovement_entity"
	libraryImage        = "editor.library.patternmovement_entity"
	placeable           = Placeable_Everyone
	movementset         = "Static"
	collisionRect       = [ 0.0, 0.75, 1.5, 1.5 ]
/>
{
	</
		type        = "bool"
		order       = 0
		group       = "Cloning"
		description = ""
	/>
	useCloneForStickToEntities = true;
	
	</
		type   = "integer"
		order  = 1
		min    = -180
		max    = 180
		group  = "Direction"
	/>
	rotation = 0;
	
	</
		type   = "float"
		order  = 2
		min    = 0.0
		max    = 50.0
		group  = "Direction"
	/>
	speed = 0;
	
	</
		type   = "float"
		order  = 3
		min    = 0.0
		max    = 150.0
		group  = "X Wave"
	/>
	sizeX = 7;
	
	</
		type   = "float"
		order  = 4
		min    = 0.25
		max    = 100.0
		group  = "X Wave"
	/>
	durationX = 1.5;
	
	</
		type   = "float"
		order  = 5
		min    = 0.0
		max    = 1.0
		group  = "X Wave"
	/>
	offsetX = 0.0;
	
	</
		type   = "float"
		order  = 6
		min    = 0.0
		max    = 150.0
		group  = "Y Wave"
	/>
	sizeY = 7;
	
	</
		type   = "float"
		order  = 7
		min    = 0.25
		max    = 100.0
		group  = "Y Wave"
	/>
	durationY = 1.5;
	
	</
		type   = "float"
		order  = 8
		min    = 0.0
		max    = 1.0
		group  = "Y Wave"
	/>
	offsetY = 0.0;
	
	</
		type  = "bool"
		order = 9
		group = "Debug"
	/>
	visualize = false;
	
	</
		type   = "bool"
		order  = 10
		group  = "Enable Controls"
	/>
	enabled     = true;
	_enabled    = true;
	_enabledSet = false;
	
	</
		type           = "entity"
		filter         = ::getTriggerTypes()
		group          = "Enable Controls"
		order          = 11
		referenceColor = ReferenceColors.Enable
		description    = "When set, this entity will be enabled when an incoming signal fires."
	/>
	enableSignal = null;
	
	</
		type        = "float"
		order       = 12
		min         = 0.0
		max         = 100.0
		group       = "Enable Controls"
		description = "When > 0, the entity will disable itself after the timeout."
	/>
	disableTimeout = 0.0;
	_timeAlive     = 0.0;
	
	// X and Y axis internal values
	_tx                = 0.0;
	_ty                = 0.0;
	_vx                = 0.0;
	_vy                = 0.0;
	
	// Constant speed
	_direction         = null;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		::removeEntityFromWorld(this);
		
		createMovementController(false);
		local settings = getPhysicsSettings();
		settings.setCollisionWithSolid(false);
		setPhysicsSettings(settings);
		
		_tx        = offsetX * durationX;
		_ty        = offsetY * durationY;
		_vx        = (1.0 / durationX) * 2 * ::PI;
		_vy        = (1.0 / durationY) * 2 * ::PI;
		_direction = ::getVectorFromAngle(rotation) * speed;
		
		setEnabled(enabled);
		
		if (enableSignal != null)
		{
			enableSignal.addChildTrigger(this);
		}
	}
	
	_visualization = null;
	function onSpawn()
	{
		if (::isTestBuild() && visualize)
		{
			_visualization = spawnParticleContinuous("particles/patternmovement", ::Vector2(0, 0), true, 0, false,
			                                         ParticleLayer_UseLayerFromParticleEffect, 1.0);
		}
		
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onStickToEntityReceived(p_entity)
	{
		if (useCloneForStickToEntities)
		{
			local entity = ::respawnEntityAtPosition(getID(), p_entity.getCenterPosition());
			p_entity.stickToEntity = entity;
			p_entity.subscribeDeathEvent(this, "onCloneParentDied");
		}
	}
	
	function onCloneParentDied(p_entity)
	{
		::killEntity(p_entity.stickToEntity);
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
		if (p_enabled == _enabled && _enabledSet)
		{
			return;
		}
		
		if (p_enabled)
		{
			_timeAlive = 0.0;
		}
		
		_enabledSet = true;
		_enabled = p_enabled;
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Update
	
	function update(p_deltaTime)
	{
		if (_enabled == false)
		{
			setSpeed(::Vector2(0, 0));
			return;
		}
		
		if (disableTimeout > 0.0 && _timeAlive >= disableTimeout)
		{
			setEnabled(false);
		}
		
		local v = ::Vector2(::cos(_tx * _vx) * sizeX * _vx * 0.5, ::cos(_ty * _vy) * sizeY * _vy * 0.5);
		setSpeed(_direction + v);
		
		_timeAlive += p_deltaTime;
		_tx        += p_deltaTime;
		_ty        += p_deltaTime;
	}
}
Trigger.makeTriggerTarget(PatternMovementEntity);
