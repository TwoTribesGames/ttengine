include_entity("triggers/Trigger");

class CountdownTrigger extends Trigger
</
	editorImage    = "editor.countdowntrigger"
	libraryImage   = "editor.library.countdowntrigger"
	placeable      = Placeable_Developer
	sizeShapeColor = Colors.Trigger
	displayName    = "Trigger - Countdown"
	group          = "04.1 Action Triggers"
	stickToEntity  = true
/>
{
	</
		type   = "float"
		min    = 0.0
		max    = 600.0
		order  = 1
		group  = "Specific Settings"
	/>
	timeout = 30.0;
	
	_label = null;
	once = true;
	_entity = null;
	_parent = null;
	_timeLeft = null;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		base.onInit();
		
		registerEntityByTag("CountdownTrigger");
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onDie()
	{
		base.onDie();
		
		::Hud.destroyTextElement(_label);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Inherited methods
	
	function _triggerEnter(p_entity, p_parent)
	{
		_entity = p_entity;
		_parent = p_parent;
		_timeLeft = timeout;
		
		if (_label == null)
		{
			_label = ::Hud.createTextElement(this,
			{
				locID                = "",
				width                = 0.70,
				height               = 0.15,
				glyphset             = GlyphSetID_Header,
				hudalignment         = HudAlignment.Center,
				position             = ::Vector2(0.0, -0.375),
				color                = ColorRGBA(255,255,255,115),
				horizontalAlignment  = HorizontalAlignment_Center,
				autostart            = true,
				layer                = HudLayer.Normal | HudLayer.VirusUploadMode | HudLayer.Death
			});
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Update
	
	function update(p_deltaTime)
	{
		base.update(p_deltaTime);
		
		local player = ::getFirstEntityByTag("PlayerBot");
		if (player != null && enabled && _label != null)
		{
			_timeLeft -= p_deltaTime;
			if (_timeLeft <= 0.0)
			{
				base._triggerEnter(_entity, _parent);
				setEnabled(false);
				_timeLeft = 0;
			}
			_label.label.setText(::formatTime(_timeLeft));
		}
	}
}
CountdownTrigger.setattributes("once", null);
