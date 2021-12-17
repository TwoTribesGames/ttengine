function addNotification(p_id)
{
	local notifications = ::getRegistry().getPersistent("notifications");
	if (notifications == null)
	{
		notifications = {};
	}
	
	if ((p_id in notifications) == false)
	{
		notifications[p_id] <- false;
	}
	// Show changes popup
	::getRegistry().setPersistent("notifications", notifications);
}

function getNotification()
{
	local notifications = ::getRegistry().getPersistent("notifications");
	if (notifications == null)
	{
		return null;
	}
	
	foreach (id, shown in notifications)
	{
		if (shown == false)
		{
			notifications[id] = true;
			::getRegistry().setPersistent("notifications", notifications);
			return id;
		}
	}
	return null;
}
