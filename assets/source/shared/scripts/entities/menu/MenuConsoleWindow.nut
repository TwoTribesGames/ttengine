include_entity("menu/MenuWindow");

class MenuConsoleWindow extends MenuWindow
</
	placeable      = Placeable_Hidden
	collisionRect  = [ 0.0, 0.0, 6.0, 4.0 ]
/>
{
	static c_blinkTimeout = 0.2;
	static c_lineSpacing   = 0.04;
	
	_lineCount   = 7;
	_queue      = null;
	_index      = 0;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		_queue = [];
		
		_height = (_lineCount * c_lineSpacing) + c_margin * 2;
		
		base.onInit();
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onTimer(p_name)
	{
		base.onTimer(p_name);
		
		if (p_name == "process")
		{
			local command = _queue[_index];
			_showCurrentCommand();
		}
		else if (p_name ==  "nextCommand")
		{
			++_index;
			_processTop();
		}
		else if (p_name == "blink")
		{
			local prefix = ::getLocalizedStringUTF8("CONSOLE_PREFIX");
			local label = _getCurrentLabel();
			if (label.getNumberOfVisibleCharacters() == prefix.len())
			{
				label.setTextUTF8(prefix + ::getLocalizedStringUTF8("CONSOLE_CURSOR"));
			}
			else
			{
				label.setTextUTF8(prefix);
			}
			startTimer("blink", c_blinkTimeout);
		}
	}
	
	function onTextLabelTyped(p_id)
	{
		// Don't forward to base otherwise multiple callbacks will be fired at same time
		// onConsoleWindowCommandTyped replaces onTextLabelTyped
		
		if (_parent)
		{
			_parent.customCallback("onConsoleWindowCommandTyped", this, _queue[_index][2]);
		}
		
		_queueNextCommand();
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Inherited Methods
	
	function addCommands(p_commands)
	{
		_queue.extend(p_commands);
		if (hasTimer("process") == false)
		{
			_processTop();
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// 'Private' Methods

	function _queueNextCommand()
	{
		local command = _queue[_index];
		startTimer("nextCommand", command[3]);
	}
	
	function _getCurrentLabel()
	{
		return _labels[_index.tostring()].label;
	}

	function _createNewLine(p_locID)
	{
		local yPos = _index < _lineCount ? _index * c_lineSpacing : _lineCount * c_lineSpacing ;
		createTextArea(_index.tostring(), p_locID, ::Vector2(0, -yPos), ::TextColors.Light,
			_width, c_defaultTextLabelHeight);
		if (_index >= _lineCount)
		{
			local lbound = (_index-_lineCount);
			for (local i = _index; i > lbound; --i)
			{
				local pos = _labels[(i-1).tostring()].presentation.getCustomTranslation();
				_labels[i.tostring()].presentation.setCustomTranslation(pos);
			}
			// Delete the row at lbound
			removeLabelNoFade(lbound.tostring())
		}
	}
	
	function _createCursor()
	{
		_createNewLine("CONSOLE_PREFIX");
		startTimer("blink", c_blinkTimeout);
	}
	
	function _showCurrentCommand()
	{
		if (_queue.len() == 0)
		{
			return;
		}
		
		if (_index >= _queue.len())
		{
			::tt_panic("Invalid index");
			return;
		}
		
		local command = _queue[_index];
		if (command[1] == ConsoleCommandType.Command)
		{
			local id = _index.tostring();
			local label = _getCurrentLabel();
			local prefix = ::getLocalizedStringUTF8("CONSOLE_PREFIX");
			label.setTextUTF8(prefix + ::getLocalizedStringUTF8(command[2]));
			label.setNumberOfVisibleCharacters(prefix.len());
			_typedTextLabels[id] <- label;
			startTimer(c_updateTypedTextPrefix + id, 0.0);
			stopTimer("blink");
		}
		else
		{
			_createNewLine(command[2]);
			if (_parent)
			{
				_parent.customCallback("onConsoleWindowResultShown", this, _queue[_index][2]);
			}
			_queueNextCommand();
		}
	}
	
	function _processTop()
	{
		if (_index >= _queue.len())
		{
			return;
		}
		
		startTimer("process", _queue[_index][0]);
		if (_queue[_index][1] == ConsoleCommandType.Command)
		{
			_createCursor();
		}
	}
}
