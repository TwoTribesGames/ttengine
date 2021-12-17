include_entity("menu/menuscreens/MenuScreenSelectDifficulty");

class MenuScreenChangeDifficulty extends MenuScreenSelectDifficulty
</
	placeable      = Placeable_Hidden
	collisionRect  = [ 0.0, 0.0, 2.0, 1.0 ]  // center X, center Y, width, height
/>
{
	static c_title = "MENU_DIFFICULTY";
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onHandleSelection()
	{
		close();
	}
	
	function onEasyAccepted(p_arg1 = null, p_arg2 = null)
	{
		onEasySelected();
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Inherited Methods
	
	function displayButtons()
	{
		local easyMode = ::ProgressMgr.isEasyModeEnabled();
		if (easyMode)
		{
			createPanelButtons(::Vector2(0.0, 0.10), ::MainMenu.c_buttonsSpacing, 0,
			[
				["btn_easy", "GUIButton", ["MENU_DIFFICULTY_EASY_SELECTED"], onEasySelected],
				["btn_normal", "GUIButton", ["MENU_DIFFICULTY_NORMAL"], onNormalSelected]
			]);
		}
		else
		{
			createPanelButtons(::Vector2(0.0, 0.10), ::MainMenu.c_buttonsSpacing, 1,
			[
				["btn_easy", "GUIConfirmationButton", ["MENU_DIFFICULTY_EASY", "_CHANGE", ::Vector2(0.0, 0.2)], onEasyAccepted],
				["btn_normal", "GUIButton", ["MENU_DIFFICULTY_NORMAL_SELECTED"], onNormalSelected]
			]);
		}
		createFaceButtons(["back"]);
	}
}
