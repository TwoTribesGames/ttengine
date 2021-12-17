/*
*/

class Blur
{
	maxBackgroundBlurLayers = 5;
	maxForegroundBlurLayers = 3;

	//_lastBackgroundRanges = [];
	//_lastForegroundRanges = [];
	
	function addBackgroundRangeMembers(p_class, p_memberName, p_order, p_attributes = null)
	{
		local blurVal = -10;
		for (local i = 0; i < maxBackgroundBlurLayers; i++, blurVal -= 10)
		{
			addRangeMember(p_class, p_memberName + i, blurVal, p_order + (i * 0.1), -500.0, -1.0, p_attributes);
		}
	}
	
	function addForegroundRangeMembers(p_class, p_memberName, p_order, p_attributes = null)
	{
		local blurVal = 2;
		for (local i = 0; i < maxForegroundBlurLayers; i++, blurVal *= 2)
		{
			addRangeMember(p_class, p_memberName + i, blurVal, p_order + (i * 0.1), 0.0, 50.0, p_attributes);
		}
	}
	
	function addRangeMember(p_class, p_memberName, p_val, p_order, p_min, p_max, p_attributes)
	{
		p_class[p_memberName] <- p_val;
		
		local attributes = {type = "float", min = p_min, max = p_max, order = p_order};
		
		if (p_attributes != null) attributes = ::mergeTables(attributes, p_attributes);
		
		p_class.setattributes(p_memberName, attributes);
	}

	function setBackgroundFromMembers(p_entity, p_baseMemberName)
	{
		local ranges = getRangeArrayFromMembers(p_entity, p_baseMemberName);
		::setBackgroundBlurLayers(ranges);
	}

	function setForegroundFromMembers(p_entity, p_baseMemberName)
	{
		local ranges = getRangeArrayFromMembers(p_entity, p_baseMemberName);
		setForegroundBlurLayers(ranges);
	}

	/*function restoreLastBackgroundRanges()
	{
		::setBackgroundBlurLayers(_lastBackgroundRanges);
	}
	
	function restoreLastForegroundRanges()
	{
		setForegroundBlurLayers(_lastForegroundRanges);
	}*/
	
	function disableBackground()
	{
		::setBackgroundBlurLayers([]);
	}
	
	function disableForeground()
	{
		setForegroundBlurLayers([]);
	}
	
	function getRangeArrayFromMembers(p_entity, p_baseMemberName)
	{
		local array = [];
		
		for (local i = 0; (p_baseMemberName + i) in p_entity; i++)
		{
			array.append(p_entity[p_baseMemberName + i]);
		}
		return array;
	}
}