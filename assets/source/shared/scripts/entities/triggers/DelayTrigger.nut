include_entity("triggers/Trigger");

enum TriggerAction {
	Enter,
	Exit
}

class DelayTrigger extends Trigger
</
	editorImage    = "editor.triggerdelay"
	libraryImage   = "editor.library.triggerdelay"
	placeable      = Placeable_Everyone
	sizeShapeColor = Colors.Trigger
	displayName    = "Trigger - Delay"
	group          = "04. Input Triggers"
	stickToEntity  = true
/>
{
	</
		type        = "float"
		min         = 0.0
		max         = 100.0
		description = "Delay before the enter action triggers"
		order       = 0.0
		group       = "Specific Settings"
	/>
	enterDelay = 0.0;
	
	</
		type        = "float"
		min         = 0.0
		max         = 100.0
		description = "Delay before the exit action triggers"
		order       = 1.5
		group       = "Specific Settings"
	/>
	exitDelay = 0.0;
	
	_timerID        = 0;
	_queuedTriggers = null;
	function onInit()
	{
		base.onInit();
		_queuedTriggers = {};
		
		// Make sure the difference between enter and exit is at least one frame.
		// Enter and exit should NEVER end up at the same frame, otherwise
		// the exit might be triggered before the enter.
		exitDelay += enterDelay;
		if ((exitDelay - enterDelay) < 0.02)
		{
			exitDelay = enterDelay + 0.02;
		}
	}
	
	function onTimer(p_name)
	{
		local id = p_name.tointeger();
		
		if (id in _queuedTriggers)
		{
			local queued = _queuedTriggers[id];
			
			switch (queued.action)
			{
			case TriggerAction.Enter:
				base._triggerEnter(queued.entity, this);
				break;
			case TriggerAction.Exit:
				base._triggerExit(queued.entity, this);
				break;
			}
			
			delete _queuedTriggers[id];
		}
	}
	
	function queueTrigger(p_action, p_entity, p_delay)
	{
		_timerID++;
		
		_queuedTriggers[_timerID] <- { action = p_action, entity = p_entity.weakref() };
		
		startTimer(_timerID.tostring(), p_delay);
	}
	
	function _triggerEnter(p_entity, p_parent)
	{
		queueTrigger(TriggerAction.Enter, p_entity, enterDelay);
	}
	
	function _triggerExit(p_entity, p_parent)
	{
		queueTrigger(TriggerAction.Exit, p_entity, exitDelay);
	}
}
