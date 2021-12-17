include_entity("ColorGrading");
include_entity("triggers/Trigger");

class ColorGradingTrigger extends Trigger
</
	editorImage    = "editor.colorgradingtrigger"
	libraryImage   = "editor.library.colorgradingtrigger"
	placeable      = Placeable_Developer
	sizeShapeColor = Colors.Trigger
	displayName    = "Trigger - Color Grading Trigger"
	group          = "04.1 Action Triggers"
	stickToEntity  = true
/>

{
	</
		type   = "string"
		choice = getColorGradingNames()
		order  = 1
		group  = "Specific Settings"
	/>
	colorGradingEnter = null;
	
	</
		type   = "bool"
		order  = 2
		group  = "Specific Settings"
	/>
	SetColorGradingOnEnter = true;
	
	</
		type        = "string"
		choice      = getColorGradingNames()
		order       = 3
		description = "Leave this set to the default to restore to the value that was active when entering the trigger"
		group       = "Specific Settings"
	/>
	colorGradingExit = null;
	
	</
		type   = "bool"
		order  = 4
		group  = "Specific Settings"
	/>
	SetColorGradingOnExit = true;
	
	</
		type   = "float"
		min    = 0.0
		max    = 10.0
		order  = 5
		group  = "Specific Settings"
	/>
	duration = 1;
	
	_idEnter = null;
	_idExit  = null;
	function onTriggerEnterFirst(p_entity)
	{
		base.onTriggerEnterFirst(p_entity);
		
		if (SetColorGradingOnEnter)
		{
			if (colorGradingEnter != null)
			{
				if (_idEnter != null)
				{
					::ColorGrading.remove(_idEnter, duration);
				}
				_idEnter = ::ColorGrading.add(colorGradingEnter, duration);
			}
		}
	}
	
	function onTriggerExitLast(p_entity)
	{
		base.onTriggerExitLast(p_entity);
		
		if (SetColorGradingOnExit)
		{
			if (SetColorGradingOnEnter)
			{
				::assert(_idEnter != null, "_idEnter should not be null");
				::ColorGrading.remove(_idEnter, duration);
				_idEnter = null;
			}
			
			if (colorGradingExit != null)
			{
				_idExit = ::ColorGrading.add(colorGradingExit, duration);
			}
		}
	}
	
	function onDie()
	{
		base.onDie();
		
		::ColorGrading.remove(_idExit, duration);
		::ColorGrading.remove(_idEnter, duration);
	}
}

