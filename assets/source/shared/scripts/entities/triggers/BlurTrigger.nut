include_entity("triggers/Trigger");
include_entity("LevelSettings");

class BlurTrigger extends Trigger
</ 
	editorImage    = "editor.blurtrigger"
	libraryImage   = "editor.library.blurtrigger"
	placeable      = Placeable_Developer
	collisionRect  = [ 0.0, 1.0, 2.0, 2.0 ]
	sizeShapeColor = Colors.Trigger
	displayName    = "Trigger - Blur"
	group          = "04.1 Action Triggers"
	stickToEntity  = true
/>

{
	</
		type   = "bool"
		order  = 1
		group  = "Specific Settings"
	/>
	useBackgroundBlur = true;
	
	</
		type   = "bool"
		order  = 2
		group  = "Specific Settings"
	/>
	useForegroundBlur = true;
	
	</
		type   = "float"
		min    = 0.0
		max    = 10.0
		order  = 5
		group  = "Specific Settings"
	/>
	easingDuration = 1;
	
	</
		type   = "string"
		choice = getEasingTypes()
		order  = 6
		group  = "Specific Settings"
	/>
	easingType = "QuadraticInOut";
	
	_easingTypeValue = null;
	_easingState = null;
	_startFlag = false;
	_isRunning = false;
	_levelSettings = null;

	function onInit()
	{
		base.onInit();
		_easingTypeValue = ::getEasingTypeFromName(easingType);
		_easingState = EasingReal(0.0, 1.0, easingDuration, _easingTypeValue);
	}
	
	function onSpawn()
	{
		base.onSpawn();
	}
	
	function onTriggerEnterFirst(p_entity)
	{
		base.onTriggerEnterFirst(p_entity);
		
		_startFlag = true;
		_isRunning = true;
	}
	
	function onTriggerExitLast(p_entity)
	{
		base.onTriggerExitLast(p_entity);
		
		/*if (useBackgroundBlur)
		{
			::Blur.restoreLastBackgroundRanges();
		}
		
		if (useForegroundBlur)
		{
			::Blur.restoreLastForegroundRanges();
		}*/
	}
	
	_startStateBackground = null;
	_startStateForeground = null;
	_currStateBackground = null;
	_currStateForeground = null;
	_endStateBackground = null;
	_endStateForeground = null;
	function update(p_deltaTime)
	{
		if (_isRunning)
		{
			_easingState.update(p_deltaTime);
			
			if (_startFlag)
			{
				_levelSettings = ::getEntitiesByTag("level_settings");
				_levelSettings = _levelSettings[0];
				
				_easingState = EasingReal(0.0, 1.0, easingDuration, _easingTypeValue);
				
				_startStateBackground = _levelSettings.getBackgroundLayers();
				_startStateForeground = _levelSettings.getForegroundLayers();
				
				_endStateBackground = ::Blur.getRangeArrayFromMembers(this, "backgroundBlurRange");
				_endStateForeground = ::Blur.getRangeArrayFromMembers(this, "foregroundBlurRange");
				
				if (useBackgroundBlur == false)
				{
					// I realise this is a hack, but the interpolation code below...
					// ...means I can't do setBackgroundLayers([])
					// so instead we have to push the blur layers as far from zero as possible
					_endStateBackground = [-250,-250,-250,-250,-250];
				}
				if (useForegroundBlur == false)
				{
					_endStateForeground = [50,50,50];
				}
				
				_startFlag = false;
			}
			else if (_easingState.getTime() >= _easingState.getDuration()) 
			{
				_isRunning = false;
				return;
			}
			
			_currStateBackground = interpolateBlurLayers(_startStateBackground, _endStateBackground, _easingState.getValue());
			_currStateForeground = interpolateBlurLayers(_startStateForeground, _endStateForeground, _easingState.getValue());
			
			_levelSettings.setBackgroundLayers(_currStateBackground);
			_levelSettings.setForegroundLayers(_currStateForeground);
		}
	}
	
	function interpolateBlurLayers(p_start, p_end, p_weight)
	{
		::assert(p_start.len() == p_end.len(), "Cannot interpolate arrays of differing size!");
		
		local output = array(p_start.len());
		for (local i = 0; i < p_start.len(); i++)
		{
			output[i] = ::lerp(p_start[i], p_end[i], p_weight);
		}
		return output;
	}
}

::Blur.addBackgroundRangeMembers(BlurTrigger, "backgroundBlurRange", 1.1, { group = "Specific Settings"});
::Blur.addForegroundRangeMembers(BlurTrigger, "foregroundBlurRange", 2.1, { group = "Specific Settings"});