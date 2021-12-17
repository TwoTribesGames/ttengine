include_entity("triggers/Trigger");

class TriggerKill extends Trigger
</
	editorImage    = "editor.skull"
	libraryImage   = "editor.library.skull"
	placeable      = Placeable_Developer
	sizeShapeColor = Colors.TriggerKill
	displayName    = "Trigger - Kill"
	group          = "04.1 Action Triggers"
	stickToEntity  = true
/>
{
	</
		type           = "entityid"
		description    = "The entity type picked here is used as a filter for the trigger to work, e.g if you'd pick A the trigger only kills entities of type A"
		referenceColor = ReferenceColors.Kill
		order          = -1
		group          = "Specific Settings"
	/>
	entityFilter = null;
	
	</
		type        = "bool"
		description = "If enabled, the killed entities won't spawn corspes"
		order       = -0.5
		group       = "Specific Settings"
	/>
	noCorpse = false;
	
	</
		type        = "bool"
		description = "If enabled, the killed entities won't explode or spawn energy"
		order       = 0.0
		group       = "Specific Settings"
	/>
	silent = true;
	
	_filterClass = null;
	
	function onInit()
	{
		base.onInit();
		
		if (entityFilter != null)
		{
			_filterClass = ::getEntityClassByID(entityFilter);
		}
	}
	
	function onSpawn()
	{
		base.onSpawn();
		
		// Kill triggers use custom filtering behavior, so don't use the target optimization used by Triggers
		_triggerSensor.setTarget(null);
	}
	
	// override these
	function isEntityEligible(p_entity)
	{
		if (::isValidEntity(p_entity) == false)
		{
			return false;
		}
		
		// If no entity is explicitly selected, kill only RewindEntities (and no PlayerBotCorpse)
		if (_filterClass == null)
		{
			return p_entity instanceof ::RewindEntity && 
			       (p_entity instanceof ::PlayerBotCorpse) == false;
		}
		// else
		return p_entity.getclass() == _filterClass;
	}
	
	function onTouchEnter(p_entity, p_sensor)
	{
		if (p_entity.getclass() != ::PlayerBot)
		{
			if (noCorpse)
			{
				p_entity.removeProperty("hasCorpse");
			}
			
			if (silent)
			{
				p_entity.addProperty("dieQuietly");
			}
		}
		
		p_entity.addProperty("noRandomPickupsOnDie");
		
		// Force this entity to trigger this trigger
		base.onTouchEnter(p_entity, p_sensor);
		
		::killEntity(p_entity);
	}
	
	function onTriggerEnter(p_entity)
	{
	}
	
	function onTriggerExit(p_entity)
	{
	}
	
	function onTriggerEnterFirst(p_entity)
	{
	}
	
	function onTriggerExitLast(p_entity)
	{
	}
}
TriggerKill.setattributes("triggerFilter", null);
TriggerKill.setattributes("filterAllEntitiesOfSameType", null);
TriggerKill.setattributes("triggerOnUncull", null);
TriggerKill.setattributes("parentTrigger", null);

