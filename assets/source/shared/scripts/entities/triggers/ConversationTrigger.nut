include_entity("rewind/conversation/ConversationMgr");
include_entity("triggers/Trigger");

class ConversationTrigger extends Trigger
</
	editorImage    = "editor.conversationtrigger"
	libraryImage   = "editor.library.conversationtrigger"
	placeable      = Placeable_Developer
	sizeShapeColor = Colors.Trigger
	displayName    = "Trigger - Conversation"
	group          = "04.3 Conversation Triggers"
	stickToEntity  = true
/>
{
	</
		type           = "string"
		choice         = ::getAllConversationNames()
		group          = "Specific Settings"
		order          = 0
	/>
	conversation = null;
	
	</
		type           = "entityid_array"
		filter         = ["RewindEntity"]
		order          = 1
		group          = "Specific Settings"
		referenceColor = ReferenceColors.List
		description    = "The actors of this entity. Actor 0 will always be the player and should not be selected."
	/>
	actors = null;
	
	</
		type           = "entityid_array"
		filter         = ["Trigger", "RewindEntity"]
		order          = 2
		group          = "Specific Settings"
		referenceColor = ReferenceColors.List
		description    = "The triggers of this entity"
	/>
	triggers = null;
	
	// Usually we only want this conversation to be displayed once
	once = true;
	
	// Constants
	static c_defaultDelay    = 0.01;
	static c_defaultDuration = 0.01;
	static c_defaultEmotion  = "neutral";
	
	_conversationData   = null;
	_startSequence      = null;
	_abortSequence      = null;
	_currentSequenceIdx = null;
	_actualActors       = null;
	_actualTriggers     = null;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onSpawn()
	{
		base.onSpawn();
		
		if (conversation == null)
		{
			editorWarning("Missing conversation");
			::killEntity(this);
			return;
		}
		
		_conversationData = ::getJSONFromFile("conversations/" + conversation + ".json");
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onTriggerEnterFirst(p_entity)
	{
		// Now extend actors with player
		local player = ::getFirstEntityByTag("PlayerBot");
		if (player == null)
		{
			editorWarning("Missing player");
			::killEntity(this);
			return;
		}
		
		if (actors == null)
		{
			actors = [player.getID()];
		}
		else
		{
			actors.insert(0, player.getID());
		}
		
		// Convert actor-ids to real entities
		_actualActors = [];
		foreach (id in actors)
		{
			local actor = ::getEntityByID(id);
			if (actor == null)
			{
				::tt_warning("Conversation uses non-existent actor with ID '" + id +
				             "'. Has it been spawned yet? If it was killed, ignore this warning!");
				return;
			}
			else if ("_healthBar" in actor && actor._healthBar != null)
			{
				actor.subscribeDeathEvent(this, "onActorDied");
			}
			_actualActors.push(actor);
		}
		
		// Convert trigger-ids to real entities
		_actualTriggers = [];
		if (triggers != null)
		{
			foreach (id in triggers)
			{
				local trigger = ::getEntityByID(id);
				if (trigger == null)
				{
					::tt_panic("Conversation uses non-existent trigger with ID '" + id + "'. Has it been spawned yet?");
				}
				_actualTriggers.push(trigger);
			}
		}
		
		if ("start" in _conversationData)
		{
			_startSequence = _conversationData.start;
			if (typeof(_startSequence) != "array")
			{
				::tt_panic("conversation '" + conversation + ".json' has enter field that with incorrect type: " + 
				         typeof(_startSequence) + ". It should be an array.");
				_startSequence = null;
			}
		}
		
		if ("abort" in _conversationData)
		{
			_abortSequence = _conversationData.abort.weakref();
			if (typeof(_abortSequence) != "array")
			{
				::tt_panic("conversation '" + conversation + ".json' has exit field that with incorrect type: " + 
				typeof(_abortSequence) + ". It should be an array.");
				_abortSequence = null;
			}
		}
		
		// Do validation
		if (_startSequence != null)
		{
			local i = 1;
			foreach (section in _startSequence)
			{
				validateSection(section, "start", i++);
			}
		}
		
		if (_abortSequence != null)
		{
			local i = 1;
			foreach (section in _abortSequence)
			{
				validateSection(section, "abort", i++);
			}
		}
		
		if (_startSequence != null)
		{
			::ConversationMgr.clear();
			_currentSequenceIdx = ::ConversationMgr.queueSequence(_startSequence, _abortSequence);
		}
	}
	
	function onActorDied(p_entity)
	{
		if (_currentSequenceIdx == null)
		{
			// Conversation not started yet, kill trigger
			::killEntity(this);
			return;
		}
		
		if (::ConversationMgr.getCurrentSequenceIndex() == _currentSequenceIdx)
		{
			::ConversationMgr.playAbortSequence();
		}
		::ConversationMgr.remove(_currentSequenceIdx);
	}
	
	function onDie()
	{
		base.onDie();
		
		foreach (actor in _actualActors)
		{
			if (::isValidEntity(actor) && ("_healthBar" in actor) && actor._healthBar != null)
			{
				actor.unsubscribeDeathEvent(this, "onActorDied");
			}
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods
	
	function validateSection(p_section, p_sequenceName, p_sectionIdx)
	{
		// Trigger
		if ("trigger" in p_section)
		{
			local triggerIdx = p_section.trigger.tointeger();
			if (triggerIdx < 1 || _actualTriggers == null || triggerIdx > _actualTriggers.len())
			{
				if (_actualTriggers == null)
				{
					editorWarning("Invalid trigger '" + triggerIdx + 
					              "' found in section '" + p_sequenceName + "[" + p_sectionIdx + "]'" +
					              "'. No triggers selected.");
				}
				else
				{
					editorWarning("Invalid trigger '" + triggerIdx + 
					              "' found in section '" + p_sequenceName + "[" + p_sectionIdx + "]'" +
					              "'. Should be in range [1.." + _actualTriggers.len() + "]");
				}
				p_section.trigger = null;
			}
			else
			{
				p_section.trigger = _actualTriggers[triggerIdx - 1];
			}
		}
		else
		{
			p_section.trigger <- null;
		}
		
		// Actor
		if ("actor" in p_section)
		{
			local actorIdx = p_section.actor.tointeger();
			if (actorIdx < 0 || actorIdx >= _actualActors.len())
			{
				editorWarning("Invalid actor '" + actorIdx +
				              "' found in section '" + p_sequenceName + "[" + p_sectionIdx + "]'" +
				              "'. Should be in range [0.." + (_actualActors.len()-1) + "], 0 being the player.");
				p_section.actor = _actualActors[0]; // player
			}
			else
			{
				p_section.actor = _actualActors[actorIdx];
			}
		}
		else
		{
			p_section.actor <- _actualActors[0]; // player
		}
		
		if ("textID" in p_section)
		{
			if ("delay" in p_section)              p_section.delay    =  p_section.delay.tofloat();
			else                                   p_section.delay    <- c_defaultDelay;
			if ("duration" in p_section)           p_section.duration =  p_section.duration.tofloat();
			else                                   p_section.duration <- c_defaultDuration;
			if (("emotion" in p_section) == false) p_section.emotion  <- c_defaultEmotion;
		}
		else
		{
			// Without textID, delay and duration defaults are 0
			if ("delay" in p_section)              p_section.delay    =  p_section.delay.tofloat();
			else                                   p_section.delay    <- 0.0;
			if ("duration" in p_section)           p_section.duration =  p_section.duration.tofloat();
			else                                   p_section.duration <- 0.0;
			
			if (("emotion" in p_section) == false)
			{
				p_section.emotion <- null;
			}
			
			if (p_section.emotion != null && p_section.duration <= 0.0)
			{
				editorWarning("Emotion '" + p_section.emotion + "' without textID or duration " + 
					            "found in section '" + p_sequenceName + "[" + p_sectionIdx + "]'");
				p_section.emotion = null;
			}
			p_section.textID <- null;
		}
		
		if ("callback" in p_section)
		{
			if (("name" in p_section.callback) == false)
			{
				editorWarning("Callback missing required 'name' field " + 
				              "in section '" + p_sequenceName + "[" + p_sectionIdx + "]'");
			}
			
			if (("value" in p_section.callback) == false)
			{
				p_section.callback.value <- null;
			}
		}
		else
		{
			p_section.callback <- null;
		}
	}
}
