include_entity("ColorGrading");
include_entity("rewind/hud/Hud");
include_entity("rewind/EntityChild");

// Don't use a class 'global' for this, as it causes issues with serialization
::g_visibleAffectedEntities <- {}

class VirusUploader extends EntityChild
</
	placeable      = Placeable_Hidden
	movementset    = "Static"
	collisionRect  = [ 0.0, 0.0, 0.1, 0.1 ]
	group          = "Rewind"
/>
{
	// Constants
	static c_uploadTime      = 0.5;
	static c_enableBeamDelay = 0.125;
	static c_spread          = 10;
	static c_range           = 13;
	static c_maxDistance     = 20.0;

	// Store / Restore Constants
	static c_internalsStoredOnLevelExit = ["_lastHackedEntitySpawnProps"];
	static c_internalsStoredForUpdate   = ["_lastHackedEntitySpawnProps"];

	// Internal values
	_sightDetectShape           = null;
	_sightUploadShape           = null;
	_sightSensor                = null;
	_closeRangeSensor           = null;
	_beam                       = null;
	_targetEntity               = null;
	_isActive                   = false;
	_beamSound                  = null;
	_colorGradingID             = null;
	_lastHackedEntity           = null;
	_lastHackedEntitySpawnProps = null;
	_uploadBulletTime           = null;

	_staticNoisePresentation    = null;
	_enablePresentation         = null;
	_leftPresentation           = null;
	_rightPresentation          = null;
	_signalText                 = null;

	// stats
	_previousHackableCount      = null;
	_entityInfoLabels           = null;
	_statsLabels                = null;
	_timeActive                 = 0;

	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn

	function onSpawn()
	{
		base.onSpawn();

		::g_visibleAffectedEntities.clear();

		_sightDetectShape = ConeShape(2.0, c_range, 0, c_spread);
		_sightUploadShape = ::CircleShape(0, c_maxDistance);

		_sightSensor = addSightSensor(_sightDetectShape, null, ::Vector2(0, 0));
		_sightSensor.setDefaultEnterAndExitCallback();
		_sightSensor.setFilterCallback("onSightFilter");
		_sightSensor.setDistanceSort(true);
		_sightSensor.setStopOnCrystal(false);
		_sightSensor.setEnabled(false);

		_closeRangeSensor = addSightSensor(::CircleShape(0, 2.0), null, ::Vector2(0, 0));
		_closeRangeSensor.setEnterCallback("onSightEnter");
		_closeRangeSensor.setFilterCallback("onSightFilter");
		_closeRangeSensor.setDistanceSort(true);
		_closeRangeSensor.setStopOnCrystal(false);
		_closeRangeSensor.setEnabled(false);

		_staticNoisePresentation = ::Hud.createElement(this, "presentation/hud_hackmode_static", HudAlignment.Center, HudLayer.VirusUploadMode);
		_staticNoisePresentation.start("", [], false, 0);
		_enablePresentation      = ::Hud.createElement(this, "presentation/hud_hackmode_enable", HudAlignment.Center, HudLayer.VirusUploadMode);

		// Left and right presentations
		_leftPresentation       = ::Hud.createElement(this, "presentation/hud_hackmode_left", HudAlignment.Left, HudLayer.VirusUploadMode);
		_leftPresentation.start("", [], false, 0);

		_rightPresentation       = ::Hud.createElement(this, "presentation/hud_hackmode_right", HudAlignment.Right, HudLayer.VirusUploadMode);
		_rightPresentation.start("", [], false, 0);

		_signalText = ::Hud.createTextElement(this,
		{
			locID               = "HACKMODE_FOOTER_SCANNING",
			width               = 0.7,
			height              = 0.040,
			glyphset            = GlyphSetID_Text,
			hudalignment        = HudAlignment.Center,
			presentation        = "hud_hackmode_footer",
			position            = ::Vector2(0.0, -0.441),
			color               = ColorRGBA(255,255,255,155),
			horizontalAlignment = HorizontalAlignment_Center,
			layer               = HudLayer.VirusUploadMode,
			autoStart           = false
		});

		_statsLabels =
		{
			ANGLE = createStatsLabel(-0.403, "ANGLE")
			RANGE = createStatsLabel(-0.423, "RANGE")
			TIME  = createStatsLabel(-0.443, "TIME")
		};

		// Create all entity labels
		local posy = -0.443;
		_entityInfoLabels = {};
		_entityInfoLabels["TOTAL"] <- createEntityLabel(posy, "TOTAL");
		posy += 0.03;
		local viruses = ::VirusHelpers.getAllViruses();
		foreach(virus in viruses)
		{
			_entityInfoLabels[virus] <- createEntityLabel(posy, virus);
			posy += 0.02;
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks

	function onSightEnter(p_entity, p_sensor)
	{
		attemptToStartUploading(p_entity, p_sensor);
	}

	function onSightExit(p_entity, p_sensor)
	{
		if (p_entity.equals(_targetEntity))
		{
			p_sensor.setStopOnSolid(false);
		}
	}

	function onSightFilter(p_entity)
	{
		return ::VirusHelpers.canUploadVirus(p_entity);
	}

	function onTimer(p_name)
	{
		if (p_name == "enableBeam")
		{
			_sightSensor.setEnabled(true);
			_closeRangeSensor.setEnabled(true);
		}
		else if (p_name == "uploading")
		{
			uploadVirus(_targetEntity);
			stop();
		}
	}

	function onEndMission(p_missionID)
	{
		if (::ProgressMgr.getPlayMode() == PlayMode.Mission)
		{
			resetLastHackedEntity();
		}
	}

	function onProgressRestored(p_id)
	{
		if (p_id == "editor")
		{
			// Clean up affected entities as some might've deleted when returning from editor
			local newVisibleEntities = {};
			foreach (ref, presentation in ::g_visibleAffectedEntities)
			{
				if (ref != null && ::isValidAndInitializedEntity(ref.ref()))
				{
					newVisibleEntities[ref] <- presentation;
				}
			}
			::g_visibleAffectedEntities = newVisibleEntities;
		}

		// Make sure there are no more running timers and that the correct sight shape is set
		stop();

		// reconnect the shape with the sensor because that gets lost
		_sightSensor.setShape(_sightDetectShape);
	}

	function onReloadRequested()
	{
		// Get rid if potential hacked entity
		resetLastHackedEntity();

		base.onReloadRequested();
	}

	function onDie()
	{
		base.onDie();

		::Hud.destroyTextElement(_signalText);
		::Hud.destroyElement(_staticNoisePresentation);
		::Hud.destroyElement(_leftPresentation);
		::Hud.destroyElement(_rightPresentation);
		::Hud.destroyElement(_enablePresentation);

		foreach (elem in _entityInfoLabels)
		{
			::Hud.destroyTextElement(elem.name);
			::Hud.destroyTextElement(elem.number);
		}

		foreach (elem in _statsLabels)
		{
			::Hud.destroyTextElement(elem.name);
			::Hud.destroyTextElement(elem.number);
		}

		stop();

		::g_visibleAffectedEntities.clear();
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods

	function resetLastHackedEntity()
	{
		_lastHackedEntity = null;
	}

	function registerVisibleEntity(p_entity)
	{
		::g_visibleAffectedEntities[p_entity.weakref()] <- null;
	}

	function unregisterVisibleEntity(p_entity)
	{
		local ref = p_entity.weakref();
		if (ref in ::g_visibleAffectedEntities)
		{
			destroyMarker(p_entity);
			delete ::g_visibleAffectedEntities[ref];
		}
	}

	function setAngle(p_angle)
	{
		_sightDetectShape.setAngle(p_angle);
	}

	function setRange(p_range)
	{
		_sightDetectShape.setMaxRadius(p_range);
	}

	function start()
	{
		if (_isActive)
		{
			return;
		}

		_isActive = true;
		_timeActive = 0.0;
		_previousHackableCount = null;

		startTimer("enableBeam", c_enableBeamDelay);

		if (_parent._virusUploaderLight != null)
		{
			_parent._virusUploaderLight.enableLight(0.1);
		}

		_parent.setDimmedSpotLight(true);

		if (_parent._reticule != null)
		{
			_parent._reticule.hide();
		}

		_colorGradingID = ::ColorGrading.add("hackvision", 0.2, ColorGradingPriority.High);
		::setColorGradingEffectsEnabled(false);
		_enablePresentation.start("", [], false, 0);
		Hud.showLayers(HudLayer.VirusUploadMode);

		updateMarkers();

		::Audio.duckVolume(DuckingPreset.VirusUploader);

		_parent.customCallback("onVirusUploaderStarted");
	}

	function stop()
	{
		if (_isActive == false)
		{
			return;
		}

		_isActive = false;
		_sightSensor.setEnabled(false);
		_sightSensor.setStopOnSolid(true);
		_closeRangeSensor.setEnabled(false);

		stopUploading();
		stopAllTimers();

		_parent.setDimmedSpotLight(false);

		::ColorGrading.remove(_colorGradingID, 0.2);
		::setColorGradingEffectsEnabled(true);

		if (Hud.getLayers() == HudLayer.VirusUploadMode)
		{
			Hud.showLayers(HudLayer.Normal);
		}

		updateMarkers();

		::Audio.unduckVolume(DuckingPreset.VirusUploader);

		_parent.customCallback("onVirusUploaderStopped");
	}

	function stopUploading()
	{
		if (::isValidEntity(_uploadBulletTime))
		{
			// Hack failed if _uploadBulletTime is still active
			// Spawn end bullettime speedup effect
			::killEntity(_uploadBulletTime);
			::spawnEntity("BulletTime", ::Vector2(0, 0),
				{
					_startDelay      = 0.0,
					_slowdownTime    = 0.0,
					_bulletTime      = 0.0,
					_speedupTime     = 0.1,
					_endScale        = 0.15,
				}
			);
			playSoundEffect("hackseq_fail");

			if (::isValidEntity(_targetEntity))
			{
				_targetEntity.spawnParticleOneShot("particles/virusuploader_failed", ::Vector2(0, 0), true, 0, false,
				                                   ParticleLayer_UseLayerFromParticleEffect, 1.0);
			}
		}

		_targetEntity = null;
		if (_beam != null)
		{
			_parent.removePowerBeamGraphic(_beam);
		}
		_sightSensor.setTarget(null);
		_sightSensor.setShape(_sightDetectShape);
		stopAllTimers();

		if (_parent._reticule != null)
		{
			_parent._reticule.show();
		}

		if (_parent._virusUploaderLight != null)
		{
			_parent._virusUploaderLight.disableLight(0.1);
		}

		if (_beamSound != null)
		{
			_beamSound.stop();
			_beamSound = null;
		}
	}

	function attemptToStartUploading(p_entity, p_sensor)
	{
		if (_isActive == false || _targetEntity != null)
		{
			// Not active or already established connection
			return;
		}

		local slowdownTime = 0.1;
		local timeScale = 0.15;

		_targetEntity = p_entity.weakref();
		_beam = _parent.addPowerBeamGraphic(PowerBeamType_Hack, _sightSensor);
		startTimer("uploading", slowdownTime + (c_uploadTime * timeScale));
		_sightSensor.setTarget(_targetEntity);
		_sightSensor.setShape(_sightUploadShape);
		_beamSound = playSoundEffect("hackseq_loop");
		playSoundEffect("hackseq_start");

		::setRumble(RumbleStrength_Low, 0.0);

		local ref = p_entity.weakref();
		if (::g_visibleAffectedEntities[ref] != null)
		{
			::g_visibleAffectedEntities[ref].start("hack_hud", ["uploading"], false, 10);
		}

		_uploadBulletTime = ::spawnEntity("BulletTime", ::Vector2(0, 0),
		{
			_startDelay      = 0.0,
			_slowdownTime    = slowdownTime,
			_bulletTime      = 10.0, // More than enough
			_speedupTime     = 0.0,
			_endScale        = timeScale,
		});

		_parent._virusUploaderLight.disableLight();
	}

	function uploadVirus(p_entity)
	{
		if (::isValidEntity(p_entity) == false)
		{
			return;
		}

		// Remove virus from last hacked entity, but only if target isn't a virusuploadtrigger or train
		if ((p_entity instanceof ::VirusUploadTrigger) == false &&
		    (p_entity instanceof ::VirusUploadSensor) == false)
		{
			if (_lastHackedEntity != null)
			{
				_lastHackedEntity.customCallback("onVirusRemoved");
			}

			_lastHackedEntity = p_entity.weakref();
		}

		// Remove marker
		destroyMarker(p_entity);

		spawnParticleOneShot("particles/virusuploader_success", ::Vector2(0, 0), true, 0, false, ParticleLayer_UseLayerFromParticleEffect, 1.0);
		p_entity.customCallback("onVirusUploaded", _parent);
		::setRumble(RumbleStrength_Medium, 0.0);

		::killEntity(_uploadBulletTime);
		_uploadBulletTime = null;
		playSoundEffect("hackseq_succes");

		// Spawn end bullettime speedup effect
		::spawnEntity("BulletTime", ::Vector2(0, 0),
		{
			_startDelay      = 0.0,
			_slowdownTime    = 0.0,
			_bulletTime      = 0.0,
			_speedupTime     = 0.15,
			_endScale        = 0.15,
		});
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Markers

	function createHackedIndicator(p_entity)
	{
		//Add the hacked particle effect (scale based on collisionrect)
		local collisionRect = p_entity.getCollisionRect();

		local presentation = p_entity.createPresentationObjectInLayer("presentation/hud_hackmode_hacked_indicator",
		                                                              ParticleLayer_BehindHud);
		presentation.setAffectedByOrientation(false);
		presentation.setAffectedByFloorDirection(false);

		local label = p_entity.addTextLabel("HACKMODE_HACK_INDICATOR", 3.0, 0.5, GlyphSetID_Text);
		label.setHorizontalAlignment(HorizontalAlignment_Left);
		label.setVerticalAlignment(VerticalAlignment_Center);
		label.setColor(ColorRGBA(255,255,255,158));
		presentation.addTextLabel(label, ::Vector2(-1.5, 1.6));

		// Use a high prio like 10 so that it remains intact even if entity uses startAllPresentationObjects
		presentation.addCustomValue("health", p_entity.getVirusHealthForIndicator());
		presentation.addTag(p_entity.getType());
		presentation.start("", [], false, 10);

		local effect = p_entity.spawnParticleContinuous("particles/virusupload_binary_small", ::Vector2(0, 0), true, 0, false,
		                                                ParticleLayer_UseLayerFromParticleEffect, 1.0);
		effect.setEmitterProperties(0, { radius = collisionRect.getWidth() * 0.5, inner_radius = collisionRect.getWidth() * 0.5});

		return { presentation = presentation, effect = effect }
	}

	function destroyHackedIndicator(p_entity, p_indicator)
	{
		if (p_indicator != null)
		{
			p_indicator.effect.stop(false);
			p_indicator.effect = null;
			p_entity.destroyPresentationObject(p_indicator.presentation);
			p_indicator.presentation = null;
		}
	}

	function createMarker(p_entity)
	{
		local presentation = p_entity.createPresentationObjectInLayer("presentation/hud_hackmode_marker",
		                                                              ParticleLayer_BehindHud);
		presentation.setAffectedByOrientation(false);
		presentation.setAffectedByFloorDirection(false);
		presentation.addTag(p_entity.getType());
		presentation.addTag(::getDirectionName(p_entity.getFloorDirection()));

		// Use a high prio like 10 so that it remains intact even if entity uses startAllPresentationObjects
		presentation.start(_isActive ? "hack_hud" : "normal_hud", [], false, 10);
		return presentation;
	}

	function destroyMarker(p_entity)
	{
		local ref = p_entity.weakref();
		if (::g_visibleAffectedEntities[ref] != null)
		{
			p_entity.destroyPresentationObject(::g_visibleAffectedEntities[ref]);
			::g_visibleAffectedEntities[ref] = null;
		}
	}

	function updateMarkers()
	{
		local startName = _isActive ? "hack_hud" : "normal_hud";

		foreach (ref, presentation in ::g_visibleAffectedEntities)
		{
			if (presentation != null)
			{
				local entity = ref.ref();

				// Use a high prio like 10 so that it remains intact even if entity uses startAllPresentationObjects
				presentation.start(startName, [], false, 10);
			}
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Information labels

	function createStatsLabel(p_posy, p_label)
	{
		local name = ::Hud.createTextElement(this,
		{
			locID               = "HACKMODE_STATS_" + p_label,
			presentation        = "hud_hackmode_text",
			width               = 0.375,
			height              = 0.035,
			glyphset            = GlyphSetID_Text,
			hudalignment        = HudAlignment.Right,
			position            = ::Vector2(-0.36, p_posy),
			color               = ColorRGBA(255,255,255,155),
			horizontalAlignment = HorizontalAlignment_Left,
			layer               = HudLayer.VirusUploadMode,
			autoStart           = true
		});

		local number = ::Hud.createTextElement(this,
		{
			locID               = "",
			presentation        = "hud_hackmode_text",
			width               = 0.12,
			height              = 0.035,
			glyphset            = GlyphSetID_Text,
			hudalignment        = HudAlignment.Right,
			position            = ::Vector2(-0.065, p_posy),
			color               = ColorRGBA(255,255,255,155),
			horizontalAlignment = HorizontalAlignment_Right,
			layer               = HudLayer.VirusUploadMode,
			autoStart           = true
		});

		return { name = name, number = number };
	}

	function createEntityLabel(p_posy, p_label)
	{
		local name = ::Hud.createTextElement(this,
		{
			locID               = "HACKMODE_ENTITYINFO_" + p_label.toupper(),
			presentation        = "hud_hackmode_text",
			width               = 0.325,
			height              = 0.035,
			glyphset            = GlyphSetID_Text,
			hudalignment        = HudAlignment.Left,
			position            = ::Vector2(0.060, p_posy),
			color               = ColorRGBA(255,255,255,155),
			horizontalAlignment = HorizontalAlignment_Left,
			layer               = HudLayer.VirusUploadMode
		});
		name.label.setNumberOfVisibleCharacters(0);

		local number = ::Hud.createTextElement(this,
		{
			locID               = "",
			presentation        = "hud_hackmode_text",
			width               = 0.04,
			height              = 0.035,
			glyphset            = GlyphSetID_Text,
			hudalignment        = HudAlignment.Left,
			position            = ::Vector2(0.29, p_posy),
			color               = ColorRGBA(255,255,255,155),
			horizontalAlignment = HorizontalAlignment_Right,
			layer               = HudLayer.VirusUploadMode
		});
		number.label.setNumberOfVisibleCharacters(0);

		return { name = name, number = number };
	}

	function updateEntityLabels(p_hackables, p_total)
	{
		// Hide locked labels
		local viruses = ::VirusHelpers.getVirusData();
		foreach (virus, unlocked in viruses)
		{
			if (unlocked)
			{
				_entityInfoLabels[virus].name.label.setNumberOfVisibleCharacters(-1);
				_entityInfoLabels[virus].number.label.setNumberOfVisibleCharacters(-1);
				_entityInfoLabels[virus].number.label.setText("0");
			}
			else if (unlocked == false)
			{
				_entityInfoLabels[virus].name.label.setNumberOfVisibleCharacters(0);
				_entityInfoLabels[virus].number.label.setNumberOfVisibleCharacters(0);
			}
		}

		// Update numbers
		foreach (type, number in p_hackables)
		{
			_entityInfoLabels[type].number.label.setText(number.tostring());
		}

		_entityInfoLabels["TOTAL"].name.label.setNumberOfVisibleCharacters(-1);
		_entityInfoLabels["TOTAL"].number.label.setText(p_total.tostring());
	}

	function updateFooter(p_hackableCount)
	{
		if (p_hackableCount > 1)
		{
			_signalText.label.setTextLocalized("HACKMODE_FOOTER_INTERCEPTING");
			_signalText.presentation.start("intercepting", [], false, 0);
		}
		else if (p_hackableCount == 1)
		{
			_signalText.label.setTextLocalized("HACKMODE_FOOTER_INTERCEPTINGSINGULAR");
			_signalText.presentation.start("intercepting", [], false, 0);
		}
		else
		{
			_signalText.label.setTextLocalized("HACKMODE_FOOTER_SCANNING");
			_signalText.presentation.start("scanning", [], false, 0);
		}
	}

	function updateStats()
	{
		local angle = _sightDetectShape.getAngle();
		local range = _sightDetectShape.getMaxRadius();
		_statsLabels["TIME"].number.label.setText(::format("%.2f", _timeActive));
		_statsLabels["ANGLE"].number.label.setText(::format("%.2f", angle));
		_statsLabels["RANGE"].number.label.setText(::format("%.2f", range));
	}


	////////////////////////////////////////////////////////////////////////////////////////////////
	// Child update

	function childUpdate(p_deltaTime)
	{
		local hackableCount = 0;
		local hackables = {};

		foreach (ref, presentation in ::g_visibleAffectedEntities)
		{
			if (ref == null)
			{
				::tt_panic("Invalid reference present in ::g_visibleAffectedEntities. This should not happen.");
				delete ::g_visibleAffectedEntities[ref];
				break;
			}

			local entity = ref.ref();
			if (::isValidEntity(entity) == false)
			{
				::tt_panic("Invalid entity present in ::g_visibleAffectedEntities. This should not happen.");
				delete ::g_visibleAffectedEntities[ref];
				break;
			}

			local type = entity.getType();
			if (entity.containsVirus() == false &&
			    ::VirusHelpers.hasUnlockedVirus(type))
			{
				if (type in hackables)
				{
					hackables[type]++;
				}
				else
				{
					hackables[type] <- 1;
				}
				hackableCount++;

				if (presentation == null)
				{
					::g_visibleAffectedEntities[ref] = createMarker(entity);
				}
			}
		}

		if (_isActive)
		{
			_timeActive += p_deltaTime;
			updateStats();
			if (_previousHackableCount != hackableCount)
			{
				updateFooter(hackableCount);
				updateEntityLabels(hackables, hackableCount);
			}
		}

		_previousHackableCount = hackableCount;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// LevelExit Handling Methods

	function onLevelExit()
	{
		if (::isValidEntity(_lastHackedEntity))
		{
			// Prepare spawnprops
			_lastHackedEntitySpawnProps =
			{
				type   = _lastHackedEntity.getType(),
				health = _lastHackedEntity._healthBar.getNormalizedHealth(),
				props  = { _virusHealth = _lastHackedEntity._virusHealth }
			};
		}
		else
		{
			_lastHackedEntitySpawnProps = null;
		}

		base.onLevelExit();
	}

	function _restoreTemporaryInternals()
	{
		base._restoreTemporaryInternals();

		if (_lastHackedEntitySpawnProps != null)
		{
			local props = _lastHackedEntitySpawnProps;
			local entity = ::spawnEntity(props.type, _parent.getCenterPosition(), props.props);
			if (entity != null)
			{
				_lastHackedEntity = entity.weakref();
				entity.customCallback("onVirusUploaded", _parent);
				entity._healthBar.setNormalizedHealth(props.health);
			}
			_lastHackedEntitySpawnProps = null;
		}
	}
}
