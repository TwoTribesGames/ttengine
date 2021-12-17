include_entity("triggers/Trigger");

class GameModeTrigger extends Trigger
</
	editorImage    = "editor.gamemodetrigger"
	libraryImage   = "editor.library.gamemodetrigger"
	placeable      = Placeable_Developer
	collisionRect  = [ 0.0, 1.0, 2.0, 2.0 ]
	sizeShapeColor = Colors.Trigger
	displayName    = "Trigger - GameMode"
	group          = "04.1 Action Triggers"
	stickToEntity  = true
/>
{
	</
		type   = "string"
		choice = ["Normal", "Brutal", "SpeedRun", "SingleLife"]
		group  = "Specific Settings"
		order  = 0.1
	/>
	gameMode = null;
	
	</
		type   = "string"
		choice = ["Campaign", "Mission"]
		group  = "Specific Settings"
		order  = 0.2
	/>
	playMode = null;
	
	function _triggerEnter(p_entity, p_parent)
	{
		local shouldTrigger = (gameMode == null || gameMode == ::ProgressMgr.getGameModeName(::ProgressMgr.getGameMode())) &&
		                      (playMode == null || playMode == ::ProgressMgr.getPlayModeName(::ProgressMgr.getPlayMode()));
		
		if (shouldTrigger)
		{
			base._triggerEnter(p_entity, p_parent);
		}
	}
}