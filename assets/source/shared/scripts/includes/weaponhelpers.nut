/*
weapon helpers
*/

// not an enum because for some magical reason that's not consistently recognized in other files
// even when those files include this one
class WeaponGroup
{
	Neutral = "neutral";
	Player  = "player";
	Enemy   = "enemy";
}

enum GunPresetFlags
{
	None      = 0x0,
	Droppable = 0x1,
	Pickup    = 0x2,
	
	All       = 0xFF
}

function getGunPresetNames(p_presetTable)
{
	local result = [];
	foreach (key, value in p_presetTable)
	{
		result.push(key);
	}
	result.sort();
	return result;
}

function getLaserPresetNames(p_presetTable)
{
	local result = [];
	foreach (key, value in p_presetTable)
	{
		if (::stringStartsWith(key, "laser"))
		{
			result.push(key);
		}
	}
	result.sort();
	return result;
}

function getNonLaserPresetNames(p_presetTable)
{
	local result = [];
	foreach (key, value in p_presetTable)
	{
		if (::stringStartsWith(key, "laser") == false)
		{
			result.push(key);
		}
	}
	result.sort();
	return result;
}

// Helper to workaround the table serialization bug
function getGunPreset(p_presetTable, p_preset)
{
	return p_presetTable[p_preset];
}

// Helper to workaround the table serialization bug
function getGunPresets(p_presetTable)
{
	return p_presetTable;
}


function createGun(p_parent, p_offset, p_presetProps)
{
	switch (p_presetProps._gunType)
	{
	case "FlameGun":           return p_parent.addChild("FlameGun", p_offset, p_presetProps);
	case "ProjectileGun":      return p_parent.addChild("ProjectileGun", p_offset, p_presetProps);
	case "PlayerBotGun":       return p_parent.addChild("PlayerBotGun", p_offset, p_presetProps);
	case "PlayerBotCustomGun": return p_parent.addChild("PlayerBotCustomGun", p_offset, p_presetProps);
	case "LaserGun":           return p_parent.addChild("LaserGun", p_offset, p_presetProps);
	default:
		::tt_panic("Unhandled gunType '" + p_presetProps._gunType + "'");
		break;
	}
	
	return null;
}
