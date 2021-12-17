include_entity("triggers/Trigger");

class CampaignEndTrigger extends Trigger
</
	editorImage    = "editor.campaignendtrigger"
	libraryImage   = "editor.library.campaignendtrigger"
	placeable      = Placeable_Developer
	sizeShapeColor = Colors.Trigger
	displayName    = "Trigger - Campaign End"
	group          = "04.2 Mission / Campaign Triggers"
	stickToEntity  = true
/>
{
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		base.onInit();
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onTriggerEnterFirst(p_entity)
	{
		::MenuScreen.pushScreen("CampaignEndScreen", { _activator = p_entity.weakref() });
	}
}
