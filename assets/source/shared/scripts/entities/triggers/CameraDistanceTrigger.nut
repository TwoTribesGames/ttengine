include_entity("triggers/Trigger");

class CameraDistanceTrigger extends Trigger
</
	editorImage    = "editor.cameradistancetrigger"
	libraryImage   = "editor.library.cameradistancetrigger"
	placeable      = Placeable_Everyone
	sizeShapeColor = Colors.Trigger
	displayName    = "Trigger - Camera Distance"
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
	mode = "<=";
	
	</
		type        = "integer"
		min         = 1
		max         = 100
		order       = 0.1
		group       = "Specific Settings"
	/>
	distance = 30;
	
	once = true;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		base.onInit();
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
		
		if (enabled == false)
		{
			return;
		}
		
		local dist = (getCenterPosition() - Camera.getPosition()).length();
		if ((mode == "<=" && dist <= distance) ||
			(mode == ">=" && dist >= distance) )
		{
			_triggerEnter(this, this);
			_triggerExit(this, this);
		}
	}
}
CameraDistanceTrigger.setattributes("once", null);
CameraDistanceTrigger.setattributes("width", null);
CameraDistanceTrigger.setattributes("height", null);
CameraDistanceTrigger.setattributes("radius", null);
CameraDistanceTrigger.setattributes("triggerOnTouch", null);
CameraDistanceTrigger.setattributes("triggerOnUncull", null);
CameraDistanceTrigger.setattributes("parentTrigger", null);
CameraDistanceTrigger.setattributes("triggerFilter", null);
CameraDistanceTrigger.setattributes("filterAllEntitiesOfSameType", null);
