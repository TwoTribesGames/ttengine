// Helper so we can easily switch to different platforms on in test mode
function getCurrentPlatform()
{
	return ::getPlatform();
}

function getPlatformString()
{
	local platform = ::getCurrentPlatform();
	
	switch (platform)
	{
		case Platform_WIN:
		case Platform_MAC:
		case Platform_LNX:
			return "pc";
	}
	
	::tt_panic("No valid string found for platform: " + platform);
	return "";
}

function isPlayingOnPC()
{
	local platform = ::getCurrentPlatform();
	return platform == Platform_WIN || platform == Platform_MAC || platform == Platform_LNX;
}

function getControllerTypeString()
{
	return ::getCurrentControllerType() == ControllerType_Keyboard ? "keyboard"  : "controller";
}
