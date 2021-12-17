include_entity("triggers/Trigger");

class IntervalTrigger extends Trigger
</
	editorImage    = "editor.intervaltrigger"
	libraryImage   = "editor.library.intervaltrigger"
	placeable      = Placeable_Developer
	sizeShapeColor = Colors.Trigger
	displayName    = "Trigger - Interval"
	group          = "04. Input Triggers"
	stickToEntity  = true
/>
{
	</
		type        = "float"
		min         = 0.0
		max         = 25.0
		description = ""
		group       = "Specific Settings"
		order       = 0
	/>
	interval = 3.0;
	
	</
		type        = "float"
		min         = 0.0
		max         = 25.0
		description = ""
		group       = "Specific Settings"
		order       = 1
	/>
	duration = 1.0;
	
	_initiator = null;
	
	function onInit()
	{
		base.onInit();
		
		// "Fixing" 0 length timers and/or annoying sliders (if we'd set the min to nonzero)
		if (interval == 0.0) interval = 0.001;
		if (duration == 0.0) duration = 0.001;
	}
	
	function onTimer(p_name)
	{
		if (p_name == "interval")
		{
			_triggerEnter(_initiator, this);
			startTimer("duration", duration);
		}
		else if (p_name == "duration")
		{
			_triggerExit(_initiator, this);
			startTimer("interval", interval);
		}
	}
	
	function onTriggerEnterFirst(p_entity)
	{
		base.onTriggerEnterFirst(p_entity);
		_initiator = p_entity.weakref();
		
		startTimer("duration", duration);
	}
	
	function onTriggerExitLast(p_entity)
	{
		base.onTriggerExitLast(p_entity);
		
		stopTimer("interval");
		stopTimer("duration");
	}
	
	function setEnabled(p_enabled)
	{
		base.setEnabled(p_enabled);
		
		if (p_enabled == false)
		{
			stopTimer("interval");
			stopTimer("duration");
		}
	}

}