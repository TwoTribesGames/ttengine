include_entity("gui/GUIElement");

class GUIFaceButton extends GUIElement
</
	placeable      = Placeable_Hidden
	collisionRect  = [ 0.0, 0.0, 0.3, 0.05 ]
/>
{
	// Constants
	static c_isSelectable   = true;
	static c_canBeFocusedOn = false;

	// Creation params
	_type             = null;
	_hudAlignment     = null;
	_textAlignment    = null;
	_delegate         = null;
	_caller           = null;

	// Internals
	_isPressed        = false;
	_label            = null;
	_args             = null;
	_offset           = null;

	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn

	function onInit()
	{
		base.onInit();
		setCanBePaused(false);

		_args = [_caller];
		_offset = _presentation.getCustomTranslation();
		registerEntityByTag("GUIFaceButton");
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks

	function onPressed()
	{
		if (isEnabled() == false)
		{
			::Audio.playGlobalSoundEffect("Effects", "menu_select_cannot");
		}
		::setRumble(RumbleStrength_Low, 0.0);
		base.onPressed();
	}

	function onCursorPressed()
	{
		setPressed(true);
		base.onCursorPressed();
	}

	function onCursorReleased(p_duration)
	{
		setPressed(false);
		base.onCursorReleased(p_duration);
	}

	function onCursorReleasedOnEntity(p_duration)
	{
		onCursorReleased(p_duration);

		// call button delegate
		if (_delegate != null)
		{
			_delegate.acall(_args);
		}
		::Audio.playGlobalSoundEffect("Effects", "menu_next");
		::setRumble(RumbleStrength_Low, 0.0);
	}


	function onDie()
	{
		::Hud.destroyTextElement(_label);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods

	function getLocID()
	{
		local locID = "MENU_" + _type.toupper() + "_BUTTON_" + ::getPlatformString().toupper();
		if (::isPlayingOnPC())
		{
			locID += "_" + ::getControllerTypeString().toupper();
		}
		return locID;
	}

	function createPresentation()
	{
		_label = ::Hud.createTextElement(this,
		{
			locID               = getLocID(),
			width               = 0.5,
			height              = 0.05,
			glyphset            = GlyphSetID_Text,
			position            = getPosition(),
			hudalignment        = _hudAlignment,
			color               = ::TextColors.Light,
			horizontalAlignment = _textAlignment,
			verticalAlignment   = VerticalAlignment_Center,
			presentation        = "menu/elements/guifacebutton",
			layer               = HudLayer.Menu,
			autostart           = true
		});
		_presentation = _label.presentation;
	}

	function setPressed(p_pressed)
	{
		_isPressed = p_pressed;
		if (p_pressed)
		{
			_animName = "pressed";
			startAnimation();
		}
		else
		{
			_animName = "idle";
			startAnimation(["released"]);
		}
	}

	function setVisible(p_visible)
	{
		base.setVisible(p_visible);

		_label.label.setNumberOfVisibleCharacters(p_visible ? -1 : 0);
	}

	function update(p_deltaTime)
	{
		local hud = ::Hud.getInstance();
		local alignerPosition = hud._hudAligners[_hudAlignment].getPosition();
		setPosition(_offset + alignerPosition);
	}
}
::registerClassForSetPlayerCallbacks(GUIFaceButton);
