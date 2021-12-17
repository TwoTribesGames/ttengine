include_entity("triggers/Trigger");

class TriggerHealthIncrease extends Trigger
</
	editorImage    = "editor.healthincreasetrigger"
	libraryImage   = "editor.library.healthincreasetrigger"
	placeable      = Placeable_Developer
	sizeShapeColor = Colors.TriggerHealthDecrease
	displayName    = "Trigger - Health Increase"
	group          = "04.1 Action Triggers"
	stickToEntity  = true
/>
{
	</
		type        = "integer"
		min         = 1
		max         = 99
		description = "Amount of health to decrease (in points)"
		order       = 0
		group       = "Specific Settings"
	/>
	amount = 50;
	
	function onInit()
	{
		base.onInit();
	}
	
	function onSpawn()
	{
		base.onSpawn();
	}
	
	function onTriggerEnter(p_entity)
	{
		if ("_healthBar" in p_entity && p_entity._healthBar != null)
		{
			p_entity._healthBar.doHeal(amount);
		}
	}
}
