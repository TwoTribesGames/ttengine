include_entity("triggers/Trigger");

class AmbientLightTrigger extends Trigger
</
	editorImage    = "editor.ambientlighttrigger"
	libraryImage   = "editor.library.ambientlighttrigger"
	placeable      = Placeable_Developer
	sizeShapeColor = Colors.Trigger
	displayName    = "Trigger - Ambient Light"
	group          = "04.1 Action Triggers"
	stickToEntity  = true
/>
{
	</
		type  = "integer"
		min   = 0
		max   = 255
		order = 1
		group = "Specific Settings"
	/>
	targetAmbientLight = 128;
	
	</
		type   = "float"
		min    = 0.0
		max    = 10.0
		order  = 11
		group  = "Specific Settings"
	/>
	easingDuration = 1;
	
	</
		type   = "string"
		choice = getEasingTypes()
		order  = 12
		group  = "Specific Settings"
	/>
	easingType = "QuadraticInOut";
	
	_easingState = null;
	_isRunning = false;
	
	function onInit()
	{
		base.onInit();
	}
	
	function onTriggerEnterFirst(p_entity)
	{
		base.onTriggerEnterFirst(p_entity);
		
		_easingState = EasingReal(::LightHelpers.getAmbientLight(), targetAmbientLight,
		                          easingDuration, ::getEasingTypeFromName(easingType));
		
		_isRunning = true;
	}
	
	function update(p_deltaTime)
	{
		if (_isRunning)
		{
			_easingState.update(p_deltaTime);
			
			if (_easingState.getTime() > _easingState.getDuration() ||
			    _easingState.getValue() == targetAmbientLight)
			{
				_isRunning = false;
				return;
			}
			
			::LightHelpers.setAmbientLight(_easingState.getValue());
		}
	}
}
