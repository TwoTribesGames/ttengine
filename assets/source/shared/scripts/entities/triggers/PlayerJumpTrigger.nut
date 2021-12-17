include_entity("triggers/Trigger");

class PlayerJumpTrigger extends Trigger
</
	editorImage    = "editor.playerjumptrigger"
	libraryImage   = "editor.library.playerjumptrigger"
	placeable      = Placeable_Developer
	sizeShapeColor = Colors.Trigger
	displayName    = "Trigger - Player Jump"
	group          = "04.4 Player Triggers"
	stickToEntity  = true
/>
{
	function onTriggerEnter(p_entity)
	{
		local playerBot = ::getFirstEntityByTag("PlayerBot");
		if (playerBot == null)
		{
			return;
		}
		
		playerBot.callOnChildren("onButtonJumpPressed");
	}
}
