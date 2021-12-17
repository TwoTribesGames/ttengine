::g_physicsSettings <- 
{
	gravity = null,
	isEasingGravity = false,
	gravityEasingX = null,
	gravityEasingY = null
}

::g_gravityStack <- [];

enum GravityType
{
	Normal,
	Water,
	ZeroG
};

// Static class
class Gravity
{
	function set(p_type, p_duration = 0.0)
	{
		local vector = getVector(p_type);
		if (p_duration == 0.0)
		{
			// Instantly set it
			::g_physicsSettings.isEasingGravity = false;
			::g_physicsSettings.gravity = vector;
		}
		else
		{
			::g_physicsSettings.isEasingGravity = true;
			::g_physicsSettings.gravityEasingX = EasingReal(
				::g_physicsSettings.gravity.x,
				vector.x,
				p_duration,
				0
			);
			::g_physicsSettings.gravityEasingY = EasingReal(
				::g_physicsSettings.gravity.y,
				vector.y,
				p_duration,
				0
			);
		}
		
		::g_gravityStack.clear();
	}
	
	function update(p_deltaTime)
	{
		if (::g_physicsSettings.isEasingGravity)
		{
			::g_physicsSettings.gravityEasingX.update(p_deltaTime);
			::g_physicsSettings.gravityEasingY.update(p_deltaTime);
			
			::g_physicsSettings.gravity = 
				::Vector2(::g_physicsSettings.gravityEasingX.getValue()
				        ::g_physicsSettings.gravityEasingY.getValue());
		
			if (::g_physicsSettings.gravityEasingX.getTime() >= ::g_physicsSettings.gravityEasingX.getDuration())
			{
				::g_physicsSettings.isEasingGravity = false;
			}
		}
	}
	
	function isZeroGravity()
	{
		return ::g_physicsSettings.gravity.approximatelyZero();
	}
	
	function getVector(p_type)
	{
		switch (p_type)
		{
		case GravityType.Normal: return ::Vector2(0, -250).copy();
		case GravityType.Water:  return ::Vector2(0,   -8).copy();
		case GravityType.ZeroG:  return ::Vector2(0,    0).copy();
		default:
			::tt_panic("Invalid p_type '" + p_type + "'");
			break;
		}
		
		return ::Vector2(0, 0);
	}
}
