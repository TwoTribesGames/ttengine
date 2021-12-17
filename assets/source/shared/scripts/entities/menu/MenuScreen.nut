include_entity("menu/MenuCanvas");

class MenuScreen extends MenuCanvas
</
	placeable      = Placeable_Hidden
	collisionRect  = [ 0.0, 0.0, 2.0, 1.0 ]  // center X, center Y, width, height
/>
{
	static c_buttonScale = 0.9; // Required

	// Creation members
	_width   = 2.0;
	_height  = 1.0;
	_zOffset = 0.0;

	_cursor                     = null;
	_windows                    = null;
	_pressedElements            = null;
	_hoveredElement             = null;
	_mouseEnabled               = true;
	_hideVisualsPreviousScreen  = true;
	_screenStack                = []; // static
	_activator                  = null;
	_isClosing                  = false;

	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn

	function onInit()
	{
		_pressedElements      = [];
		_windows              = {};

		base.onInit();

		local returnFocusElement = null;
		if (_screenStack.len() > 0)
		{
			local activeScreen = _screenStack.top().screen;
			returnFocusElement = activeScreen.getFocusElement();
			activeScreen.unfocus(_hideVisualsPreviousScreen);
		}
		_screenStack.push({ screen = this, returnFocusElement = returnFocusElement });

		// Make sure this entity blocks all input
		::addButtonInputBlockingListeningEntity(this, InputPriority.GUI);
		::addMouseInputListeningEntity(this, InputPriority.GUI);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks

	function onProgressRestored(p_id)
	{
		// Menus should never be active when restoring
		closeAll(true);
	}

	function onReloadRequested()
	{
		// Menus should never be active when reloading
		closeAll(true);
	}

	function onDie()
	{
		base.onDie();

		removeCursor();
		::removeButtonInputListeningEntity(this);
		::removeMouseInputListeningEntity(this);

		foreach (window in _windows)
		{
			::killEntity(window);
		}

		local prev = _screenStack.top();
		local unhideVisuals = true;
		if (prev.screen.equals(this))
		{
			local screen = _screenStack.pop().screen;
			unhideVisuals = screen._hideVisualsPreviousScreen;

			if (_screenStack.len() > 0)
			{
				local screen = _screenStack.top().screen;
				screen.refocus(prev.returnFocusElement, unhideVisuals);
			}
			return;
		}
		else
		{
			// Look for screen in stack
			for (local i = 0; i < _screenStack.len()-1; ++i)
			{
				if (_screenStack[i].screen.equals(this))
				{
					_screenStack.remove(i);
					return;
				}
			}
		}
		::tt_panic("Menu deletion problem. Cannot find screen in screenstack");
	}

	function onButtonMenuPressed()
	{
		if (_isClosing == false)
		{
			close();
			_isClosing = true;
		}
	}

	function onButtonCancelPressed()
	{
		if (_isClosing == false)
		{
			close();
			_isClosing = true;
		}
	}

	// Since this entity handles ALL input, make sure to forward the button presses to the cursor
	function onButtonUpPressed()
	{
		if (_cursor != null)
		{
			_cursor.customCallback("onButtonUpPressed");
		}
	}

	function onButtonDownPressed()
	{
		if (_cursor != null)
		{
			_cursor.customCallback("onButtonDownPressed");
		}
	}

	function onButtonLeftPressed()
	{
		if (_cursor != null)
		{
			_cursor.customCallback("onButtonLeftPressed");
		}
	}

	function onButtonRightPressed()
	{
		if (_cursor != null)
		{
			_cursor.customCallback("onButtonRightPressed");
		}
	}

	function onButtonAcceptPressed()
	{
		if (_cursor != null)
		{
			_cursor.customCallback("onButtonAcceptPressed");
		}
	}

	function onButtonAcceptReleased(p_duration)
	{
		if (_cursor != null)
		{
			_cursor.customCallback("onButtonAcceptReleased", p_duration);
		}
	}

	function onPointerPressed(p_event)
	{
		if (_mouseEnabled == false)
		{
			return;
		}

		foreach (element in _elements)
		{
			local rect = element.getCollisionRectWorldSpace();
			if (element._visible && element.c_isSelectable && rect.containsPoint(p_event.getPosition()))
			{
				if (_cursor != null)
				{
					_cursor.setPosition(p_event.getPosition());
					_cursor.customCallback("onPointerPressed", element);
				}
				_pressedElements.push(element.weakref());
			}
		}
		if (_pressedElements.len() > 1)
		{
			::tt_warning("Overlapping menu elements!");
		}
	}

	function onPointerReleased(p_event)
	{
		if (_mouseEnabled == false)
		{
			return;
		}

		if (_cursor != null)
		{
			foreach (element in _pressedElements)
			{
				if (element != null && element._visible)
				{
					_cursor.setPosition(p_event.getPosition());
					local rect = element.getCollisionRectWorldSpace();
					_cursor.customCallback("onPointerReleased", element, p_event.getPressDuration(),
						rect.containsPoint(p_event.getPosition()));
				}
			}
		}
		_pressedElements.clear();
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// 'Static' Helpers Methods

	function pushScreen(p_type, p_props = null)
	{
		if (::isValidAndInitializedEntity(_activator))
		{
			if (p_props == null)
			{
				p_props = {};
			}

			if (("_activator" in p_props) == false && ::isValidAndInitializedEntity(_activator))
			{
				p_props._activator <- _activator;
			}
			else if (p_props._activator == null)
			{
				p_props._activator = _activator;
			}
		}
		return (p_props == null) ? ::spawnEntity(p_type, ::Vector2(0, 0)).weakref() :
		                           ::spawnEntity(p_type, ::Vector2(0, 0), p_props).weakref();
	}

	function closeAll(p_force = true)
	{
		for (local i = _screenStack.len() - 1; i >= 0; --i)
		{
			local elem = _screenStack[i];
			if (p_force)
			{
				::killEntity(elem.screen);
			}
			else
			{
				elem.screen.close();
			}
		}
	}

	function refocusOnCurrentElement()
	{
		if (_screenStack.len() > 0)
		{
			local top = _screenStack.top();
			top.screen._cursor.refocusOnCurrentElement();
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Inherited Methods

	function hide()
	{
		foreach (window in _windows)
		{
			window.hide();
		}
		base.hide();
	}

	function show()
	{
		foreach (window in _windows)
		{
			window.show();
		}

		base.show();
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Helpers Methods

	function close()
	{
		base.close();

		// Don't play sfx when closing an ingame menu
		if ((this instanceof ::IngameMenu) == false)
		{
			::Audio.playGlobalSoundEffect("Effects", "menu_back");
		}
	}

	function refocus(p_focusElement, p_unhideVisuals)
	{
		if (p_unhideVisuals)
		{
			foreach (element in _elements)
			{
				if (element == null)
				{
					continue;
				}
				element.customCallback("onRefocus");

				if (p_focusElement == null)
				{
					p_focusElement = element;
				}
			}
			show();
		}
		::addButtonInputBlockingListeningEntity(this, InputPriority.GUI);
		::addMouseInputListeningEntity(this, InputPriority.GUI);
		::assert(_cursor == null, "Cursor should be null");
		createCursor(p_focusElement);
	}

	function unfocus(p_hideVisuals)
	{
		// Remove input listening
		::removeButtonInputListeningEntity(this);
		::removeMouseInputListeningEntity(this);
		removeCursor();

		if (p_hideVisuals)
		{
			hide();
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Element Methods

	function createPanelButtons(p_offset, p_ySpacing, p_selectedIndex, p_buttons)
	{
		local height = p_buttons.len() * p_ySpacing;
		local panel = createStoppedPresentation("panel", "elements/panel");
		panel.addCustomValue("height", height);
		panel.setCustomTranslation(::Vector2(0.0, p_offset.y - ((height - p_ySpacing) / 2)));
		panel.start("fadein", [], false, 0);

		return createButtons(p_offset, p_ySpacing, p_selectedIndex, p_buttons);
	}

	function createButtons(p_offset, p_ySpacing, p_selectedIndex, p_buttons)
	{
		local pos     = p_offset;
		local buttons = [];

		for (local i = 0; i < p_buttons.len(); ++i)
		{
			local cur = p_buttons[i];
			if (cur == null)
			{
				pos.y -= p_ySpacing;
				continue;
			}

			if (cur.len() < 4)
			{
				::tt_panic("Too few button data: " + ::niceStringFromObject(cur));
				continue;
			}

			local button = createButton(cur[0], cur[1], pos,
			{
				_content  = cur[2],
				_delegate = cur[3],
				_caller   = this
			});
			buttons.push(button);
			pos.y -= p_ySpacing;
		}

		// Knit the buttons together
		for (local i = 0; i < buttons.len(); ++i)
		{
			local button = buttons[i];
			button._previousVerticalElement = (i == 0) ? buttons[buttons.len()-1] : buttons[i-1];
			button._nextVerticalElement     = (i == buttons.len()-1) ? buttons[0] : buttons[i+1];
		}

		// Create GUI cursor
		if (_cursor != null)
		{
			removeCursor();
		}
		createCursor(buttons[p_selectedIndex]);
		return buttons;
	}

	function createButton(p_id, p_type, p_position, p_properties = {})
	{
		local button = createElement(p_id, p_type, p_position, p_properties);
		button._presentation.setCustomUniformScale(c_buttonScale);
		return button;
	}

	function createCursor(p_currentElement)
	{
		if (_cursor != null)
		{
			removeCursor();
		}

		_cursor = ::spawnEntity("GUICursor", ::Vector2(0, 0),
		{
			_currentElement = p_currentElement
		});
	}

	function removeCursor()
	{
		::killEntity(_cursor);
		_cursor = null;
	}

	function getFocusElement()
	{
		return _cursor != null ? _cursor.getFocusElement() : null;
	}

	function createFaceButtons(p_buttons)
	{
		if (p_buttons.len() < 1 || p_buttons.len() > 4)
		{
			::tt_panic("createFaceButtons can make [1..4] buttons.");
			return;
		}

		local firstButton = null;

		foreach (button in p_buttons)
		{
			local hudAlignment    = HudAlignment.Center;
			local textAlignment   = HorizontalAlignment_Center;
			local xPosition       = 0.0;
			local delegate        = null;
			switch (button)
			{
			case "rom_back":
				hudAlignment  = HudAlignment.Right;
				textAlignment = HorizontalAlignment_Right;
				xPosition     = -0.1;
				delegate      = onButtonMenuPressed;
				break;

			case "back":
			case "log":
				hudAlignment  = HudAlignment.Left;
				textAlignment = HorizontalAlignment_Left;
				xPosition     = 0.1;
				delegate      = onButtonCancelPressed;
				break;

			case "exit":
			case "next":
			case "score":
			case "skip":
				hudAlignment  = HudAlignment.Right;
				textAlignment = HorizontalAlignment_Right;
				xPosition     = -0.1;
				delegate      = onButtonAcceptPressed;
				break;

			case "forums":
				hudAlignment  = HudAlignment.Center;
				textAlignment = HorizontalAlignment_Center;
				xPosition     = 0.0;
				delegate = onButtonFaceLeftPressed;
				break;

			case "more":
				hudAlignment  = HudAlignment.Right;
				textAlignment = HorizontalAlignment_Right;
				xPosition     = -0.1;
				delegate = onButtonFaceLeftPressed;
				break;

			case "applet":
				switch (p_buttons.len())
				{
				case 1:
				case 2:
					hudAlignment  = HudAlignment.Right;
					textAlignment = HorizontalAlignment_Right;
					xPosition     = -0.1;
					break;

				case 3:
					// Special case
					if (p_buttons.find("forums") == null)
					{
						hudAlignment  = HudAlignment.Center;
						textAlignment = HorizontalAlignment_Center;
						xPosition     = 0.0;
					}
					else
					{
						hudAlignment  = HudAlignment.Right;
						textAlignment = HorizontalAlignment_Right;
						xPosition     = -0.1;
					}
					break;

				case 4:
					hudAlignment  = HudAlignment.CenterRight;
					textAlignment = HorizontalAlignment_Center;
					xPosition     = 0.025;
					break;
				}
				delegate = onButtonFaceUpPressed;
				break;

			case "statistics":
				switch (p_buttons.len())
				{
				case 1:
				case 2:
					hudAlignment  = HudAlignment.Left;
					textAlignment = HorizontalAlignment_Left;
					xPosition     = 0.1;
					break;

				case 3:
					if (p_buttons.find("log") != null)
					{
						hudAlignment  = HudAlignment.Center;
						textAlignment = HorizontalAlignment_Center;
						xPosition     = 0.0;
					}
					else
					{
						hudAlignment  = HudAlignment.Left;
						textAlignment = HorizontalAlignment_Left;
						xPosition     = 0.1;
					}
					break;

				case 4:
					hudAlignment  = HudAlignment.CenterLeft;
					textAlignment = HorizontalAlignment_Center;
					xPosition     = -0.025;
					break;
				}
				delegate = onButtonFaceLeftPressed;
				break;

			default:
				tt_panic("Unhandled button type '" + button + "'");
				hudAlignment = HudAlignment.Center;
				break;
			}

			local elem = createElement("facebutton_" + button, "GUIFaceButton", ::Vector2(xPosition, -0.425),
			{
				_type          = button,
				_textAlignment = textAlignment,
				_hudAlignment  = hudAlignment,
				_caller        = this,
				_delegate      = delegate
			});

			if (firstButton == null)
			{
				firstButton = elem;
			}
		}

		if (firstButton != null && _cursor == null)
		{
			createCursor(firstButton);
		}
	}

	function createWindow(p_id, p_position, p_properties)
	{
		if (p_id in _windows)
		{
			::tt_panic("Window with id '" + p_id + "' is already added.");
			return;
		}

		local window = ::spawnEntity("MenuWindow", p_position, p_properties);
		_windows[p_id] <- window;
		return window;
	}

	function createConsoleWindow(p_id, p_position, p_properties)
	{
		if (p_id in _windows)
		{
			::tt_panic("Window with id '" + p_id + "' is already added.");
			return;
		}

		local window = ::spawnEntity("MenuConsoleWindow", p_position, p_properties);
		_windows[p_id] <- window;
		return window;
	}

	function hasWindow(p_id)
	{
		return (p_id in _windows)
	}

	function removeWindow(p_id)
	{
		if (p_id in _windows)
		{
			::killEntity(_windows[p_id]);
			delete _windows[p_id];
		}
	}

	function hideAllWindows()
	{
		foreach (window in _windows)
		{
			window.hide();
		}
	}

	function instantlyShowAllWindowElements()
	{
		local result = true;
		foreach (window in _windows)
		{
			result = result && window.instantlyShowAllElements();
		}
		return result;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Update

	function update(p_deltaTime)
	{
		base.update(p_deltaTime);

		if (_cursor != null && _mouseEnabled && ::isPointerVisible())
		{
			local pos = ::Camera.worldToScreen(::getMouseWorldPosition());
			_cursor.setPosition(pos);

			local hoveredElement = null;
			foreach (element in _elements)
			{
				local rect = element.getCollisionRectWorldSpace();
				if (element._visible && element.c_isSelectable && rect.containsPoint(pos))
				{
					hoveredElement = element;
					break;
				}
			}

			if (hoveredElement != _hoveredElement)
			{
				if (_hoveredElement != null) _hoveredElement.customCallback("onHoverExit");
				if (hoveredElement  != null) hoveredElement.customCallback("onHoverEnter");

				_hoveredElement = hoveredElement;
				if (hoveredElement != null && hoveredElement.c_canBeFocusedOn)
				{
					_cursor.focusOn(hoveredElement);
					customCallback("onElementHovered", hoveredElement);
				}
			}
		}
	}
}
