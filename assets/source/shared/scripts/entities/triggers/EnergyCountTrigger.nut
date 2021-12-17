include_entity("triggers/Trigger");

class EnergyCountTrigger extends Trigger
</
	editorImage    = "editor.energycounttrigger"
	libraryImage   = "editor.library.energycounttrigger"
	placeable      = Placeable_Developer
	sizeShapeColor = Colors.Trigger
	displayName    = "Trigger - Energy Count"
	group          = "04. Input Triggers"
	stickToEntity  = true
/>
{
	</
		type        = "string"
		choice      = ["<=", ">="]
		order       = 0.0
		group       = "Specific Settings"
	/>
	mode = ">=";
	
	</
		type        = "string"
		order       = 0.1
		group       = "Specific Settings"
		description = "Type a number here"
	/>
	threshold = "10";
	
	// Constants
	static c_sensorRecheckTime = 0.4;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		base.onInit();
		
		threshold = threshold.tointeger();
		
		// Add random to ensure not all sensors recheck at the exact same time
		startCallbackTimer("recheck", c_sensorRecheckTime + ::frnd_minmax(0.0, 1.0));
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Inherited Methods
	
	function isEntityEligible(p_entity)
	{
		if (base.isEntityEligible(p_entity))
		{
			local energy = p_entity._energyContainer.getEnergy();
			return ((mode == "<=" && energy <= threshold) ||
			       (mode == ">=" && energy >= threshold) )
		}
		
		return false;
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods
	
	function recheck()
	{
		_triggerSensor.removeAllSensedEntities();
		startCallbackTimer("recheck", c_sensorRecheckTime);
	}
}
EnergyCountTrigger.setattributes("triggerFilter", null);
EnergyCountTrigger.setattributes("filterAllEntitiesOfSameType", null);
EnergyCountTrigger.setattributes("triggerOnUncull", null);
