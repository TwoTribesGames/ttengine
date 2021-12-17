include_entity("triggers/Trigger");

::g_zeroGravityTriggerInstance <- null;

class ZeroGravityTrigger extends Trigger
</
	editorImage    = "editor.zerogravitytrigger"
	libraryImage   = "editor.library.zerogravitytrigger"
	placeable      = Placeable_Developer
	collisionRect  = [ 0.0, 1.0, 2.0, 2.0 ]
	sizeShapeColor = Colors.Trigger
	displayName    = "Trigger - Zero Gravity"
	group          = "04.1 Action Triggers"
	stickToEntity  = true
/>
{
	</
		type  = "float"
		min   = 0.0
		max   = 10.0
		order = 3
		group = "Specific Settings"
	/>
	tweenInDuration = 0.0;
	
	</
		type  = "float"
		min   = 0.0
		max   = 10.0
		order = 4
		group = "Specific Settings"
	/>
	tweenOutDuration = 0.4;
	
	function onTriggerEnter(p_entity)
	{
		base.onTriggerEnter(p_entity);
		
		if (::g_zeroGravityTriggerInstance == null)
		{
			::g_zeroGravityTriggerInstance = this.weakref();
			
			// Tell all entities that react on zero gravity, that zero gravity is now switched on
			foreach (entity in ::getEntitiesByTag("noticeZeroGravity"))
			{
				handleZeroGravityEnter(entity, this);
			}
			
			return;
		}
		
		::tt_panic("More than one ZeroGravity triggers are active at the same time; this is not allowed");
	}
	
	function onTriggerExit(p_entity)
	{
		base.onTriggerExit(p_entity);
		
		::g_zeroGravityTriggerInstance = null;
		
		// Tell all entities that react on zero gravity, that zero gravity is now switched off
		foreach (entity in ::getEntitiesByTag("noticeZeroGravity"))
		{
			handleZeroGravityExit(entity, this);
		}
	}
	
	function hasActiveTrigger()
	{
		return ::g_zeroGravityTriggerInstance != null;
	}
	
	function getActiveTrigger()
	{
		return ::g_zeroGravityTriggerInstance;
	}
}
