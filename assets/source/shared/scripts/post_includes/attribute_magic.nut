function handleRootTableMemberAttributes()
{
	foreach (member, val in getroottable())
	{
		if (typeof val == "class")
		{
			local attr = val.getattributes(null);
			
			local shouldAddStickToEntity = ("stickToEntity" in attr && attr["stickToEntity"] == true);
			makeEntityClassStickToEntity(val, shouldAddStickToEntity);
			
			handleClassMemberAttributes(val);
		}
	}
}

function handleClassMemberAttributes(p_class)
{
	local attributeModifiers = {};
	attributeModifiers.autoGetSet <- function(p_member, p_val)
		{
			if (p_val)
			{
				local startChar = (p_member[0].tochar() == "_") ? 1 : 0;
				
				local baseMembername = p_member[startChar].tochar().toupper() + p_member.slice(startChar + 1);
				local setterName = "set" + baseMembername;
				local getterName = "get" + baseMembername;
				
				if ((setterName in p_class) == false)
				{
					p_class[setterName] <- function(p_val) { this[p_member] = p_val; };
				}
				
				if ((getterName in p_class) == false)
				{
					p_class[getterName] <- function() { return this[p_member]; };
				}
			}
		};
	
	foreach (member, val in p_class)
	{
		local attributes = p_class.getattributes(member);
		if (attributes == null) continue;
		
		foreach (attribute, val in attributes)
		{
			if (attribute in attributeModifiers)
			{
				attributeModifiers[attribute](member, val);
				
				/*
				foreach (member, val in p_class)
				{
					if (member != null && member.slice(0, 3) == "set")
					{
						::echo(member, getClassName(p_class) + "." + member.tostring());
					}
				}
				*/
			}
		}
	}
}


handleRootTableMemberAttributes();
