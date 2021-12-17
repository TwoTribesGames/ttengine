include_entity("triggers/Trigger");

class CameraRotationTrigger extends Trigger
</ 
	editorImage    = "editor.camerarotationtrigger"
	libraryImage   = "editor.library.camerarotationtrigger"
	placeable      = Placeable_Everyone
	sizeShapeColor = Colors.Trigger
	displayName    = "Trigger - Camera Rotate"
	group          = "05. Camera"
	stickToEntity  = true
/>
{
	</
		type        = "bool"
		order       = 1
		group       = "Specific Settings"
	/>
	setOnEnter = true;
	
	</
		type        = "integer"
		min         = -45
		max         = 45
		group       = "Specific Settings"
		order       = 0.0
	/>
	rotationEnter = 15;
	
	</
		type        = "bool"
		group       = "Specific Settings"
		order       = 0.1
	/>
	setOnExit = false;
	
	</
		type        = "integer"
		min         = -45
		max         = 45
		group       = "Specific Settings"
		order       = 0.3
	/>
	rotationExit = 0;
	
	</
		type        = "bool"
		description = "Automatically perform exitAction after a set delay, turn 'once' on to make this a one time event"
		conditional = "setOnExit == true"
		group       = "Specific Settings"
		order       = 0.4
	/>
	autoExit = false;
	
	</
		type        = "float"
		min         = 0.5
		max         = 100.0
		description = "Delay for when autoExit is used"
		conditional = "autoExit == true"
		group       = "Specific Settings"
		order       = 0.5
	/>
	autoExitDelay = 5.0;
	
	</
		type        = "float"
		min         = 0.0
		max         = 10.0
		group       = "Specific Settings"
		order       = 0.6
	/>
	easingDuration = 2.0;
	
	</
		type        = "string"
		choice      = getEasingTypes()
		group       = "Specific Settings"
		order       = 0.7
	/>
	easingType = "QuadraticInOut";
	
	_easingTypeValue = null;
	_storedRotation  = 0;
	
	function onInit()
	{
		base.onInit();
		_easingTypeValue = ::getEasingTypeFromName(easingType);
	}
	
	function onTriggerEnterFirst(p_entity)
	{
		base.onTriggerEnterFirst(p_entity);
		
		_storedRotation = ::Camera.getTargetRotation();
		if (setOnEnter)
		{
			::Camera.setRotation(rotationEnter, easingDuration, _easingTypeValue);
		}
		
		if (autoExit)
		{
			startCallbackTimer("handleExit", autoExitDelay);
		}
	}
	
	function onTriggerExitLast(p_entity)
	{
		base.onTriggerExitLast(p_entity);
		
		if (autoExit == false)
		{
			handleExit();
		}
	}
	
	function handleExit()
	{
		if (setOnExit)
		{
			local rotation = setOnExit ? rotationExit : _storedRotation;
			::Camera.setRotation(rotation, easingDuration, _easingTypeValue);
		}
	}
}
