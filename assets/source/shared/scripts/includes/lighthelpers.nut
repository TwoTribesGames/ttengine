tt_include("includes/optionshelpers");
/*
*/

class LightHelpers
{
	minAmbientLightLevel = 0;
	maxAmbientLightLevel = 255;
	
	_settings = [-30, -15, -5, 0, 10, 25, 50];
	
	function getAmbientOffsetFromUserSetting()
	{
		local userSetting = 3; // hardcoded
		// we expect an int between 0 and the _settings len()
		userSetting = ::clamp(userSetting.tointeger(), 0, _settings.len() - 1);
		
		return ::LightHelpers._settings[userSetting];
	}
	
	function getUserAmbientValue(p_value)
	{
		local value = p_value + getAmbientOffsetFromUserSetting();
		value = ::clamp(value, minAmbientLightLevel, maxAmbientLightLevel);
		
		return value;
	}
	
	function setAmbientLight(p_value)
	{
		::Light.setLevelLightAmbient(::LightHelpers.getUserAmbientValue(p_value));
	}
	
	function getAmbientLight()
	{
		return ::Light.getLevelLightAmbient();
	}
	
	function getLevelAmbientLight()
	{
		local levelSettings = ::getFirstEntityByTag("level_settings");
		
		if (::isValidEntity(levelSettings))
		{
			return levelSettings.levelAmbientLight;
		}
		return 128;
	}
}