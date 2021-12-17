include_entity("triggers/Trigger");

class MissionEndTrigger extends Trigger
</
	editorImage    = "editor.missionendtrigger"
	libraryImage   = "editor.library.missionendtrigger"
	placeable      = Placeable_Developer
	sizeShapeColor = Colors.Trigger
	displayName    = "Trigger - Mission End"
	group          = "04.2 Mission / Campaign Triggers"
	stickToEntity  = true
/>
{
	once = true;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onTriggerEnterFirst(p_entity)
	{
		p_entity.customCallback("onPrepareEndMission");
	}
}
MissionEndTrigger.setattributes("once", null);
