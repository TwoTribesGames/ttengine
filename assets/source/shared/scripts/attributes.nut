function _processElement(p_className, p_memberName, p_attributeName, p_attributeValue)
{
	local valueArray = [];
	local type = typeof(p_attributeValue)
	switch (type)
	{
	case "integer":
	case "string" :
	case "float"  :
	case "bool"   :
		valueArray.push(p_attributeValue.tostring());
		break;

	case "array"  :
		if (p_attributeValue.len() > 0)
		{
			// Add to string array and make sure all elements must be of same type
			local baseElement = p_attributeValue[0];
			type = typeof(baseElement) + "_" + type;
			foreach(element in p_attributeValue)
			{
				if (typeof(element) != typeof(baseElement))
				{
					::tt_panic("Array of attribute '" + p_className + "' contains mismatching types '"
							 + typeof(baseElement) + "' and '" + typeof(element) + "'");
					return {};
				}
				else
				{
					valueArray.push(element.tostring());
				}
			}
		}
		else
		{
			if (p_memberName == null)
			{
				::tt_panic("Array '" + p_attributeName + "' in '" + p_className + "' is empty");
			}
			else
			{
				::tt_panic("Array '" + p_attributeName + "' in '" + p_className + "." + p_memberName + "'  is empty");
			}
			return {};
		}
		break;

	case "null"   :
		break;

	default:
		::tt_panic("Unsupported attribute type: '" +  typeof(p_attributeValue) + "'");
		return {};
	}

	local result = {};
	result.memberName <- p_memberName;
	result.attributeName <- p_attributeName;
	result.attributeValue <- valueArray;
	result.attributeType <- type;

	return result;
}


function _doRegister(p_class, p_className, p_memberName)
{
	local validAttributeNames = ["min", "max", "type", "choice", "description", "filter", "group", "conditional", "referenceColor"]; // Note that 'order' isn't in this list
	local allAttributes = [];
	local defaultValue = {};

	if (p_memberName == null)
	{
		// Register class name
		allAttributes.push(_processElement(p_className, "", "_className", p_className));
	}
	else
	{
		// Register default value
		allAttributes.push(_processElement(p_className, p_memberName, "_default", p_class[p_memberName]));
	}

	local attr;
	local foundTypeDefinition = false;
	if((attr = p_class.getattributes(p_memberName)) != null)
	{
		foreach(name, value in attr)
		{
			if (p_memberName == null)
			{
				allAttributes.push(_processElement(p_className, "", name, value));
			}
			else if (validAttributeNames.find(name) != null) // only parse valid attribute names
			{
				allAttributes.push(_processElement(p_className, p_memberName, name, value));
				if (name == "type")
				{
					foundTypeDefinition = true;
				}
			}
		}
	}

	if (foundTypeDefinition || p_memberName == null)
	{
		// Do actual registration
		foreach (entry in allAttributes)
		{
			// table entries should be size 4 otherwise, something went wrong
			if (entry.len() == 4)
			{
				registerAttribute(entry.memberName, entry.attributeName, entry.attributeValue, entry.attributeType);
			}
		}
	}
	else
	{
		// Commented this for now, because the editor should not monopolize member attributes
		// (it should only use the one it needs and leave the rest alone)
		//::tt_panic(p_className + "." + p_memberName + ": cannot find required type definition");
		return;
	}
}


function __registerAttributes(p_className)
{
	local root = getroottable();
	if (p_className in root)
	{
		local entry = root[p_className];

		// register all class attributes including class name itself
		_doRegister(entry, p_className, null);

		// register all class members
		local allMembers = [];

		// first process all ordered items
		foreach(member, val in entry)
		{
			// skip functions
			if (typeof(val) != "function")
			{
				// only register when attributes are present
				local attributes = entry.getattributes(member);
				if (attributes != null)
				{
					// check for order
					if ("order" in attributes)
					{
						allMembers.push([attributes["order"], member]);
					}
				}
			}
		}

		// Sort based on order (which is in [0])
		allMembers.sort(@(a, b) a[0] <=> b[0]);

		// next add all unordered items
		foreach(member, val in entry)
		{
			// skip functions
			if (typeof(val) != "function")
			{
				// only register when attributes are present
				local attributes = entry.getattributes(member);
				if (attributes != null)
				{
					// skip ordered attributes
					if (("order" in attributes) == false)
					{
						// 0 is just a dummy value
						allMembers.push([0, member]);
					}
				}
			}
		}

		foreach(v in allMembers)
		{
			_doRegister(entry, p_className, v[1]);
		}

		// Mission 'hack' always add this property to every entity
		registerAttribute("MISSION_ID", "type", ["string"], "string");
		registerAttribute("MISSION_ID", "_default", ["*"], "string");
		registerAttribute("MISSION_ID", "group", ["Mission"], "string");
		registerAttribute("MISSION_ID", "description", ["Specify mission id. Use , or ; for multiple entries. Use ? and/or * for wildcards. This property only 'lives' in the editor and is not part of the entity!"], "string");
	}
	else
	{
		::tt_panic("Class '" + p_className + "' not in roottable");
	}
}