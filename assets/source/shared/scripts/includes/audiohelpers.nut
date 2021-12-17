enum DuckingPreset
{
	Conversation,
	EndMission,
	VirusUploader,
	SpecialPickup
};

::g_audioDuckingStack <- [];

// Make sure global volumes are always max 50%, to prevent distortion
::g_globalVolume <- 50.0;

function Audio::resetVolumes()
{
	::g_audioDuckingStack = [];
	initVolumes();
}

function Audio::initVolumes()
{
	::Audio.setVolumeForCategory("Music", ::g_globalVolume);
	::Audio.setVolumeForCategory("Effects", ::g_globalVolume);
	::Audio.setVolumeForCategory("Ambient", ::g_globalVolume);
	::Audio.setVolumeForCategory("VoiceOver", ::g_globalVolume);
}

function Audio::restoreVolumes()
{
	if (::g_audioDuckingStack.len() == 0)
	{
		initVolumes();
		return;
	}
	// Search for lowest volumes in stack and set those
	local lowest = { Music = 100, Effects = 100, Ambient = 100, VoiceOver = 100 };
	
	foreach (preset in ::g_audioDuckingStack)
	{
		local categories = [];
		local volume = 100.0;
		switch (preset)
		{
		case DuckingPreset.Conversation:
			categories = ["Music", "Effects", "Ambient"];
			volume     = 60.0;
			break;
		
		case DuckingPreset.EndMission:
			categories = ["Music", "Ambient"];
			volume     = 0.0;
			break;
		
		case DuckingPreset.VirusUploader:
			categories = ["Music", "Ambient"];
			volume     = 25.0;
			break;
			
		case DuckingPreset.SpecialPickup:
			categories = ["Music", "Ambient"];
			volume     = 0.0;
			break;
		
		default:
			::tt_panic("Unhandled preset '" + p_preset + "'");
			return;
		}
		
		foreach (category in categories)
		{
			if (volume < lowest[category])
			{
				lowest[category] = volume;
			}
		}
	}
	
	foreach (category, volume in lowest)
	{
		::Audio.setVolumeForCategory(category, (volume * ::g_globalVolume) / 100.0);
	}
}

function Audio::duckVolume(p_preset)
{
	::g_audioDuckingStack.push(p_preset);
	restoreVolumes();
}

function Audio::unduckVolume(p_preset)
{
	if (::g_audioDuckingStack.len() == 0)
	{
		::tt_panic("Ducking stack is empty");
		return;
	}
	
	// Look for preset in stack and remove it. Order doesn't matter, as we use the lowest volume anyway
	for (local i = 0; i < ::g_audioDuckingStack.len(); ++i)
	{
		if (::g_audioDuckingStack[i] == p_preset)
		{
			::g_audioDuckingStack.remove(i);
			restoreVolumes();
			return;
		}
	}
	::tt_panic("Couldn't find preset '" + p_preset + "' in audio ducking stack");
}
