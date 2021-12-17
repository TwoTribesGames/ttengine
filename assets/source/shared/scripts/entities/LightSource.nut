include_entity("rewind/RewindEntity");

class LightSource extends RewindEntity
</ 
	editorImage    = "editor.lightsource"
	libraryImage   = "editor.library.lightsource"
	placeable      = Placeable_Everyone
	collisionRect  = [ 0.0, 0.5, 2.0, 2.0 ]  // center X, center Y, width, height
	displayName    = "Light - Light Source"
	//sizeShapeColor = Colors.Light -- FIX ME -- Messes up positioning in editor.
	stickToEntity  = true
/>
{
	</
		type        = "float"
		min         = 1.0
		max         = 250.0
		order       = 0.2
		group       = "Appearance"
	/>
	radius = 10.0;
	
	</
		type        = "float"
		min         = 0.0
		max         = 10.0
		order       = 0.3
		group       = "Appearance"
	/>
	radiusGrowDuration = 0.0;
	
	</
		type        = "color_rgba"
		order       = 0.4
		description = "The color of this light. NULL defaults to white"
		group       = "Appearance"
	/>
	color = null;
	
	</
		type        = "string"
		choice      = getLightNames()
		order       = 0.5
		group       = "Appearance"
	/>
	texture = null;
	
	</
		type        = "float"
		min         = -720.0
		max         =  720.0
		order       = 0.6
		group       = "Appearance"
	/>
	textureRotationSpeed = 0.0;
	
	</
		type        = "float"
		min         = 0.0
		max         = 360.0
		order       = 0.7
		group       = "Appearance"
	/>
	direction = 0.0;
	
	</
		type        = "float"
		min         = 0.0
		max         = 360.0
		order       = 0.8
		group       = "Appearance"
	/>
	spread = 360.0;
	
	</
		type        = "float"
		min         = 0.0
		max         = 10.0
		order       = 9
		group       = "Appearance"
	/>
	spreadGrowDuration = 0.0;
	
	</
		type        = "bool"
		order       = 1.0
		group       = "Flickering"
	/>
	enableFlickering = false;
	
	</
		type           = "entity"
		filter         = ::getTriggerTypes()
		order          = 1.1
		group          = "Flickering"
		referenceColor = ReferenceColors.Enable
		description    = "When set, flickering will be enabled when an incoming signal fires."
	/>
	enableFlickeringSignal = null;
	
	</
		type        = "float"
		min         = 0.1
		max         = 10.0
		order       = 1.2
		group       = "Flickering"
	/>
	flickerDelay = 1.0
	
	</
		type        = "float"
		min         = 0.01
		max         = 0.5
		order       = 1.3
		group       = "Flickering"
	/>
	flickerDuration = 0.2;
	
	</
		type        = "float"
		min         = 1.0
		max         = 250.0
		order       = 1.4
		group       = "Flickering"
	/>
	flickerRadius = 8.0;
	
	</
		type        = "color_rgba"
		order       = 1.5
		description = "The flicker color of this light. NULL defaults to white"
		group       = "Flickering"
	/>
	flickerColor = null;
	
	</
		type        = "bool"
		order       = 100
		group       = "Misc."
	/>
	enabled = true;
	
	</
		type           = "entity"
		filter         = ::getTriggerTypes()
		order          = 101
		group          = "Misc."
		referenceColor = ReferenceColors.Enable
		description    = "When set, this entity will be enabled when an incoming signal fires."
	/>
	enableSignal = null;
	
	_light            = null;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		base.onInit();
		
		::removeEntityFromWorld(this);
		
		_light = addLight(::Vector2(0, 0), 0.0, 1.0);
		if (color != null)
		{
			_light.setColor(color);
		}
		if (texture != null)
		{
			_light.setTexture(texture);
		}
		_light.setTextureRotationSpeed(textureRotationSpeed);
		_light.setDirection(direction);
		_light.setSpread(0.0, 0.0);
		_light.setRadius(0.0, 0.0);
		
		setEnabled(enabled);
		
		if (enableSignal  != null)
		{
			enableSignal.addChildTrigger(this);
		}
		
		if (enableFlickeringSignal != null)
		{
			enableFlickeringSignal.addChildTrigger(this);
		}
		
		if (enableFlickering)
		{
			if (radiusGrowDuration > 0.0)
			{
				startCallbackTimer("enableFlickering", radiusGrowDuration);
			}
			else
			{
				setEnabledFlickering(true);
			}
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function setEnabled(p_enabled)
	{
		_light.setEnabled(p_enabled);
		if (p_enabled)
		{
			_light.setRadius(radius, radiusGrowDuration);
			_light.setSpread(spread, spreadGrowDuration);
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods
	
	function flickerOn()
	{
		_light.setRadius(flickerRadius, 0.0);
		if (flickerColor != null)
		{
			_light.setColor(flickerColor);
		}
		
		startCallbackTimer("flickerOff", ::frnd_minmax(0.01, flickerDuration));
	}
	
	function flickerOff()
	{
		_light.setRadius(radius, 0.0);
		if (color != null)
		{
			_light.setColor(color);
		}
		
		startCallbackTimer("flickerOn", ::frnd_minmax(0.01, flickerDelay));
	}
	
	function setEnabledFlickering(p_enabled)
	{
		enableFlickering = p_enabled;
		if (enableFlickering)
		{
			startCallbackTimer("flickerOn", ::frnd_minmax(0.01, flickerDelay));
		}
		else
		{
			stopTimer("flickerOn");
			stopTimer("flickerOff");
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Inherited methods
	
	function _triggerEnter(p_entity, p_parent)
	{
		if (p_parent == null)
		{
			::tt_panic("_triggerEnter should have parent != null");
			return;
		}
		
		if (p_parent.equals(enableFlickeringSignal))
		{
			setEnabledFlickering(true);
		}
		
		// Default behavior
		setEnabled(true);
	}
}
Trigger.makeTriggerTarget(LightSource);
