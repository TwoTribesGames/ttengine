include_entity("menu/IngameMenu");

class IngameEasyMode extends IngameMenu
</
	placeable      = Placeable_Hidden
	collisionRect  = [ 0.0, 1.0, 2.0, 2.0 ]  // center X, center Y, width, height
/>
{
	static c_colorGradingTexture  = "ingamemenu";     // Required
	static c_musicTrack           = "ingamemenu";     // Optional
	static c_showCrackedScreenAtDeath = false;
	
	// Mimick player being dead
	_isPlayerDead = true;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Script Item Handling
	
	function onScriptItemProcessed(p_id)
	{
		if (p_id == "start")
		{
			createPresentation("bootscreen", "ingameeasymode");
			createPresentation("loadingbar", "ingameeasymode");
			createHeader("loading_header", "MENU_EASYMODE_LOADING_HEADER", ::Vector2(0, 0.1), ::TextColors.Light);
			startScriptItemTimer("showNext", 2.0);
		}
		else if (p_id == "showNext")
		{
			removePresentation("bootscreen");
			removePresentation("loadingbar");
			removeLabel("loading_header");
			removeLabel("loading_description");
			
			// Create labels
			createTitle("MENU_EASYMODE_TITLE");
			
			createTextArea("description", "MENU_EASYMODE_DESCRIPTION", ::Vector2(0, 0.15), ::TextColors.Light,
				1.0, 0.25, HorizontalAlignment_Center, VerticalAlignment_Center);
			
			createPanelButtons(::Vector2(0.0, -0.08), ::MainMenu.c_buttonsSpacing, 0,
			[
				["btn_accept", "GUIButton", ["MENU_EASYMODE_ACCEPT"], onAcceptSelected],
				["btn_cancel", "GUIButton", ["MENU_EASYMODE_CANCEL"], onCancelSelected],
				["btn_never", "GUIButton", ["MENU_EASYMODE_NEVER"], onNeverSelected]
			]);
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onAcceptSelected()
	{
		::Stats.submitTelemetryEvent("ingame_easymode_accept");
		::ProgressMgr.setEasyMode(true);
		::ProgressMgr.setIsEasyModeOffered(true);
		close();
	}
	
	function onCancelSelected()
	{
		::Stats.submitTelemetryEvent("ingame_easymode_cancel");
		::ProgressMgr.setEasyMode(false);
		::ProgressMgr.setIsEasyModeOffered(true);
		close();
	}
	
	function onNeverSelected()
	{
		::Stats.submitTelemetryEvent("ingame_easymode_never");
		::ProgressMgr.setEasyMode(false);
		::ProgressMgr.setIsEasyModeOffered(null); // null indicates not to bug me again
		close();
	}
	
	function onDelayedLabelShown(p_id)
	{
	}
	
	function onButtonMenuPressed()
	{
	}
	
	function onButtonCancelPressed()
	{
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Inherited Methods
	
	function create()
	{
		base.create();
		
		customCallback("onScriptItemProcessed", "start");
	}
}
