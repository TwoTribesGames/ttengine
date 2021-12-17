tt_include("includes/Dictionary");

class EntityCollection extends Dictionary
{
	function containsEntity(p_entity)
	{
		return containsKey(p_entity.getHandleValue());
	}
	
	// adds the entity to the collection and returns the key (that can be used for timers etc)
	function addEntity(p_entity)
	{
		local key = p_entity.getHandleValue();
		addEntry(key, p_entity.weakref());
		
		return key;
	}
	
	// removes an entity, returns true if succesful
	function removeEntity(p_entity)
	{
		return removeByKey(p_entity.getHandleValue());
	}
	
	function len()
	{
		purgeInvalidEntities();
		return base.len();
	}
	
	function purgeInvalidEntities()
	{
		if (_entries == null)
		{
			return;
		}
		
		local clean = {};
		foreach(key, entity in _entries)
		{
			if (::isValidEntity(entity))
			{
				clean[key] <- entity;
			}
		}
		_entries = clean;
	}
	
	function getAllEntities()
	{
		purgeInvalidEntities();
		return _entries;
	}
}
