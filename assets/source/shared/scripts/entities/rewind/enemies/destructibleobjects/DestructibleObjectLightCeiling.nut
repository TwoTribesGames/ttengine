include_entity("rewind/enemies/destructibleobjects/DestructibleObject");

class DestructibleObjectLightCeiling extends DestructibleObject
</
	editorImage         = "editor.destructibleobjectlightceiling"
	libraryImage        = "editor.library.destructibleobjectlightceiling"
	placeable           = Placeable_Everyone
	movementset         = "Static"
	collisionRect       = [ 0.0, 0.5, 8.0, 1.0 ]
	group               = "01.2 Destructible Objects"
	displayName         = "Light Ceiling"
	stickToEntity       = true
/>
{
	// Visuals
	static c_presentationLayer        = ParticleLayer_InFrontOfShoeboxZeroOne;
	
	// Misc.
	static c_maxHealth                = 5;
	static c_score                    = 10;
	static c_hasPermanentCorpse       = true;
	
	// General settings
	static c_damagedByBullets         = true;
	static c_rumbleStrength           = RumbleStrength_Low;
	
	// Internal members
	_light = null;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		base.onInit();
		
		_light = addLight(::Vector2(0, 0), 12.0, 1.0);
		_light.setTexture("ambientlight");
		_light.setTextureRotationSpeed(::frnd_minmax(-4.0, 4.0));
		_light.setDirection(orientation == "right" ? 177.5 : 182.5);
		_light.setSpread(160, 0.0);
		
		setDefaultRadiusAndColor();
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onDamage(p_healthbar, p_damage)
	{
		if (hasTimer("damageTimeout") == false)
		{
			_light.setRadius(10.0, 0.0);
			_light.setColor(ColorRGBA(100, 255, 255, 150));
			startTimer("damageTimeout", 0.1);
		}
	}
	
	function onTimer(p_name)
	{
		if (p_name == "damageTimeout")
		{
			setDefaultRadiusAndColor();
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods
	
	function setDefaultRadiusAndColor()
	{
		_light.setRadius(12.0, 0.0);
		_light.setColor(ColorRGBA(180, 255, 255, 200));
	}
}
