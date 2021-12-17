::g_tileNames <- {}

::g_tileNames["Air"]                      <- " "
::g_tileNames["Solid"]                    <- "s"
::g_tileNames["Crystal"]                  <- "c"
::g_tileNames["Water"]                    <- "W"
::g_tileNames["Lava"]                     <- "L"
::g_tileNames["WaterSource"]              <- "w"
::g_tileNames["LavaSource"]               <- "l"
::g_tileNames["Solid_Allow_Pathfinding"]  <- "p"

function getTileNames()
{
	return ::tableKeysAsArray(::g_tileNames);
}

function getTileCharByName(p_name)
{
	return ::g_tileNames[p_name];
}

// Calculate the hitpoint on the tileboundary when tracing from p_start to p_end.
// Assumes that the tile is at p_end
function calcTileHitLocation(p_start, p_end)
{
	local hitTile = ::Vector2(p_end.x.tointeger(), p_end.y.tointeger());
	
	local left   = hitTile.x;
	local right  = hitTile.x + 1;
	local bottom = hitTile.y;
	local top    = hitTile.y + 1;
	
	// Startpoint is in hit tile
	if (p_start.x >= left   && p_start.x <= right && 
		p_start.y >= bottom && p_start.y <= top)
	{
		return p_start;
	}
	
	local min = 0.0;
	local max = 1.0;
	local t0;
	local t1;
	
	// Check for div by zero
	local dx = p_end.x - p_start.x;
	if (::fabs(dx) > 0.00001)
	{
		local invDx = 1.0 / dx;
		if (dx > 0)
		{
			t0 = (left - p_start.x) * invDx;
			t1 = (right - p_start.x) * invDx;
		}
		else
		{
			t1 = (left - p_start.x) * invDx;
			t0 = (right - p_start.x) * invDx;
		}
			
		if (t0 > min) min = t0;
		if (t1 < max) max = t1;
	}
	
	local dy = p_end.y - p_start.y;
	if (::fabs(dy) > 0.00001)
	{
		local invDy = 1.0 / dy;
		if (dy > 0)
		{
			t0 = (bottom - p_start.y) * invDy;
			t1 = (top - p_start.y) * invDy;
		}
		else
		{
			t1 = (bottom - p_start.y) * invDy;
			t0 = (top - p_start.y) * invDy;
		}
		
		if (t0 > min) min = t0;
		if (t1 < max) max = t1;
	}
	
	return ::Vector2(p_start.x + dx * min, p_start.y + dy * min);
}
