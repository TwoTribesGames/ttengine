class TextHelpers
{
	_horizontalTextLUT =
	{
		left   = HorizontalAlignment_Left,
		center = HorizontalAlignment_Center,
		right  = HorizontalAlignment_Right,
	}

	_verticalTextLUT = 
	{
		top    = VerticalAlignment_Top,
		center = VerticalAlignment_Center,
		bottom = VerticalAlignment_Bottom,
	}

	function getHorizontalAlignmentFromString(p_string)
	{
		if (p_string in _horizontalTextLUT)
		{
			return _horizontalTextLUT[p_string];
		}
		return _horizontalTextLUT["left"];
	}
	
	function getVerticalAlignmentFromString(p_string)
	{
		if (p_string in _verticalTextLUT)
		{
			return _verticalTextLUT[p_string];
		}
		return _verticalTextLUT["top"];
	}
	
	function getHorizontalAlignmentStrings()
	{
		return ::tableKeysAsArray(_horizontalTextLUT);
	}
	
	function getVerticalAlignmentStrings()
	{
		return ::tableKeysAsArray(_verticalTextLUT);
	}
}