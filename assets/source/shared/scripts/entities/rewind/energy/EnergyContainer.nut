include_entity("rewind/EntityChild");

class EnergyContainer extends EntityChild
</
	placeable      = Placeable_Hidden
	movementset    = "Static"
	collisionRect  = [ 0.0, 0.0, 0.1, 0.1 ]
/>
{
	_energy = 0;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		base.onInit();
		
		setEnergy(_energy);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onParentDied(p_parent)
	{
		if (p_parent.hasProperty("dieQuietly") == false &&
		    p_parent.hasProperty("noRandomPickupsOnDie") == false &&
		    ::Game.isFarmingEnabled() == false)
		{
			releaseEnergy();
		}
		
		base.onParentDied(p_parent);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods
	
	function setEnergy(p_energy)
	{
		// Make sure _energy is an integer
		_energy = ::ceil(p_energy);
	}
	
	function getEnergy()
	{
		return _energy;
	}
	
	function releaseEnergy()
	{
		::EnergyMgr.queueEnergyRelease(_energy, _parent.getCenterPosition());
		setEnergy(0);
	}
	
	function reduceEnergy(p_amount)
	{
		if (_energy < p_amount)
		{
			return false;
		}
		
		setEnergy(_energy - p_amount);
		return true;
	}
}
