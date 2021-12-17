class SpawnSettings
{
	function get()
	{
		return ::getRegistry().get("SpawnSettings");
	}
	
	function set(p_option, p_value)
	{
		local spawnData = get();
		if (spawnData == null)
		{
			spawnData = {};
		}
		
		spawnData[p_option] <- p_value;
		
		::getRegistry().set("SpawnSettings", spawnData);
	}
	
	function clear()
	{
		::getRegistry().erase("SpawnSettings");
	}
}
