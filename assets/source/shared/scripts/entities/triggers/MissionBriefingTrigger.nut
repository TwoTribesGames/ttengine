include_entity("triggers/Trigger");

class MissionBriefingTrigger extends Trigger
</
	editorImage    = "editor.missionbriefingtrigger"
	libraryImage   = "editor.library.missionbriefingtrigger"
	placeable      = Placeable_Developer
	sizeShapeColor = Colors.Trigger
	displayName    = "Trigger - Mission Briefing"
	group          = "04.2 Mission / Campaign Triggers"
	stickToEntity  = true
/>
{
	_missionBriefing = null;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onTriggerEnterFirst(p_entity)
	{
		if (::ProgressMgr.getPlayMode() != PlayMode.BattleArena)
		{
			_missionBriefing = ::spawnEntity("MissionBriefing", ::Vector2(0, 0));
		}
	}
	
	function onTriggerExitLast(p_entity)
	{
		if (_missionBriefing != null)
		{
			_missionBriefing.removeMe();
		}
	}
	
}
