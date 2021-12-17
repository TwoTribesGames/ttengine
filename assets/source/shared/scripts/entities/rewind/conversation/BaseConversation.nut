include_entity("rewind/RewindEntity");

class BaseConversation extends RewindEntity
</
	placeable      = Placeable_Hidden
	movementset    = "Static"
	collisionRect  = [ 0.0, 0.0, 0.1, 0.1 ]
/>
{
	// Inherited members
	positionCulling            = false;
	
	// Internal members
	_labels                    = null;
	_background                = null;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		base.onInit();
		
		_background  = createBackground();
		
		createLabels();
		
		::removeEntityFromWorld(this);
	}
	
	function onSpawn()
	{
		base.onSpawn();
		
		if (_background != null)
		{
			_background.start("fadein", [], false, 0);
		}
		
		showLabels();
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onTimer(p_name)
	{
		if (p_name == "timeout")
		{
			removeMe();
		}
		else if (p_name == "remove")
		{
			::tt_panic("Conversation '" + this + "' is removed using a timer. " +
			         "It should be removed on 'fadeout' ended callback of background. Is there a looping frameanim?");
			::killEntity(this);
		}
	}
	
	function onPresentationObjectEnded(p_object, p_name)
	{
		if (p_name == "remove")
		{
			::killEntity(this);
		}
	}
	
	function onDie()
	{
		base.onDie();
		
		destroyBackground();
		destroyLabels();
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods
	
	function createBackground()
	{
		// Implement this
		return null;
	}
	
	function destroyBackground()
	{
		// Implement this
	}
	
	function createLabels()
	{
		// Implement this
	}
	
	function destroyLabels()
	{
		// Implement this
	}
	
	function showLabels(p_tags = [])
	{
		if (_labels != null)
		{
			foreach (label in _labels)
			{
				label.presentation.start("fadein", p_tags, false, 1);
			}
		}
	}
	
	function hideLabels()
	{
		if (_labels != null)
		{
			foreach (label in _labels)
			{
				label.presentation.start("fadeout", [], false, 1);
			}
		}
	}
	
	function removeMe()
	{
		if (hasTimer("remove"))
		{
			// Already removing
			return;
		}
		
		if (_background != null)
		{
			_background.startEx("fadeout", [], false, 1, getPresentationCallback("remove"));
		}
		
		hideLabels();
		
		// Backup remove time. If the fadeout callback isn't working in a timely manner, the conversation
		// will forcibly be removed. This causes a panic to inform the designer.
		startTimer("remove", 5.0);
	}
}
