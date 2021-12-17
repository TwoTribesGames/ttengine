// Returns initial state when refcount = 0 and !initial state when refcount != 0
// Useful for enabled flags that can be enabled/disabled from multiple sources (e.g., controlsEnabled flag)
class RefCountedBool
{
	_initialState = null;
	_refCount     = 0;
	
	constructor(p_initialState)
	{
		_initialState = p_initialState;
		reset();
	}
	
	function reset()
	{
		_refCount = 0;
	}
	
	// Returns true when set for first time or when it returns to initial state
	// false otherwise
	function set(p_state)
	{
		if (p_state != _initialState)
		{
			++_refCount;
			return _refCount == 1;
		}
		
		if (_refCount > 0)
		{
			--_refCount;
			return _refCount == 0;
		}
		
		return false;
	}
	
	function get()
	{
		return _refCount == 0 ? _initialState : _initialState == false;
	}
}
