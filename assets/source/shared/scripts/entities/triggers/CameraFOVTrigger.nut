include_entity("triggers/Trigger");

class CameraFOVTrigger extends Trigger
</
	editorImage    = "editor.camerafovtrigger"
	libraryImage   = "editor.library.camerafovtrigger"
	placeable      = Placeable_Everyone
	sizeShapeColor = Colors.Trigger
	displayName    = "Trigger - Camera FOV"
	group          = "05. Camera"
	stickToEntity  = true
/>
{
	</
		type   = "bool"
		order  = 1
		group  = "Specific Settings"
	/>
	SetFOVOnEnter = true;
	
	</
		type   = "integer"
		min    = 1
		max    = 180
		order  = 2
		group  = "Specific Settings"
	/>
	FOVEnter = 60;
	
	</
		type    = "bool"
		order   = 3
		group   = "Specific Settings"
	/>
	SetFOVOnExit = false;
	
	</
		type        = "integer"
		min         = 0
		max         = 180
		order       = 4
		description = "FOV when exiting the trigger, set to 0 to restore to the FOV when entering the trigger"
		group       = "Specific Settings"
	/>
	FOVExit = 0;
	
	</
		type   = "float"
		min    = 0.0
		max    = 60.0
		order  = 5
		group  = "Specific Settings"
	/>
	easingDuration = 2;
	
	</
		type   = "string"
		choice = getEasingTypes()
		order  = 6
		group  = "Specific Settings"
	/>
	easingType = "QuadraticInOut";
	
	_easingTypeValue = null;
	
	function onInvalidProperties(p_properties)
	{
		local valueFixes = {}
		
		valueFixes.FOVEnter <- { result = @(val) val.tointeger() };
		valueFixes.FOVExit <- { result = @(val) val.tointeger() };
		
		fixInvalidValues(p_properties, valueFixes); 
		
		if ("instant" in p_properties)
		{
			p_properties.easingDuration <- "0";
			delete p_properties.instant;
		}
		
		if ("FOVSpeed" in p_properties)
		{
			// use a ballpark figure for converting the old speeds to a duration
			p_properties.easingDuration <- (0.01 / p_properties.FOVSpeed.tofloat()).tostring();
			delete p_properties.FOVSpeed;
		}
		
		return base.onInvalidProperties(p_properties);
	}
	
	function onInit()
	{
		base.onInit();
		_easingTypeValue = ::getEasingTypeFromName(easingType);
	}
	
	_storedFOV = 40;
	function onTriggerEnterFirst(p_entity)
	{
		base.onTriggerEnterFirst(p_entity);
		
		_storedFOV = ::Camera.getTargetFOV();
		if (SetFOVOnEnter)
		{
			::Camera.setFOV(FOVEnter, easingDuration, _easingTypeValue);
		}
	}
	
	function onTriggerExitLast(p_entity)
	{
		base.onTriggerExitLast(p_entity);
		
		if (SetFOVOnExit)
		{
			::Camera.setFOV(FOVExit == 0 ? _storedFOV: FOVExit, easingDuration, _easingTypeValue);
		}
	}
}

