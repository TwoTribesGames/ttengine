include_entity("triggers/Trigger");

class TripwireTrigger extends Trigger
</
	editorImage    = "editor.tripwiretrigger"
	libraryImage   = "editor.library.tripwiretrigger"
	placeable      = Placeable_Everyone
	collisionRect  = [ 0.0, 0.5, 1.0, 1.0 ]
	sizeShapeColor = Colors.Trigger
	group          = "04.1 Action Triggers"
	stickToEntity  = true
/>
{
	</
		type = "entity"
		filter = ["Waypoint"]
		order = -1
		group = "Specific Settings"
	/>
	target = null
	
	function onValidateScriptState()
	{
		if (target == null)
		{
			editorWarning("No target set");
		}
	}
	
	function getTouchSensorShape()
	{
		return RayShape(::Vector2(0, 0), target);
	}
}
TripwireTrigger.setattributes("width", null);
TripwireTrigger.setattributes("height", null);
TripwireTrigger.setattributes("triggerOnTouch", null);