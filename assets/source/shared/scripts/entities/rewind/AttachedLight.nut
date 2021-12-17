include_entity("rewind/EntityChild");

class AttachedLight extends EntityChild
</
	placeable      = Placeable_Hidden
	movementset    = "Static"
	collisionRect  = [ 0.0, 0.0, 0.1, 0.1 ]
/>
{
	_presentation     = null;
	_presentationFile = null;
	
	_lightAngle    = 0.0;
	_lightSpread   = 30;
	_lightRange    = 8.0;
	_lightStrength = 1.0;
	
	_lightTexture              = null;
	_lightTextureRotationSpeed = 0;
	_lightColor                = null;
	
	_light                     = null;
	_lightSoundCue             = null;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		base.onInit();
		
		// Light is disabled by default
		_light = addLight(::Vector2(0, 0), 0.0, 0.0);
		_light.setColor(_lightColor || ColorRGBA(255, 255, 255, 255));
		_light.setAffectsEntities(false);
		
		if (_lightTexture != null)
		{
			_light.setTexture(_lightTexture);
			if (_lightTextureRotationSpeed != 0.0)
			{
				_light.setTextureRotationSpeed(_lightTextureRotationSpeed);
			}
		}
		
		if (_lightSpread < 360)
		{
			_light.setSpread(_lightSpread, 0.0);
		}
		
		if (_presentationFile != null)
		{
			_presentation = createPresentationObjectInLayer(_presentationFile, ParticleLayer_InFrontOfEntities);
			_presentation.setAffectedByOrientation(false);
		}
		
		setAngle(_lightAngle);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onTimer(p_name)
	{
		if (p_name == "disableLight")
		{
			disableLight();
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods
	
	function setSpread(p_spread, p_duration = 0.0)
	{
		if (p_spread < 360 && p_spread >= 0)
		{
			_light.setSpread(p_spread, p_duration);
		}
	}
	
	function getSpread()
	{
		return _light.getSpread();
	}
	
	function setAngle(p_angle)
	{
		if (_lightSpread >= 360) return;
		
		_light.setDirection(p_angle);
		
		_lightAngle = p_angle;
		
		if (_presentation != null)
		{
			_presentation.setCustomRotation(-p_angle); // presentation so flipped angle
		}
	}
	
	function setRange(p_range, p_duration = 0.0)
	{
		_lightRange = p_range;
		_light.setRadius(p_range, p_duration);
	}
	
	function setColor(p_color)
	{
		_lightColor = p_color;
		_light.setColor(_lightColor);
	}
	
	function setStrength(p_strength, p_duration = 0.0)
	{
		_lightStrength = p_strength;
		_light.setStrength(_lightStrength, p_duration);
	}
	
	function setTexture(p_texture)
	{
		_lightTexture = p_texture;
		_light.setTexture(p_texture);
	}
	
	function disableLight(p_duration = 0.0)
	{
		if (p_duration == 0.0)
		{
			_light.setStrength(0.0, 0.0);
			_light.setRadius  (0.0, 0.0);
			if (_presentation != null)
			{
				_presentation.start("hide", [], false, 0);
			}
			stopAllTimers();
		}
		else
		{
			_light.setStrength(0.0, p_duration);
			_light.setRadius  (0.0, p_duration);
			startTimer("disableLight", p_duration);
		}
	}
	
	function enableLight(p_duration = 0.0)
	{
		stopAllTimers();
		_light.setStrength(_lightStrength, p_duration);
		_light.setRadius  (_lightRange,    p_duration);
		if (_presentation != null)
		{
			_presentation.start("idle", [], false, 0);
		}
	}
}

