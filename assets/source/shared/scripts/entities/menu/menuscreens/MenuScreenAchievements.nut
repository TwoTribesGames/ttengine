include_entity("menu/MenuScreen");

class MenuScreenAchievements extends MenuScreen
</
	placeable      = Placeable_Hidden
	collisionRect  = [ 0.0, 0.0, 2.0, 1.0 ]  // center X, center Y, width, height
/>
{
	_sortedIDs = null;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onPageSelected(p_page)
	{
		for (local i = 0; i < 3; ++i)
		{
			local pres = _presentations["icon_" + i];
			local goal = _labels["goal_" + i];
			local description = _labels["description_" + i];
			local extra = _labels["extra_" + i];
			extra.label.setTextUTF8("");
			pres.stop();
			
			// find name for id
			local idx = (p_page * 3) + i;
			if (idx >= ::achievementTable.len())
			{
				goal.label.setTextUTF8("");
				description.label.setTextUTF8("");
				continue;
			}
			
			local entry = _sortedIDs[idx];
			local id = entry[0];
			local name = entry[2];
			local hidden = entry[3];
			local locid = entry[4];
			local unlockdate = entry[5];
			
			local tag = "locked";
			
			if (unlockdate != null)
			{
				tag = "unlocked";
				extra.label.setTextLocalizedAndFormatted("ACHIEVEMENT_UNLOCKDATE", [::formatDate(unlockdate)]);
			}
			else if (::Stats.isProgressAchievement(name))
			{
				local progress = ::Stats.getProgressAchievementCompletionPercentage(name);
				if (progress > 0.0)
				{
					extra.label.setTextLocalizedAndFormatted("ACHIEVEMENT_PROGRESS", [::format("%.2f", progress)]);
				}
			}
			
			pres.addCustomValue("id", id);
			pres.start("", [tag], false, 0);
			if (hidden && unlockdate == null)
			{
				goal.label.setTextLocalized("ACHIEVEMENT_HIDDEN_HEADER");
				description.label.setTextLocalized("ACHIEVEMENT_HIDDEN_DESCRIPTION");
			}
			else
			{
				goal.label.setTextUTF8(::getLocalizedAchievementStringUTF8(locid + "_NAME"));
				description.label.setTextUTF8(::getLocalizedAchievementStringUTF8(locid + "_DESC"));
			}
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Inherited Methods
	
	function create()
	{
		base.create();
		
		createTitle("MENU_ACHIEVEMENTS");
		
		createFaceButtons(["back"]);
		
		local items = getPageItems();
		createPanelButtons(::Vector2(0.0, 0.28), ::MainMenu.c_buttonsSpacing, 0,
		[
			["lst_pages", "GUIListButton", items, onPageSelected]
		]);
		
		createPresentation("icon_0", "achievements", ::Vector2(-0.45, 0.1));
		createPresentation("icon_1", "achievements", ::Vector2(-0.45, -0.11));
		createPresentation("icon_2", "achievements", ::Vector2(-0.45, -0.32));
		
		createTextArea("goal_0", null, ::Vector2(-0.33, 0.175), ::TextColors.Light, 0.8, 0.05);
		createTextArea("extra_0", null, ::Vector2(0.53, 0.175), ::TextColors.Light, 0.8, 0.05, HorizontalAlignment_Right);
		createTextArea("description_0", null, ::Vector2(-0.33, 0.115), ::TextColors.Dark, 0.87, 0.12);
		
		createTextArea("goal_1", null, ::Vector2(-0.33, -0.035), ::TextColors.Light, 0.8, 0.05);
		createTextArea("extra_1", null, ::Vector2(0.53, -0.035), ::TextColors.Light, 0.8, 0.05, HorizontalAlignment_Right);
		createTextArea("description_1", null, ::Vector2(-0.33, -0.095), ::TextColors.Dark, 0.87, 0.12);
		
		createTextArea("goal_2", null, ::Vector2(-0.33, -0.245), ::TextColors.Light, 0.8, 0.05);
		createTextArea("extra_2", null, ::Vector2(0.53, -0.245), ::TextColors.Light, 0.8, 0.05, HorizontalAlignment_Right);
		createTextArea("description_2", null, ::Vector2(-0.33, -0.305), ::TextColors.Dark, 0.87, 0.12);
		
		local unlockedAchievements = ::getRegistry().getPersistent("achievements");
		_sortedIDs = [];
		foreach (key, achievement in ::achievementTable)
		{
			local date = (key in unlockedAchievements) ? unlockedAchievements[key] : null;
			_sortedIDs.push([achievement.id, achievement.order, key, achievement.hidden, achievement.locid, date]);
		}
		// Sort by unlock date first, then sort id
		_sortedIDs.sort(@(a, b) a[5] == null && b[5] == null ? a[1] <=> b[1] : -(a[5] <=> b[5]));
		
		onPageSelected(0);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods
	
	function getPageItems()
	{
		local items = [];
		
		local pages = ::ceil(::achievementTable.len() / 3.0);
		
		for (local i = 0; i < pages; ++i)
		{
			items.push([["MENU_PAGE", (i+1) + "", pages + ""], i]);
		}
		
		return ::GUIListButton.getScrolledItems(items, 0);
	}
}
