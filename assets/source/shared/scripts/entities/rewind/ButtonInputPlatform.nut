include_entity("rewind/RewindEntity");

class ButtonInputPlatform extends RewindEntity
</
	placeable      = Placeable_Hidden
	movementset    = "Static"
	collisionRect  = [0.0, 1.0, 2.0, 2.0] // center X, center Y, width, height
/>
{
	static c_width         = 10.0;
	static c_height        = 2.0;
	static c_buttonYOffset = -2.8;

	_centerSensor         = null;
	_buttonPresentation   = null;
	_activator            = null;
	_triggers             = null;
	_presentations        = null;
	_tags                 = null;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		base.onInit();
		
		// Remove from world (no entity interaction)
		::removeEntityFromWorld(this);
		
		setCanBePushed(false);
		
		_triggers = {};
		_presentations = [];
		
		// Make this one seperate as we don't want to restart it every time
		_buttonPresentation = createPresentationObjectInLayer("presentation/hud_buttons", ParticleLayer_BehindHud);
		_buttonPresentation.setCustomTranslation(::Vector2(0.0, c_buttonYOffset));
		
		// Button prompt sensor
		local player = ::getFirstEntityByTag("PlayerBot");
		if (player != null)
		{
			{
				local sensor = addTouchSensor(::BoxShape(c_width, c_height), player);
				sensor.setEnterCallback("onPlatformSensorEnter");
				sensor.setExitCallback("onPlatformSensorExit");
			}
		
			// Center sensor
			{
				_centerSensor = addTouchSensor(::BoxShape(0.5, c_height), player);
				_centerSensor.setEnterCallback("onPlatformCenterSensorEnter");
				_centerSensor.setEnabled(false);
			}
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onStartCinematic()
	{
		createTrigger("DisableControlsTrigger",
		{
			disableAimControls = true,
			stickToEntity      = _activator
		});
	}
	
	function onExitCinematic()
	{
		deleteTrigger("DisableControlsTrigger");
		onPlatformExit();
	}
	
	function onMoveTimeout()
	{
		enableInputListening();
		deleteTrigger("PlayerMoveTrigger");
		_activator.setLookAheadEnabled(true);
		if (_centerSensor != null)
		{
			_centerSensor.setEnabled(false);
		}
	}
	
	function onPlatformSensorEnter(p_entity, p_sensor)
	{
		if (::isValidEntity(p_entity) == false)
		{
			return;
		}
		
		_activator = p_entity;
		
		// Get notified if activator dies
		_activator.subscribeDeathEvent(this, "onActivatorDied");
		
		// Display the hovering A presentation
		enableInputListening();
	}
	
	function onPlatformSensorExit(p_entity, p_sensor)
	{
		if (hasTimer("onMoveTimeout"))
		{
			// Exitting platform while moving to center; abort!
			stopTimer("onMoveTimeout");
			customCallback("onMoveTimeout");
		}
		disableInputListening();
	}
	
	function onPlatformCenterSensorEnter(p_entity, p_sensor)
	{
		deleteTrigger("PlayerMoveTrigger");
		if (_centerSensor != null)
		{
			_centerSensor.setEnabled(false);
		}
		_activator.setLookAheadEnabled(true);
		stopTimer("onMoveTimeout");
		
		customCallback("onStartCinematic");
	}
	
	function onButtonAcceptPressed()
	{
		_activator.customCallback("onPlatformEnter");
		if (_centerSensor != null)
		{
			_centerSensor.removeAllSensedEntities();
			_centerSensor.setEnabled(true);
		}
		
		local moveRight = _activator.getPosition().x < getPosition().x;
		local offset = ::Vector2(0.0, getCenterOffset().y - 1.0); // stupid - 1 is because triggers default have centerpoint at 0, 1
		
		// Stop displaying the hovering A presentation
		_buttonPresentation.start("hide", [], false, 0);
		
		createTrigger("PlayerMoveTrigger",
		{
			height   = c_height,
			width    = c_width,
			rotation = moveRight ? 90 : 270
			enabled  = true
		}, offset);
		
		startCallbackTimer("onMoveTimeout", 2.0);
		disableInputListening();
		
		_activator.setLookAheadEnabled(false);
	}
	
	function onPlatformExit()
	{
		if (_activator != null)
		{
			_activator.customCallback("onPlatformExit");
		}
	}
	
	function onActivatorDied(p_activator)
	{
		disableInputListening();
	}
	
	function onCheckTags()
	{
		local tags = ::getButtonTags();
		
		if (::arraysEqual(tags, _tags) == false)
		{
			_tags = tags;
			_buttonPresentation.clearTags();
			_buttonPresentation.addTag("accept");
			::addTagsToPresentation(_buttonPresentation, _tags);
			_buttonPresentation.start("show", [], false, 0);
		}
		startCallbackTimer("onCheckTags", 1.0);
	}
	
	function onDie()
	{
		base.onDie();
		
		onPlatformExit();
		cleanup();
		disableInputListening();
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods
	
	function showButton()
	{
		onCheckTags();
	}
	
	function hideButton()
	{
		stopTimer("onCheckTags");
		_buttonPresentation.start("hide", [], false, 0);
		_tags = null;
	}
	
	function enableInputListening()
	{
		showButton();
		::addButtonInputListeningEntity(this, InputPriority.Pickup);
	}
	
	function disableInputListening()
	{
		hideButton();
		::removeButtonInputListeningEntity(this);
	}
	
	function createTrigger(p_type, p_props, p_offset = ::Vector2(0, 0))
	{
		if (p_type in _triggers)
		{
			::tt_panic("Already have a trigger with type '" + p_type + "'");
			return;
		}
		
		local trigger = ::spawnEntity(p_type, getPosition() + p_offset, p_props);
		_triggers[p_type] <- trigger;
		return trigger;
	}
	
	function deleteTrigger(p_type)
	{
		if (p_type in _triggers)
		{
			::killEntity(_triggers[p_type]);
			delete _triggers[p_type];
		}
	}
	
	function deleteAllTriggers()
	{
		foreach (trigger in _triggers)
		{
			::killEntity(trigger);
		}
		_triggers.clear();
	}
	
	function addPresentation(p_name, p_layer)
	{
		local pres = createPresentationObjectInLayer("presentation/" + p_name, p_layer);
		_presentations.push(pres);
		return pres;
	}
	
	function startPresentations(p_name, p_tags = null)
	{
		foreach (pres in _presentations)
		{
			pres.start(p_name, p_tags == null ? [] : p_tags, false, 0);
		}
	}
	
	function cleanup()
	{
		// Delete triggers
		deleteAllTriggers();
		
		// A little delay here since jump/weapon selection can also be accept/cancel at same time. Don't let them interfere
		startCallbackTimer("onPlatformExit", 0.1);
	}
}
