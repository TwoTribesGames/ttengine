include_entity("triggers/Trigger");
include_entity("rewind/bullets/Bullet");

class HealthSensingTrigger extends Trigger
</
	editorImage    = "editor.healthsensingtrigger"
	libraryImage   = "editor.library.healthsensingtrigger"
	placeable      = Placeable_Developer
	sizeShapeColor = Colors.Trigger
	displayName    = "Trigger - Health Sensing"
	group          = "04. Input Triggers"
	stickToEntity  = true
/>
{
	</
		type           = "entity"
		filter         = ["BaseEnemy", "PlayerBot"]
		order          = 0
		referenceColor = ReferenceColors.Sensing
		group          = "Specific Settings"
	/>
	target = null;
	
	</
		type   = "string"
		choice = ["<=", ">="]
		order  = 1
		group  = "Specific Settings"
	/>
	mode = ">=";
	
	</
		type        = "float"
		min         = 0.0
		max         = 1.0
		order       = 1
		description = "The normalized health percentage"
		group       = "Specific Settings"
	/>
	healthPercentage = 0.5;
	
	once = true;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		base.onInit();
		
		if (("_healthBar" in target) == false)
		{
			editorWarning("Cannot use a target without healthbar");
			target = null;
			return;
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Inherited Methods
	
	function createTouchSensor()
	{
		// Do nothing
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Update
	
	function update(p_deltaTime)
	{
		base.update(p_deltaTime);
		
		if (enabled && ::isValidEntity(target))
		{
			local health = target._healthBar.getNormalizedHealth();
			if ((mode == "<=" && health <= healthPercentage) ||
				(mode == ">=" && health >= healthPercentage) )
			{
				_triggerEnter(target, this);
				_triggerExit(target, this);
			}
		}
	}
}
HealthSensingTrigger.setattributes("once", null);
HealthSensingTrigger.setattributes("width", null);
HealthSensingTrigger.setattributes("height", null);
HealthSensingTrigger.setattributes("radius", null);
HealthSensingTrigger.setattributes("triggerOnTouch", null);
HealthSensingTrigger.setattributes("triggerOnUncull", null);
HealthSensingTrigger.setattributes("parentTrigger", null);
HealthSensingTrigger.setattributes("triggerFilter", null);
HealthSensingTrigger.setattributes("filterAllEntitiesOfSameType", null);
