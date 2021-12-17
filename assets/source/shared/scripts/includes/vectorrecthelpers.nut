/*
VectorRect helper functions
*/

function VectorRect::_tostring() 
{	
	return "rect [(" + getPosition().x + ", " + getPosition().y +") ; " + getWidth() + ", " + getHeight() + "]"; 
}

function VectorRect::scale(p_other)
{
	switch (typeof(p_other))
	{
		case "Vector2":
			return ::VectorRect(getPosition(), getWidth() * p_other.x, getHeight() * p_other.y);
		case "float":
		case "integer":
		{
			return ::VectorRect(getPosition(), getWidth() * p_other, getHeight() * p_other);
		}
		default:
		{
			::tt_panic("Can't scale VectorRect with " + typeof(p_other));
		}
	}
}