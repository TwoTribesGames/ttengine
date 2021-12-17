// a class because an enum is weird with compilation and includes (order not guaranteed or something)
class OptionsKeys
{
	Rumble        = "Rumble";
	ScreenShake   = "ScreenShake";
	GamePadScheme = "GamePadScheme";
	
	static c_defaults  =
	{
		Rumble             = true,
		ScreenShake        = true,
		GamePadScheme      = GamepadControlScheme_A1
	}
}

::g_optionsData <- {}; // local copy, for super fast access

class OptionsData
{
	static c_version = 1.0;
	
	optionDelegates = {};
	
	function set(p_key, p_value)
	{
		::g_optionsData[p_key] <- p_value;
		setRegistryData(::g_optionsData);
		
		if (p_key in optionDelegates)
		{
			optionDelegates[p_key](p_value);
		}
	}
	
	function get(p_key)
	{
		// return local copy; much faster
		return ::g_optionsData[p_key];
	}
	
	function restore()
	{
		::g_optionsData = getRegistryData();
		
		local version = ("version" in ::g_optionsData) ? ::g_optionsData.version : null;
		
		if (::g_optionsData == null || c_version != version)
		{
			::g_optionsData = clone ::OptionsKeys.c_defaults;
		}
		
		foreach (key, val in ::g_optionsData)
		{
			set(key, val);
		}
		
		// set version
		set("version", c_version);
	}
	
	// Fetch the level data table from the registry or create a fresh table if either
	// the registry value does not exist yet or the level does not yet appear in the table
	function getRegistryData()
	{
		return ::getRegistry().getPersistent("Options");
	}
	
	// Overwrite the registry data for a level or create fresh data if data for the level has not
	// yet been stored
	function setRegistryData(p_data)
	{
		::getRegistry().setPersistent("Options", p_data);
	}
}
::OptionsData.optionDelegates[::OptionsKeys.Rumble]         <- @(value) ::setRumbleEnabled(value);
::OptionsData.optionDelegates[::OptionsKeys.GamePadScheme]  <- @(value) ::setGamepadControlScheme(value);
::OptionsData.restore();
