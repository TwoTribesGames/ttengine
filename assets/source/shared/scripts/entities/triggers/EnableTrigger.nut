include_entity("triggers/Trigger");

class EnableTrigger extends Trigger
</
	editorImage    = "editor.enabletrigger"
	libraryImage   = "editor.library.enabletrigger"
	placeable      = Placeable_Developer
	sizeShapeColor = Colors.Trigger
	displayName    = "Trigger - Enable"
	group          = "04. Input Triggers"
	stickToEntity  = true
/>
{
	</
		type           = "entityid_array"
		referenceColor = ReferenceColors.Enable
		group          = "Specific Settings"
		order          = 0.0
	/>
	enableEntities = null;
	
	</
		type           = "entityid_array"
		referenceColor = ReferenceColors.Disable
		group          = "Specific Settings"
		order          = 0.1
	/>
	disableEntities = null;
	
	</
		type           = "string"
		choice         = ["Enable", "Disable", "Enable / Disable", "Do nothing"]
		group          = "Specific Settings"
		order          = 0.2
	/>
	enterAction = "Enable";
	
	</
		type           = "string"
		choice         = ["Enable", "Disable", "Enable / Disable", "Do nothing"]
		group          = "Specific Settings"
		order          = 0.3
	/>
	exitAction = "Do nothing";
	
	</
		type           = "float"
		min            = 0.0
		max            = 100.0
		description    = "Delay before the enter action triggers"
		group          = "Specific Settings"
		order          = 0.4
	/>
	enterDelay = 0.0;
	
	</
		type           = "float"
		min            = 0.0
		max            = 10.0
		description    = "Delay before the exit action triggers"
		group          = "Specific Settings"
		order          = 0.5
	/>
	exitDelay = 0.0;
	
	function onInit()
	{
		base.onInit();
		
		if (enableEntities == null && disableEntities == null)
		{
			editorWarning("Specify entities");
			enableEntities  = [];
			disableEntities = [];
		}
		
		if (enterAction == "Do nothing" && exitAction == "Do nothing")
		{
			editorWarning("Both actions are 'Do nothing'");
			::killEntity(this);
			return;
		}
		
		if (enableEntities == null)
		{
			enableEntities = [];
		}
		
		if (disableEntities == null)
		{
			disableEntities = [];
		}
	}
	
	function setEntitiesEnabled(p_entities, p_enabled)
	{
		foreach (id in p_entities)
		{
			local entity = ::getEntityByID(id);
			if (::isValidEntity(entity) && ("setEnabled" in entity))
			{
				entity.setEnabled(p_enabled);
			}
		}
	}
	
	function performEnterAction()
	{
		switch (enterAction)
		{
		case "Enable":
			setEntitiesEnabled(enableEntities, true);
			break;
			
		case "Disable":
			setEntitiesEnabled(disableEntities, false);
			break;
			
		case "Enable / Disable":
			setEntitiesEnabled(enableEntities, true);
			setEntitiesEnabled(disableEntities, false);
			break;
			
		default:
			::tt_panic("Unhandled enterAction '" + enterAction + "'");
		}
	}
	
	function performExitAction()
	{
		switch (exitAction)
		{
		case "Enable":
			setEntitiesEnabled(enableEntities, true);
			break;
			
		case "Disable":
			setEntitiesEnabled(disableEntities, false);
			break;
			
		case "Enable / Disable":
			setEntitiesEnabled(enableEntities, true);
			setEntitiesEnabled(disableEntities, false);
			break;
			
		default:
			::tt_panic("Unhandled exitAction '" + exitAction + "'");
		}
	}
	
	function onTriggerEnterFirst(p_entity)
	{
		if (enterAction == "Do nothing")
		{
			return;
		}
		
		if (enterDelay > 0)
		{
			startCallbackTimer("performEnterAction", enterDelay);
		}
		else
		{
			performEnterAction();
		}
	}
	
	function onTriggerExitLast(p_entity)
	{
		if (exitAction == "Do nothing")
		{
			return;
		}
		
		if (exitDelay > 0)
		{
			startCallbackTimer("performExitAction", exitDelay);
		}
		else
		{
			performExitAction();
		}
	}
}
