include_entity("triggers/Trigger");

class FogTrigger extends Trigger
</ 
	editorImage    = "editor.fogtrigger"
	libraryImage   = "editor.library.fogtrigger"
	placeable      = Placeable_Everyone
	sizeShapeColor = Colors.Trigger
	displayName    = "Trigger - Fog"
	group          = "04.1 Action Triggers"
	stickToEntity  = true
/>
{
	</
		type        = "color_rgb"
		order       = 0
		group       = "Specific Settings"
	/>
	fogColorEnter = null;
	
	</
		type        = "bool"
		order       = 1
		group       = "Specific Settings"
	/>
	setNearFarOnEnter = true;
	
	</
		type        = "integer"
		min         = -250
		max         = 100
		order       = 2
		group       = "Specific Settings"
		conditional = "setNearFarOnEnter == true"
	/>
	fogNearEnter = 0;
	
	</
		type        = "integer"
		min         = -250
		max         = 100
		order       = 3
		group       = "Specific Settings"
		conditional = "setNearFarOnEnter == true"
	/>
	fogFarEnter = -30;
	
	</
		type        = "bool"
		order       = 4
		group       = "Specific Settings"
		conditional = "fogColorExit == null"
	/>
	resetColorOnExit = false;
	
	</
		type        = "color_rgb"
		order       = 5
		description = "Leave this set to the default to restore to the value that was active when entering the trigger"
		group       = "Specific Settings"
		conditional = "resetColorOnExit == false"
	/>
	fogColorExit = null;

	</
		type        = "bool"
		order       = 6
		group       = "Specific Settings"
		conditional = "setNearFarOnExit == false"
	/>
	resetNearFarOnExit = false;
	
	</
		type        = "bool"
		order       = 7
		group       = "Specific Settings"
		conditional = "resetNearFarOnExit == false"
	/>
	setNearFarOnExit = false;
	
	</
		type        = "integer"
		min         = -250
		max         = 100
		order       = 8
		group       = "Specific Settings"
		conditional = "resetNearFarOnExit == false"
	/>
	fogNearExit = 0;
	
	</
		type        = "integer"
		min         = -250
		max         = 100
		order       = 9
		description = "When exiting the trigger, set to -1 to restore to the value when entering the trigger"
		group       = "Specific Settings"
		conditional = "resetNearFarOnExit == false"
	/>
	fogFarExit = -30;
	
	</
		type        = "float"
		min         = 0.0
		max         = 10.0
		order       = 10
		group       = "Specific Settings"
	/>
	easingDuration = 1;
	
	</
		type        = "string"
		choice      = getEasingTypes()
		order       = 11
		group       = "Specific Settings"
	/>
	easingType = "QuadraticInOut";
	
	_easingTypeValue = null;
	_storedFogColor  = null;
	_storedFogNear   = 0;
	_storedFogFar    = 0;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		base.onInit();
		
		_easingTypeValue = ::getEasingTypeFromName(easingType);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onTriggerEnterFirst(p_entity)
	{
		base.onTriggerEnterFirst(p_entity);
		
		_storedFogColor = ::getDefaultFogColor();
		if (fogColorEnter != null)
		{
			::setDefaultFogColor(fogColorEnter, easingDuration, _easingTypeValue);
		}
		
		local distance = ::Camera.getCameraDistance();
		_storedFogNear = -::getDefaultFogNear() + distance;
		_storedFogFar  = -::getDefaultFogFar() + distance;
		
		if (setNearFarOnEnter)
		{
			setNearFar(fogNearEnter, fogFarEnter);
		}
	}
	
	function onTriggerExitLast(p_entity)
	{
		base.onTriggerExitLast(p_entity);
		
		if (resetColorOnExit)
		{
			::setDefaultFogColor(_storedFogColor, easingDuration, _easingTypeValue);
		}
		else if (fogColorExit != null)
		{
			::setDefaultFogColor(fogColorExit, easingDuration, _easingTypeValue);
		}
		
		if (resetNearFarOnExit)
		{
			setNearFar(_storedFogNear, _storedFogFar);
		}
		else if (setNearFarOnExit)
		{
			setNearFar(fogNearExit, fogFarExit);
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods
	
	function setNearFar(p_near, p_far)
	{
		local distance = ::Camera.getCameraDistance();
		::setDefaultFogNearFar(-p_near + distance, -p_far + distance, easingDuration, _easingTypeValue);
	}
}
