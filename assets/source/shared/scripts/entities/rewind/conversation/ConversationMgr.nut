::g_conversationMgr <- null;

class ConversationMgr extends EntityBase
</
	placeable      = Placeable_Hidden
	movementset    = "Static"
	collisionRect  = [ 0.0, 0.0, 0.1, 0.1 ]
/>
{
	_queue                = null;
	_sequenceIdx          = null;
	_currentSection       = null;
	_currentSequenceIdx   = null;
	_currentSequence      = null;
	_currentAbortSequence = null;
	_currentSectionIdx    = null;
	_currentBalloon       = null;
	_abortSequence        = null;
	_balloonPending       = false;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Singleton Instance Management
	
	function getInstance()
	{
		if (::isValidEntity(::g_conversationMgr) == false)
		{
			::g_conversationMgr = ::spawnEntity("ConversationMgr", ::Vector2(0, 0));
		}
		return ::g_conversationMgr;
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		::removeEntityFromWorld(this);
		_queue       = [];
		_sequenceIdx = 0;
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onConversationRemoved(p_conversation)
	{
		_currentBalloon = null;
		if (_balloonPending)
		{
			_showBalloon();
		}
		resumeAllTimers();
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// 'Static' Methods
	
	function queueSequence(p_sequence, p_abortSequence = null)
	{
		local instance = getInstance();
		return instance._addToQueue(p_sequence, p_abortSequence);
	}
	
	function playSequenceNext(p_sequence, p_abortSequence = null)
	{
		local instance = getInstance();
		return instance._addNextToQueue(p_sequence, p_abortSequence);
	}

	function queueConversation(p_conversation)
	{
		local instance = getInstance();
		return instance._addToQueue([p_conversation], null);
	}
	
	function getCurrentSequenceIndex()
	{
		local instance = getInstance();
		return instance._currentSequenceIdx;
	}
	
	function remove(p_sequenceIdx)
	{
		local instance = getInstance();
		return instance._removeFromQueue(p_sequenceIdx);
	}
	
	function isPlaying()
	{
		local instance = getInstance();
		return instance._currentSequence != null;
	}
	
	function setAbortSequence(p_sequence)
	{
		local instance = getInstance();
		instance._setAbortSequence(p_sequence);
	}
	
	function playAbortSequence()
	{
		local instance = getInstance();
		instance._playAbortSequence();
	}
	
	function clear()
	{
		local instance = getInstance();
		instance._clearQueue();
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Private Methods (work on instance)
	
	// Add sequence to end of queue; about sequence is be played if current sequence is aborted
	function _addToQueue(p_sequence, p_abortSequence = null)
	{
		_queue.push([_sequenceIdx, p_sequence, p_abortSequence]);
		if (_queue.len() == 1)
		{
			// First element; process it
			_processTop();
		}
		return _sequenceIdx++;
	}
	
	// Add sequence to head of queue
	function _addNextToQueue(p_sequence, p_abortSequence = null)
	{
		if (_currentSequence == null)
		{
			// Nothing currently playing; simply use _addToQueue
			_addToQueue(p_sequence, p_abortSequence);
			return;
		}
		
		_queue.insert(1, [_sequenceIdx, p_sequence, p_abortSequence]);
		return _sequenceIdx++;
	}
	
	function _removeCurrentBalloon()
	{
		if (_currentBalloon != null)
		{
			_currentBalloon.removeMe();
		}
		_currentBalloon = null;
	}
	
	// Immediately clear queue and remove potential active textballoon
	function _clearQueue()
	{
		_stop();
		
		_removeCurrentBalloon();
		
		_queue.clear();
		_currentSection       = null;
		_currentSequenceIdx   = null;
		_currentSequence      = null;
		_currentSectionIdx    = null;
		_currentAbortSequence = null;
	}
	
	// Removes a sequence from the queue. If it is the active queue, the current section is completed first
	function _removeFromQueue(p_sequenceIdx)
	{
		if (_currentSequence != null && _currentSequenceIdx == p_sequenceIdx)
		{
			// If removing current sequence, instantly move to end of sequence so that the sequence ends after current section
			_currentSectionIdx = _currentSequence.len();
			return true;
		}
		else
		{
			// Find sequence in queue
			for (local i = 1; i < _queue.len(); ++i)
			{
				if (_queue[i][0] == p_sequenceIdx)
				{
					delete _queue[i];
					return true;
				}
			}
		}
		return false;
	}
	
	function _playAbortSequence()
	{
		if (_currentAbortSequence != null)
		{
			// Only start new abort sequence when its a different one
			if (_currentSequence != _currentAbortSequence)
			{
				// Store abortSequence as that got lost when clearing the queue
				local abortSequence = _currentAbortSequence;
				_clearQueue();
				_queue.push([_sequenceIdx, abortSequence, null]);
				_processTop();
				
				// Set _currentAbortSequence again so we can check at consecutive calls
				// if it is the same abort sequence
				_currentAbortSequence = abortSequence;
			}
		}
		else
		{
			_clearQueue();
		}
	}
	
	// Processed the sequences at the head of the queue
	function _processTop()
	{
		if (_queue.len() == 0)
		{
			_currentSequenceIdx   = null;
			_currentSequence      = null;
			_currentAbortSequence = null;
			return;
		}
		
		_currentSequenceIdx   = _queue[0][0];
		_currentSequence      = _queue[0][1];
		_currentAbortSequence = _queue[0][2];
		
		_currentSectionIdx  = 0;
		_play();
	}
	
	function _nextSection()
	{
		++_currentSectionIdx;
		_play();
	}
	
	function _nextSequence()
	{
		_stop();
		_removeTop();
		_processTop();
	}
	
	function _removeTop()
	{
		_queue.remove(0);
	}
	
	function _showBalloon()
	{
		suspendAllTimers();
		if (_currentBalloon != null)
		{
			_balloonPending = true;
			return;
		}
		_balloonPending = false;
		
		local actor = _currentSection.actor;
		if (::isValidEntity(actor))
		{
			local tags = _currentSection.emotion == null ? null : [_currentSection.emotion];
			_currentBalloon = ::spawnEntity("Conversation", actor.getCenterPosition(),
			{
				stickToEntity = actor,
				_textID       = _currentSection.textID,
				_timeout      = _currentSection.duration,
				_tags         = tags,
				_parent       = this.weakref()
			}).weakref();
		}
	}
	
	function _fireTrigger()
	{
		local trigger = _currentSection.trigger;
		if (::isValidEntity(trigger))
		{
			local player = ::getFirstEntityByTag("PlayerBot");
			trigger._triggerEnter(player, this);
			trigger._triggerExit(player, this);
		}
	}
	
	function _stop()
	{
		stopAllTimers();
		_currentSequence    = null;
		_currentSequenceIdx = null;
	}
	
	function _play()
	{
		if (_currentSectionIdx >= _currentSequence.len())
		{
			// sequence done; move to next element in queue
			_nextSequence();
			return;
		}
		
		_currentSection = _currentSequence[_currentSectionIdx];
		
		local delay    = _currentSection.delay;
		local duration = _currentSection.duration; 
		
		// Check for text
		if (_currentSection.textID != null)
		{
			startCallbackTimer("_showBalloon", delay);
		}
		
		// Ensure timeout always happen at least one frame AFTER the delay and schedule _nextSection BEFORE
		// firing the trigger, as the trigger might kill the current conversation
		local timeout = delay + duration;
		if (timeout <= delay) timeout = delay + 0.02;
		
		startCallbackTimer("_nextSection", timeout);
		
		local actor = _currentSection.actor;
		if (::isValidEntity(actor))
		{
			// Set emotion in actor through callback
			if (_currentSection.emotion != null)
			{
				actor.customCallback("onConversationCallback", "setEmotion", _currentSection.emotion);
			}
			
			// Handle callback logic
			if (_currentSection.callback != null)
			{
				actor.customCallback("onConversationCallback",
					_currentSection.callback.name, _currentSection.callback.value);
			}
		}
		
		// Handle trigger logic. Do this at the very end to prevent mess because of recursive calls of
		// _play as the trigger itself might call another conversation.
		if (_currentSection.trigger != null)
		{
			startCallbackTimer("_fireTrigger", delay);
		}
		
		if (_currentBalloon != null)
		{
			suspendAllTimers();
		}
	}
}
