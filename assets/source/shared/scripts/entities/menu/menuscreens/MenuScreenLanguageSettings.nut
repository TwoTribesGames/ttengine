include_entity("menu/MenuScreen");

class MenuScreenLanguageSettings extends MenuScreen
</
	placeable      = Placeable_Hidden
	collisionRect  = [ 0.0, 0.0, 2.0, 1.0 ]  // center X, center Y, width, height
/>
{
	static c_entriesPerPage = 8;
	_languages = null;
	_language = null;
	_page = 0;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onLanguageSelected(p_lang)
	{
		setLanguage(p_lang);
	}

	function onPageSelected(p_page)
	{
		removePresentationNoFade("panel");
		if (hasElement("lst_pages"))
		{
			removeElement("lst_pages");
		}
		for (local i = 0; i < c_entriesPerPage; ++i)
		{
			local id = "btn_" + i;
			if (hasElement(id))
			{
				removeElement(id);
			}
		}
		
		local buttons = [];
		local pages = getPageItems(p_page);
		buttons.push(["lst_pages", "GUIListButton", pages, onPageSelected]);
		buttons.push(null);
		local startIdx = p_page * c_entriesPerPage;
		local endIdx = startIdx + c_entriesPerPage;
		if (endIdx > _languages.len()) endIdx = _languages.len();
		
		local btnIdx = 0;
		local foundIdx = null;
		for (local i = startIdx; i < endIdx; ++i, ++btnIdx)
		{
			local id = "btn_" + btnIdx;
			local lang = _languages[i];
			if (lang == _language) foundIdx = btnIdx;
			buttons.push([id,  "GUIConfirmationWithFadeButton", ["MENU_LANGUAGE_" + lang.toupper(), lang], onLanguageSelected]);
		}
		createPanelButtons(::Vector2(0.0, 0.22), ::MainMenu.c_buttonsSpacing, 0, buttons);
		
		if (foundIdx != null)
		{
			_elements["btn_" + foundIdx].setEnabled(false);
		}
	}
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Inherited Methods
	
	function create()
	{
		base.create();
		
		createTitle("MENU_LANGUAGE_SETTINGS");
		createFaceButtons(["back"]);
		
		_language = ::ProgressMgr.getLanguage();
		if (_language == null)
		{
			_language = "default";
		}
		
		_languages = ["default", "en", "nl", "pt", "de", "fr", "es", "it", "jp", "ru", "hu", "tr", "zh", "zt", "ko"];
		
		local page = 0;
		for (local i = 0; i < _languages.len(); ++i)
		{
			if (_language == _languages[i]) page = i / c_entriesPerPage;
		}
		onPageSelected(page);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods
	
	function setLanguage(p_language)
	{
		::Stats.submitTelemetryEvent("set_language", p_language);
		::ProgressMgr.setLanguage(p_language);
		::ProgressMgr.startMainMenu();
	}
	
	function getPageItems(p_page)
	{
		local items = [];
		
		local pages = ::ceil(_languages.len() / c_entriesPerPage) + 1;
		
		for (local i = 0; i < pages; ++i)
		{
			items.push([["MENU_PAGE", (i+1) + "", pages + ""], i]);
		}
		
		return ::GUIListButton.getScrolledItems(items, p_page);
	}
}
