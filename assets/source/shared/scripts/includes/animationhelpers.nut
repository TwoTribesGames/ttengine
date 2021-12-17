function getPresentationCallback(p_name)
{
	local settings = PresentationStartSettings();
	settings.setEndCallbackName(p_name);
	
	return settings;
}

// helpers to expose layers in the editor
function getLayerNames()
{
	return [
	"behind_shoebox_zero",
	"behind_entities",
	"behind_entities_sub_only",
	"in_front_of_entities",
	"in_front_of_entities_sub_only",
	"in_front_of_water",
	"in_front_of_split",
	"in_front_of_shoebox_zero_one",
	"in_front_of_shoebox_zero_two",
	"behind_hud",
	"hud",
	"in_front_of_hud"];
}

function getLayerFromName(p_name)
{
	// no sub screens on steam!
	if (::isPlayingOnPC())
	{
		local subIdx = p_name.find("_sub_only");
		if (subIdx != null)
		{
			p_name = p_name.slice(0, subIdx);
		}
	}
	
	local layer = getParticleLayerFromName(p_name);
	
	if (isValidParticleLayer(layer))
	{
		return layer;
	}
	else
	{
		return null;
	}
}