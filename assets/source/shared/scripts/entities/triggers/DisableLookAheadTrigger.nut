include_entity("triggers/Trigger");

class DisableLookAheadTrigger extends Trigger
</ 
	editorImage    = "editor.disablelookaheadtrigger"
	libraryImage   = "editor.library.disablelookaheadtrigger"
	placeable      = Placeable_Developer
	sizeShapeColor = Colors.Trigger
	displayName    = "Trigger - Disable Lookahead"
	group          = "04.4 Player Triggers"
	stickToEntity  = true
/>
{
	function onTriggerEnterFirst(p_entity)
	{
		local playerBot = ::getFirstEntityByTag("PlayerBot");
		if (playerBot == null)
		{
			return;
		}
		
		playerBot.setLookAheadEnabled(false);
	}
	
	function onTriggerExitLast(p_entity)
	{
		local playerBot = ::getFirstEntityByTag("PlayerBot");
		if (playerBot == null)
		{
			return;
		}
		
		playerBot.setLookAheadEnabled(true);
	}
}
