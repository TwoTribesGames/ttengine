/*
Helper functions to fix entities in onInvalidProperties callbacks

Example:
	
	function onInvalidProperties(p_properties)
	{
		local valueFixes = {}
		valueFixes.width <- { condition = @(val) val.tointeger() < 2, result = 2 };
		
		fixInvalidValues(p_properties, valueFixes); 
		
		return p_properties;
	}
*/

// The fix functions take a table with a "condition" and "result" slot
// condition takes the value/property name as argument and when it
// evaluates to true the value/property is replaced by result.
// If the condition slot is omitted, it's evaluated as true
function fixInvalidValues(p_properties, p_changetable)
{
	foreach(prop, val in p_properties)
	{
		if (prop in p_changetable)
		{
			if (("condition" in p_changetable) == false ||
			    p_changetable[prop].condition(val))
			{
				if (typeof(p_changetable[prop].result) == "function")
				{
					p_properties[prop] = p_changetable[prop].result(val).tostring();
				}
				else
				{
					p_properties[prop] = p_changetable[prop].result.tostring();
				}
			}
		}
	}
	
	return p_properties;
}

function fixInvalidProperties(p_properties, p_changetable)
{
	foreach(prop, val in p_properties)
	{
		if (prop in p_changetable)
		{
			if (("condition" in p_changetable) == false ||
		         p_changetable[prop].condition(prop))
			{
				// add the new property
				p_properties[p_changetable[prop].result.tostring()] <- val;
				// delete the old one
				delete p_properties[prop];
			}
		}
	}
	
	return p_properties;
}

function removeNonexistentProperties(p_instance, p_properties)
{
	local propstToDelete = [];
	
	foreach (key, val in p_properties)
	{
		if ((key in p_instance) == false || p_instance.getclass().getattributes(key) == null)
		{
			propstToDelete.append(key);
		}
	}
	
	foreach (prop in propstToDelete)
	{
		delete p_properties[prop];
	}
	
	return p_properties;
}
