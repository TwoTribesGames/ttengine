include_entity("rewind/conversation/ConversationMgr");
include_entity("rewind/energy/EnergyMgr");
include_entity("rewind/hud/Hud");
include_entity("ColorGrading");

class g_ShoeboxSplitPriorities
{
	behind = -10;
	middle =   0;
	front  =  10;
}

class LevelSettings extends EntityBase
</
	editorImage         = "editor.levelsettings"
	libraryImage        = "editor.library.levelsettings"
	placeable           = Placeable_Everyone
	collisionRect       = [ 0.0, 1.0, 2.0, 1.0 ]
	order               = -1
	ignoreSpawnSections = true
/>
{
	</
		type    = "string"
		choice  = ["Light", "Dark", "LightWithNoStopOnSplit", "DarkWithStopOnSplit"]
		order   = 0.0
		group   = "Appearance"
	/>
	levelLightMode = "Light";

	</
		type   = "string"
		choice = ::tableKeysAsArray(::g_ShoeboxSplitPriorities)
		order  = 0.1
		group  = "Appearance"
	/>
	shoeboxSplitPriority = null;

	</
		type  = "integer"
		min   = 0
		max   = 255
		order = 0.2
		group = "Appearance"
	/>
	levelAmbientLight = 128;

	</
		type  = "integer"
		min   = 0
		max   = 180
		order = 0.3
		group = "Appearance"
	/>
	tvFOV = 60;

	</
		type   = "string"
		choice = getColorGradingNames()
		order  = 0.4
		group  = "Appearance"
	/>
	colorGrading = "clean_table";

	</
		type        = "string"
		choice      = getColorGradingNames()
		order       = 0.5
		group       = "Appearance"
		description = "If not set 'submerged' will be used"
	/>
	colorGradingWater = null;

	</
		type  = "bool"
		order = 0.7
		group = "Appearance"
	/>
	showHud = true;

	</
		type  = "bool"
		order = 0.8
		group = "Appearance"
	/>
	applyColorGradingAfterHUD = false;

	</
		type   = "string"
		choice = ::getMusicSystemTracks()
		order = 1.0
		group = "Music"
	/>
	musicTrack = null;

	</
		type   = "string"
		choice = ["none", "slow", "medium", "fast"]
		order = 1.1
		group = "Music"
	/>
	musicStartIntensity = "none";

	</
		type  = "float"
		min   = 0.0
		max   = 10.0
		order = 1.2
		group = "Music"
	/>
	musicStartFadeDuration = 1.0;

	</
		type  = "bool"
		order = 100
		group = "Blur"
	/>
	useBackgroundBlur = true;

	</
		type  = "bool"
		order = 101
		group = "Blur"
	/>
	useForegroundBlur = true;

	</
		type  = "bool"
		order = 1
		group = "Fog"
	/>
	useFog = true;

	</
		type   = "color_rgb"
		order  = 2
		group = "Fog"
		conditional = "useFog == true"
	/>
	fogColor = null;

	</
		type   = "integer"
		min    = -250
		max    = 100
		order  = 3
		group = "Fog"
		conditional = "useFog == true"
	/>
	fogNear = 0;

	</
		type   = "integer"
		min    = -250
		max    = 100
		order  = 4
		group = "Fog"
		conditional = "useFog == true"
	/>
	fogFar = -30;

	function getName()
	{
		return "level_settings";
	}

	function onInit()
	{
		// Instantiate the instance as early as possible
		::ProgressMgr.getInstance();
		::OptionsData.restore();

		::Camera.reset();
		::Camera.setFOV(tvFOV, 0.0, EasingType_Linear);

		local levelLightModeEnum = ::Light.getLevelLightModeFromName(levelLightMode);

		local levelSettings = ::getEntitiesByTag(getName());
		if (levelSettings.len() != 0)
		{
			::tt_panic("More than one level settings entities found in this level. Only one is allowed per level. " +
			         "Are you starting a mission? If you are starting a level, verify the 'default mission' has been set.");
			return;
		}

		if (musicTrack != null)
		{
			local ms = ::MusicSystem.getInstance();
			local startPlaying = musicStartIntensity != "none";
			ms.init(musicTrack, startPlaying);
		}

		registerEntityByTag(getName());

		::Light.setLevelLightMode(levelLightModeEnum);
		::LightHelpers.setAmbientLight(levelAmbientLight);

		if (shoeboxSplitPriority != null)
		{
			setShoeboxSplitPriority(::g_ShoeboxSplitPriorities[shoeboxSplitPriority]);
		}

		//setTileAttributesVisible(showTiles);

		_backgroundLayers = ::Blur.getRangeArrayFromMembers(this, "backgroundBlurRange");
		_foregroundLayers = ::Blur.getRangeArrayFromMembers(this, "foregroundBlurRange");
		_backgroundLayersDirty = true;
		_foregroundLayersDirty = true;

		// ColorGrading
		::ColorGrading.reset();
		if (colorGrading != null)
		{
			::ColorGrading.add(colorGrading, 0.0);
		}

		// Fog
		if (useFog)
		{
			if (fogColor == null) fogColor = ColorRGB(128, 128, 128);
			local distance = ::Camera.getCameraDistance();
			::setDefaultFogColor(fogColor, 0.0, EasingType_Linear);
			::setDefaultFogNearFar(-fogNear + distance, -fogFar + distance, 0.0, EasingType_Linear);
		}
		else
		{
			resetFogSettings();
		}

		::BaseEnemy.setWeaponDropFactor(1.0);
		::BaseEnemy.setHealthDropFactor(1.0);
		::BaseEnemy.setDropRate(1.0);

		::Game.init();
		::Audio.resetVolumes();

		::Gravity.set(GravityType.Normal);
		::setColorGradingAfterHud(applyColorGradingAfterHUD);

		::Camera.setRotation(0, 0, EasingType_QuadraticInOut);

		// Ensure the singletons exists after this onInit()
		::Hud.getInstance();
		::ConversationMgr.getInstance();
		::EnergyMgr.getInstance();

		::removeEntityFromWorld(this);

		if (showHud)
		{
			::Hud.showLayers(HudLayer.Normal);
		}
		else
		{
			::Hud.hide();
		}
	}

	function onSpawn()
	{
		if (musicTrack != null)
		{
			// Don't do this immediately after the MusicSystem.init(), otherwise the fade will fail
			local ms = ::MusicSystem.getInstance();
			local intensityLevel = 0;
			if      (musicStartIntensity == "slow")   intensityLevel = 1;
			else if (musicStartIntensity == "medium") intensityLevel = 2;
			else if (musicStartIntensity == "fast")   intensityLevel = 3;

			ms.setIntensityLevel(intensityLevel, musicStartFadeDuration);
		}
	}

	function onProgressRestored(p_id)
	{
		::ProgressMgr.restore();
		::OptionsData.restore();
		// FIXME: Move this to the game class (requiring Game to become an entity)
		::Game.setDeltaTimeScale(1.0);
		::Audio.restoreVolumes();
	}

	function onReloadRequested()
	{
		::Audio.resetVolumes();
	}

	function update(p_deltaTime)
	{
		if (_backgroundLayersDirty)
		{
			::setBackgroundBlurLayers(_backgroundLayers);
			_backgroundLayersDirty = false;
		}
		if (_foregroundLayersDirty)
		{
			setForegroundBlurLayers(_foregroundLayers);
			_foregroundLayersDirty = false;
		}

		// FIXME: Not ideal that we manually have to update the physics in the LevelSettings entity
		::Gravity.update(p_deltaTime);
	}

	_backgroundLayers = null;
	_foregroundLayers = null;
	_backgroundLayersDirty = false;
	_foregroundLayersDirty = false;
	function setBackgroundLayers(p_layers)
	{
		_backgroundLayers = p_layers;
		_backgroundLayersDirty = true;
	}

	function setForegroundLayers(p_layers)
	{
		_foregroundLayers = p_layers;
		_foregroundLayersDirty = true;
	}

	function getBackgroundLayers()
	{
		return _backgroundLayers;
	}

	function getForegroundLayers()
	{
		return _foregroundLayers;
	}

	function getColorGradingWater()
	{
		return colorGradingWater != null ? colorGradingWater : "submerged";
	}
}

// Add members based on virus names
foreach (index, virus in ::VirusHelpers.getLockedViruses())
{
	LevelSettings.newmember(virus, false);
	LevelSettings.setattributes(virus, {type = "bool", order = 3 + index, group = "Hackable Entities" });
}

::Blur.addBackgroundRangeMembers(LevelSettings, "backgroundBlurRange", 100.1, { group = "Blur", conditional = "useBackgroundBlur == true" } );
::Blur.addForegroundRangeMembers(LevelSettings, "foregroundBlurRange", 101.1, { group = "Blur", conditional = "useForegroundBlur == true" } );
