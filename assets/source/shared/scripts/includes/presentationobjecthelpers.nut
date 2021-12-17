/*
*/

function PresentationObject::startDefaultAnimation(p_priority = 0)
{
	if (getPriority() <= p_priority)
	{
		stop();
	}
}