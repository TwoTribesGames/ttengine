/*
Some math (i.e. functions that work on numbers) helper functions
*/

radToDeg <- 57.2957795;
degToRad <- 0.0174532925;

function clamp(p_value, p_min, p_max)
{
	return (p_value < p_min) ? p_min : ((p_value > p_max) ? p_max : p_value);
}

// returns the lowest value
// tip: to avoid confusion with local vars named "min" call the function as follows 
// local min = ::min(-1, -4);
function min(p_value, p_max)
{
	return (p_value < p_max) ? p_value : p_max;
}

// returns the highest value
// tip: to avoid confusion with local vars named "max" call the function as follows 
// local max = ::max(1, 4);
function max(p_value, p_min)
{
	return (p_value > p_min) ? p_value : p_min;
}

// returns a random bool
function brnd()
{
	return (frnd() > 0.5);
}

// returns an array with values from p_min to p_max (inclusive) with (optional) stepsize p_step
function range(p_min, p_max, p_step = 1)
{
	local ret = [];
	
	local steps = (p_max - p_min) / p_step;
	for (local i = 0; i <= steps; i++)
	{
		ret.append(p_min + i * p_step);
	}
	
	return ret;
}

function approximately(p_left, p_right)
{
	return (::fabs(p_left - p_right) < 0.001);
}

function sign(p_value)
{
	return p_value < 0 ? -1 : 1;
}

// works for Vector2's too (and anything that can be added, subtracted and multiplied
function lerp(start, end, factor)
{
	return ((end - start) * factor) + start;
}

function angleLerp(start, end, factor)
{
	local diff = end - start;
	
	if (::fabs(diff) > 180)
	{
		if (end > start)
		{
			start += 360;
		}
		else
		{
			end += 360;
		}
	}
	
	return ::wrapAngle(::lerp(start, end, factor));
}

// wrap angle in [0, 360) range
function wrapAngle(p_angle)
{
	while (p_angle >= 360)
	{
		p_angle -= 360;
	}
	
	while (p_angle < 0)
	{
		p_angle += 360;
	}
	
	return p_angle;
}

// Angles should be in [0, 360) range
function isAngleInRange(p_angle, p_rangeAngleA, p_rangeAngleB)
{
	if (p_rangeAngleA > p_angle)
	{
		p_rangeAngleA -= 360.0;
	}
	
	if (p_rangeAngleB < p_rangeAngleA)
	{
		p_rangeAngleB += 360.0;
	}
	else if ( (p_rangeAngleB - p_rangeAngleA) >= 360.0 )
	{
		p_rangeAngleB -= 360.0;
	}
	
	return ( (p_angle < p_rangeAngleA) || (p_angle > p_rangeAngleB) );
}

// Gets the shortest angle between two angles. Angles should be in [0, 360) range
function getAngleBetweenAngles(p_angle1, p_angle2)
{
	// See: http://gamedev.stackexchange.com/questions/4467/comparing-angles-and-working-out-the-difference
	return 180 - ::abs(::abs(p_angle1 - p_angle2) - 180);
}

function getVectorFromAngle(p_angle)
{
	local radAngle = p_angle * ::degToRad;
	return ::Vector2(::sin(radAngle), ::cos(radAngle));
}

function getAngleFromVector(p_vector)
{
	local angle = ::atan2(p_vector.x, p_vector.y) * ::radToDeg;
	return ::wrapAngle(angle);
}

 ::g_cosLUT <- [];
 ::g_sinLUT <- [];
 foreach (a in range(0.0, 255.0))
 {
	local rad = (a / 255.0) * ::PI * 2;
	
	::g_cosLUT.append(::cos(rad));
	::g_sinLUT.append(::sin(rad));
 }
 
 function getRandomDirectionVector()
 {
	local i = ::rnd_minmax(0, 255);
	
	return ::Vector2(::g_sinLUT[i], ::g_cosLUT[i]);
 }