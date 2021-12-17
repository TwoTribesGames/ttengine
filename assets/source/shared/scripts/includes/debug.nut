// TODO maybe make this empty functions and move the implementations to a script in temp_includes?
function assert(p_condition, p_message = null)
{
	if (p_condition == false)
	{
		::tt_panic("[script] " + ((p_message == null) ? "<no assert message>" : p_message));
	}
}

function null_assert(p_condition)
{
	if (p_condition == null)
	{
		::tt_panic("[script] failed null assertion");
	}
}

function checkAndPrintUnreachables()
{
	local res = resurrectunreachable();
	if (res == null) 
	{
		::echo("NO GARBAGE!"); 
		return;
	}
	else
	{
		::echo("GARBAGE" "GARBAGE"); 
	}
	
	foreach (entity in res)
	{
		local t = ("getType" in entity) ? entity.getType() : typeof(entity);
		::echo(entity, type(entity), t);
		
		/*
		if (t == "instance")
		{
			showMembers(t, "\t");
		}
		// */
	}
}

// this will be overwritten by a script in test_includes to return true
function isTestBuild()
{
	return false;
}

// debug function to show the value and names of all members
function showMembers(p_entity, prefix = "")
{
	log("*************************************************************");
	//echo(p_entity);
	log("Type: "         , memberFuncIn(p_entity, "getType", "X"/*typeof(p_entity)*/));
	log("Pos: "          , memberFuncIn(p_entity, "getPosition", "X"));
	log("ID: "           , memberFuncIn(p_entity, "getID", "X"));
	log("Current move: " , memberFuncIn(p_entity, "getCurrentMoveName", "X"));
	//log(p_entity.getclass(), "getclass" in p_entity);
	//if ("getclass" in p_entity)
	{
		foreach (member, val in p_entity.getclass())
		{
			if (member != null && typeof(val) != "function")
			{
				::echo(prefix, p_entity[member], member.tostring());
			}
		}
	}
	log("*************************************************************");
}

function memberFuncIn(p_entity, p_member, p_default = "")
{
	return (p_member in p_entity) ? p_entity[p_member]() : p_default;
}

function DebugView::drawVectorRect(p_rect, p_lifetime)
{
	local min = p_rect.getMin();
	local max = p_rect.getMaxEdge();
	
	DebugView.drawRect(min, max, p_lifetime);
}

function showConstTable()
{
	foreach( key, val in getconsttable())
	{
		echo(val, key);
	}
}