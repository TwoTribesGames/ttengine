include_entity("rewind/EntityChild");

class ExternalForces extends EntityChild
</ 
	placeable      = Placeable_Hidden
	movementset    = "Static"
	collisionRect  = [ 0.0, 0.0, 0.1, 0.1 ]
	group          = "Rewind"
/>
{
	_forces = [];
	
	_movementSettings = null;
	
	function onInit()
	{
		base.onInit();
		
		::assert(_movementSettings != null, "Thruster object needs a _movementSettings object to work on!");
		
		_forces = [];
	}
	
	function onSpawn()
	{
	}
	
	function applyForces()
	{
		for (local i = _forces.len() - 1; i >= 0; i--)
		{
			local force = _forces[i];
			
			_movementSettings.addToExtraForce(force.force);
			
			force.force *= force.dampening;
			if (force.force.approximatelyZero())
			{
				_forces.remove(i);
			}
		}
	}
	
	function addForce(p_force, p_dampening = 0.2)
	{
		_forces.append(Force(p_force, p_dampening));
	}
	
	function addImpulse(p_force)
	{
		addForce(p_force, 0);
	}
	
	function clear()
	{
		_forces.clear();
	}
	
	function getLastForce()
	{
		if (_forces.len() == 0)
		{
			return ::Vector2(0, 0);
		}
		
		return _forces[_forces.len() - 1].force;
	}
/*	function setSuspended(p_suspend)
	{
		
		base.setSuspended(p_suspend);
	}*/
	
	function childUpdate(p_deltaTime)
	{
		applyForces();
	}
}

class Force
{
	force     = null;
	dampening = 0.2;
	
	constructor(p_force, p_dampening = 0.2)
	{
		force = p_force;
		dampening = ::clamp(p_dampening, 0, 1); // Do not allow dampening outside [0, 1]
	}
}