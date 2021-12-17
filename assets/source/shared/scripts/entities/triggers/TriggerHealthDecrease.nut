include_entity("triggers/Trigger");

class TriggerHealthDecrease extends Trigger
</
	editorImage    = "editor.healthdecreasetrigger"
	libraryImage   = "editor.library.healthdecreasetrigger"
	placeable      = Placeable_Developer
	sizeShapeColor = Colors.TriggerHealthDecrease
	displayName    = "Trigger - Health Decrease"
	group          = "04.1 Action Triggers"
	stickToEntity  = true
/>
{
	</
		type        = "integer"
		min         = 1
		max         = 100
		description = "Amount of health to decrease (in percentages)"
		group       = "Specific Settings"
		order       = 0.0
	/>
	amount = 50;
	
	</
		type        = "bool"
		description = "If enabled, the trigger can kill the entity"
		group       = "Specific Settings"
		order       = 0.1
	/>
	allowKill = false;
	
	</
		type        = "bool"
		description = "When set to true, each time damage is done this will fire."
		group       = "Specific Settings"
		order       = 0.2
	/>
	triggerEachDamage = false;
	
	function onInit()
	{
		base.onInit();
	}
	
	function onSpawn()
	{
		base.onSpawn();
	}
	
	function _triggerEnter(p_entity, p_parent)
	{
		if ("_healthBar" in p_entity && p_entity._healthBar != null)
		{
			local damage = p_entity._healthBar._maxHealth * (amount / 100.0);
			if (allowKill == false)
			{
				damage = ::min(p_entity._healthBar._health - 1, damage);
				if (damage <= 0)
				{
					return;
				}
			}
			
			p_entity._healthBar.doDamage(damage, this);
			if (triggerEachDamage)
			{
				base._triggerEnter(p_entity, p_parent);
				base._triggerExit(p_entity, p_parent);
			}
		}
		// Original behavior
		if (triggerEachDamage == false)
		{
			base._triggerEnter(p_entity, p_parent);
		}
	}
}
