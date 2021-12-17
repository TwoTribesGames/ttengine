tt_include("includes/logs");

enum StateOperation
{
	Set,
	Push,
	Pop,
	Replace
};

enum StatePriority
{
	Lowest  = -2,
	Low     = -1, // E.g., Scour
	Normal  = 0,  // Default
	Medium  = 1,
	High    = 2,  // E.g., Dazed
	Highest = 3   // E.g., Die
};

class StateStack
{
	_stack = null;
	_mutations = null;
	
	constructor()
	{
		_stack = [];
		_mutations = [];
	}
	
	function exists(p_name)
	{
		return _stack.len() > 0 ? p_name in _stack : false;
	}
	
	function empty()
	{
		return _stack.len() == 0;
	}
	
	function top()
	{
		return _stack.len() > 0 ? _stack.top() : null;
	}
	
	function push(p_name, p_priority)
	{
		_mutations.push({ priority = p_priority, operation = StateOperation.Push, payload = p_name});
	}
	
	function pop(p_priority)
	{
		_mutations.push({ priority = p_priority, operation = StateOperation.Pop, payload = null});
	}
	
	function replace(p_name, p_priority)
	{
		_mutations.push({ priority = p_priority, operation = StateOperation.Replace, payload = p_name});
	}
	
	function set(p_name, p_priority)
	{
		_mutations.push({ priority = p_priority, operation = StateOperation.Set, payload = p_name});
	}
	
	function process(p_entity)
	{
		if (_mutations.len() == 0)
		{
			return false;
		}
		
		// Sort mutations based. Higher prios first
		_mutations.sort(@(a, b) b.priority <=> a.priority);
		
		// Do extra checking in test builds
		local firstMutation = _mutations[0];
		
		/*
		if (::isTestBuild())
		{
			// check if mutations with identical priority have identical operation AND payload
			foreach (mutation in _mutations)
			{
				// Don't check lower priorities (will be discarded anyway)
				if (mutation.priority == firstMutation.priority)
				{
					if (mutation.operation != firstMutation.operation)
					{
						::tt_warning(p_entity + " operation clash: mutation '" + _mutationToString(mutation) + "' != '" + 
						           _mutationToString(firstMutation) + "'");
					}
					
					if (mutation.payload != firstMutation.payload)
					{
						::tt_warning(p_entity + " payload clash: mutation '" + _mutationToString(mutation) + "' != '" + 
						           _mutationToString(firstMutation) + "'");
					}
				}
				else
				{
					//echo("Warning: discarded stack mutation: '" + _mutationToString(mutation) + "'");
				}
			}
		}
		*/
		
		_mutations.clear();
		return _applyMutationToStack(p_entity, firstMutation);
	}
	
	function _applyMutationToStack(p_entity, p_mutation)
	{
		switch (p_mutation.operation)
		{
		case StateOperation.Set:
			_stack.clear();
			_stack.push(p_mutation.payload);
			return true;
			
		case StateOperation.Push:
			// Only push when current state isn't on top
			if (_stack.len() == 0 || _stack.top() != p_mutation.payload)
			{
				_stack.push(p_mutation.payload);
				return true;
			}
			return false;
			
		case StateOperation.Pop:
			if (_stack.len() > 0)
			{
				_stack.pop();
				return true;
			}
			::tt_panic(p_entity + " pop() called on empty stack");
			return false;
			
		case StateOperation.Replace:
			if (_stack.len() > 0) _stack.pop();
			_stack.push(p_mutation.payload);
			return true;
		}
		
		::tt_panic(p_entity + " unhandled StateOperation '" + p_mutation.operation + "'");
		return false;
	}
	
	function _mutationToString(p_mutation)
	{
		local result = "{ priority: " + p_mutation.priority + "; ";
		switch (p_mutation.operation)
		{
			case StateOperation.Set :    result += "operation: \"set\"; ";     break;
			case StateOperation.Push:    result += "operation: \"push\"; ";    break;
			case StateOperation.Pop :    result += "operation: \"pop\"; ";     break;
			case StateOperation.Replace: result += "operation: \"replace\"; "; break;
			default:                     result += "operation: <unknown>; ";   break;
		}
		result += "payload: " + p_mutation.payload + " }";
		
		return result;
	}
	
	function _tostring()
	{
		return "StateStack: " + ::niceStringFromObject(_stack);
	}
	
	function _typeof()
	{
		return "StateStack";
	}
}
