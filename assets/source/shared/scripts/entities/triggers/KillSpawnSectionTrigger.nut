include_entity("triggers/Trigger");

class KillSpawnSectionTrigger extends Trigger
</
	editorImage    = "editor.killspawnsectiontrigger"
	libraryImage   = "editor.library.killspawnsectiontrigger"
	placeable      = Placeable_Everyone
	sizeShapeColor = Colors.Trigger
	displayName    = "Trigger - Spawn Section (Kill)"
	group          = "04.1 Action Triggers"
	stickToEntity  = true
/>
{
	</
		type        = "entityid_array"
		filter      = ["SpawnSection"]
		order       = 0
		group       = "Specific Settings"
	/>
	spawnSections = null;
	
	once = true;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		base.onInit();
		
		if (spawnSections == null || spawnSections.len() == 0)
		{
			editorWarning("spawnSections should be set");
			killEntity(this);
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onTriggerEnterFirst(p_entity)
	{
		base.onTriggerEnterFirst(p_entity);
		
		foreach (id in spawnSections)
		{
			::Level.killSpawnSection(id);
		}
	}
}
KillSpawnSectionTrigger.setattributes("once", null);
