/*
Some collection (i.e. tables, arrays, etc) helper functions
*/

class WeighedRandomPicker
{
	_choices      = null;
	_totalWeight  = null;
	_isNormalized = null;
	
	function constructor()
	{
		_choices      = [];
		_totalWeight  = 0;
		_isNormalized = false;
	}
	
	function addChoice(p_weight, p_value)
	{
		_isNormalized = false;
		_choices.push({weight = p_weight, value = p_value, ceiling = 0});
		_totalWeight += p_weight;
	}
	
	function getRandom()
	{
		if (_choices.len() == 0)
		{
			::tt_panic("No elements in WeighedRandomPicker. Cannot get random value.");
			return null;
		}
		
		if (_isNormalized == false)
		{
			_normalize();
		}
		
		local choice = frnd();
		
		local floor = 0.0;
		foreach (elem in _choices)
		{
			if (floor <= choice && choice <= elem.ceiling)
			{
				// Found it
				return elem.value;
			}
			floor = elem.ceiling;
		}
		
		::tt_panic("getRandom failed with choice '" + choice + "'");
		
		// Return last element
		return _choices[_choices.len()-1].value;
	}
	
	// Private method
	function _normalize()
	{
		if (_totalWeight <= 0.0)
		{
			::tt_panic("_totalWeight is 0. Cannot normalize WeighedRandomPicker");
			return;
		}
		
		local ceiling = 0.0;
		foreach (elem in _choices)
		{
			local normalizedWeight = elem.weight / _totalWeight.tofloat();
			ceiling += normalizedWeight;
			elem.ceiling = ceiling;
		}
		
		// Make sure that last element has ceiling 1.0 (float rounding errors can cause this to be lower)
		_choices[_choices.len()-1].ceiling = 1.0;
		
		_isNormalized = true;
	}
}

// returns a random item from an array
// e.g. randomArrayItem([1,2,3,4,5]);
function randomArrayItem(p_array)
{
	if (typeof(p_array) != "array") ::tt_panic("argument for randomChoice must be an array!");
	::assert(p_array.len() > 0, "Array is empty");
	
	return(p_array[::rnd_minmax(0, p_array.len() - 1)]);
}

// returns a random item from the function arguments
// e.g. randomPick("foo", 42, 12.3);
function randomPick(...)
{
	return vargv[::rnd_minmax(0, vargv.len() - 1)];
}

function isInArray(p_item, p_array)
{
	return p_array.find(p_item) != null;
}

function entityInArray(p_entity, p_array)
{
	foreach(e in p_array)
	{
		if (e.equals(p_entity)) return true;
	}
	
	return false;
}

function tableKeysAsArray(p_table)
{
	local ret = []; 
	
	foreach (key, val in p_table) 
	{
		ret.append(key)
	};
	
	return ret;
}

function tableValuesAsArray(p_table)
{
	local ret = []; 
	
	foreach (key, val in p_table) 
	{
		ret.append(val)
	};
	
	return ret;
}

// merges two tables, for duplicate fields the 'a' table fields are used
function mergeTables(p_a, p_b)
{
	local ret = clone p_b;
	
	foreach(key, val in p_a)
	{
		ret[key] <- val;
	}
	
	return ret;
}

// Since null is not an array, it'll always fail when one or both of the arguments are null
function arraysEqual(p_a, p_b)
{
	if (p_a == null || p_b == null || p_a.len() != p_b.len())
	{
		return false;
	}
	for (local i = 0; i < p_a.len(); ++i)
	{
		if (p_a[i] != p_b[i])
		{
			return false;
		}
	}
	return true;
}

function getEnumKeysAsArray(p_enumName)
{
	local ret = []; 
	
	foreach (key, val in getconsttable()[p_enumName]) 
	{
		ret.append(key);
	};
	
	return ret;
}