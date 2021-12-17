tt_include("includes/logs");

class EventPublisher
{
	_listeners = null;
	
	function subscribe(p_entity, p_callback)
	{
		if (_listeners == null)
		{
			_listeners = {};
		}
		_listeners[p_entity.getHandleValue()] <- p_callback;
	}
	
	function unsubscribe(p_entity, p_func)
	{
		delete _listeners[p_entity.getHandleValue()];
		if (_listeners.len() == 0)
		{
			_listeners = null;
		}
	}
	
	function len()
	{
		return _listeners == null ? 0 : _listeners.len();
	}
	
	function publish(...)
	{
		if (len() == 0)
		{
			// Early exit to prevent needless creation of arrays
			return;
		}
		
		local cleanTable = {};
		local calls      = [];
		foreach (handle, callback in _listeners)
		{
			local entity = ::getEntityByHandleValue(handle);
			if (::isValidEntity(entity))
			{
				local args = [entity, callback];
				args.extend(vargv);
				
				calls.push([entity, args]);
				cleanTable[handle] <- callback;
			}
		}
		_listeners = cleanTable;
		
		// Do the actual calls at the very last with a copy, because _listeners might change as a result of it
		foreach (call in calls)
		{
			// use customCallback so entities can respond in states (or ignore the callback)
			call[0].customCallback.acall(call[1]);
		}
	}
	
	function publishToType(p_type, ...)
	{
		if (len() == 0)
		{
			// Early exit to prevent needless creation of arrays
			return;
		}
		
		local cleanTable = {};
		local calls      = [];
		foreach (handle, callback in _listeners)
		{
			local entity = ::getEntityByHandleValue(handle);
			if (::isValidEntity(entity))
			{
				if (entity instanceof p_type)
				{
					local args = [entity, callback];
					args.extend(vargv);
					
					calls.push([entity, args]);
				}
				cleanTable[handle] <- callback;
			}
		}
		_listeners = cleanTable;
		
		// Do the actual calls at the very last with a copy, because _listeners might change as a result of it
		foreach (call in calls)
		{
			// use customCallback so entities can respond in states (or ignore the callback)
			call[0].customCallback.acall(call[1]);
		}
	}
}
