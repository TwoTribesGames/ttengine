tt_include("includes/math");
/*
Some Vector2 helper functions, add tostring some arithmic operations 
and proper type identification. Some of these should eventually be
replaced by native code
*/

function Vector2::_typeof()
{ 
	return "Vector2"; 
}

function Vector2::_tostring() 
{
	return "(" + x + ", " + y +")"; 
}

function Vector2::set(p_x, p_y)
{
	x = p_x;
	y = p_y;
}

function Vector2::approximately(p_vector, p_margin = 0.001)
{
	return ::fabs(x - p_vector.x) < p_margin && ::fabs(y - p_vector.y) < p_margin;
}

function Vector2::approximatelyZero(p_margin = 0.001)
{
	return ::fabs(x) < p_margin && ::fabs(y) < p_margin;
}

function Vector2::setX(p_val)
{
	x = p_val;
	return this;
}

function Vector2::setY(p_val)
{
	y = p_val;
	return this;
}

// the casing is intentional to match Squirrel's tointeger etc. metamethod styles
function Vector2::toTable()
{
	return {x = x, y = y};
}

function Vector2::fromTable(p_table)
{
	
	return ::Vector2(p_table.x, p_table.y);
}

function Vector2::distanceTo(p_vector)
{
	return (this - p_vector).length();
}

function Vector2::distanceToRect(p_rect)
{
	// Inspired by: http://stackoverflow.com/questions/5254838/calculating-distance-between-a-point-and-a-rectangular-box-nearest-point
	local minx = p_rect.getLeft();
	local miny = p_rect.getBottom();
	local maxx = p_rect.getRight();
	local maxy = p_rect.getTop();
	
	local dx = ::max(0, ::max(minx - x, x - maxx));
	local dy = ::max(0, ::max(miny - y, y - maxy));
	
	return sqrt(dx * dx + dy * dy);
}

function Vector2::getHorizontal()
{
	return ::Vector2(x, 0);
}

function Vector2::getVertical()
{
	return ::Vector2(0, y);
}

// clone is a reserved word so I can't use that, which is funny because clone is what we're emulating
// here (cloning a vector2 the correct way crashes the game)
function Vector2::copy()
{
	return ::Vector2(x, y);
}

// can't use frnd_minmax or ::rnd_minmax because that conflicts with the bound functions somehow
function Vector2::frnd(p_minX, p_maxX, p_minY, p_maxY)
{
	return ::Vector2(::frnd_minmax(p_minX, p_maxX), ::frnd_minmax(p_minY, p_maxY));
}

function Vector2::rnd(p_minX, p_maxX, p_minY, p_maxY)
{
	return ::Vector2(::rnd_minmax(p_minX, p_maxX), ::rnd_minmax(p_minY, p_maxY));
}

// random vector between two vectors
function Vector2::vrnd(p_minVector, p_maxVector)
{
	return ::Vector2.frnd(p_minVector.x, p_maxVector.x, p_minVector.y, p_maxVector.y);
}

function Vector2::_add(p_other)
{
	switch (typeof(p_other))
	{
		case "Vector2":
		{
			return ::Vector2(x + p_other.x, y + p_other.y);
		}
		case "float":
		case "integer":
		{
			return ::Vector2(x + p_other, y + p_other);
		}
		default:
		{
			::tt_panic("Can't add Vector2 and " + typeof(p_other));
		}
	}
}

function Vector2::_sub(p_other)
{
	switch (typeof(p_other))
	{
		case "Vector2":
		{
			return ::Vector2(x - p_other.x, y - p_other.y);
		}
		case "float":
		case "integer":
		{
			return ::Vector2(x - p_other, y - p_other);
		}
		default:
		{
			::tt_panic("Can't subtract Vector2 and " + typeof(p_other));
		}
	}
}

function Vector2::_mul(p_other)
{
	switch (typeof(p_other))
	{
		case "Vector2":
		{
			return ::Vector2(x * p_other.x, y * p_other.y);
		}
		case "float":
		case "integer":
		{
			return ::Vector2(x * p_other, y * p_other);
		}
		default:
		{
			::tt_panic("Can't multiply Vector2 and " + typeof(p_other));
		}
	}
}

function Vector2::_div(p_other)
{
	switch (typeof(p_other))
	{
		case "Vector2":
		{
			return ::Vector2(x / p_other.x, y / p_other.y);
		}
		case "float":
		case "integer":
		{
			return ::Vector2(x / p_other, y / p_other);
		}
		default:
		{
			::tt_panic("Can't divide Vector2 and " + typeof(p_other));
		}
	}
}

function Vector2::dot(p_lhs, p_rhs)
{
	return (p_lhs.x * p_rhs.x + p_lhs.y * p_rhs.y);
}

function Vector2::perpDot(p_lhs, p_rhs)
{
	return (p_lhs.x * p_rhs.y - p_lhs.y * p_rhs.x);
}

// project vector on normalized axis
function Vector2::projectOn(p_axis)
{
	local s = (x * p_axis.x + y * p_axis.y) / (p_axis.x * p_axis.x + p_axis.y * p_axis.y);
	return ::Vector2(p_axis.x * s, p_axis.y * s);
}

function Vector2::_unm()
{
	return ::Vector2(-x, -y);
}

function Vector2::rotate(p_angleInDegrees)
{
	local radians = p_angleInDegrees * ::degToRad;
	local sinAngle = ::sin(radians);
	local cosAngle = ::cos(radians);
	
	return ::Vector2(x * cosAngle - y * sinAngle, y * cosAngle + x * sinAngle);
}

function Vector2::reflect(p_normal, p_bouncyness = 1.0)
{
	local dotProduct = dot(this, p_normal);
	return ::Vector2(p_bouncyness * (-2 * dotProduct * p_normal.x + this.x),
	               p_bouncyness * (-2 * dotProduct * p_normal.y + this.y));
}

function Vector2::equals(p_other)
{
	return (x == p_other.x && y == p_other.y);
}

// Return angle in radians between (0.. 2 * PI)
function Vector2::angle(p_lhs, p_rhs)
{
	local diffx = p_rhs.x - p_lhs.x;
	local diffy = p_rhs.y - p_lhs.y;
	local angle = ::atan2(diffx, diffy);
	if (angle < 0) return (2 * ::PI) + angle;
	return angle;
}

function Vector2::_serialize()
{
	return [
		// x
		[
			["kt", "s"],
			["kv", "x"],
			["vt", "f"],
			["v", x]
		],
		// y
		[
			["kt", "s"],
			["kv", "y"],
			["vt", "f"],
			["v", y]
		]
	];
}
