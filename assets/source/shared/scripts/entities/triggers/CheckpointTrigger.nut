include_entity("triggers/Trigger");

class CheckpointTrigger extends Trigger
</
	editorImage    = "editor.checkpoint"
	libraryImage   = "editor.library.checkpoint"
	placeable      = Placeable_Everyone
	sizeShapeColor = Colors.Checkpoint
	displayName    = "Trigger - Checkpoint"
	collisionRect  = [ 0.0, 0.9, 1.8, 1.8 ]  // center X, center Y, width, height
	stickToEntity  = true
/>
{
	</
		type        = "float"
		min         = 0.0
		max         = 5.0
		order       = 0
		group       = "Specific Settings"
		description = "Wait amount of seconds before saving (serializing) the checkpoint. If player dies during this time, checkpoint will NOT be stored."
	/>
	saveDelay = 0.5;
	
	</
		type        = "float"
		min         = 1.0
		max         = 5.0
		order       = 0
		group       = "Specific Settings"
		description = "Wait amount of seconds after the saveDelay before actually committing the checkpoint. If player dies during this time, checkpoint will NOT be stored."
	/>
	commitDelay = 3.0;
	
	</
		type        = "float"
		choice      = [0.25, 1.0]
		order       = 1
		group       = "Specific Settings"
		description = "Sets the weapon drop factor for the section after this checkpoint."
	/>
	weaponDropFactor = 1.0;
	
	</
		type        = "float"
		choice      = [0.0, 1.0]
		order       = 1
		group       = "Specific Settings"
		description = "Sets the health drop factor for the section after this checkpoint."
	/>
	healthDropFactor = 1.0;
	
	width = 4;
	height = 4;
	
	// Internals
	_id         = null;
	_queueDelay = 0.0;
	_activator  = null;
	_previousDeaths = 0;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		base.onInit();
		
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onTimer(p_name)
	{
		// WARNING, if we'd ever want to use a timer for checkpoint for anything other than two stage 
		// checkpoint activation, we'd need to check if p_name is a checkpoint ID
		commit(p_name);
	}
	
	function onProgressRestored(p_id)
	{
		// All checkpoints that are in the middle of storing their checkpoint data when progress is
		// restored are invalid
		if (_id != null)
		{
			stopAllTimers();
			_id = null;
		}
	}
	
	function onActivatorDied(p_activator)
	{
		// a bit blunt, but if the player died we can sort-of assume no new checkpoints are needed
		// so we don't need to figure out which exact checkpoint was still in the queue
		stopAllTimers();
	}
	
	function onTriggerEnterFirst(p_entity)
	{
		// store progress and set to activated state
		setEnabled(false);
		
		::BaseEnemy.setWeaponDropFactor(weaponDropFactor);
		::BaseEnemy.setHealthDropFactor(healthDropFactor);
		
		startCallbackTimer("storeTwoStageCheckpoint", saveDelay);
	}
	
	function onDie()
	{
		base.onDie();
		
		if (_activator != null)
		{
			_activator.unsubscribeDeathEvent(this, "onActivatorDied");
		}
	}
	
	function onReloadGame()
	{
		// When game is reloaded (debug feature) the playerbot will automatically save a checkpoint.
		// However an entity might also instantly trigger a checkpoint, hence add a queue delay here
		_queueDelay = 0.1;
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods
	
	function storeTwoStageCheckpoint()
	{
		::Game.setFarmingEnabled(false); // Reset the farming
		// Don't use p_entity here; make sure it is always the PlayerBot who activates this trigger
		local playerBot = ::getFirstEntityByTag("PlayerBot");
		if (playerBot == null)
		{
			// Never store when player is dead
			return;
		}
		
		if ((commitDelay + saveDelay) > 0.0)
		{
			if (_queueDelay > 0.0)
			{
				startCallbackTimer("queue", _queueDelay);
				_queueDelay = 0.0;
			}
			else
			{
				queue();
			}
			
			_activator = playerBot;
			playerBot.subscribeDeathEvent(this, "onActivatorDied");
		}
		else
		{
			_previousDeaths = ::ProgressMgr.getLastCheckPointDeathCount();
			::ProgressMgr.setLastCheckPoint(getID());
			::ProgressMgr.storeCheckPoint();
			setEnabled(false);
			::Stats.submitTelemetryEvent("checkpoint_reached", _previousDeaths);
		}
	}
	
	function queue()
	{
		_id = ::ProgressMgr.queueCheckPoint();
		startTimer(_id, commitDelay + saveDelay);
	}
	
	function commit(p_id)
	{
		_previousDeaths = ::ProgressMgr.getLastCheckPointDeathCount();
		::ProgressMgr.setLastCheckPoint(getID());
		::ProgressMgr.commitCheckPoint(p_id);
		setEnabled(false);
		::Stats.submitTelemetryEvent("checkpoint_reached", _previousDeaths);
	}
	
	function setEnabled(p_enabled)
	{
		base.setEnabled(p_enabled);
		
		if (enabled == false)
		{
			stopAllTimers();
		}
	}
}
CheckpointTrigger.setattributes("once", null);
