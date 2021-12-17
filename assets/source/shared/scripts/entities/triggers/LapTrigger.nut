include_entity("triggers/Trigger");

class LapTrigger extends Trigger
</
	editorImage    = "editor.laptrigger"
	libraryImage   = "editor.library.laptrigger"
	placeable      = Placeable_Developer
	sizeShapeColor = Colors.Trigger
	displayName    = "Trigger - Lap"
	group          = "04.1 Action Triggers"
	stickToEntity  = true
/>
{
	</
		type   = "integer"
		choice = ::range(0, ::ProgressMgr.c_maxLaps-2) // -2 because mission end also registers a lap
		group  = "Specific Settings"
		order  = 0
	/>
	id = 0;
	
	_presentation             = null;
	_presentationBackground   = null;
	_presentationRaster       = null;
	
	_label                    = null;
	_lapTime                  = null;
	_shouldUpdate             = true;
	
	once = true;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		base.onInit();
		
		setPositionCullingEnabled(false);
		local _virusLight = addLight(::Vector2(0, 0), 0.0, 1.0);
		_virusLight.setRadius(7, 0.1);
		_virusLight.setTexture("ambientlight_nocolor");
		
		local gameMode = ::ProgressMgr.getGameMode();
		
		if (gameMode != GameMode.SpeedRun && gameMode != GameMode.SingleLife)
		{
			::killEntity(this);
			return;
		}
		
		if (::isTestBuild())
		{
			registerEntityByTag("LapTrigger");
			// Verify IDs
			local entities = ::getEntitiesByTag("LapTrigger");
			if (entities.len() >= ::ProgressMgr.c_maxLaps)
			{
				::tt_panic("Too many LapTriggers. Max is '" + ::ProgressMgr.c_maxLaps + "'");
				::killEntity(this);
				return;
			}
			local registeredIDs = [];
			foreach (entity in entities)
			{
				if (registeredIDs.find(entity.id) != null)
				{
					::tt_panic("LapTriggers with id '" + entity.id + "' already exists.");
					::killEntity(this);
					return;
				}
				registeredIDs.push(entity.id);
			}
		}
		
		if (gameMode == GameMode.SpeedRun)
		{
			_lapTime = ::ProgressMgr.getFastestLapTime(::Level.getMissionID(), id);
		}
		else
		{
			_shouldUpdate = false;
		}
		
		_presentation = createPresentationObject("presentation/laptrigger");
		_label = addTextLabel("", 4, 0.5, GlyphSetID_Text);
		_label.setColor(ColorRGBA(255,255,255,255));
		_label.setHorizontalAlignment(HorizontalAlignment_Center);
		_label.setVerticalAlignment(VerticalAlignment_Center);
		_presentation.addTextLabel(_label, ::Vector2(0, 0.0));
		
		_presentationBackground = createPresentationObjectInLayer("presentation/laptrigger_background", ParticleLayer_BehindEntities);
		_presentationRaster     = createPresentationObjectInLayer("presentation/laptrigger_raster", ParticleLayer_BehindEntities);
	}
	
	function onSpawn()
	{
		base.onSpawn();
		
		if (_presentation != null)
		{
			_presentation.start("", [], false, 0);
			_presentationBackground.start("", [], false, 0);
			_presentationRaster.start("", [], false, 0);
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onTriggerEnterFirst(p_entity)
	{
		local gameMode = ::ProgressMgr.getGameMode();
		if (gameMode == GameMode.SpeedRun && ::ProgressMgr.hasActiveMission() == false)
		{
			::tt_panic("LapTrigger triggered after mission ending. This shouldn't happen!");
			_shouldUpdate = false;
			return;
		}
		
		if (gameMode == GameMode.SpeedRun)
		{
			p_entity._scoreContainer.addLapTime(id);
			local diff = updateTime();
			if (_presentation != null)
			{
				_presentation.start("", [diff < 0 ? "better" : "worse"], false, 0);
			}
		}
		else
		{
			local index = p_entity._scoreContainer.incrementLapIndex();
			local percentage = (::ProgressMgr.getPlayMode() == PlayMode.Campaign) ?
				::ProgressMgr.getSingleLifeCampaignPercentageFromIndex(index) : 
				::ProgressMgr.getSingleLifeMissionPercentageFromIndex(Level.getMissionID(), index);
			_label.setText(::formatPercentage(percentage));
			_presentation.start("", ["better"], false, 0);
		}
		_shouldUpdate = false;
	}
	
	function onCulled()
	{
		// Don't call base
	}
	
	function onUnculled()
	{
		// Don't call base
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods
	
	function updateTime()
	{
		if (_label == null)
		{
			return null;
		}
		
		local player = ::getFirstEntityByTag("PlayerBot");
		if (player == null)
		{
			return null;
		}
		
		local missionTime = player._scoreContainer._gameStats.mission.time;
		if (_lapTime != null)
		{
			local diff = missionTime - _lapTime;
			_label.setText(::formatLapDifference(diff));
			return diff;
		}
		
		// First run
		_label.setText(::formatTime(missionTime));
		return -1;
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Update
	
	function update(p_deltaTime)
	{
		base.update(p_deltaTime);
		
		if (_shouldUpdate)
		{
			updateTime();
		}
	}
}
LapTrigger.setattributes("once", null);
LapTrigger.setattributes("triggerFilter", null);
LapTrigger.setattributes("filterAllEntitiesOfSameType", null);
LapTrigger.setattributes("triggerOnUncull", null);
