include_entity("triggers/Trigger");

enum TagTriggerStatus
{
	None,
	Activated,
	Deactivated,
}

class ShoeboxTagTrigger extends Trigger
</
	editorImage    = "editor.tagtrigger"
	libraryImage   = "editor.library.tagtrigger"
	placeable      = Placeable_Developer
	sizeShapeColor = Colors.Trigger
	displayName    = "Trigger - Shoebox Tag"
	group          = "04.1 Action Triggers"
	stickToEntity  = true
/>
{
	</
		type   = "string"
		order  = 0
		group  = "Specific Settings"
	/>
	tag = "";
	
	</
		type        = "string"
		choice      = getShoeboxTagEventPresets()
		description = "The type of event to send the tag with on entering the trigger, can be null in which case nothing happens on enter"
		order       = 1
		group       = "Specific Settings"
	/>
	enterEvent = "show";
	
	</
		type        = "string"
		choice      = getShoeboxTagEventPresets()
		description = "The type of event to send the tag with on exiting the trigger, can be null in which case nothing happens on exiting"
		order       = 2
		group       = "Specific Settings"
	/>
	exitEvent = null;
	
	/*
	function onInvalidProperties(p_properties)
	{
		local propertyFixes = {}
		propertyFixes.killOnExit <- { result = "once" };
		
		fixInvalidProperties(p_properties, propertyFixes); 
		
		return p_properties;
	}
	*/
	
	_enterCommands = null;
	_exitCommands  = null;
	
	_lastTriggerStatus = TagTriggerStatus.None;
	
	function onInit()
	{
		base.onInit();
		
		if (enterEvent != null)
		{
			_enterCommands = getShoeboxTagEventCommands(enterEvent);
		}
		if (exitEvent != null)
		{
			_exitCommands = getShoeboxTagEventCommands(exitEvent);
		}
	}
	
	function onTriggerEnterFirst(p_entity)
	{
		if (_enterCommands != null)
		{
			foreach(command in _enterCommands)
			{
				sendShoeboxTagEvent(tag, command[0], command[1]);
			}
			
			_lastTriggerStatus = TagTriggerStatus.Activated;
		}
	}
	
	function onTriggerExitLast(p_entity)
	{
		if (_exitCommands != null)
		{
			foreach(command in _exitCommands)
			{
				sendShoeboxTagEvent(tag, command[0], command[1]);
			}
			
			_lastTriggerStatus = TagTriggerStatus.Deactivated;
		}
	}
	
	function onProgressRestored(p_id)
	{
		// note that the progress restore ignores the enabled state of the trigger
		switch(_lastTriggerStatus)
		{
		case TagTriggerStatus.Activated:   onTriggerEnterFirst(null); break;
		case TagTriggerStatus.Deactivated: onTriggerExitLast  (null); break;
		default: break; // Don't do anything
		}
	}
	
}