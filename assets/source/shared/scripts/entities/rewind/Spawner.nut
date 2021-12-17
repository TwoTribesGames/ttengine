enum SpawnMode
{
	WaveOnTimer,
	WaveWhenAllDied,
	SingleWhenSingleDied,
	ReviveWhenSingleDied,
	SingleWave
};

enum SpawnOrder
{
	Sequential,
	Random
};

class Spawner extends EntityBase
</
	editorImage   = "editor.spawner"
	libraryImage  = "editor.library.spawner"
	placeable     = Placeable_Everyone
	collisionRect = [ 0.0, 0.0, 0.1, 0.1 ]  // center X, center Y, width, height
	order         = 3 // After Dropships (as they also spawn entities and can be spawned by the Spawner)
/>
{
	</
		type   = "string"
		choice = ["Wave On Timer", "Single Wave", "Wave When All Entities Died", "Single When A Single Entity Died", "Revive Entity When It Died"]
		order  = -1
	/>
	spawnMode = "Wave On Timer";
	
	</
		type           = "delayed_entityid_array"
		filter         = ["RewindEntity", "Trigger", "Spawner", "PhysicsParticlePlayer", "Area"]
		description    = "The entities to spawn."
		referenceColor = ReferenceColors.List
		order          = 0
	/>
	entities = null;
	
	</
		type   = "string"
		choice = ["sequential", "random"]
		description = "The order in which entities are spawned."
		order       = 1
	/>
	spawnOrder = "sequential";
	
	</
		type           = "entity"
		filter         = ["Area"]
		description    = "The area to spawn the entities in. If none is selected, the original entity positions are used. If a selected area gets killed, the spawner also gets killed."
		referenceColor = ReferenceColors.Area
		order          = 2
	/>
	spawnArea = null;
	
	</
		type        = "float"
		min         = 0.1
		max         = 300.0
		description = "The delay between each spawn."
		order       = 3
	/>
	spawnDelay = 2.0;
	
	</
		type        = "float"
		min         = 0.0
		max         = 10.0
		description = "A random delay which will be added to each spawnDelay."
		order       = 4
	/>
	randomSpawnDelay = 0.0;
	
	</
		type        = "float"
		min         = 0.5
		max         = 1.5
		description = "After each spawn this factor is multiplied with the delay, causing the delay to shorten (< 1.0) or increase (> 1.0)."
		order       = 5
	/>
	spawnDelayDampening = 1.0;
	
	</
		type        = "float"
		min         = 0.1
		max         = 30.0
		description = "Minium spawn delay"
		order       = 6
	/>
	minimumSpawnDelay = 0.1;
	
	</
		type        = "float"
		min         = 0.0
		max         = 100.0
		description = "Wait for X amount of seconds before the first spawn is executed."
		order       = 7
	/>
	initialSpawnDelay = 0.0;
	
	</
		type        = "integer"
		min         = 0
		max         = 20
		description = "The amount of entities to spawn per wave ('when died' modes have 1 wave). Use 0 to spawn the amount of target entities."
		order       = 8
	/>
	entitiesPerWave = 0;
	
	</
		type        = "integer"
		min         = 0
		max         = 1000
		description = "The total maximum amount of entities to spawn. Use 0 for an infinite amount."
		order       = 9
	/>
	maxTotalSpawnedEntities = 0;
	
	</
		type        = "integer"
		min         = 1
		max         = 50
		description = "The maximum entities that allowed to be alive at any point. Won't continue spawning if this limit is reached."
		order       = 9.1
	/>
	maxEntitiesAlive = 50;
	
	</
		type        = "integer"
		min         = 10
		max         = 500
		description = "When this counter is reached, farming is detected."
		order       = 9.2
	/>
	farmingCount = 200;
	
	</
		type         = "integer"
		min          = 0
		max          = 1000
		description  = "If this spawner is the parent of a trigger, it will fire that trigger after reaching this deathcount. Use 0 to disable this functionality."
		order        = 10
	/>
	triggerAfterDeathCount = 0;
	
	</
		type        = "bool"
		description = "When set to true, a hacked entity counts as a dead entity."
		order        = 11
	/>
	hackedCountsAsDeath = true;
	
	</
		type        = "bool"
		description = "When set to true, the oldest spawned entity will be non-silently killed when reaching maxEntitiesAlive."
		order        = 12
	/>
	autoKillAtMaxEntitiesAlive = false;
	
	</
		type        = "bool"
		description = "When set to true, the deathcount is reset after triggering."
		order        = 13
	/>
	resetDeathCountAfterTrigger = false;
	
	</
		type        = "bool"
		description = "When set to true, a spawn effect presentation is spawned for each entity. NOTE: Only works on entities derived from RewindEntity."
		order        = 14
	/>
	useSpawnEffect = false;
	
	</
		type  = "bool"
		order = 100
		group = "Misc."
	/>
	enabled = false;
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
	
	_spawnIndex       = 0;
	_spawnTotalCount  = 0;
	_farmingCounter   = 0;
	_hasSpawnArea     = false;
	_deathCount       = 0;
	_curEntitiesAlive = 0;
	_deathEvent       = null;
	
	// Used to track which entities are currently active (not dead).
	_activeEntities      = null;
	_activeEntitiesOrder = null;
	
	// This variable is used for the spawnSingle 'event' to make sure there are no identical timers
	_timerCount       = 0;
	
	_spawnCompletedEvent = null;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		if (entities == null)
		{
			editorWarning("Select entities");
			::killEntity(this);
			return;
		}
		
		_activeEntities = {};
		
		if (autoKillAtMaxEntitiesAlive)
		{
			_activeEntitiesOrder = [];
		}
		::removeEntityFromWorld(this);
		
		// Translate to internal enum values
		switch (spawnMode)
		{
		case "Wave On Timer":                    spawnMode = SpawnMode.WaveOnTimer;          break;
		case "Single Wave":                      spawnMode = SpawnMode.SingleWave;           break;
		case "Wave When All Entities Died":      spawnMode = SpawnMode.WaveWhenAllDied;      break;
		case "Single When A Single Entity Died": spawnMode = SpawnMode.SingleWhenSingleDied; break;
		case "Revive Entity When It Died":       spawnMode = SpawnMode.ReviveWhenSingleDied; break;
		default:
			::tt_panic("invalid spawnMode '" + spawnMode + "'");
			spawnMode = SpawnMode.WaveOnTimer;
		}
		
		// Translate to internal enum values
		switch (spawnOrder)
		{
		case "sequential": spawnOrder = SpawnOrder.Sequential; break;
		case "random":     spawnOrder = SpawnOrder.Random;     break;
		default:
			::tt_panic("invalid spawnOrder '" + spawnOrder + "'");
			spawnOrder = SpawnOrder.Sequential;;
		}
		
		_spawnCompletedEvent = ::EventPublisher("spawnCompleted");
		
		if (spawnDelay < minimumSpawnDelay)
		{
			editorWarning("spawnDelay < minimumSpawnDelay");
		}
		
		if (enableSignal != null)
		{
			enableSignal.addChildTrigger(this);
		}
		
		if (initialSpawnDelay == 0.0)
		{
			// To enforce spawning is not happing immediately during creation
			initialSpawnDelay = 0.01;
		}
	}
	
	function onSpawn()
	{
		if (entitiesPerWave == 0 && entities != null)
		{
			entitiesPerWave = entities.len();
		}
		
		reset();
		setEnabled(enabled);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onDie()
	{
		enabled = false; // do this otherwise onTargetDied will kick in, altering the _activeEntities
		if (_activeEntities != null)
		{
			foreach (key, value  in _activeEntities)
			{
				local entity = ::getEntityByHandleValue(key);
				if (entity != null)
				{
					if (entity instanceof ::RewindEntity && entity.isPositionCulled())
					{
						entity.addProperty("dieQuietly");
					}
					::killEntity(entity);
				}
			}
		}
		
		if (_deathEvent != null)
		{
			_deathEvent.publish(this);
		}
	}
	
	function onTargetDied(p_target)
	{
		if (enabled == false)
		{
			return;
		}
		
		// Make sure we receive this only once
		p_target.unsubscribeDeathEvent(this, "onTargetDied");
		if (hackedCountsAsDeath && p_target instanceof ::BaseEnemy && p_target._virusUploadedEvent != null)
		{
			p_target._virusUploadedEvent.unsubscribe(this, "onTargetDied");
		}
		
		++_deathCount;
		_curEntitiesAlive--;
		if (_deathCount >= triggerAfterDeathCount && triggerAfterDeathCount > 0)
		{
			if (resetDeathCountAfterTrigger)
			{
				// Do both enter and exit; otherwise multiple enters might be problematic
				triggerChildTriggers(this);
				triggerExitChildTriggers(this);
				_deathCount = 0;
			}
			else
			{
				triggerChildTriggers(this);
				triggerAfterDeathCount = 0;
			}
		}
		
		// Look for this entity in the _activeEntities and set its index to -1 (inactive/dead)
		local handle = p_target.getHandleValue();
		local idx = _activeEntities[handle];
		delete _activeEntities[handle];
		
		if (autoKillAtMaxEntitiesAlive)
		{
			for (local i = 0; i < _activeEntitiesOrder.len(); ++i)
			{
				if (_activeEntitiesOrder[i] == handle)
				{
					_activeEntitiesOrder.remove(i);
					break;
				}
			}
		}
		
		if (spawnMode == SpawnMode.WaveOnTimer || spawnMode == SpawnMode.SingleWave)
		{
			return;
		}
		
		_timerCount++;
		local nextSpawnDelay = spawnDelay + (frnd() * randomSpawnDelay);
		switch (spawnMode)
		{
		case SpawnMode.SingleWhenSingleDied:
			spawnDelay > 0.0 ? startTimer(_timerCount + "_spawnSingle", nextSpawnDelay) : spawnSingle();
			break;
			
		case SpawnMode.WaveWhenAllDied:
			if (_activeEntities.len() == 0)
			{
				spawnDelay > 0.0 ? startTimer("spawnWave", nextSpawnDelay) : spawnWave();
			}
			break;
			
		case SpawnMode.ReviveWhenSingleDied:
			if (spawnDelay > 0.0)
			{
				startTimer(_timerCount + "_reviveSingle_" + idx, nextSpawnDelay)
			}
			else
			{
				_spawnIndex = idx;
				spawnSingle();
			}
			break;
			
		default:
			::tt_panic("Unhandled spawnMode '" + spawnMode + "'");
			break;
		}
	}
	
	function onTimer(p_name)
	{
		if (enabled == false)
		{
			return;
		}
		
		if (p_name.find("spawnSingle") != null)
		{
			spawnSingle();
		}
		else if (p_name.find("reviveSingle_") != null)
		{
			// Get the spawnindex from the timername info
			local idx = p_name.find("reviveSingle_") + "reviveSingle_".len();
			_spawnIndex = p_name.slice(idx).tointeger();
			
			if (_spawnIndex >= 0 && _spawnIndex < entities.len())
			{
				spawnSingle();
			}
			else
			{
				::tt_panic("Invalid spawnindex '" + _spawnIndex + "' for reviving entity. Value should be in range [0.." + entities.len() + "]");
			}
		}
		else if (p_name == "spawnWave")
		{
			spawnWave();
			
			if (spawnMode == SpawnMode.WaveOnTimer && canSpawn())
			{
				local nextSpawnDelay = spawnDelay + (frnd() * randomSpawnDelay);
				startTimer("spawnWave", nextSpawnDelay);
			}
		}
		
		// Watch out here: currently all timer calls will update the spawnDelay
		spawnDelay *= spawnDelayDampening;
		if (spawnDelay < minimumSpawnDelay)
		{
			spawnDelay = minimumSpawnDelay;
		}
	}
	
	function onInvalidProperties(p_properties)
	{
		removeNonexistentProperties(this, p_properties);
		
		return p_properties;
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods
	
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
	
	function reset()
	{
		_hasSpawnArea = isValidEntity(spawnArea);
		_enabledSet = false;
		_spawnTotalCount = 0;
		_farmingCounter = 0;
		
		stopAllTimers();
		
		// Set initial index
		switch (spawnOrder)
		{
		case SpawnOrder.Sequential:
			_spawnIndex = 0;
			break;
			
		case SpawnOrder.Random:
			_spawnIndex = ::rnd_minmax(0, entities.len() - 1);
			break;
			
		default:
			::tt_panic("Unhandled spawnOrder '" + spawnOrder + "'");
			break;
		}
	}
	
	function canSpawn()
	{
		return _spawnTotalCount < maxTotalSpawnedEntities || maxTotalSpawnedEntities == 0;
	}
	
	function spawnSingle()
	{
		if (enabled == false || canSpawn() == false)
		{
			return;
		}
		
		local spawnPosition = null;
		
		if (_hasSpawnArea)
		{
			if(isValidEntity(spawnArea) == false)
			{
				// spawnArea has been killed, stop spawning
				::killEntity(this);
				return;
			}
			
			spawnPosition = spawnArea.getPointInArea();
		}
		
		if (_curEntitiesAlive >= maxEntitiesAlive)
		{
			if (autoKillAtMaxEntitiesAlive)
			{
				// Kill first one
				::killEntity(::getEntityByHandleValue(_activeEntitiesOrder[0]));
				if (_curEntitiesAlive >= maxEntitiesAlive)
				{
					::tt_panic("Kill first entity in _activeEntitiesOrder, yet still at maxEntitiesAlive");
				}
			}
		}
		
		if (_curEntitiesAlive < maxEntitiesAlive)
		{
			local entity = spawnPosition == null ? 
				::respawnEntity(entities[_spawnIndex]) :
				::respawnEntityAtPosition(entities[_spawnIndex], spawnPosition);
			
			if (entity != null)
			{
				// Only apply spawneffect on RewindEntities
				if (useSpawnEffect && entity instanceof ::RewindEntity)
				{
					local pres = entity.createPresentationObjectInLayer("presentation/entity_spawneffect", ParticleLayer_InFrontOfWater);
					pres.addTag(entity.getType().tolower());
					pres.start("", [], false, 0);
				}
				
				_activeEntities[entity.getHandleValue()] <- _spawnIndex;
				if (autoKillAtMaxEntitiesAlive)
				{
					_activeEntitiesOrder.push(entity.getHandleValue());
				}
				
				if ("subscribeDeathEvent" in entity)
				{
					entity.subscribeDeathEvent(this, "onTargetDied");
					
					if (hackedCountsAsDeath && entity instanceof ::BaseEnemy && entity._virusUploadedEvent != null)
					{
						entity._virusUploadedEvent.subscribe(this, "onTargetDied");
					}
				}
				_curEntitiesAlive++;
				_spawnTotalCount++;
				
				if (entity instanceof ::BaseEnemy)
				{
					++_farmingCounter;
					
					if (_farmingCounter >= farmingCount)
					{
						::Game.setFarmingEnabled(true);
					}
				}
				
				if (maxTotalSpawnedEntities != 0 && _spawnTotalCount == maxTotalSpawnedEntities)
				{
					_spawnCompletedEvent.publish();
				}
			}
		}
		
		// Set index for next spawn
		switch (spawnOrder)
		{
			case SpawnOrder.Sequential:
				_spawnIndex++;
				if (_spawnIndex >= entities.len())
				{
					_spawnIndex = 0;
				}
				break
				
			case SpawnOrder.Random:
				_spawnIndex = ::rnd_minmax(0, entities.len() - 1);
				break;
				
			default:
				::tt_panic("Unhandled spawnOrder '" + spawnOrder + "'");
				break;
		}
	}
	
	function spawnWave()
	{
		if (enabled == false)
		{
			return;
		}
		
		for (local i = 0; i < entitiesPerWave; i++)
		{
			spawnSingle();
		}
	}
	
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
			if (canSpawn())
			{
				startTimer("spawnWave", initialSpawnDelay);
			}
		}
		else
		{
			stopAllTimers();
		}
	}
	
	function _triggerEnter(p_entity, p_parent)
	{
		// Enabled trigger fired; enable this entity
		setEnabled(true);
	}
}
Trigger.makeTriggerParentable(Spawner);
Trigger.makeTriggerTarget(Spawner);
