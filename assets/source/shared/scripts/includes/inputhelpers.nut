enum InputPriority
{
	Normal               = 0,
	Conversation         = 1,
	Pickup               = 2,
	BlockingConversation = 3,
	GUI                  = 4
};

function setRumble(p_strength, p_panning)
{
	if (p_panning != null)
	{
		::startRumble(p_strength, ::getRumbleDuration(p_strength), p_panning);
	}
}

function getRumbleDuration(p_strength)
{
	switch (p_strength)
	{
	case RumbleStrength_Low:        return 0.2;
	case RumbleStrength_Medium:     return 0.3;
	case RumbleStrength_High:       return 0.4;
	default:
		::tt_panic("Unhandled strength '" + p_strength + "'");
		break;
	}
	return 0.0;
}

function getRumblePanning(p_strength, p_position)
{
	local player = ::getFirstEntityByTag("PlayerBot");
	if (player != null)
	{
		local dist = player.getCenterPosition().distanceTo(p_position);
		if (dist > 100)
		{
			// Use null to signal too far away
			return null;
		}
		
		local diff = player.getCenterPosition().x - p_position.x;
		return  diff > 0.0 ? -0.7 : 0.7;
	}
	return 0.0;
}

function hasTwinstickControlScheme()
{
	local scheme = ::getGamepadControlScheme();
	return scheme == GamepadControlScheme_A1 || scheme == GamepadControlScheme_A2;
}

function hasSwitchModeControlScheme()
{
	return false;
}

function setSwitchModeControlScheme(p_scheme)
{
}

function resetSwitchModeControlScheme()
{
	::OptionsData.set(::OptionsKeys.GamePadScheme, GamepadControlScheme_A1);
}

function getGamepadControlSchemeString()
{
	local scheme = ::getGamepadControlScheme();
	switch (scheme)
	{
	case GamepadControlScheme_A1: return "a1";
	case GamepadControlScheme_A2: return "a2";
	case GamepadControlScheme_B1: return "b1";
	case GamepadControlScheme_B2: return "b2";
	}
	
	::tt_panic("Invalid control scheme");
	return "invalid";
}
