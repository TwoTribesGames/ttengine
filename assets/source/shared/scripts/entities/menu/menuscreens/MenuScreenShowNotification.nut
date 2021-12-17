include_entity("menu/MenuScreen");

class MenuScreenShowNotification extends MenuScreen
</
	placeable      = Placeable_Hidden
	collisionRect  = [ 0.0, 0.0, 2.0, 1.0 ]  // center X, center Y, width, height
/>
{
	_notification = null;
	_allowClose   = false;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onWindowTextLabelTyped(p_window, p_id)
	{
		_allowClose = true;
		createFaceButtons(["exit"]);
	}
	
	function onButtonAcceptPressed()
	{
		if (_allowClose == false)
		{
			return;
		}
		
		if (instantlyShowAllWindowElements())
		{
			close();
		}
	}
	
	function onButtonCancelPressed()
	{
	}
	
	function onTimer(p_name)
	{
		base.onTimer(p_name);
		
		if (p_name == "allowClose")
		{
			_allowClose = true;
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Inherited Methods
	
	function create()
	{
		base.create();
		
		local id = _notification.toupper();
		createTitle("NOTIFICATION_" + id + "_HEADER");
		
		startTimer("allowClose", 2.0);
		
		local window = createWindow("notification", ::Vector2(0.0, -0.06),
		{
			_titleID = "NOTIFICATION_TITLE",
			_width   = 0.95,
			_height  = 0.55,
			_zOffset = 0.2,
			_parent = this
		});
		_windows["notification"].createTypedTextArea("notification", "NOTIFICATION_" + id, ::Vector2(0.0, 0.0), ::TextColors.Light, 1.0, 1.0);
	}
}
