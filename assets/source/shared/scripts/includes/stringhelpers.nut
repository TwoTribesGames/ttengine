/*
Some string (i.e. functions that work on characters) helper functions
*/

// Creates a box of characters that can be used as entity collision
// e.g. createCollisionTileString(2, 2, "s"); // creates a 2x2 box of solid collision.
function createCollisionTileString(p_width, p_height, p_character = "s")
{
	local row = "\n";
	for (local x = 0; x < p_width; x++)
	{
		row = p_character + row;
	}
	
	local total = "";
	for (local y = 0; y < p_height; y++)
	{
		total += row;
	}
	
	return total;
}

// Creates a bordered box of characters that can be used as entity collision
// e.g. createCollisionBorderTileString(2, 2, "="); // creates a 2x2 box of stairs.
function createCollisionBorderTileString(p_width, p_height, p_character = "s")
{
	local topBottomRow = "";
	local centerRow    = "";
	for (local x = 0; x < p_width; x++)
	{
		topBottomRow += p_character;
		centerRow    += ((x == 0 || x == (p_width-1)) ? p_character : " ");
	}
	
	local total = "";
	for (local y = 0; y < p_height; y++)
	{
		total += ((y == 0 || y == (p_height-1)) ? topBottomRow : centerRow) + "\n";
	}
	
	return total;
}

function padString(p_string, p_padding)
{
	p_string = p_string.tostring(); // convenience for being able to feed it integers.
	if (p_string.len() == 0) return p_padding;
	
	if (p_string.len() < p_padding.len())
	{
		return (p_padding.slice(0, -p_string.len()) + p_string);
	}
	return p_string;
}

function stringEndsWith(p_string, p_match)
{
	local matchLen = p_match.len();
	
	if (p_string.len() < matchLen)
	{
		return false;
	}
	
	return (p_string.slice(-matchLen) == p_match);
}

function stringStartsWith(p_string, p_match)
{
	local matchLen = p_match.len();
	
	if (p_string.len() < matchLen)
	{
		return false;
	}
	
	return (p_string.slice(0, matchLen) == p_match);
}

function stringSequence(p_string, p_length)
{
	local ret = "";
	for (local i = 0; i < p_length; i++)
	{
		ret += p_string;
	}
	return ret;
}

function intToName(p_int)
{
	local names = ["zero", "one", "two", "three", "four", "five", "six", "seven", "eight", "nine", "ten", "eleven", "twelve"];
	
	if (p_int >= 0 && p_int < names.len())
	{
		return names[p_int];
	}
	return "unknown (" + p_int + ")";
}