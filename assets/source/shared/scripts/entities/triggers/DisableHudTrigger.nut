include_entity("triggers/Trigger");

class DisableHudTrigger extends Trigger
</ 
	editorImage    = "editor.disablehudtrigger"
	libraryImage   = "editor.library.disablehudtrigger"
	placeable      = Placeable_Developer
	sizeShapeColor = Colors.Trigger
	movementset    = "StaticIdle"
	displayName    = "Trigger - Disable HUD"
	group          = "04.1 Action Triggers"
	stickToEntity  = true
/>
{
	function onTriggerEnterFirst(p_entity)
	{
		::Hud.hide();
	}
	
	function onTriggerExitLast(p_entity)
	{
		::Hud.show();
	}
}
