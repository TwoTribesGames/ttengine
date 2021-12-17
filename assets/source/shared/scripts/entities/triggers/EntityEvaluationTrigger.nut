include_entity("triggers/Trigger");

class EntityEvaluationTrigger extends Trigger
</
	placeable = Placeable_Hidden
/>
{
	condition = null;
	_registeredEntities = null;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		base.onInit();
		
		if (condition == null)
		{
			editorWarning("Specify condition");
			::killEntity(this);
			return;
		}
		
		_registeredEntities = {};
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onTouchEnter(p_entity, p_sensor)
	{
		_registeredEntities[p_entity.getHandleValue()] <- [p_entity.weakref(), false];
	}
	
	function onTouchExit(p_entity, p_sensor)
	{
		local elem = _registeredEntities[p_entity.getHandleValue()];
		if (elem[1])
		{
			_triggerExit(elem[0], this);
		}
		delete _registeredEntities[p_entity.getHandleValue()];
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Update
	
	function update(p_deltaTime)
	{
		base.update(p_deltaTime);
		
		foreach (elem in _registeredEntities)
		{
			local entity = elem[0];
			if (::isValidEntity(entity))
			{
				local eval = entity.onEvaluateEntity(condition);
				if (elem[1] == false && eval)
				{
					_triggerEnter(entity, this);
					elem[1] = true;
				}
				else if (elem[1] && eval == false)
				{
					_triggerExit(entity, this);
					elem[1] = false;
				}
			}
			else
			{
				::tt_panic("There is an invalid entity in the EntityEvaluationTrigger. This shouldn't happen.");
			}
		}
	}
}
EntityEvaluationTrigger.setattributes("triggerOnTouch", null);
EntityEvaluationTrigger.setattributes("triggerOnUncull", null);
EntityEvaluationTrigger.setattributes("parentTrigger", null);
