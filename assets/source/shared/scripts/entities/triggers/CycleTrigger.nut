include_entity("triggers/Trigger");

class CycleTrigger extends Trigger
</
	editorImage    = "editor.cycletrigger"
	libraryImage   = "editor.library.cycletrigger"
	placeable      = Placeable_Developer
	sizeShapeColor = Colors.Trigger
	displayName    = "Trigger - Cycle"
	group          = "04. Input Triggers"
	stickToEntity  = true
/>
{
	</
		type           = "entityid_array"
		filter         = ["Trigger"]
		referenceColor = ReferenceColors.List
		group          = "Specific Settings"
		order          = 0.0
	/>
	entities = null;
	
	</
		type  = "bool"
		group = "Specific Settings"
	/>
	loop = true;
	
	_currentEntityIndex = 0;
	
	function onSpawn()
	{
		base.onSpawn();
		
		if (entities == null)
		{
			editorWarning("Select entities");
			::killEntity(this);
			return;
		}
		
		// Disable touch for existing entities. For entities that are spawned later, this is not
		// possible anymore
		foreach (id in entities)
		{
			local entity = ::getEntityByID(id);
			if (entity != null && entity._triggerSensor != null)
			{
				entity._triggerSensor.setEnabled(false);
			}
		}
	}
	
	function _triggerEnter(p_entity, p_parent)
	{
		base._triggerEnter(p_entity, p_parent);
		
		if (isEnabled() == false)
		{
			return;
		}
		
		if (entities.len() > 0)
		{
			local entity = ::getEntityByID(entities[_currentEntityIndex]);
			if (entity != null)
			{
				entity._triggerEnter(p_entity, p_parent);
				entity._triggerExit(p_entity, p_parent);
			}
			else
			{
				::tt_warning("Trigger with id '" + entities[_currentEntityIndex] + "' doesn't exist, yet?");
			}
			
			_currentEntityIndex += 1;
			if (loop)
			{
				_currentEntityIndex = _currentEntityIndex % entities.len();
			}
			else
			{
				_currentEntityIndex = ::min(_currentEntityIndex, entities.len() - 1);
			}
		}
	}
}
