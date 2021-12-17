class Waypoint extends EntityBase
</
	editorImage    = "editor.waypoint"
	libraryImage   = "editor.library.waypoint"
	placeable      = Placeable_Everyone
	collisionRect  = [ 0.0, 0.5, 1.0, 1.0 ]  // center X, center Y, width, height
//	group          = "Rewind"
	stickToEntity  = true
/>

{
	</
		type           = "entity"
		filter         = ["Waypoint"]
		order          = 0
		autoGetSet     = true
		referenceColor = ReferenceColors.Waypoint
	/>
	nextWaypoint = null;
	
	</
		type = "bool"
		order = 2
		description = "When autoAdvance is on the entity will automatically move to the next waypoint on arriving. Can be changed with enable triggers."
	/>
	autoAdvance = true;
	
	</
		type = "float"
		min = 0
		max = 100
		order = 3
	/>
	advanceDelay = 0.0;
	
	</
		type = "float"
		min = -10
		max = 10
		order = 4
		description = "Generic custom scale. Usage depends on caller. E.g., the CameraOnRails uses this scale for scaling its speed."
	/>
	customScale = 1.0;
	
	</
		type = "bool"
		order = 5
		description = "Generic custom action. Usage depends on caller. E.g., the DropShip uses this to indicate it should drop its payload or not."
	/>
	customAction = false;
	
	</
		type = "bool"
		order = 6
		description = "Kills entity quietly upon reaching this waypoint. NOTE: Only works for scour enemies!!!"
	/>
	killWhenReached = false;
	
	_enableEvent = null;
	_deathEvent  = null;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		if (advanceDelay == 0) advanceDelay = 0.01; // because timers don't like timouts of 0, but not too small either then it just never completes
		
		::removeEntityFromWorld(this);
		_enableEvent = ::EventPublisher("enableevent");
		_deathEvent = ::EventPublisher("deathevent");
	}
	
	function onSpawn()
	{
		initStickToEntity();
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onDie()
	{
		_deathEvent.publish(this);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods
	
	function setEnabled(p_enabled)
	{
		autoAdvance = p_enabled;
		_enableEvent.publish(this, p_enabled);
	}
	
	function isEnabled()
	{
		return autoAdvance;
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Update
	
	function update(p_deltaTime)
	{
		updateStickToEntity();
	}
}
