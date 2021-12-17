include_entity("menu/MenuScreen");

class MenuScreenVolumeSettings extends MenuScreen
</
	placeable      = Placeable_Hidden
	collisionRect  = [ 0.0, 0.0, 2.0, 1.0 ]  // center X, center Y, width, height
/>
{
	_volumeChanged = false;
	static c_normalSequence = ["DO", "RE", "MI", "FA", "SO", "LA", "TI"];
	_normalIndex      = -1;

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks

	function onMusicVolumeChanged(p_category, p_volume)
	{
		handleVolumeChanged();
		::Audio.setUserSettingVolumeMusic(p_volume);
	}

	function onSfxVolumeChanged(p_category, p_volume)
	{
		handleVolumeChanged();
		::Audio.setUserSettingVolumeSfx(p_category, p_volume);
		if (p_category == "Effects")
		{
			::Audio.setUserSettingVolumeSfx("Ambient", p_volume);
		}
	}

	function onVoiceOverVolumeChanged(p_category, p_volume, p_diff)
	{
		_normalIndex += p_diff;
		if (_normalIndex < 0) _normalIndex = c_normalSequence.len()-1;
		if (_normalIndex >= c_normalSequence.len()) _normalIndex = 0;
		onSfxVolumeChanged(p_category, p_volume);
		::Audio.playGlobalSoundEffect("VoiceOver", "MENU_VOLUME_" + c_normalSequence[_normalIndex]);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Inherited Methods

	function create()
	{
		base.create();

		createTitle("MENU_VOLUME_SETTINGS");
		createFaceButtons(["back"]);

		createPanelButtons(::Vector2(0.0, 0.10), ::MainMenu.c_buttonsSpacing, 0,
		[
			["lst_music",     "GUIListButton", getVolumeItemsForCategory("Music"), onMusicVolumeChanged],
			["lst_sfx",       "GUIListButton", getVolumeItemsForCategory("Effects"), onSfxVolumeChanged],
			["lst_voiceover", "GUIListButtonWithUpDown", getVolumeItemsForCategory("VoiceOver"), onVoiceOverVolumeChanged]
		]);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods

	function handleVolumeChanged()
	{
		_volumeChanged = true;
	}

	function getVolumeItemsForCategory(p_category)
	{
		local supportedVolumes = [0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100];
		local currentVolume = ::Audio.getUserSettingVolumeSfx(p_category).tointeger();
		local foundIdx = -1;
		for (local i = 0; i < supportedVolumes.len(); ++i)
		{
			if (supportedVolumes[i] == currentVolume)
			{
				foundIdx = i;
				break;
			}
		}

		if (foundIdx == -1)
		{
			::tt_panic("Cannot find matching supported volume for current category '" +
			           p_category + "' volume: " + currentVolume);
			// Default to 100%
			foundIdx = 0;
			::Audio.setUserSettingVolumeSfx(p_category, supportedVolumes[foundIdx]);
			_volumeChanged = true;
		}

		local items = [];
		local i = foundIdx;
		local key = "MENU_VOLUME_" + p_category.toupper();
		do
		{
			local volume = supportedVolumes[i];
			items.push([[key, volume.tostring()], p_category, volume]);
			++i;
			if (i >= supportedVolumes.len()) i = 0;
		} while (i != foundIdx);
		return items;
	}
}
