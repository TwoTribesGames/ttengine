include_entity("triggers/Trigger");

class AssassinateTrigger extends Trigger
</
	editorImage    = "editor.assassinatetrigger"
	libraryImage   = "editor.library.assassinatetrigger"
	placeable      = Placeable_Developer
	sizeShapeColor = Colors.TriggerKill
	displayName    = "Trigger - Assassinate"
	group          = "04.1 Action Triggers"
	stickToEntity  = true
/>
{
	</
		type           =  "entityid_array"
		description    = "The entities to kill"
		referenceColor = ReferenceColors.Kill
		order          = 0
		group          = "Specific Settings"
	/>
	entities = null;
	
	</
		type        = "bool"
		description = "If enabled, the killed entities won't spawn corspes"
		order       = 1
		group       = "Specific Settings"
	/>
	noCorpse = false;
	
	</
		type        = "bool"
		description = "If enabled, the entities won't be killed if they are hacked."
		order       = 2
		group       = "Specific Settings"
	/>
	noHacked = false;
	
	</
		type        = "bool"
		description = "If enabled, entities die quietly when kiled"
		order       = 3
		group       = "Specific Settings"
	/>
	dieQuietly = false;
	
	function onInit()
	{
		base.onInit();
		
		if (entities == null)
		{
			editorWarning("Select entities");
			return;
		}
	}
	
	function onTriggerEnterFirst(p_entity)
	{
		foreach (id in entities)
		{
			local entity = ::getEntityByID(id);
			if (entity != null)
			{
				if (noHacked && entity instanceof ::BaseEnemy && entity.containsVirus())
				{
					return;
				}
				
				if (noCorpse)
				{
					entity.removeProperty("hasCorpse");
				}
				
				entity.addProperty("noRandomPickupsOnDie");
				if (dieQuietly)
				{
					entity.addProperty("dieQuietly");
				}
				::killEntity(entity);
			}
		}
	}
}
