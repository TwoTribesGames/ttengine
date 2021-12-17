include_entity("triggers/Trigger");

class CameraMoveTrigger extends Trigger
</ 
	editorImage    = "editor.cameratrigger"
	libraryImage   = "editor.library.cameratrigger"
	placeable      = Placeable_Everyone
	collisionRect  = [ 0.0, 1.0, 2.0, 2.0 ]
	sizeShapeColor = Colors.Trigger
	displayName    = "Trigger - Camera Move"
	group          = "05. Camera"
	stickToEntity  = true
/>

{
	</
		type           = "entity"
		filter         = ["Waypoint"]
		referenceColor = ReferenceColors.Waypoint
		group          = "Specific Settings"
		order          = 0.0
	/>
	wayPoint = null;
	
	</
		type   = "float"
		min    = 0.0
		max    = 30.0
		group  = "Specific Settings"
		order  = 0.1
	/>
	durationToWaypoint = 1.0;
	
	</
		type   = "string"
		choice = getEasingTypes()
		group  = "Specific Settings"
		order  = 0.2
	/>
	easingTypeToWaypoint = "QuadraticInOut";
	
	</
		type   = "float"
		min    = 0.0
		max    = 30.0
		group  = "Specific Settings"
		order  = 0.3
	/>
	durationFromWaypoint = 0.0;
	
	</
		type   = "string"
		choice = getEasingTypes()
		group  = "Specific Settings"
		order  = 0.4
	/>
	easingTypeFromWaypoint = "QuadraticInOut";
	
	</
		type        = "float"
		min         = 0.0
		max         = 60.0
		description = "After X seconds the trigger exits automatically. Use 0 for no timeout."
		group       = "Specific Settings"
		order       = 0.5
	/>
	timeout = 0.0;
	
	_followEntity  = null;
	_startPosition = null;
	
	function onValidateScriptState()
	{
		if (::isValidAndInitializedEntity(wayPoint) == false)
		{
			editorWarning("No valid waypoint set, please pick a waypoint for the CameraTrigger to focus on!");
		}
	}
	
	function onInvalidProperties(p_properties)
	{
		local propertyFixes = {}
		
		propertyFixes.easingDuration <- { result = "durationToWaypoint" };
		
		fixInvalidProperties(p_properties, propertyFixes); 
		
		return p_properties;
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onTriggerEnter(p_entity)
	{
		base.onTriggerEnter(p_entity);
		
		if (p_entity instanceof ::PlayerBot)
		{
			p_entity.setLookAheadEnabled(false);
		}
		
		stopAllTimers();
		
		_followEntity  = ::Camera.getPrimaryFollowEntity();
		_startPosition = ::Camera.getTargetPosition();
		::Camera.setPosition(wayPoint.getCenterPosition(), durationToWaypoint,
		                     ::getEasingTypeFromName(easingTypeToWaypoint));
		
		if (timeout > 0.0)
		{
			startTimer("exitTrigger", timeout);
		}
	}
	
	function onTriggerExit(p_entity)
	{
		base.onTriggerExit(p_entity);
		
		if (p_entity instanceof ::PlayerBot)
		{
			p_entity.setLookAheadEnabled(true);
		}
		
		stopAllTimers();
		
		if (durationFromWaypoint > 0)
		{
			::Camera.setPosition(_startPosition, durationFromWaypoint,
			                     ::getEasingTypeFromName(easingTypeFromWaypoint));
			startCallbackTimer("onMoveCameraBack", durationFromWaypoint);
		}
		else
		{
			onMoveCameraBack();
		}
	}
	
	function onMoveCameraBack()
	{
		stopAllTimers();
		::Camera.setPrimaryFollowEntity(_followEntity);
	}
	
	function onTimer(p_name)
	{
		if (p_name == "exitTrigger")
		{
			_triggerExit(_followEntity, this);
		}
	}
}
