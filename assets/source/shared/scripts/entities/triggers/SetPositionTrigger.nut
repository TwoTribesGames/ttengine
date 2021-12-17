include_entity("triggers/Trigger");

class SetPositionTrigger extends Trigger
</
	editorImage    = "editor.setpositiontrigger"
	libraryImage   = "editor.library.setpositiontrigger"
	placeable      = Placeable_Developer
	collisionRect  = [ 0.0, 1.0, 2.0, 2.0 ]
	sizeShapeColor = Colors.Trigger
	displayName    = "Trigger - Set Position"
	group          = "04.1 Action Triggers"
	stickToEntity  = true
/>
{
	</
		type   = "entity"
		filter = ["Waypoint"]
		order  = 0
		group  = "Specific Settings"
	/>
	wayPoint = null;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		base.onInit();
		
		if (wayPoint == null)
		{
			editorWarning("No waypoint selected");
			return;
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onTriggerEnterFirst(p_entity)
	{
		base.onTriggerEnterFirst(p_entity);
		
		if (wayPoint != null)
		{
			p_entity.setPosition(wayPoint.getCenterPosition());
		}
	}
}
