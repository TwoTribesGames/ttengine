include_entity("triggers/Trigger");

class PlayerMoveTrigger extends Trigger
</
	editorImage    = "editor.playermovetrigger"
	libraryImage   = "editor.library.playermovetrigger"
	placeable      = Placeable_Developer
	sizeShapeColor = Colors.Trigger
	displayName    = "Trigger - Player Move"
	group          = "04.4 Player Triggers"
	collisionRect  = [ 0.0, 1.0, 2.0, 2.0 ]
	stickToEntity  = true
/>
{
	</
		type        = "integer"
		min         = -180
		max         = 180
		group       = "Specific Settings"
		order       = 0.0
	/>
	rotation = 90;
	
	</
		type        = "float"
		min         = 0.1
		max         = 1.0
		group       = "Specific Settings"
		order       = 0.1
	/>
	force = 1.0;
	
	</
		type        = "bool"
		group       = "Specific Settings"
		order       = 0.2
	/>
	disableAimControls = false;
	
	function onTriggerEnter(p_entity)
	{
		local direction = ::getVectorFromAngle(rotation) * force;
		if (::fabs(direction.x) < 0.001) direction.x = 0;
		if (::fabs(direction.y) < 0.001) direction.y = 0;
		p_entity.setDirectionInputOverride(direction);
		
		if (disableAimControls)
		{
			p_entity.setAimControlsEnabled(false);
		}
	}
	
	function onTriggerExit(p_entity)
	{
		if (disableAimControls)
		{
			p_entity.setAimControlsEnabled(true);
		}
	}
	
	function onTriggerExitLast(p_entity)
	{
		p_entity.resetDirectionInputOverride();
	}
}
PlayerMoveTrigger.setattributes("triggerOnTouch", null);
PlayerMoveTrigger.setattributes("triggerOnUncull", null);
PlayerMoveTrigger.setattributes("parentTrigger", null);
PlayerMoveTrigger.setattributes("triggerFilter", null);
PlayerMoveTrigger.setattributes("filterAllEntitiesOfSameType", null);
