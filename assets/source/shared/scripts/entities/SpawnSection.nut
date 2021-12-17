class SpawnSection extends EntityBase
</
	editorImage    = "editor.spawnsection"
	libraryImage   = "editor.library.spawnsection"
	placeable      = Placeable_Everyone
	sizeShapeColor = Colors.SpawnSection
	displayName    = "Area - SpawnSection"
/>
{
	</
		type           = "integer"
		min            = 2
		max            = 500
		group          = "Section"
		order          = 0.1
	/>
	width = 100;
	
	</
		type           = "integer"
		min            = 2
		max            = 500
		group          = "Section"
		order          = 0.2
	/>
	height = 100;
	
	</
		type           = "entityid_array"
		description    = "The entities to spawn."
		referenceColor = ReferenceColors.List
		group          = "Section"
		order          = 0.3
	/>
	included = null;
	
	</
		type           = "entityid_array"
		description    = "The entities to spawn."
		referenceColor = ReferenceColors.List
		group          = "Section"
		order          = 0.4
	/>
	excluded = null;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		::Level.createSpawnSection(getID(), ::VectorRect(getCenterPosition(), width, height), included, excluded);
	}
	
	// SpawnSections DONT have an onSpawn
	/*
	function onSpawn()
	{
		trace();
		
	}
	*/
}
