include_entity("triggers/Trigger");
include_entity("rewind/RewindEntity");

class CreditsEntity extends RewindEntity
</
	editorImage    = "editor.hamster"
	placeable      = Placeable_Hidden
	movementset    = "Static"
	collisionRect  = [ 0.0, 0.0, 1.5, 1.5 ]
/>
{
	positionCulling = false;
	
	// Constants
	static c_timeout     = 108.0;
	static c_skipTimeout = 1.0;
	
	_canBeSkipped = false;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		base.onInit();
		
		::ProgressMgr.setPlayMode(PlayMode.Invalid);
	}
	
	function onSpawn()
	{
		base.onSpawn();
		
		::Camera.setPrimaryFollowEntity(this);
		::Camera.setScrollingEnabled(false);
		::addButtonInputListeningEntity(this, InputPriority.Normal);
		
		// trigger possible child triggers first, so the level exits can override the "defaults"
		triggerChildTriggers(this);
		
		startTimer("skipAllowed", c_skipTimeout);
		startCallbackTimer("exit", c_timeout);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onTimer(p_name)
	{
		if (p_name == "skipAllowed")
		{
			_canBeSkipped = true;
		}
	}
	
	function onButtonRespawnPressed()
	{
		if (_canBeSkipped)
		{
			exit();
		}
	}
	
	function exit()
	{
		::Level.fadeOutAndLoad("menu");
		::removeButtonInputListeningEntity(this);
	}
}
CreditsEntity.setattributes("positionCulling", null);
Trigger.makeTriggerParentable(CreditsEntity);
