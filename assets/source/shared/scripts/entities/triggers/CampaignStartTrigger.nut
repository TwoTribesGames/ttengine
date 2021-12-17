include_entity("triggers/Trigger");

class CampaignStartTrigger extends Trigger
</
	editorImage    = "editor.campaignstarttrigger"
	libraryImage   = "editor.library.campaignstarttrigger"
	placeable      = Placeable_Developer
	sizeShapeColor = Colors.Trigger
	displayName    = "Trigger - Campaign Start"
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
		// Only do this in Normal campaign mode
		if (::ProgressMgr.getGameMode() == GameMode.Normal &&
		    ::ProgressMgr.getPlayMode() == PlayMode.Campaign)
		{
			::MenuScreen.pushScreen("CampaignStartScreen", { _activator = p_entity.weakref() });
		}
	}
}
