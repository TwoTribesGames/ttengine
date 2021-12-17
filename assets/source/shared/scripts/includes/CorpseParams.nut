class CorpseParams
{
	corpseEntity         = "Corpse";
	overrideCorpseEntity = false;
	spawnProperties      = null;
	copyOrientation      = true;
	copyFloorDirection   = true;
	
	// p_corpseEntity: the type of corpse to spawn, if it does not derive from corpse set p_overrideCorpseEntity to true
	// p_spawnProperties: the spawnproperties to spawn the corpse with (some settings will be added automatically)
	// p_copyOrientation: copy the orientation (forwardAsLeft) of the entity that died
	// p_overrideCorpseEntity: set to true if you want to spawn a completely different entity (e.g. a bubble) on death
	constructor(p_corpseEntity = "Corpse", p_spawnProperties = null,  p_copyOrientation = true, p_overrideCorpseEntity = false)
	{
		if (p_spawnProperties == null)
		{
			p_spawnProperties = {};
		}
		
		spawnProperties      = p_spawnProperties;
		corpseEntity         = p_corpseEntity;
		overrideCorpseEntity = p_overrideCorpseEntity;
		copyOrientation      = p_copyOrientation;
	}
	
	// add presentation tags to the spawnProperties (creates the property if it doesn't exist)
	function addPresentationTags(...)
	{
		if (("_presentationTags" in spawnProperties) == false)
		{
			spawnProperties._presentationTags <- [];
		}
		
		spawnProperties._presentationTags.extend(vargv);
	}
	
	// add presentation tags to the spawnProperties (creates the property if it doesn't exist)
	function addPresentationCustomValues(...)
	{
		if (("_presentationCustomValues" in spawnProperties) == false)
		{
			spawnProperties._presentationCustomValues <- [];
		}
		
		spawnProperties._presentationCustomValues.extend(vargv);
	}
	
	function _tostring()
	{
		return "CorpseParams: spawnProperties" + ::niceStringFromObject(spawnProperties);
	}
	
	function _typeof()
	{
		return "CorpseParams";
	}
}