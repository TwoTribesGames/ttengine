class Dictionary
{
	_entries = null;
	
	constructor()
	{
		_entries = {};
	}
	function len()
	{
		return _entries == null ? 0 : _entries.len();
	}
	
	// overload the _get so we can access entries by keyname like this: dict["key"]
	function _get(p_key)
	{
		if (containsKey(p_key))
		{
			return _entries[p_key];
		}
		throw null;
	}
	
	function isEmpty()
	{
		return len() == 0;
	}
	
	function containsKey(p_key)
	{
		return p_key in _entries;
	}
	
	function addEntry(p_key, p_val)
	{
		if (containsKey(p_key) == false)
		{
			_entries[p_key] <- p_val;
		}
	}
	
	// removes an entity, returns true if succesful
	function removeByKey(p_key)
	{
		if (containsKey(p_key))
		{
			delete _entries[p_key];
			return true;
		}
		return false;
	}
	
	function clear()
	{
		_entries = {};
	}
	
	function _typeof()
	{
		return "Dictionary";
	}
	
	function _tostring()
	{
		local ret = "Collection: " + _entries + " ";
		
		foreach (key, val in _entries)
		{
			ret += "(" + key + ", " + val + ") ";
			//ret += "<" + key + ", " + ::niceStringFromObject(val) + "> ";
		}
		
		return ret;
	}
}