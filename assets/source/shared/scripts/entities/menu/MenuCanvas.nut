class MenuCanvas extends EntityBase
</
	placeable      = Placeable_Hidden
	collisionRect  = [ 0.0, 0.0, 2.0, 1.0 ]  // center X, center Y, width, height
/>
{
	static c_defaultTitleLabelWidth   = 1.50;
	static c_defaultTitleLabelHeight  = 0.09;
	static c_defaultHeaderLabelWidth  = 0.90;
	static c_defaultHeaderLabelHeight = 0.06;
	static c_defaultTextLabelWidth    = 0.70;
	static c_defaultTextLabelHeight   = 0.04;
	
	// Used for timer magic; make sure length is 2 and don't change/override these values!
	static c_showPresentationPrefix   = "p#";
	static c_showLabelPrefix          = "l#";
	static c_updateTypedTextPrefix    = "u#";
	static c_removePresentationPrefix = "q#";
	static c_removeLabelPrefix        = "r#";
	static c_removeTimeoutPrefix      = "t#";
	static c_removeTimeout            = 5.0; // in seconds, assert when element is not removed after this time
	
	// Creation members
	_width                = null;
	_height               = null;
	_zOffset              = null;
	
	// Internal members
	_labels               = null;
	_presentations        = null;
	_typedTextLabels      = null;
	_timers               = null;
	_delayedPresentations = null;
	_delayedLabels        = null;
	_elements             = null;
	_visible              = true;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		// Make sure the screen covers the entire screenspace area
		setCollisionRectWithVectorRect(::VectorRect(::Vector2(0, 0), _width, _height));
		
		::removeEntityFromWorld(this);
		
		setCanBePaused(false);
		makeScreenSpaceEntity();
		
		_presentations        = {};
		_labels               = {};
		_typedTextLabels      = {};
		_timers               = {};
		_delayedPresentations = {};
		_delayedLabels        = {};
		_elements             = {};
		
		create();
	}
	
	function onSpawn()
	{
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onPresentationObjectCallback(p_object, p_name)
	{
		if (p_name == "remove" || p_name == "stop")
		{
			foreach (id, pres in _presentations)
			{
				if (pres.equals(p_object))
				{
					if (p_name == "stop")
					{
						pres.stop();
					}
					else
					{
						::Hud.destroyElement(pres);
						delete _presentations[id];
						stopTimer(c_removeTimeoutPrefix + id);
					}
					return;
				}
			}
			
			foreach (id, label in _labels)
			{
				if (label.presentation.equals(p_object))
				{
					if (p_name == "stop")
					{
						label.presentation.stop();
					}
					else
					{
						removeLabelNoFade(id);
					}
					return;
				}
			}
		}
		::tt_panic("Failed to handle callback '" + p_name + "'");
	}
	
	function onTimer(p_name)
	{
		if (p_name.len() > 2)
		{
			local action = p_name.slice(0, 2);
			local target = p_name.slice(2);
			switch (action)
			{
			case c_updateTypedTextPrefix:    updateTypedTextLabel(target); return;
			case c_showPresentationPrefix:   showDelayedPresentation(target); return;
			case c_showLabelPrefix:          showDelayedLabel(target); return;
			case c_removePresentationPrefix:
				if (target in _presentations)
				{
					_presentations[target].start("fadeout", ["remove"], false, 1);
					startTimer(c_removeTimeoutPrefix + target, c_removeTimeout);
				}
				break;
			case c_removeLabelPrefix:
				if (target in _labels)
				{
					_labels[target].presentation.start("fadeout", ["remove"], false, 1);
					startTimer(c_removeTimeoutPrefix + target, c_removeTimeout);
				}
				break;
			case c_removeTimeoutPrefix:
				::tt_panic("Failed to remove id '" + target + "'. Endless presentation?");
				break;
				
			default:
				// Don't do anything
				break;
			}
		}
	}
	
	function onDie()
	{
		foreach (element in _elements)
		{
			::killEntity(element);
		}
		_elements.clear();
		
		foreach (pres in _presentations)
		{
			::Hud.destroyElement(pres);
		}
		
		foreach (label in _labels)
		{
			::Hud.destroyTextElement(label);
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Create
	
	function create()
	{
		// Implement this
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods
	
	function isVisible()
	{
		return _visible;
	}
	
	function toggleVisibility()
	{
		_visible ? hide() : show();
	}
	
	function hide()
	{
		_visible = false;
		foreach (element in _elements)
		{
			element.setVisible(false);
		}
		
		foreach (pres in _presentations)
		{
			pres.stop();
			pres.start("fadeout", ["stop"], false, 1);
		}
		
		foreach (label in _labels)
		{
			label.presentation.stop();
			label.presentation.start("fadeout", ["stop"], false, 1);
		}
	}
	
	function show()
	{
		_visible = true;
		foreach (element in _elements)
		{
			element.setVisible(true);
		}
		
		foreach (pres in _presentations)
		{
			pres.stop();
			pres.start("fadein", [], false, 1);
		}
		
		foreach (label in _labels)
		{
			label.presentation.stop();
			label.presentation.start("fadein", [], false, 1);
		}
	}
	
	function showDelayedPresentation(p_id)
	{
		local info = _delayedPresentations[p_id];
		local pres = createPresentation(info.id, info.file, info.position, info.hudAlignment, info.extraTags);
		pres.start("fadein", [], false, 0);
		delete _delayedPresentations[p_id];
		customCallback("onDelayedPresentationShown", p_id);
	}
	
	function showDelayedLabel(p_id)
	{
		local label = _delayedLabels[p_id];
		label.presentation.start("fadein", [], false, 0);
		delete _delayedLabels[p_id];
		customCallback("onDelayedLabelShown", p_id);
	}
	
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Helpers Methods
	
	function close()
	{
		::killEntity(this);
	}
	
	function createElement(p_id, p_type, p_position, p_properties = {})
	{
		if (p_id in _elements)
		{
			::tt_panic("Element with id '" + p_id + "' already exists");
			return;
		}
		
		p_properties._parentMenu <- this.weakref();
		p_properties._id <- p_id;
		local elem = ::spawnEntity(p_type, p_position, p_properties);
		if (elem != null)
		{
			_elements[p_id] <- elem;
		}
		return elem;
	}
	
	function removeElement(p_id)
	{
		if (p_id in _elements)
		{
			local elem = _elements[p_id];
			::killEntity(elem);
			delete _elements[p_id];
			return true;
		}
		
		::tt_panic("Cannot find element with id '" + p_id + "' in menuscreen");
		return false;
	}
	
	function hasElement(p_id)
	{
		return (p_id in _elements);
	}
	
	function createTable(p_id, p_width, p_rowCount, p_position, p_columnProperties, p_glyphSetID = GlyphSetID_Text)
	{
		return createElement(p_id, "GUITable", p_position,
		{
			_width            = p_width,
			_rowCount         = p_rowCount,
			_glyphSetID       = p_glyphSetID,
			_columnProperties = p_columnProperties,
			_zOffset          = _zOffset + 0.005, // in front of other elements but behind text
			_showPanel        = false
		});
	}
	
	function createPanelTable(p_id, p_width, p_rowCount, p_position, p_columnProperties, p_glyphSetID = GlyphSetID_Text)
	{
		return createElement(p_id, "GUITable", p_position,
		{
			_width            = p_width,
			_rowCount         = p_rowCount,
			_glyphSetID       = p_glyphSetID,
			_columnProperties = p_columnProperties,
			_zOffset          = _zOffset + 0.005, // in front of other elements but behind text
			_showPanel        = true
		});
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Presentation Methods
	
	function addPresentationTags(p_pres, p_id = null, p_extraTags = null)
	{
		if (p_id != null)
		{
			p_pres.addTag(p_id);
		}
		
		if (p_extraTags != null)
		{
			foreach (tag in p_extraTags)
			{
				p_pres.addTag(tag);
			}
		}
	}
	
	function createPresentation(p_id, p_file, p_position = ::Vector2(0, 0), p_hudAlignment = HudAlignment.Center, p_extraTags = [])
	{
		local pres = createStoppedPresentation(p_id, p_file, p_position, p_hudAlignment, p_extraTags);
		pres.start("fadein", [], false, 0);
		return pres;
	}
	
	function createPresentationInRenderLayer(p_id, p_file, p_renderLayer, p_position = ::Vector2(0, 0), p_hudAlignment = HudAlignment.Center, p_extraTags = [])
	{
		local pres = createStoppedPresentation(p_id, p_file, p_position, p_hudAlignment, p_extraTags, p_renderLayer);
		pres.start("fadein", [], false, 0);
		return pres;
	}
	
	function createStoppedPresentation(p_id, p_file, p_position = ::Vector2(0, 0),
	                                   p_hudAlignment = HudAlignment.Center, p_extraTags = [],
	                                   p_renderLayer = ParticleLayer_Hud)
	{
		if (p_id in _presentations)
		{
			::tt_panic("Element with id '" + p_id + "' is already added as a presentation");
			return;
		}
		
		local pres = ::Hud.createElementInRenderLayer(this, "presentation/menu/" + p_file, p_hudAlignment, p_renderLayer, HudLayer.Menu);
		pres.setCustomTranslation(getPosition() + p_position);
		pres.setCustomZOffset(_zOffset);
		addPresentationTags(pres, p_id, p_extraTags);
		_presentations[p_id] <- pres;
		return pres;
	}
	
	function createDelayedPresentation(p_delay, p_id, p_file, p_position = ::Vector2(0, 0), p_hudAlignment = HudAlignment.Center, p_extraTags = [])
	{
		_delayedPresentations[p_id] <- { id = p_id, file = p_file, position = p_position,
		                                 hudAlignment = p_hudAlignment, extraTags = p_extraTags };
		startTimer(c_showPresentationPrefix + p_id, p_delay);
	}
	
	function hasPresentation(p_id)
	{
		return p_id in _presentations;
	}
	
	function removePresentation(p_id, p_delay = 0.0)
	{
		local id = c_removePresentationPrefix + p_id;
		startTimer(id, p_delay);
	}
	
	function removePresentationNoFade(p_id)
	{
		if (p_id in _presentations)
		{
			::Hud.destroyElement(_presentations[p_id]);
			delete _presentations[p_id];
			stopTimer(c_removeTimeoutPrefix + p_id);
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Title and Header Methods
	
	function createTitle(p_locID)
	{
		return createLabel("title", p_locID, ::Vector2(0, 0.4), ::TextColors.Light,
		                   c_defaultTitleLabelWidth, c_defaultTitleLabelHeight,
		                   GlyphSetID_Title, HorizontalAlignment_Center, VerticalAlignment_Center);
	}
	
	function createHeader(p_id, p_locID, p_position, p_color,
	                      p_alignment = HorizontalAlignment_Center)
	{
		return createLabel(p_id, p_locID, p_position, p_color,
		                   c_defaultHeaderLabelWidth, c_defaultHeaderLabelHeight,
		                   GlyphSetID_Header, p_alignment, VerticalAlignment_Center);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Text  / Typed Text / Delayed Text Methods
	
	function createText(p_id, p_locID, p_position, p_color,
	                    p_alignment = HorizontalAlignment_Center)
	{
		return createLabel(p_id, p_locID, p_position, p_color,
		                   c_defaultTextLabelWidth, c_defaultTextLabelHeight,
		                   GlyphSetID_Text, p_alignment, VerticalAlignment_Center);
	}
	
	function createTextWithGlyphSet(p_id, p_locID, p_position, p_color, p_glyphSet,
	                                p_alignment = HorizontalAlignment_Center)
	{
		return createLabel(p_id, p_locID, p_position, p_color,
		                   c_defaultTextLabelWidth, c_defaultTextLabelHeight,
		                   p_glyphSet, p_alignment, VerticalAlignment_Center);
	}
	
	function createDelayedText(p_delay, p_id, p_locID, p_position, p_color,
	                           p_alignment = HorizontalAlignment_Center)
	{
		return createDelayedLabel(p_delay, p_id, p_locID, p_position, p_color,
		                          c_defaultTextLabelWidth, c_defaultTextLabelHeight,
		                          GlyphSetID_Text, p_alignment, VerticalAlignment_Center);
	}
	
	function createTypedText(p_id, p_locID, p_position, p_color,
	                         p_alignment = HorizontalAlignment_Center)
	{
		return createDelayedTypedTextLabel(0.0, p_id, p_locID, p_position, p_color,
		                                   c_defaultTextLabelWidth, c_defaultTextLabelHeight,
		                                   GlyphSetID_Text, p_alignment, VerticalAlignment_Center);
	}
	
	function createDelayedTypedText(p_delay, p_id, p_locID, p_position, p_color,
	                                p_alignment = HorizontalAlignment_Center)
	{
		return createDelayedTypedTextLabel(p_delay, p_id, p_locID, p_position, p_color,
		                                   c_defaultTextLabelWidth, c_defaultTextLabelHeight,
		                                   GlyphSetID_Text, p_alignment, VerticalAlignment_Center);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// TextArea  / Typed TextArea / Delayed TextArea Methods
	
	function createTextArea(p_id, p_locID, p_position, p_color, p_width, p_height,
	                        p_horizontalAlignment = HorizontalAlignment_Left,
	                        p_verticalAlignment   = VerticalAlignment_Top)
	{
		return createLabel(p_id, p_locID, p_position, p_color, p_width, p_height,
		                   GlyphSetID_Text, p_horizontalAlignment, p_verticalAlignment);
	}
	
	function createDelayedTextArea(p_delay, p_id, p_locID, p_position, p_color, p_width, p_height,
	                               p_horizontalAlignment = HorizontalAlignment_Left,
	                               p_verticalAlignment   = VerticalAlignment_Top)
	{
		return createDelayedLabel(p_delay, p_id, p_locID, p_position, p_color, p_width, p_height,
		                          GlyphSetID_Text, p_horizontalAlignment, p_verticalAlignment);
	}
	
	function createTypedTextArea(p_id, p_locID, p_position, p_color, p_width, p_height,
	                             p_horizontalAlignment = HorizontalAlignment_Left,
	                             p_verticalAlignment   = VerticalAlignment_Top)
	{
		return createDelayedTypedTextLabel(0.0, p_id, p_locID, p_position, p_color, p_width, p_height,
		                                   GlyphSetID_Text, p_horizontalAlignment, p_verticalAlignment);
	}
	
	function createDelayedTypedTextArea(p_delay, p_id, p_locID, p_position, p_color, p_width, p_height,
	                                    p_horizontalAlignment = HorizontalAlignment_Left,
	                                    p_verticalAlignment   = VerticalAlignment_Top)
	{
		return createDelayedTypedTextLabel(p_delay, p_id, p_locID, p_position, p_color, p_width, p_height,
		                                   GlyphSetID_Text, p_horizontalAlignment, p_verticalAlignment);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// The core label methods
	
	function createStoppedLabel(p_id, p_locID, p_position, p_color, p_width, p_height, p_font,
	                            p_horizontalAlignment = HorizontalAlignment_Center,
	                            p_verticalAlignment   = VerticalAlignment_Center,
	                            p_extraTags = [])
	{
		if (p_id in _labels)
		{
			::tt_panic("Element with id '" + p_id + "' is already added as a label");
			return;
		}
		
		// check if specific exitst
		local presFile = "menu/" + getType().tolower() + "_text";
		if (::presentationFileExists(presFile) == false)
		{
			// default to generic menuscreen_text
			presFile = "menu/menuscreen_text";
		}
		
		local label = ::Hud.createTextElement(this,
		{
			locID               = p_locID == null ? "" : p_locID,
			width               = ::min(_width, p_width),
			height              = ::min(_height, p_height),
			glyphset            = p_font,
			position            = p_position + getPosition(),
			zOffset             = _zOffset + 0.01, // text should be in front of presentations
			hudalignment        = HudAlignment.Center,
			color               = p_color,
			horizontalAlignment = p_horizontalAlignment,
			verticalAlignment   = p_verticalAlignment,
			presentation        = presFile,
			layer               = HudLayer.Menu,
			autostart           = false
		});
		addPresentationTags(label.presentation, p_id, p_extraTags);
		_labels[p_id] <- label;
		return label;
	}
	
	function createLabel(p_id, p_locID, p_position, p_color, p_width, p_height, p_font,
	                     p_horizontalAlignment = HorizontalAlignment_Center,
	                     p_verticalAlignment   = VerticalAlignment_Center,
	                     p_extraTags = [])
	{
		return createDelayedLabel(0.0, p_id, p_locID, p_position, p_color, p_width, p_height, p_font,
		                          p_horizontalAlignment, p_verticalAlignment, p_extraTags);
	}
	
	function createDelayedLabel(p_delay, p_id, p_locID, p_position, p_color, p_width, p_height, p_font,
	                            p_horizontalAlignment = HorizontalAlignment_Center,
	                            p_verticalAlignment   = VerticalAlignment_Center,
	                            p_extraTags = [])
	{
		local label = createStoppedLabel(p_id, p_locID, p_position, p_color, p_width, p_height,
		                                 p_font, p_horizontalAlignment, p_verticalAlignment, p_extraTags);
		
		if (p_delay > 0.0)
		{
			_delayedLabels[p_id] <- label;
			startTimer(c_showLabelPrefix + p_id, p_delay);
		}
		else
		{
			label.presentation.start("fadein", [], false, 0);
		}
		return label;
	}
	
	function createDelayedTypedTextLabel(p_delay, p_id, p_locID, p_position, p_color, p_width, p_height, p_font,
	                                     p_horizontalAlignment = HorizontalAlignment_Center,
	                                     p_verticalAlignment   = VerticalAlignment_Center,
	                                     p_extraTags = [])
	{
		local label = createDelayedLabel(p_delay, p_id, p_locID, p_position, p_color, p_width, p_height,
		                                 p_font, p_horizontalAlignment, p_verticalAlignment, p_extraTags);
		
		_typedTextLabels[p_id] <- label.label;
		label.label.setNumberOfVisibleCharacters(0);
		startTimer(c_updateTypedTextPrefix + p_id, p_delay);
		return label;
	}
	
	function hasLabel(p_id)
	{
		return p_id in _labels;
	}
	
	function removeLabel(p_id, p_delay = 0.0)
	{
		local id = c_removeLabelPrefix + p_id;
		startTimer(id, p_delay);
	}
	
	function removeLabelNoFade(p_id)
	{
		if (p_id in _labels)
		{
			::Hud.destroyTextElement(_labels[p_id]);
			delete _labels[p_id];
			stopTimer(c_removeTimeoutPrefix + p_id);
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Other Methods
	
	function instantlyShowAllElements()
	{
		local maxDepth = 10;
		if (_delayedLabels.len() == 0 && _delayedPresentations.len() == 0 && _typedTextLabels.len() == 0)
		{
			return true;
		}
		
		for (local i = 0; i < maxDepth; ++i)
		{
			_timers.clear();
			{
				local cleanLabels = clone _delayedLabels;
				_delayedLabels.clear();
				foreach (id, label in cleanLabels)
				{
					label.presentation.start("fadein", [], false, 0);
					customCallback("onDelayedLabelShown", id);
				}
			}
			
			{
				local cleanPresentations = clone _delayedPresentations;
				_delayedPresentations.clear();
				foreach (id, pres in cleanPresentations)
				{
					pres.start("fadein", [], false, 0);
					customCallback("onDelayedPresentationShown", id);
				}
			}
			
			{
				local cleanLabels = clone _typedTextLabels;
				_typedTextLabels.clear();
				foreach (id, label in cleanLabels)
				{
					label.setNumberOfVisibleCharacters(-1);
					customCallback("onTextLabelTyped", id);
				}
			}
			
			if (_timers.len() == 0)
			{
				::Audio.playGlobalSoundEffect("Effects", "key_enter");
				return false;
			}
		}
		
		::tt_panic("Max callback depth reached and still there are elements queued");
		return true;
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Timer Emulation
	
	function startTimer(p_name, p_delay)
	{
		_timers[p_name] <- p_delay;
	}
	
	function stopTimer(p_name)
	{
		if (p_name in _timers)
		{
			delete _timers[p_name];
		}
	}
	
	function stopAllTimers()
	{
		_timers.clear();
	}
	
	function hasTimer(p_name)
	{
		return p_name in _timers;
	}
	
	function suspendTimer(p_name)
	{
		::tt_panic("Not implemented yet");
	}
	
	function resumeTimer(p_name)
	{
		::tt_panic("Not implemented yet");
	}
	
	function getTimerTimeout(p_name)
	{
		if (p_name in _timers)
		{
			return _timers[p_name];
		}
		return -1.0;
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Update Helpers
	
	function update(p_deltaTime)
	{
		if (_timers == null || _visible == false || ::Game.isInBackGround())
		{
			// Before onInit()
			return;
		}
		
		// p_deltaTime 
		if (p_deltaTime == 0)
		{
			p_deltaTime = 1.0 / 60.0;
		}
		
		local queue = [];
		local cleanTimers = {};
		foreach (name, delay in _timers)
		{
			delay -= p_deltaTime;
			
			if (delay <= 0.0)
			{
				queue.push(name);
			}
			else
			{
				cleanTimers[name] <- delay;
			}
		}
		_timers = cleanTimers;
		
		// Fire the callbacks at the very latest to prevent messing around with the _timers table
		foreach (name in queue)
		{
			customCallback("onTimer", name);
		}
	}
	
	function updateTypedTextLabel(p_id)
	{
		local label = _typedTextLabels[p_id];
		::null_assert(label);
		if (label != null)
		{
			local delay = 0.3;
			if (label.getNumberOfVisibleCharacters() < label.getTextLength())
			{
				local index = label.getNumberOfVisibleCharacters();
				local char = label.getWCharAtIndex(index);
				switch (char)
				{
				case ' ': delay = ::frnd_minmax(0.08, 0.10);
					::Audio.playGlobalSoundEffect("Effects", "key_space");
					::setRumble(RumbleStrength_Low, 0.0);
					break;
					
				case '\n': delay = ::frnd_minmax(0.10, 0.20);
					::Audio.playGlobalSoundEffect("Effects", "key_enter");
					::setRumble(RumbleStrength_Low, 0.0);
					break;
					
				default: delay = ::frnd_minmax(0.03, 0.05);
					::Audio.playGlobalSoundEffect("Effects", "key_normal");
					::setRumble(RumbleStrength_Low, 0.0);
					break;
				}
				
				++index;
				label.setNumberOfVisibleCharacters(index);
				if (index == label.getTextLength())
				{
					// Delay before enter sound effect
					delay = ::frnd_minmax(0.2, 0.3);
				}
				startTimer(c_updateTypedTextPrefix + p_id, delay);
			}
			else
			{
				// Done; remove from _typedTextLabels
				::setRumble(RumbleStrength_Low, 0.0);
				::Audio.playGlobalSoundEffect("Effects", "key_enter");
				delete _typedTextLabels[p_id];
				customCallback("onTextLabelTyped", p_id);
			}
		}
	}
}
