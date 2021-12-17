include_entity("menu/MenuScreen");

class MenuScreenSelectDifficulty extends MenuScreen
</
	placeable      = Placeable_Hidden
	collisionRect  = [ 0.0, 0.0, 2.0, 1.0 ]  // center X, center Y, width, height
/>
{
	static c_title = "MENU_SELECT_DIFFICULTY";

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks

	function onButtonFaceUpPressed()
	{
	}

	function onFadeEnded(p_fade, p_animation)
	{
		closeAll();
		::ProgressMgr.startCampaign();
	}

	function onHandleSelection()
	{
		unfocus(false); // disable input during fade
		::createFade(this, "transparent_to_opaque");
	}

	function onEasySelected()
	{
		::ProgressMgr.setEasyMode(true);
		onHandleSelection();
	}

	function onNormalSelected()
	{
		::ProgressMgr.setEasyMode(false);
		onHandleSelection();
	}

	function onCursorFocus(p_element)
	{
		_labels["description"].label.setTextLocalized(p_element == _elements["btn_easy"] ?
			"MENU_DIFFICULTY_EASY_DESCRIPTION" : "MENU_DIFFICULTY_NORMAL_DESCRIPTION");
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods

	function displayButtons()
	{
		// This method is here for overloading
		createPanelButtons(::Vector2(0.0, 0.10), ::MainMenu.c_buttonsSpacing, 0,
		[
			["btn_easy", "GUIButton", ["MENU_DIFFICULTY_EASY"], onEasySelected],
			["btn_normal", "GUIButton", ["MENU_DIFFICULTY_NORMAL"], onNormalSelected]
		]);

		createFaceButtons(["back"]);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Inherited Methods

	function create()
	{
		createTextArea("description", null, ::Vector2(0, -0.063), ::TextColors.Dark,
		               1.0, 0.3, HorizontalAlignment_Center,
		               VerticalAlignment_Top);

		displayButtons();

		// Create this after textarea and panel buttons because we use the onCursorFocus callback in this menu
		base.create();

		createTitle(c_title);
	}
}
