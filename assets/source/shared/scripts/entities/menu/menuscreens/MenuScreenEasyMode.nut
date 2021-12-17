include_entity("menu/MenuScreen");

class MenuScreenEasyMode extends MenuScreen
</
	placeable      = Placeable_Hidden
	collisionRect  = [ 0.0, 1.0, 2.0, 2.0 ]  // center X, center Y, width, height
/>
{
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onAcceptSelected()
	{
		::Stats.submitTelemetryEvent("easymode_enabled");
		::ProgressMgr.setEasyMode(true);
		close();
	}
	
	function onCancelSelected()
	{
		::Stats.submitTelemetryEvent("easymode_disabled");
		::ProgressMgr.setEasyMode(false);
		::ProgressMgr.setIsEasyModeOffered(false); // reset the offer here again
		close();
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Inherited Methods
	
	function create()
	{
		base.create();
		
		// Create labels
		createTitle("MENU_EASYMODE_TITLE");
		
		createTextArea("description", "MENU_EASYMODE_DESCRIPTION", ::Vector2(0, 0.15), ::TextColors.Light,
			1.0, 0.25, HorizontalAlignment_Center, VerticalAlignment_Center);
		
		createPanelButtons(::Vector2(0.0, -0.02), ::MainMenu.c_buttonsSpacing, 0,
		[
			["btn_accept", "GUIButton", ["MENU_EASYMODE_ACCEPT"], onAcceptSelected],
			["btn_cancel", "GUIButton", ["MENU_EASYMODE_CANCEL"], onCancelSelected]
		]);
	}
}
