include_entity("menu/MenuScreen");

class MenuScreenResolutionSettings extends MenuScreen
</
	placeable      = Placeable_Hidden
	collisionRect  = [ 0.0, 0.0, 2.0, 1.0 ]  // center X, center Y, width, height
/>
{
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onResolutionChanged(p_scale)
	{
		local scale = p_scale / 100.0;
		::ResolutionChanger.setScale(scale);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Inherited Methods
	
	function create()
	{
		base.create();
		
		createTitle("MENU_RESOLUTION_SETTINGS");
		createFaceButtons(["back"]);
		
		createPanelButtons(::Vector2(0.0, 0.04), ::MainMenu.c_buttonsSpacing, 0,
		[
			["lst_items", "GUIListButton", getResolutionItems(), onResolutionChanged]
		]);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods
	
	function getResolutionItems()
	{
		// Populate the resolutions
		local supportedScales = [0.33, 0.4, 0.5, 0.67, 0.8, 0.9, 1.0];
		
		// First find the closest scale
		local currentScale = ::ResolutionChanger.getScale();
		local foundIdx = -1;
		for (local i = 0; i < supportedScales.len(); ++i)
		{
			if (::fabs(supportedScales[i] - currentScale) < 0.05)
			{
				foundIdx = i;
				break;
			}
		}
		if (foundIdx == -1)
		{
			::tt_panic("Cannot find matching scale for current resolution");
			// Default to 100%
			foundIdx = 0;
			::ResolutionChanger.setScale(1.0);
		}
		
		local resolutionItems = [];
		local i = foundIdx;
		do
		{
			local scale = supportedScales[i];
			local intScale = (scale * 100).tointeger();
			if (::ResolutionChanger.supportsScale(scale))
			{
				resolutionItems.push(["MENU_RESOLUTION_" + intScale, intScale]);
			}
			++i;
			if (i >= supportedScales.len()) i = 0;
		} while (i != foundIdx);
		return resolutionItems;
	}
}
