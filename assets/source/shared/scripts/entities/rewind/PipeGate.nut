include_entity("rewind/RewindEntity");

class PipeGate extends RewindEntity
</
	placeable                 = Placeable_Hidden
	movementset               = "Static"
	collisionRect             = [ 0.0, 1.5, 1.0, 3.0 ]
	sizeShapeColor            = Colors.PipeGate
	sizeShapeFromEntityCenter = false
/>
{
	rotation = null;
	width    = null;
	height   = null;
	
	_presentation     = null;
	_presentationBack = null;
	_refCount         = 0;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit/onSpawn
	
	function onInit()
	{
		base.onInit();
		
		::removeEntityFromWorld(this);
		setPositionCullingEnabled(false);
		
		setCollisionRect(::Vector2(0, height * 0.5), width, height);
		setEntityCollisionTiles(createCollisionTileString(width, height, "p"));
		setEntityCollisionTilesActive(true);
		setLightBlocking(true);
		
		_presentation     = createPresentationObjectInLayer("presentation/pipegate", ParticleLayer_InFrontOfShoeboxZeroTwo);
		_presentationBack = createPresentationObjectInLayer("presentation/pipegate_back", ParticleLayer_InFrontOfEntities);
		
		local scale = ::max(width, height) / 3.0;
		_presentation.addCustomValue("scale", scale);
		_presentationBack.addCustomValue("scale", scale);
		
		// Create sensor
		{
			local isRotated = rotation == 90 || rotation == 270;
			local sensorWidth  = isRotated ? width      : width + 8;
			local sensorHeight = isRotated ? height + 8 : height;
			local touchSensor = addTouchSensor(::BoxShape(sensorWidth, sensorHeight));
			touchSensor.setDefaultEnterAndExitCallback();
			touchSensor.setFilterCallback("onTouchFilter");
			
			// Create invisible wall sensor for playerbot
			local player = ::getFirstEntityByTag("PlayerBot");
			if (player != null)
			{
				local playerSensor = addTouchSensor(
					::BoxShape(isRotated ? sensorWidth : 0.5, isRotated ? 0.5 : sensorHeight), player);
				playerSensor.setEnterCallback("onPlayerEnter");
			}
		}
	}
	
	function onSpawn()
	{
		base.onSpawn();
		
		startAnimation("idle");
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onTouchEnter(p_entity, p_sensor)
	{
		++_refCount;
		if (_refCount == 1)
		{
			open();
		}
	}
	
	function onTouchExit(p_entity, p_sensor)
	{
		--_refCount;
		if (_refCount == 0)
		{
			close();
		}
	}
	
	function onTouchFilter(p_entity)
	{
		return p_entity.hasProperty("opensPipeGates") && p_entity.containsVirus() == false;
	}
	
	function onPlayerEnter(p_entity, p_sensor)
	{
		p_entity.setSpeed(::getVectorFromAngle(rotation + 90) * 30);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods
	
	function open()
	{
		startAnimation("open");
		setEntityCollisionTiles(createCollisionTileString(width, height, " "));
	}
	
	function close()
	{
		startAnimation("close");
		setEntityCollisionTiles(createCollisionTileString(width, height, "p"));
	}
	
	function startAnimation(p_name)
	{
		_presentation.start(p_name, [rotation.tostring()], false, 0);
		_presentationBack.start(p_name, [rotation.tostring()], false, 0);
	}
}
PipeGate.setattributes("positionCulling", null);
