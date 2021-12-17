include_entity("triggers/Trigger");

class SignalCountTrigger extends Trigger
</
	editorImage    = "editor.signalcounttrigger"
	libraryImage   = "editor.library.signalcounttrigger"
	placeable      = Placeable_Developer
	displayName    = "Trigger - Signal Count"
	group          = "04. Input Triggers"
	stickToEntity  = false
/>
{
	</
		type           =  "entityid_array"
		filter         = ::getTriggerTypes()
		description    = "The signallers (Triggers and all other parentable entities)."
		referenceColor = ReferenceColors.List
		group          = "Specific Settings"
		order          = 0
	/>
	signallers = null;
	
	</
		type        = "integer"
		min         = 0
		max         = 50
		description = "This trigger fires when it has received count amount of signals from the signallers. If 0, count equals the total signallers count."
		group       = "Specific Settings"
		order       = 1
	/>
	count = 0;
	
	_currentCount = 0;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		base.onInit();
		
		if (signallers == null || signallers.len() == 0)
		{
			editorWarning("Input signallers required");
			::killEntity(this);
			return;
		}
		
		if (count == 0)
		{
			count = signallers.len();
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Inherited Methods
	
	function _triggerEnter(p_entity, p_parent)
	{
		if (isEnabled() == false)
		{
			return;
		}
		
		++_currentCount;
		if (_currentCount == count)
		{
			base._triggerEnter(this, p_parent);
			base._triggerExit(this, p_parent);
			_currentCount = 0;
		}
	}
	
	function _triggerExit(p_entity, p_parent)
	{
		// Don't do anything
	}
	
	function createTouchSensor()
	{
		// No touch sensor
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Update
	
	function update(p_deltaTime)
	{
		base.update(p_deltaTime);
		
		if (signallers.len() == 0)
		{
			return;
		}
		
		// Signallers can be spawned later, so keep a watch on them
		local cleanList = [];
		foreach (id in signallers)
		{
			local trigger = ::getEntityByID(id);
			if (trigger != null)
			{
				trigger.addChildTrigger(this);
			}
			else
			{
				cleanList.push(id);
			}
		}
		signallers = cleanList;
	}
}
SignalCountTrigger.setattributes("width", null);
SignalCountTrigger.setattributes("height", null);
SignalCountTrigger.setattributes("radius", null);
SignalCountTrigger.setattributes("type", null);
SignalCountTrigger.setattributes("triggerOnTouch", null);
SignalCountTrigger.setattributes("triggerOnUncull", null);
SignalCountTrigger.setattributes("parentTrigger", null);
SignalCountTrigger.setattributes("triggerFilter", null);
SignalCountTrigger.setattributes("filterAllEntitiesOfSameType", null);
