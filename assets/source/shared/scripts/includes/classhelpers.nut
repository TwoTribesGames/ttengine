function getClassName(p_class)
{
	local name = "*Unknown*";
	
	foreach(member, val in getroottable())
	{
		if (val == p_class)
		{
			name = member;
			break;
		}
	}
	
	return name;
}