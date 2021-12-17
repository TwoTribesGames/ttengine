::g_energyMgr <- null;

class EnergyMgr extends EntityBase
</
	placeable      = Placeable_Hidden
	movementset    = "Static"
	collisionRect  = [ 0.0, 0.0, 0.1, 0.1 ]
/>
{
	static c_maxReleasedPerFrame = 10;
	
	_queuedReleases = null;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Singleton Instance Management
	
	function getInstance()
	{
		if (::isValidEntity(::g_energyMgr) == false)
		{
			::g_energyMgr = ::spawnEntity("EnergyMgr", ::Vector2(0, 0));
		}
		return ::g_energyMgr;
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		::removeEntityFromWorld(this);
		_queuedReleases = [];
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// 'Static' Methods
	
	function queueEnergyRelease(p_amount, p_position)
	{
		local instance = getInstance();
		return instance._addToQueue(p_amount, p_position);
	}
	
	function clear()
	{
		local instance = getInstance();
		instance._clearQueue();
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Private Methods (work on instance)
	
	// Add sequence to end of queue; about sequence is be played if current sequence is aborted
	function _addToQueue(p_amount, p_position)
	{
		_queuedReleases.push([p_amount, p_position]);
	}
	
	// Immediately clear queue and remove potential active textballoon
	function _clearQueue()
	{
		_queuedReleases.clear();
	}
	
	function update(p_deltaTime)
	{
		if (_queuedReleases.len() == 0)
		{
			return;
		}
		
		local releaseCount = 0;
		for (local i = 0; i < _queuedReleases.len(); ++i)
		{
			local batchCount = _queuedReleases[i][0];
			if (releaseCount + batchCount > c_maxReleasedPerFrame)
			{
				batchCount = c_maxReleasedPerFrame - releaseCount;
			}
			
			_queuedReleases[i][0] -= batchCount;
			releaseCount += batchCount;
			
			local pos = _queuedReleases[i][1];
			for (local k = 0; k < batchCount; k++)
			{
				::spawnEntity("FreeEnergy", pos);
			}
			
			if (releaseCount >= c_maxReleasedPerFrame)
			{
				break;
			}
		}
		
		// Cleanup queue
		local newQueue = [];
		for (local i = 0; i < _queuedReleases.len(); ++i)
		{
			if (_queuedReleases[i][0] > 0)
			{
				newQueue.push(_queuedReleases[i]);
			}
		}
		_queuedReleases = newQueue;
	}
}
