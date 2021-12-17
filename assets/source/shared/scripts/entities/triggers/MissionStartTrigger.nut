include_entity("triggers/Trigger");

class MissionStartTrigger extends Trigger
</
	editorImage    = "editor.missionstarttrigger"
	libraryImage   = "editor.library.missionstarttrigger"
	placeable      = Placeable_Developer
	sizeShapeColor = Colors.Trigger
	displayName    = "Trigger - Mission Start"
	group          = "04.2 Mission / Campaign Triggers"
	stickToEntity  = true
/>
{
	</
		type   = "string"
		choice = ::Level.getAllCampaignMissionIDs()
		order  = 0
		group  = "Specific Settings"
	/>
	missionID = null;
	
	</
		type        = "bool"
		group       = "Specific Settings"
		order       = 1.0
	/>
	discardHackedEntity= true;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		base.onInit();
		
		if (missionID == null)
		{
			editorWarning("No mission selected!");
			return;
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onTriggerEnterFirst(p_entity)
	{
		if (discardHackedEntity)
		{
			local player = ::getFirstEntityByTag("PlayerBot");
			if (player != null)
			{
				player._virusUploader.resetLastHackedEntity();
			}
		}
		
		if (missionID != null)
		{
			createFade(this, "transparent_to_opaque");
		}
	}
	
	function onFadeEnded(p_fade, p_animation)
	{
		if (::ProgressMgr.getPlayMode() == PlayMode.Mission)
		{
			local player = ::getFirstEntityByTag("PlayerBot");
			if (player != null)
			{
				::MenuScreen.pushScreen("IngamePostMissionSelection", { _activator = player });
			}
		}
		else
		{
			::ProgressMgr.startMission(missionID);
		}
	}
}
