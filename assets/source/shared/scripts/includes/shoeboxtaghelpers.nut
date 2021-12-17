/*
-- Tag Event Types --
No param  : "show", "hide", "start", "stop", "reset", "pause", "resume"
With param: "set-directiontype", "set-timetype", "set-tweentype"

-- Direction Types -- (set-directiontype event)
"forward", "backward", "pingpong", "reversepingpong"

-- Time Types -- (set-timetype event)
"linear", "easein", "easeout", "easeends", "easecenter"

-- Tween Types -- (set-tweentype event)
"sine", "quint", "quart", "quad", "expo", "elastic", "cubic", "circ", "bounce", "back"
*/

::g_shoeboxPresetCommands <- 
{
	show           = [["show", ""]],
	hide           = [["hide", ""]],
	start          = [["set-directiontype", "forward"] , ["start" , ""]],
	start_and_show = [["show", ""] , ["start" , ""]],
	stop_and_hide  = [["stop", ""] , ["hide" , ""]],
	start_reverse  = [["set-directiontype", "backward"], ["start" , ""]],
	stop           = [["stop", ""]],
	reset          = [["reset", ""]],
	pause          = [["pause", ""]],
	["resume"]     = [["resume", ""]], // resume is a reserved squirrel keyword so add it as a string
};


function getShoeboxTagEventPresets()
{
	return ::tableKeysAsArray(::g_shoeboxPresetCommands);
}


function getShoeboxTagEventCommands(p_preset)
{
	if (p_preset in ::g_shoeboxPresetCommands)
	{
		return ::g_shoeboxPresetCommands[p_preset];
	}
	
	::tt_panic("getShoeboxTagEventCreationInfo: Unknown preset requested '" + p_preset + "'");
	return null;
}

function sendShoeboxTagPreset(p_tag, p_preset = "start_and_show")
{
	local commands = getShoeboxTagEventCommands(p_preset);
	
	foreach(command in commands)
	{
		sendShoeboxTagEvent(p_tag, command[0], command[1]);
	}
}
