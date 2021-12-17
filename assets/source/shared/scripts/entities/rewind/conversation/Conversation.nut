include_entity("rewind/conversation/BaseConversation");

class Conversation extends BaseConversation
</
	placeable      = Placeable_Hidden
	movementset    = "Static"
	collisionRect  = [ 0.0, 0.0, 0.1, 0.1 ]
	group          = "Rewind"
	stickToEntity  = true
/>
{
	static c_voiceOverDelay       = 0.2;
	static c_voiceOverEndFadeTime = 0.2;
	static c_labelWidth           = 12;
	static c_labelHeight          = 2.0;
	
	// Creation members
	_textID                    = null;
	_timeout                   = 7.0; // use <= 0 for infinite
	_tags                      = null;
	_voiceOverEffect           = null;
	_parent                    = null;
	_shouldUnduckVolume        = false;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		if (_tags == null)
		{
			_tags = [];
		}
		
		if (stickToEntity instanceof ::PlayerBot)
		{
			_tags.push("player");
		}
		
		if (stickToEntity != null)
		{
			stickToEntity.subscribeDeathEvent(this, "onActorDied");
		}
		
		base.onInit();
	}
	
	function onSpawn()
	{
		base.onSpawn();
		
		startCallbackTimer("onVoiceOver", c_voiceOverDelay);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onActorDied(p_actor)
	{
		removeMe();
	}
	
	function onVoiceOver()
	{
		_voiceOverEffect = ::Audio.playGlobalSoundEffect("VoiceOver", _textID);
		if (_voiceOverEffect != null)
		{
			if (_voiceOverEffect.isPlaying())
			{
				// Only duck music when VO is actually audible
				if (::Audio.getUserSettingVolumeSfx("VoiceOver").tointeger() > 0)
				{
					::Audio.duckVolume(DuckingPreset.Conversation);
					_shouldUnduckVolume = true;
				}
			}
			else
			{
				_voiceOverEffect = null;
				
				// No VO, use timeout
				if (_timeout > 0.0)
				{
					startTimer("timeout", _timeout);
				}
			}
		}
	}
	
	function onDie()
	{
		base.onDie();
		
		if (stickToEntity != null)
		{
			stickToEntity.unsubscribeDeathEvent(this, "onActorDied");
		}
		
		if (_shouldUnduckVolume)
		{
			::Audio.unduckVolume(DuckingPreset.Conversation);
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Inherited methods
	
	function removeMe()
	{
		stopTimer("onVoiceOver");
		if (_voiceOverEffect != null)
		{
			_voiceOverEffect.fadeAndStop("Volume", 0.0, c_voiceOverEndFadeTime);
			_voiceOverEffect = null;
		}
		
		if (hasTimer("remove"))
		{
			// Already removing
			return;
		}
		
		if (_parent != null)
		{
			_parent.customCallback("onConversationRemoved", this);
		}
		
		base.removeMe();
	}
	
	function createBackground()
	{
		local pres = createPresentationObjectInLayer("presentation/conversation_balloon", ParticleLayer_BehindHud);
		::addTagsToPresentation(pres, _tags);
		return pres;
	}
	
	function destroyBackground()
	{
		// Cleaned up automatically when entity dies
	}
	
	function createLabels()
	{
		local pres = createPresentationObjectInLayer("presentation/conversation_text", ParticleLayer_BehindHud);
		local label = addTextLabel(_textID, c_labelWidth, c_labelHeight, GlyphSetID_Text);
		label.setHorizontalAlignment(HorizontalAlignment_Center);
		label.setVerticalAlignment(VerticalAlignment_Center);
		label.setColor(ColorRGBA(220,220,220,255));
		label.addDropShadow(::Vector2(0.0, -0.05), ::ColorRGBA(0,0,0,128));
		local size = label.getUsedSize();
		pres.addCustomValue("labelWidth", size.getWidth());
		pres.addCustomValue("labelHeight", size.getHeight());
		pres.addTextLabel(label, ::Vector2(0, 0));
		::addTagsToPresentation(pres, _tags);
		
		// Single label in array
		_labels = [{ label = label, presentation = pres }];
	}
	
	function destroyLabels()
	{
		// Cleaned up automatically when entity dies
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Update
	
	function update(p_deltaTime)
	{
		base.update(p_deltaTime);
		
		// Make sure conversation is always oriented upright
		local rot = ::Camera.getCurrentRotation();
		
		foreach (label in _labels)
		{
			label.presentation.setCustomRotation(-rot);
		}
		
		if (_background != null)
		{
			_background.setCustomRotation(-rot);
		}
		
		if (_voiceOverEffect != null && _voiceOverEffect.isPaused() == false && _voiceOverEffect.isPlaying() == false)
		{
			// VO finished; remove this
			_voiceOverEffect = null;
			startTimer("timeout", 0.01);
		}
	}
}
