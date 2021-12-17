include_entity("triggers/Trigger");

class KilledSensingTrigger extends Trigger
</
	editorImage    = "editor.killedsensingtrigger"
	libraryImage   = "editor.library.killedsensingtrigger"
	placeable      = Placeable_Developer
	sizeShapeColor = Colors.Trigger
	displayName    = "Trigger - Killed sensing trigger"
	group          = "04. Input Triggers"
/>
{
	</
		type           = "entityid_array"
		filter         = ["RewindEntity"]
		referenceColor = ReferenceColors.Sensing
		group          = "Specific Settings"
		order          = 0.0
	/>
	entities = null;
	
	</
		type        = "integer"
		min         = 0
		max         = 100
		description = "Triggers when X amount of entities are left."
		group       = "Specific Settings"
		order       = 0.1
	/>
	triggerLimit = 0;
	
	</
		type        = "bool"
		description = "When set to true, a hacked entity counts as a dead entity."
		group       = "Specific Settings"
		order       = 0.2
	/>
	hackedCountsAsDeath = true;
	
	</
		type           = "entityid"
		referenceColor = ReferenceColors.Parent
		description    = "When specified, dead/hacked entity is only counted if killed/hacked by this entity."
		group          = "Specific Settings"
		order          = 0.3
	/>
	filter = null;
	
	</
		type        = "bool"
		description = "When set to true, each kill will trigger this trigger until the triggerlimit is reached."
		group       = "Specific Settings"
		order       = 0.4
	/>
	triggerEachKill = false;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		once = triggerEachKill == false;
		
		base.onInit();
		
		if (entities == null)
		{
			editorWarning("Select entities");
			::killEntity(this);
			return;
		}
		
		if (triggerLimit == entities.len())
		{
			editorWarning("triggerLimit should be lower than number of entities.");
			::killEntity(this);
			return;
		}
		triggerLimit = entities.len() - triggerLimit;
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onEntityHacked(p_entity)
	{
		if (filter != null)
		{
			// FIXME: In case of multiplayer we need to know WHO hacked this entity
			local player = ::getFirstEntityByTag("PlayerBot");
			if (player != null && player.getID() == filter)
			{
				--triggerLimit;
				checkTrigger();
			}
		}
		else
		{
			--triggerLimit;
			checkTrigger();
		}
	}
	
	function onEntityDied(p_entity)
	{
		if (filter != null)
		{
			if (p_entity.hasProperty("killer") &&
			    filter == p_entity.getProperty("killer").getID())
			{
				--triggerLimit;
				checkTrigger();
			}
		}
		else
		{
			--triggerLimit;
			checkTrigger();
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Inherited Methods
	
	function createTouchSensor()
	{
		// no touch sensor
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods
	
	function checkTrigger()
	{
		if (triggerLimit == 0 || triggerEachKill)
		{
			local player = ::getFirstEntityByTag("PlayerBot");
			if (player != null)
			{
				onTouchEnter(player, null);
				if (triggerEachKill && triggerLimit != 0)
				{
					onTouchExit(player, null);
				}
			}
			if (triggerLimit == 0)
			{
				setEnabled(false);
			}
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Update
	
	function update(p_deltaTime)
	{
		base.update(p_deltaTime);
		
		if (entities.len() == 0)
		{
			return;
		}
		
		local cleanList = [];
		foreach (id in entities)
		{
			local entity = ::getEntityByID(id);
			if (entity != null)
			{
				// Entity is alive; register to its death/hack events and remove from this list
				entity.subscribeDeathEvent(this, "onEntityDied");
				if (hackedCountsAsDeath && entity instanceof ::BaseEnemy && entity._virusUploadedEvent != null)
				{
					entity._virusUploadedEvent.subscribe(this, "onEntityHacked");
				}
			}
			else
			{
				cleanList.push(id);
			}
		}
		entities = cleanList;
	}
}
KilledSensingTrigger.setattributes("type", null);
KilledSensingTrigger.setattributes("width", null);
KilledSensingTrigger.setattributes("height", null);
KilledSensingTrigger.setattributes("radius", null);
KilledSensingTrigger.setattributes("once", null);
KilledSensingTrigger.setattributes("triggerOnTouch", null);
KilledSensingTrigger.setattributes("triggerOnUncull", null);
KilledSensingTrigger.setattributes("parentTrigger", null);
KilledSensingTrigger.setattributes("triggerFilter", null);
KilledSensingTrigger.setattributes("filterAllEntitiesOfSameType", null);
