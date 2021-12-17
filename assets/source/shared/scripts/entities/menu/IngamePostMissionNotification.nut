include_entity("menu/IngameMenu");

class IngamePostMissionNotification extends IngameMenu
</
	placeable      = Placeable_Hidden
	collisionRect  = [ 0.0, 1.0, 2.0, 2.0 ]  // center X, center Y, width, height
/>
{
	static c_colorGradingTexture  = "ingamemenu";     // Required
	static c_musicTrack           = "ingamemenu";     // Optional
	
	_notification = null;
	
	_isPlayerDead = false;
	_allowClose = false;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		base.onInit();
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onWindowTextLabelTyped(p_window, p_id)
	{
		_allowClose = true;
		createFaceButtons(["exit"]);
	}
	
	function onButtonAcceptPressed()
	{
		if (_allowClose)
		{
			close();
		}
	}
	
	function onButtonCancelPressed()
	{
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Inherited Methods
	
	function create()
	{
		base.create();
		
		local id = _notification.toupper();
		createTitle("NOTIFICATION_" + id + "_HEADER");
		
		local window = createWindow("notification", ::Vector2(0.0, -0.06),
		{
			_titleID = "NOTIFICATION_INGAME_TITLE",
			_width   = 0.95,
			_height  = 0.55,
			_zOffset = 0.2,
			_parent = this
		});
		_windows["notification"].createTypedTextArea("notification", "NOTIFICATION_" + id, ::Vector2(0.0, 0.0), ::TextColors.Light, 1.0, 1.0);
	}
}
