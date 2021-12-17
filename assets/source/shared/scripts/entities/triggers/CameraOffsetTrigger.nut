include_entity("triggers/Trigger");

::g_cameraOffsetTriggerEffect <- null;

class CameraOffsetTrigger extends Trigger
</
	editorImage    = "editor.cameraoffsettrigger"
	libraryImage   = "editor.library.cameraoffsettrigger"
	placeable      = Placeable_Everyone
	sizeShapeColor = Colors.Trigger
	displayName    = "Trigger - Camera Offset"
	group          = "05. Camera"
	stickToEntity  = true
/>
{
	</
		type        = "bool"
		group       = "Specific Settings"
		order       = 0.0
	/>
	setOnEnter = true;
	
	</
		type        = "float"
		min         = -32.0
		max         = 160.0
		conditional = "setOnEnter == true"
		group       = "Specific Settings"
		order       = 0.1
	/>
	offsetXEnter = 0.0;
	
	</
		type        = "float"
		min         = -32.0
		max         = 160.0
		conditional = "setOnEnter == true"
		group       = "Specific Settings"
		order       = 0.2
	/>
	offsetYEnter = 0.0;
	
	</
		type        = "bool"
		group       = "Specific Settings"
		order       = 0.3
	/>
	setOnExit = false;
	
	</
		type        = "float"
		min         = -32.0
		max         = 160.0
		conditional = "setOnExit == true"
		group       = "Specific Settings"
		order       = 0.4
	/>
	offsetXExit = 0.0;
	
	</
		type        = "float"
		min         = -32.0
		max         = 160.0
		conditional = "setOnExit == true"
		group       = "Specific Settings"
		order       = 0.5
	/>
	offsetYExit = 0.0;
	
	</
		type        = "bool"
		description = "Automatically perform exitAction after a set delay, turn 'once' on to make this a one time event"
		conditional = "setOnExit == true"
		group       = "Specific Settings"
		order       = 0.6
	/>
	autoExit = false;
	
	</
		type        = "float"
		min         = 0.0
		max         = 160.0
		description = "Delay for when autoExit is used"
		conditional = "autoExit == true"
		group       = "Specific Settings"
		order       = 0.7
	/>
	autoExitDelay = 5.0;
	
	</
		type        = "float"
		min         = 0.0
		max         = 20.0
		group       = "Specific Settings"
		order       = 0.7
	/>
	easingDuration = 2.0;
	
	</
		type        = "string"
		choice      = getEasingTypes()
		group       = "Specific Settings"
		order       = 0.8
	/>
	easingType = "QuadraticInOut";
	
	function onInvalidProperties(p_properties)
	{
		local propertyFixes = {}
		
		propertyFixes.offsetX        <- { result = "offsetXEnter" };
		propertyFixes.offsetY        <- { result = "offsetYEnter" };
		propertyFixes.transitionTime <- { result = "easingDuration" };
		
		fixInvalidProperties(p_properties, propertyFixes); 
		
		return p_properties;
	}
	
	function onTriggerEnterFirst(p_entity)
	{
		base.onTriggerEnterFirst(p_entity);
		
		if (setOnEnter)
		{
			if (::g_cameraOffsetTriggerEffect == null)
			{
				::g_cameraOffsetTriggerEffect = ::Camera.createCameraEffect();
			}
			
			local offset = ::Vector2(offsetXEnter, offsetYEnter);
			::g_cameraOffsetTriggerEffect.setOffset(
				offset, easingDuration, ::getEasingTypeFromName(easingType));
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
		if (setOnExit && ::g_cameraOffsetTriggerEffect != null)
		{
			local offset = ::Vector2(offsetXExit, offsetYExit);
			::g_cameraOffsetTriggerEffect.setOffset(offset, easingDuration, ::getEasingTypeFromName(easingType));
		}
	}
	
	function onReloadRequested()
	{
		::g_cameraOffsetTriggerEffect = null;
	}
}
