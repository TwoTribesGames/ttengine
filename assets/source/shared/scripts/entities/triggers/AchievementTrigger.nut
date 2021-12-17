include_entity("triggers/Trigger");

class AchievementTrigger extends Trigger
</ 
	editorImage    = "editor.achievementtrigger"
	libraryImage   = "editor.library.achievementtrigger"
	placeable      = Placeable_Developer
	collisionRect  = [ 0.0, 1.0, 2.0, 2.0 ]
	sizeShapeColor = Colors.Trigger
	displayName    = "Trigger - Achievement"
	group          = "04.1 Action Triggers"
	stickToEntity  = true
/>

{
	</
		type   = "string"
		order  = 0
		group  = "Specific Settings"
	/>
	achievementID = "";
	
	function onSpawn()
	{
		base.onSpawn();
	}
	
	function onTriggerEnterFirst(p_entity)
	{
		base.onTriggerEnterFirst(p_entity);
		
		::Stats.unlockAchievement(achievementID);
		::Stats.storeAchievements();
		//::echo("unlocked" achievementID "!!!", "Achievement Unlocked");
	}
	
	function onTriggerExitLast(p_entity)
	{
		base.onTriggerExitLast(p_entity);
		
	}
}