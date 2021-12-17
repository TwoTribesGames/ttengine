include_entity("triggers/Trigger");

class ButtonPromptTrigger extends Trigger
</
	editorImage    = "editor.buttonprompttrigger"
	libraryImage   = "editor.library.buttonprompttrigger"
	placeable      = Placeable_Developer
	collisionRect  = [ 0.0, 1.0, 2.0, 2.0 ]
	sizeShapeColor = Colors.Trigger
	displayName    = "Trigger - Button Prompt"
	group          = "04. Input Triggers"
	stickToEntity  = true
/>
{
	</
		type           = "string"
		choice         = ["accept", "cancel", "virusUpload", "jump", "primaryFire", "secondaryFire", "virusAim"]
		group          = "Triggers"
		group          = "Specific Settings"
		order          = 0.1
	/>
	buttonType = "accept";
	
	// Usually we only want this conversation to be displayed once
	once = true;
	
	_confirmButton = null;
	_activator     = null;
	_tags          = null;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		base.onInit();
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onButtonAcceptPressed()
	{
		return handlePress("accept");
	}
	
	function onButtonCancelPressed()
	{
		return handlePress("cancel");
	}
	
	function onButtonJumpPressed()
	{
		handlePress("jump");
		return false;
	}
	
	function onButtonVirusUploadReleased(p_duration)
	{
		handlePress("virusUpload");
		return false;
	}
	
	function onButtonPrimaryFirePressed()
	{
		handlePress("primaryFire");
		return false;
	}
	
	function onButtonSecondaryFirePressed()
	{
		handlePress("secondaryFire");
		return false;
	}
	
	function onButtonSelectWeapon1Pressed()
	{
		return isAcceptOrCancelButton();
	}
	
	function onButtonSelectWeapon2Pressed()
	{
		return isAcceptOrCancelButton();
	}
	
	function onButtonSelectWeapon3Pressed()
	{
		return isAcceptOrCancelButton();
	}
	
	function onButtonSelectWeapon4Pressed()
	{
		return isAcceptOrCancelButton();
	}
	
	function onButtonToggleWeaponsPressed()
	{
		return isAcceptOrCancelButton();
	}
	
	function onActivatorDied(p_activator)
	{
		disableInputListening();
	}
	
	function onDie()
	{
		base.onDie();
		
		if (_isListeningForInput)
		{
			disableInputListening();
		}
	}
	
	function onCheckTags()
	{
		local tags = ::getButtonTags();
		
		if (::arraysEqual(tags, _tags) == false)
		{
			_tags = tags;
			_confirmButton.clearTags();
			_confirmButton.addTag(buttonType);
			::addTagsToPresentation(_confirmButton, _tags);
			_confirmButton.start("show", [], false, 1);
		}
		startCallbackTimer("onCheckTags", 1.0);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Inherited Methods
	
	function _triggerEnter(p_entity, p_parent)
	{
		// Don't forward to base; trigger will trigger when pressing accept button
		if (::isValidEntity(p_entity) == false)
		{
			return;
		}
		_activator  = p_entity;
		_confirmButton = _activator.createPresentationObjectInLayer("presentation/hud_buttons", ParticleLayer_BehindHud);
		_confirmButton.setAffectedByOrientation(false);
		_confirmButton.setCustomTranslation(::Vector2(0, -1.5));
		
		// Get notified if activator (which should be a RewindEntity) dies
		_activator.subscribeDeathEvent(this, "onActivatorDied");
		
		::addButtonInputListeningEntity(this, InputPriority.Pickup);
		onCheckTags();
	}
	
	function _triggerExit(p_entity, p_parent)
	{
		// Don't forward to base; trigger will trigger when pressing accept button
		if (::isValidEntity(_activator))
		{
			_activator.unsubscribeDeathEvent(this, "onActivatorDied")
		}
		stopTimer("onCheckTags");
		_tags = null;
		disableInputListening();
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods
	
	function isAcceptOrCancelButton()
	{
		return buttonType == "accept" || buttonType == "cancel";
	}
	
	function handlePress(p_type)
	{
		if (p_type == buttonType && _activator != null)
		{
			// Here we trigger the trigger
			base._triggerEnter(_activator, this);
			base._triggerExit(_activator, this);
			return true;
		}
		return false;
	}
	
	function disableInputListening()
	{
		_activator = null;
		// Stop displaying the button
		::removeButtonInputListeningEntity(this);
		_confirmButton.start("hide", [], false, 1);
	}
}
