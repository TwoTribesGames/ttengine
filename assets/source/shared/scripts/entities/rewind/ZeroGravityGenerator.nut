include_entity("rewind/RewindEntity");

class ZeroGravityGenerator extends RewindEntity
</
	editorImage    = "editor.zerogravitygenerator"
	libraryImage   = "editor.library.zerogravitygenerator"
	placeable      = Placeable_Developer
	collisionRect  = [ 0.0, 1.0, 2.0, 2.0 ]
	sizeShapeColor = Colors.ZeroGravityField
	stickToEntity  = true
/>
{
	</
		type = "string"
		choice = ["circle", "rectangle"]
		order = 0
	/>
	fieldType = "circle";
	
	</
		type = "bool"
		order = 3
	/>
	displayGenerator = true;
	
	</
		type        = "bool"
		order       = 1
		group       = "Color Grading"
	/>
	useColorGrading = true;
	
	</
		type        = "string"
		choice      = getColorGradingNames()
		order       = 2
		conditional = "useColorGrading == true"
		group       = "Color Grading"
		description = "If not set the default zero gravity colorgrading texture is used"
	/>
	overrideDefaultColorGrading = null;
	
	</
		type        = "integer"
		min         = 1
		max         = 100
		order       = 2
		conditional = "fieldType == rectangle"
	/>
	width  = 10;
	
	</
		type = "integer"
		min  = 1
		max  = 100
		order = 3
		conditional = "fieldType == rectangle"
	/>
	height = 10;
	
	</
		type = "bool"
		order = 4
		conditional = "fieldType == rectangle"
	/>
	displayRectangleTop = true;
	
	</
		type = "bool"
		order = 5
		conditional = "fieldType == rectangle"
	/>
	displayRectangleBottom = true;
	</
		type = "bool"
		order = 6
		conditional = "fieldType == rectangle"
	/>
	displayRectangleLeft = true;
	</
		type = "bool"
		order = 7
		conditional = "fieldType == rectangle"
	/>
	displayRectangleRight = true;
	
	</
		type = "integer"
		min  = 1
		max  = 50
		order = 2
		conditional = "fieldType == circle"
	/>
	radius = 10;
	
	// Internal fields
	_presentationField     = null;
	_particleEffects       = null;
	_presentationGenerator = null;
	_fieldSensor           = null;
	
	_particleRectCenter = null;
	_particleRectTop    = null;
	_particleRectBottom = null;
	_particleRectLeft   = null;
	_particleRectRight  = null;
	
	// FIXME: Move this specific PlayerBot only code to the PlayerBot?
	_globals            = { soundEffect = null };
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		base.onInit();
		
		if (useColorGrading)
		{
			addProperty("colorGrading", overrideDefaultColorGrading == null ? 
			            "zerogravity" : overrideDefaultColorGrading);
			addProperty("colorGradingFadeDuration", 0.2);
		}
		
		_particleEffects = [];
		
		addInvincibilityProperties();
		setCanBePushed(false);
	}
	
	function onSpawn()
	{
		base.onSpawn();
		
		switch (fieldType)
		{
		case "circle":
			createCircularField(radius);
			if (displayGenerator)
			{
				addShoeboxInclude("levels/shoebox_includes/zerogravitygenerator_circle", 
					getShoeboxSpaceFromWorldPos(getPosition() - ::Vector2(radius/8.0, radius/8.0)),
					-radius / 3.0, 0, radius / 15.0);
			}
			
			break;
			
		case "rectangle":
			createRectangularField(width, height)
			break;
			
		default:
			::tt_panic("Unhandled fieldType '" + fieldType + "'");
			break;
		}
		
		if (_fieldSensor != null)
		{
			_fieldSensor.setDefaultEnterAndExitCallback();
			_fieldSensor.setFilterCallback("onTouchFilter");
		}
		
		if (_presentationField != null)
		{
			_presentationField.start("enable", [], false, 0);
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onProgressRestored(p_id)
	{
		restoreRectangleEmitterProps(width + 2.5, height + 2.5);
	}
	
	function onTouchFilter(p_entity)
	{
		return p_entity.hasProperty("noticeZeroGravity");
	}
	
	function onTouchEnter(p_entity, p_sensor)
	{
		local refCount = handleZeroGravityEnter(p_entity, this);
		
		if (refCount == 1 && p_entity instanceof ::PlayerBot)
		{
			::setRumble(RumbleStrength_Low, 0.0);
			p_entity.spawnParticleOneShot("particles/zerogravityfield_enter", ::Vector2(0, 0), true, 0, false, ParticleLayer_UseLayerFromParticleEffect, 1.0);
			playSoundEffect("gravityfield_enter");
			_globals.soundEffect = playSoundEffect("zerogravity_field");
		}
	}
	
	function onTouchExit(p_entity, p_sensor)
	{
		local refCount = handleZeroGravityExit(p_entity, this);
		
		if (refCount == 0 && p_entity instanceof ::PlayerBot)
		{
			::setRumble(RumbleStrength_Low, 0.0);
			p_entity.spawnParticleOneShot("particles/zerogravityfield_enter", ::Vector2(0,0), true, 0, false, ParticleLayer_UseLayerFromParticleEffect, 1.0);
			playSoundEffect("gravityfield_exit");
			
			_globals.soundEffect.stop();
			_globals.soundEffect = null;
		}
	}
	
	function onDie()
	{
		base.onDie();
		
		if (_fieldSensor != null)
		{
			_fieldSensor.setEnabled(false);
		}
		
		// Force stop all particles
		foreach (effect in _particleEffects)
		{
			effect.stop(true);
		}
		
		if (hasProperty("dieQuietly") == false)
		{
			playSoundEffect("gravityfield_disabled");
		}
	}
	
	function onReloadGame()
	{
		if (_globals.soundEffect != null)
		{
			_globals.soundEffect.stop();
			_globals.soundEffect = null;
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods
	
	function createCircularField(p_radius)
	{
		// Set collision rect for culling
		setCollisionRectWithVectorRect(::VectorRect(getCenterOffset(), p_radius * 2.0, p_radius * 2.0));
		
		_fieldSensor = addTouchSensor(::CircleShape(0, p_radius));
		_presentationField = createPresentationObject("presentation/zerogravityfield_circle");
		_presentationField.addCustomValue("radius", p_radius + 1.5);
		
		// Particles
		{
			local surface = (p_radius * p_radius * ::PI);
			addParticleEffect("particles/zerogravityfield_circle_center", ::Vector2(0, 0),
				[
					[0, { radius = p_radius * 0.8, inner_radius = 0, particles = surface / 6}],
					[1, { radius = p_radius * 0.8, inner_radius = 0, particles = surface / 6}]
				]);
				
			local circumreference = 2 * ::PI * p_radius;
			addParticleEffect("particles/zerogravityfield_circle_range", ::Vector2(0, 0),
				[
					[0, { radius = p_radius - 0.3, inner_radius = p_radius - 0.3, particles = circumreference * 30}]
				]);
		}
	}
	
	function createRectangularField(p_width, p_height)
	{
		// Set collision rect for culling
		setCollisionRectWithVectorRect(::VectorRect(::Vector2(0, 0), p_width, p_height));
		
		_fieldSensor = addTouchSensor(::BoxShape(p_width, p_height));
		
		// Make sure the visuals use a bigger area than the touch rectangle
		p_width += 2.5;
		p_height += 2.5;
		
		local halfWidth    = (p_width * 0.5);
		local halfHeight   = (p_height * 0.5);
		
		_presentationField = createPresentationObject("presentation/zerogravityfield_rectangle");
		_presentationField.addCustomValue("quad_width" , p_width - 0.0);
		_presentationField.addCustomValue("quad_height", p_height - 0.0);
		
		_presentationField.addCustomValue("u", p_width * 7.5  / getMaxTileWidth().tofloat());
		_presentationField.addCustomValue("v", p_height * 7.5 / getMaxTileHeight().tofloat());
		
		_presentationField.addCustomValue("edge_width" , p_width - 0.3);
		_presentationField.addCustomValue("edge_height", p_height - 0.3);
		
		_presentationField.addCustomValue("half_width",  halfWidth - 1.0);
		_presentationField.addCustomValue("half_height", halfHeight -1.0);
		_presentationField.addCustomValue("minus_half_width",  -halfWidth + 1.0);
		_presentationField.addCustomValue("minus_half_height", -halfHeight + 1.0);
		
		// Particles
		{
			// Center
			_particleRectCenter = addParticleEffect("particles/zerogravityfield_rectangle_center", ::Vector2(0, 0), []);
				
			// Edges
			local offset = 1.75;
			if (displayRectangleTop)
			{
				_particleRectTop = addParticleEffect("particles/zerogravityfield_rectangle_range", ::Vector2(0, halfHeight - offset), []);
				_presentationField.addTag("top");
			}
			
			if (displayRectangleBottom)
			{
				_particleRectBottom = addParticleEffect("particles/zerogravityfield_rectangle_range", ::Vector2(0, -halfHeight + offset), []);
				_presentationField.addTag("bottom");
			}
			
			if (displayRectangleRight)
			{
				_particleRectRight = addParticleEffect("particles/zerogravityfield_rectangle_range", ::Vector2(halfWidth - offset, 0), []);
				_presentationField.addTag("right");
			}
			
			if (displayRectangleLeft)
			{
				_particleRectLeft = addParticleEffect("particles/zerogravityfield_rectangle_range", ::Vector2(-halfWidth + offset, 0), []);
				_presentationField.addTag("left");
			}
		}
		
		restoreRectangleEmitterProps(p_width, p_height);
	}
	
	function restoreRectangleEmitterProps(p_width, p_height)
	{
		if (_particleRectCenter != null)
		{
			setEmitterProperties(_particleRectCenter,
				[
					[0, { rect_width = p_width * 0.8, rect_height = p_height * 0.8, particles = p_height * p_width / 6}],
					[1, { rect_width = p_width * 0.8, rect_height = p_height * 0.8, particles = p_height * p_width / 6}]
				]);
		}
		if (_particleRectTop    != null)
		{
			setEmitterProperties(_particleRectTop,
				[
					[0, { rect_width  = p_width,  velocity_x =   0, velocity_y =  14, particles = p_width  * 25 }]
				]);
		}
		if (_particleRectBottom != null)
		{
			setEmitterProperties(_particleRectBottom,
				[
					[0, { rect_width  = p_width,  velocity_x =   0, velocity_y = -14, particles = p_width  * 25 }]
				]);
		}
		if (_particleRectRight  != null)
		{
			setEmitterProperties(_particleRectRight,
				[
					[0, { rect_height = p_height, velocity_x =  14, velocity_y =   0, particles = p_height * 25 }]
				]);
		}
		if (_particleRectLeft   != null)
		{
			setEmitterProperties(_particleRectLeft,
				[
					[0, { rect_height = p_height, velocity_x = -14, velocity_y =   0, particles = p_height * 25 }]
				]);
		}
	}
	
	function getMaxTileWidth()
	{
		return ZeroGravityGenerator.getattributes("width").max;
	}
	
	function getMaxTileHeight()
	{
		return ZeroGravityGenerator.getattributes("height").max;
	}
	
	function addParticleEffect(p_name, p_offset, p_emitterProperties)
	{
		local effect = spawnParticleContinuous(p_name, p_offset, true, 0, false, ParticleLayer_UseLayerFromParticleEffect, 1.0);
		foreach (prop in p_emitterProperties)
		{
			effect.setEmitterProperties(prop[0], prop[1]);
		}
		_particleEffects.push(effect);
		return effect;
	}
	
	function setEmitterProperties(p_emitter, p_emitterProperties)
	{
		foreach (prop in p_emitterProperties)
		{
			p_emitter.setEmitterProperties(prop[0], prop[1]);
		}
	}
}
