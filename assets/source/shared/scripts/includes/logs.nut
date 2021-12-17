::noLogging <- false; // Set to true if you want to disable logging from script.

function trace()
{
	local count = 0;
	local info = getstackinfos(2);
	
	local res      = ::regexp(@"([a-zA-Z]+)\.bnut").capture(info.src);
	local filename = info.src.slice(res[1].begin, res[1].end);
	
	local args = "";
	foreach (key, val in info.locals)
	{
		if (key.slice(0, 2) == "p_")
		{
			if (args != "") { args += ", "; } // in case you have multiple args :)
			args += key + " = " + val;
		}
	}
	
	::echo(filename + "(" + (info.line-1) + ")::" + info.func + "(" + args + ", this = " + this + ")");
}

function echo(...)
{
	if (vargv.len() == 0)
	{
		return;
	}
	
	if (vargv.len() == 1)
	{
		::tagLog(::niceStringFromObject(vargv[0]));
	}
	else
	{
		local message = "";
		// merge all but the last params
		foreach(i, val in vargv)
		{
			if (i < vargv.len() - 1)
			{
				message += ::niceStringFromObject(val) + " "; //ALSO this works, but because it's the end of the line it does not matter: ((i < vargv.len() - 2) ? " " : "");
			}
		}
		::tagLog(message, vargv.top());
	}
}

function tagLog(p_message, p_tag = null)
{
	::error("<" + ::format("%6.2f", ::getAppTimeInSeconds()) + "> ");
	if (p_tag != null)
	{
		::error("[" + ::niceStringFromObject(p_tag) +"] ");
	}
	::print(p_message + "\n");
}

function log(...)
{
	foreach (txt in vargv)
	{
		::print(txt + " ");
	}
	::print("\n");
}

function screenLog(p_message, p_entity)
{
	DebugView.drawFilledCircle(p_entity.getPosition(), 0.05, 5);
	DebugView.drawTextInWorld(p_entity.getPosition(), p_message + " | " + ::niceStringFromObject(p_entity), 5);
}

function topStackInfo()
{
	local stack = 0;
	//local info  = getstackinfos(stack);
	
	while (getstackinfos(stack) != null)
	{
		stack++;
	}
	
	return getstackinfos(stack - 1);
}

function logStackInfo()
{
	local stack = 0;
	local info  = getstackinfos(stack);
	
	while (getstackinfos(stack) != null)
	{
		echo(getstackinfos(stack), stack);
		stack++;
	}
}

function stacktrace()
{
	echo("stacktrace");
	local stack = 2;
	while (getstackinfos(stack) != null)
	{
		local info     = getstackinfos(stack++);
		local res      = ::regexp(@"([a-zA-Z]+)\.bnut").capture(info.src);
		if (res == null)
		{
			continue;
		}
		local filename = info.src.slice(res[1].begin, res[1].end);
		
		local args = "";
		foreach (key, val in info.locals)
		{
			if (key.len() >= 2 && key.slice(0, 2) == "p_")
			{
				if (args != "") { args += ", "; } // in case you have multiple args :)
				args += key + " = " + val;
			}
		}
		echo(" - " + filename + "(" + (info.line-1) + ")::" + info.func + "(" + args + ", this = " + this + ")");
	}
}

function getfunctionStackInfo(p_skipLevel = 0)
{
	local stack = p_skipLevel;
	local info  = getstackinfos(stack);
	local ret = "";
	
	while (info != null)
	{
		ret = info.func + " -> " + ret;
		stack++;
		info = getstackinfos(stack);
	}
	
	return ret;
}

function niceStringFromObject(p_object)
{
	if (p_object instanceof ::EntityBase)
	{
		return ::entityString(p_object);
	}
	
	switch (typeof(p_object))
	{
		case "table":
		{
			return ::tableString(p_object);
		}
		case "array":
		{
			return ::arrayString(p_object);
		}
		default:
		{
			return p_object; // note that we don't call the tostring method explicitly; that causes crashes.
		}
	}
}

local tableStingIndent = 0;

function tableString(p_table)
{
	tableStingIndent += 1;
	local indent = "\n";
	
	for(local i = 0; i < tableStingIndent; i++)
	{
		indent += "  ";
	}
	local message = indent + "{";
	foreach(key, val in p_table)
	{
		message += indent + "  " + key + " -> " + ::niceStringFromObject(val) + ""
	}
	tableStingIndent -= 1;
	
	message += indent + "}\n"
	
	return message;
}

function arrayString(p_array)
{
	local message = "[";
	
	foreach(key, val in p_array)
	{
		message += ::niceStringFromObject(val) + ((key != (p_array.len() - 1)) ? ", ": "");
	}
	
	return message + "]";
}


function getEntityHandleString(p_entity, p_useID = false)
{
	local idString = ((p_useID) ? ("ID" + p_entity.getID()) : (p_entity.getHandleValue()));
	return p_entity.getType() + " " + idString + (p_entity.isSuspended() ? "*" : "") + (p_entity.isValid() ? "" : "!");
}

function entityString(p_entity)
{
	// getState returns an empty string when it has no state
	local state = (p_entity.getState() == "") ? "" : "(" + p_entity.getState() + ")";
	
	local idString = "ID " + p_entity.getID() + " (" + p_entity.getHandleValue() + ")";
	return p_entity.getType() + " " + idString + (p_entity.isSuspended() ? "*" : "") + (p_entity.isValid() ? "" : "!") + 
		state;
}

function entityIDString(p_entity)
{
	if (p_entity == null) return "NULL";
	
	// getState returns an empty string when it has no state
	local state     = (p_entity.getState() == "") ? "No State" : p_entity.getState();
	
	return getEntityHandleString(p_entity, true) + " (" + state + " " + p_entity.getPosition() + ")";
}

if (::noLogging)
{
	::echo = function (...) {};
	::log      = function (...) {};
	::tagLog   = function (...) {};
}