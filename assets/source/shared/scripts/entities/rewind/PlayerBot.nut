include_entity("ColorGrading");
include_entity("rewind/RewindEntity");
include_entity("triggers/Trigger");

class PlayerBot extends RewindEntity
</
	editorImage         = "editor.playerbot"
	libraryImage        = "editor.library.playerbot"
	placeable           = Placeable_Everyone
	movementset         = "Static"
	collisionRect       = [0.0, 0.75, 1.5, 1.5]
	order               = 0
	ignoreSpawnSections = true
/>
{
	</
		type           = "entity"
		filter         = ["WarpPlateau"]
		order          = 1
		referenceColor = ReferenceColors.Enable
		description    = "When set, this PlayerBot will start at WarpPlateau with cinematic"
	/>
	warpPlateau = null;

	// Weapons
	_primaryWeapon   = null;
	_secondaryWeapon = null;
	_fireworksCount  = 0;

	_virusUploader = null;
	_shield        = null;  // unused but needs to be here for Healthbar hackery

	_aimAngle = 0;
	_targetAngle = 0;

	//Presentations
	_bodyPresentation                     = null;
	_bodybackPresentation                 = null;

	_recoilForce = null;
	_recoilPower = 0;
	_recoildDampening = 0.05;
	_recoilCameraDuration = 0.075; // The time the camera needs to recover to its regular position after recoiling
	_recoilCameraScale    = -0.25; // The distance the camera shoots back when firing a bullet

	_bulletHitExtraImpulse = ::Vector2(0, 2500); // external impulse force added when the player is hit
	_zeroGHitImpactFactor  = 2.5;     // scale factor for bullet knockbacks in zero g

	// FIXME: Now set to 1.0 to see if we need it at all
	_zeroGExplosionImpactFactor = 1.0; // scale factor for explosion knockbacks in zero g

	static c_mass                         = 5.0;
	static c_predictiveJumpTimeoutFalling = 0.1; // duration in seconds to allow jumping before hitting the ground
	static c_predictiveJumpTimeoutZeroG   = 0.25; // duration in seconds to allow jumping before leaving a gravity field

	static c_bulletPropertiesButterfly =
	{
		_gunType               = "ProjectileGun",
		_gunPresentationName   = "playerbot_turret_butterfly",
		_muzzleFlashEffectName = "muzzleflash_player",
		_projectileType        = "Butterfly",
		_projectileSpeed       = 25,
		_firingRate            = 20,
		_barrelCount           = 1,
		_barrelLength          = 1.0,
		_barrelRandomSpread    = 45.0,
		_infiniteAmmo          = false,
		_flags                 = GunPresetFlags.None,
		_projectileSpawnProps  =
		{
			_timeout  = 1.6
		}
	}

	static c_bulletPropertiesNormal =
	{
		_gunType               = "PlayerBotGun",
		_gunPresentationName   = "playerbot_turret",
		_muzzleFlashEffectName = "muzzleflash_player",
		_projectileType        = "PlayerBullet",
		_projectileSpeed       = 1.25,
		_firingRate            = 20,
		_barrelCount           = 1,
		_barrelLength          = 1.0,
		_barrelRandomSpread    = 8.0,
		_infiniteAmmo          = true,
		_flags                 = GunPresetFlags.None,
		_projectileSpawnProps  =
		{
			_timeout     = 1.6,
			_damageValue = 1.0
		}
	}

	static c_bulletPropertiesEasy =
	{
		_gunType               = "PlayerBotGun",
		_gunPresentationName   = "playerbot_turret",
		_muzzleFlashEffectName = "muzzleflash_player",
		_projectileType        = "PlayerBullet",
		_projectileSpeed       = 1.25,
		_firingRate            = 20,
		_barrelCount           = 1,
		_barrelLength          = 1.0,
		_barrelRandomSpread    = 8.0,
		_infiniteAmmo          = true,
		_flags                 = GunPresetFlags.None,
		_projectileSpawnProps  =
		{
			_timeout     = 1.6,
			_damageValue = 1.5
		}
	}

	static c_endMissionBulletProperties =
	{
		_gunType                        = "PlayerBotGun",
		_gunPresentationName            = "playerbot_turret",
		_muzzleFlashEffectName          = "muzzleflash_player",
		_projectileType                 = "Fireworks",
		_projectileSpeed                = 0.40,
		_projectileParentSpeedFactor    = 0.0,
		_projectileSpeedRandom          = 0.30,
		_firingRate                     = 20,
		_barrelCount                    = 4,
		_barrelLength                   = 1.0,
		_barrelSpread                   = 20,
		_barrelRandomSpread             = 20,
		_infiniteAmmo                   = true,
		_flags                          = GunPresetFlags.None,
	}

	// The thrusters
	_thrusters       = null;
	_activeThruster  = null;

	_movementSettings     = null;
	_maxSpeed             = 60;
	_maxSpeedInWater      = 60; // For press demo purposes, this is fine.

	_externalForces = null;
	_lastHitForce = null;

	_isOnFloor = null;

	_cameraOffsetEffect = null;
	_recoilCameraEffect = null;

	_energyContainer = null;
	_scoreContainer  = null;

	_healthBar = null;
	_shield = null;

	_reticule            = null;
	_spotLight           = null;
	_virusUploaderLight  = null;

	// Internal values below
	_colorGradingIDs   = null;
	_colorGradingWater = null;

	// Internal value
	_waterTweenInDuration = 0.0;
	_waterTweenOutDuration = 0.3;

	_zeroGravityTweenInDuration = 0.0;
	_zeroGravityTweenOutDuration = 0.4;
	_zeroGravitySound = null;

	// Contunue text label
	_continueTextLabel = null;

	positionCulling = false;

	// Used by force triggers
	// FIXME: Could be done more elegantly
	_extraForce = null;

	_status = null;
	_buttonPrimaryFirePressed = false;
	_lastControllerType       = null;
	_hardModePresentation     = null;

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Controls

	</
		autoGetSet = true
	/>
	_lookAheadEnabled       = true;

	_moveControlsEnabled    = null;
	_aimControlsEnabled     = null;
	_weaponsEnabled         = null;
	_hackControlsEnabled    = null;
	_lightEnabled           = false;
	_lockAimAngle           = null;
	_butterflyMode          = false;
	_isOnPlatform           = false;
	_endJingle              = null;

	function onCreate(p_id)
	{
		::Camera.resetFollowStack(); // reset the camera follow stack before anything has had the option to use camera following
		return true;
	}

	function onInit()
	{
		base.onInit();

		setCanBePushed(true);

		if (warpPlateau != null)
		{
			warpPlateau.attachPlayer(this);
		}

		// Create movement controller BEFORE calling setCanBeCarried
		createMovementController(false);
		setCanBeCarried(true);
		setUpdateSurvey(true);

		_moveControlsEnabled    = ::RefCountedBool(true);
		_aimControlsEnabled     = ::RefCountedBool(true);
		_hackControlsEnabled    = ::RefCountedBool(true);
		_weaponsEnabled         = ::RefCountedBool(true);

		// Make sure there is a levelsettings entity at startup
		{
			local levelSettings = ::getFirstEntityByTag("level_settings");
			if (levelSettings == null)
			{
				editorWarning("Missing LevelSettings entity");
			}

			// Unlock the viruses set in the LevelSettings
			::VirusHelpers.resetAllViruses();
			foreach (virus in ::VirusHelpers.getLockedViruses())
			{
				if (levelSettings[virus])
				{
					::VirusHelpers.unlockVirus(virus);
				}
			}

			// Set default colorgrading for water
			_colorGradingWater = levelSettings.getColorGradingWater();
		}

		addProperty("openDoors");
		addProperty("weaponGroup", ::WeaponGroup.Player);
		addProperty("hasCorpse", ::CorpseParams("PlayerBotCorpse", { _presentationFile = "playerbot_body" })); //playerbot_body
		addProperty("noticeExplosions");
		addProperty("noticeBullets");
		addProperty("noticeFlames");
		addProperty("noticeCrush");
		addProperty("noticeShredders");
		addProperty("noticeTrainTracks");
		addProperty("noticeForceFields");
		addProperty("noticeDeathRays");
		addProperty("attractHomingMissiles");
		addProperty("healedByHealthBot");
		//removeProperty("hasCorpse"); //We spawn our own particles

		registerEntityByTag("PlayerBot");

		// Create color grading IDs table
		_colorGradingIDs = {};

		_recoilForce = ::Vector2(0, 0);

		_cameraOffsetEffect = ::Camera.createCameraEffect();
		_recoilCameraEffect = ::Camera.createCameraEffect();
		local crushSensor = addCrushSensor();

		// Create presentations
		_bodyPresentation          = createPresentationObject("presentation/playerbot_body");
		_bodybackPresentation      = createPresentationObject("presentation/playerbot_bodyback");

		// Interract with water & lava
		enableDefaultWaterInteraction();

		local settings = addFluidSettings(FluidType_Lava);
		settings.setWaveGenerationEnabled(true);

		// Set his sight detectionPoint a bit higher so enemies can see him when he is in a 1 tile water pool
		setSightDetectionPoints([::Vector2(0, getCollisionRect().getHeight() * 0.5)]);

		// Movement setting
		_movementSettings = PhysicsSettings(2.0, 4.0, 70.0, -1, -1);
		_movementSettings.setCollisionIsMovementFailure(false);

		// Disable direct controller input
		_movementSettings.setThrust(0);

		// Set debug values
		if (::isTestBuild())
		{
			addProperty("debugCurrentSpawnPointIndex", 0);
			addProperty("debugInitialSpawnLocation", getPosition());
		}

		_extraForce = ::Vector2(0, 0);

		// Cannot be moved to onSpawn, as water callbacks depend on availability of thrusters
		// and those are fired right after the onInit
		addThrusters();
	}

	function onSpawn()
	{
		base.onSpawn();

		// Spawn player at level exit position
		{
			local spawnSettings = SpawnSettings.get();
			// Handle the spawn point set by the level exit
			if ("SpawnPointID" in spawnSettings)
			{
				local id = spawnSettings.SpawnPointID;

				foreach (entity in ::getEntitiesByTag("SpawnPoint"))
				{
					if (entity.getTargetID() == id)
					{
						entity.spawn(this);
						break;
					}
				}
			}

			SpawnSettings.clear();
		}
		// Create all children once playerbot is at correct position
		addChildren();

		// Make the camera follow the player
		// FIXME: Add logic here for multiplayer as the camera cannot simply follow every PlayerBot
		::Camera.setPrimaryFollowEntity(this);
		::Camera.setScrollingEnabled(false);
		::addButtonInputListeningEntity(this, InputPriority.Normal);

		startMovementInDirection(::Vector2(0, 0), _movementSettings);

		// trigger possible child triggers first, so the level exits can override the "defaults"
		triggerChildTriggers(this);

		registerZeroGravityAffectedEntity(this);

		if (::isTestBuild())
		{
			local godmode = getRegistry().get("godmode");
			if (godmode)
			{
				// Cheat toggles the value, so set it to false so onButtonDebugCheatPressed can enable it again
				getRegistry().set("godmode", false);
				onButtonDebugCheatPressed();
			}
		}

		// Signal player is ready
		::ProgressMgr.onPlayerReady(this);

		// Handle easy mode
		::ProgressMgr.setEasyMode(::ProgressMgr.isEasyModeEnabled());

		resetStatus();
	}

	function onHealthBarEmpty(p_healthBar, p_killer)
	{
		// Store killer for game over message
		addProperty("killer", p_killer);

		base.onHealthBarEmpty(p_healthBar, p_killer);
	}

	function onPrepareForLevelExit(p_levelExit)
	{
		::stopRumble();

		local hackedEntity = _virusUploader._lastHackedEntity;
		if (p_levelExit.discardHackedEntity ||
		   (hackedEntity != null &&
		    hackedEntity.getCenterPosition().distanceTo(getCenterPosition()) > 50.0))
		{
			_virusUploader.resetLastHackedEntity();
		}
	}

	function onLevelExit()
	{
		callOnChildren("onLevelExit");
	}

	function onMissionRestart()
	{
		callOnChildren("onMissionRestart");
	}

	function onDie()
	{
		::Hud.destroyElement(_hardModePresentation);

		local prop = getProperty("hasCorpse");
		prop.spawnProperties["_scoreContainer"] <- _scoreContainer;

		// handle game over message logic
		local dieQuietly = hasProperty("dieQuietly");
		if (dieQuietly == false)
		{
			if (::frnd() > 0.5)
			{
				::Audio.playGlobalSoundEffect("VoiceOver", "ROUGHSHOT_DIE");
			}

			local killer = getProperty("killer");
			local prop = getProperty("hasCorpse");
			local killString = null;
			if (killer != null)
			{
				killString = typeof(killer) == "string" ? killer.toupper() :
				                                          killer.getType().toupper();
				if (("_shooter" in killer && killer._shooter.equals(this)) ||
				    ::stringStartsWith(killString, "LAVA") ||
				    ::stringStartsWith(killString, "TRAIN") ||
				    killString == "SHREDDER")
				{
					_scoreContainer.addSuicide();
				}
			}

			::Stats.submitTelemetryEvent("player_died", killString);

			// Add killer to corpse params
			prop.spawnProperties["_messageID"] <- ::getGameOverMessageID(killer);
			prop.spawnProperties["_stats"] <- getActualStats();

			if (isInWater())
			{
				::Stats.unlockAchievement("sleeping_with_fishes");
				::Stats.storeAchievements();
			}

			// Make a pickup drop a bit earlier the next time the player spawns
			::BaseEnemy.incrementPickupDropCounter();

			// "release" gun if we were shooting
			if (_primaryWeapon != null)
			{
				_primaryWeapon.stopFiring();
			}

			::Camera.shakeScreen(1.0, getCenterPosition(), CameraShakeFlag.Default);

			::ColorGrading.oneShot("playerbot_hit_explosion", 0.01, 0.4, 2.5);

			// Spawn bullet time for extra dramatic effect
			::spawnEntity("BulletTime", ::Vector2(0, 0),
				{
					_startDelay      = 0.12,
					_slowdownTime    = 0.03,
					_bulletTime      = 0.0025,
					_speedupTime     = 0.45,
					_endScale        = 0.1,
				});

			// Spawn the explosion (used to be in the corpse presentation, but then it wasn't customisable)
			local effectName = "particles/playerbot_explosion";
			if      (isInWater())             effectName += "_underwater";
			else if (::isInZeroGravity(this)) effectName += "_zerog";

			local explosionEffect = spawnParticleOneShot(effectName, ::Vector2(0,0), false, 0, false, ParticleLayer_UseLayerFromParticleEffect, 1.0);

			if (_lastHitForce != null)
			{
				_lastHitForce.x *= 50.0;
				_lastHitForce.y *= 50.0;

				explosionEffect.setEmitterProperties(5,  { velocity_x = _lastHitForce.x, velocity_y = _lastHitForce.y,});
				explosionEffect.setEmitterProperties(6,  { velocity_x = _lastHitForce.x, velocity_y = _lastHitForce.y,});
				explosionEffect.setEmitterProperties(8,  { velocity_x = _lastHitForce.x, velocity_y = _lastHitForce.y,});
				explosionEffect.setEmitterProperties(9,  { velocity_x = _lastHitForce.x, velocity_y = _lastHitForce.y,});
				explosionEffect.setEmitterProperties(10, { velocity_x = _lastHitForce.x, velocity_y = _lastHitForce.y,});
				explosionEffect.setEmitterProperties(11, { velocity_x = _lastHitForce.x, velocity_y = _lastHitForce.y,});
				explosionEffect.setEmitterProperties(12, { velocity_x = _lastHitForce.x, velocity_y = _lastHitForce.y,});
				explosionEffect.setEmitterProperties(13, { velocity_x = _lastHitForce.x, velocity_y = _lastHitForce.y,});
			}
		}

		::ConversationMgr.clear()

		::removeButtonInputListeningEntity(this);

		MusicSource.stopAll();

		if (Hud.getLayers() != HudLayer.Menu)
		{
			Hud.showLayers(HudLayer.Death);
		}

		base.onDie();
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	//

	function onHUDShow()
	{
		callOnChildren("onHUDShow");
	}

	function onHUDHide()
	{
		callOnChildren("onHUDHide");
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	//
	function onTimer(p_name)
	{
		switch (p_name)
		{
		case "unlockSprayAndPray":
			::Stats.unlockAchievement("spray_and_pray");
			::Stats.storeAchievements();
			break;

		case "predictiveJumpTimeout":
			_predictiveJump = false;
			break;

		case "applyImpulseCooldown":
			setMoveControlsEnabled(true);
			break;

		default:
			break;
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////////////

	function getChildrenOffset()
	{
		// Use position of gun to position the lights, reticules etc
		// 0.5 is the offset as put in the presentation XML startvalues
		return getCenterOffset() + ::Vector2(0, 0.5);
	}

	function addThrusters()
	{
		_thrusters =
			{
				normal = addChild("Thruster", getCenterOffset(),
				{
					_presentationFile       = "presentation/playerbot_thruster",
					_movementSettings       = _movementSettings,
					_thrusterPower          = 0,
					_thrusterBoostPower     = 62,
					_thrusterBoostDampening = 0.37,
					_maxThrusterBoosts      = 2,
					_flySteerForce          = 90,
					_flyDrag                = 4.0,
					_walkSteerForce         = 1050,
					_walkDrag               = 50
				}),

				zeroGravity = addChild("RotatableThruster", getCenterOffset(),
				{
					_presentationFile       = "presentation/playerbot_thruster",
					_soundCueActiveName     = "playerbot_thruster_zerogravity"
					_movementSettings       = _movementSettings,
					_thrusterPower          = 3.5,
					_thrusterBoostPower     = 2,
					_thrusterBoostDampening = 0.5,
					_maxThrusterBoosts      = 0,
					_drag                   = 8.0
				}),

				water = addChild("WaterThruster", getCenterOffset(),
				{
					_presentationFile       = "presentation/playerbot_thruster",
					_soundCueActiveName     = "playerbot_thruster_water"
					_movementSettings       = _movementSettings,
					_thrusterPower          = 2.0,
					_maxThrusterPower       = 4.5,
					_thrusterBoostPower     = 1.5,
					_thrusterBoostDampening = 0.5,
					_maxThrusterBoosts      = 0,
					_drag                   = 10.0,
				})
			};

		setThruster(_thrusters.normal);
	}

	function addChildren()
	{
		local offset = getChildrenOffset();

		_reticule  = addChild("Reticule", offset, {});
		_spotLight = addChild("AttachedLight", offset,
			{
				_lightAngle                = 0,
				_lightSpread               = 230,
				_lightRange                = 40,
				_lightStrength             = 1,
				_lightTexture              = "playerbot_spotlight",
				_presentationFile          = "presentation/playerbot_spotlight",
			}
		);
		// This sets the spotlight color
		setDimmedSpotLight(false);

		// Determine upgrade levels
		local unlockedWeapons = [];
		local upgradeLevelHealth = 0;
		local upgradeLevelMagnet = 0;
		local energy = 1337;
		//if (::ProgressMgr.getPlayMode() != PlayMode.Campaign)
		{
			// Determine upgrade levels based on purchased items
			//if (::isTestBuild())
			{
				unlockedWeapons = ["homingmissile", "shotgun", "grenade", "emp"];
			}
			//else
			//{
			//	local items = ::ProgressMgr.getPurchasedItems();
			//	if (items.find("homingmissile") != null)  unlockedWeapons.push("homingmissile");
			//	if (items.find("shotgun") != null)        unlockedWeapons.push("shotgun");
			//	if (items.find("grenade") != null)        unlockedWeapons.push("grenade");
			//	if (items.find("emp") != null)            unlockedWeapons.push("emp");
			//}

			// Fail safe mechanism. This shouldn't happen though
			if (unlockedWeapons.len() == 0)
			{
				unlockedWeapons.push("homingmissile");
			}

			// Energy and magnet are always maxed out
			upgradeLevelHealth = 2;
			upgradeLevelMagnet = 2;

			// Only start with 1007 in campaigns
			//energy = 0;
		}

		_energyContainer = addChild("PlayerEnergyContainer", getCenterOffset(),
		{
			_energy       = energy,
			_upgradeLevel = upgradeLevelMagnet
		});
		_scoreContainer  = addChild("ScoreContainer", getCenterOffset());
		_healthBar       = addChild("PlayerBotHealthBar", getCenterOffset(),
		{
			_upgradeLevel = upgradeLevelHealth
		});

		_virusUploader = addChild("VirusUploader", offset);
		_virusUploaderLight = addChild("AttachedLight", offset,
			{
				_lightAngle                = 0,
				_lightSpread               = VirusUploader.c_spread,
				_lightRange                = VirusUploader.c_range,
				_lightStrength             = 1,
				_lightColor                = ColorRGBA(255, 255, 255, 255),
				_lightTexture              = "virusuploader_spotlight",
				_lightTextureRotationSpeed = 100,
				_presentationFile          = "presentation/playerbot_hackbeam",
			}
		);

		addChild("TouchDamager", getCenterOffset(),
			{
				_healthBar = _healthBar.weakref(),
			}
		);

		_externalForces = addChild("ExternalForces", getCenterOffset(),
			{
				_movementSettings = _movementSettings,
			}
		);

		createPrimaryWeapon();

		if (unlockedWeapons.len() > 0)
		{
			_secondaryWeapon = addChild("PlayerBotSecondaryWeapon", getCenterOffset(),
			{
				_unlockedTypes = unlockedWeapons,
				_type          = unlockedWeapons[0],
				_ammo          = 0
			});
		}
		else
		{
			_secondaryWeapon = addChild("PlayerBotSecondaryWeapon", getCenterOffset());
		}
	}

	function getMass()
	{
		return c_mass;
	}

	function getActualStats()
	{
		return _scoreContainer.getActualStats();
	}

	////////////////////////////////////////////////////////////////////////////////////////////////

	function addEnergyScore(p_energy)
	{
		if (p_energy == 0)
		{
			return;
		}

		if (isInEasyMode())
		{
			p_energy /= 2.0;
		}

		_scoreContainer.addEnergyScore(p_energy);
	}

	function addKillScore(p_score, p_popupPos, p_sourceContainsVirus = false)
	{
		if (p_score == 0)
		{
			return;
		}

		if (p_sourceContainsVirus)
		{
			p_score *= 2.0;
		}

		if (isInEasyMode())
		{
			p_score /= 2.0;
		}

		local addedScore = _scoreContainer.addKillScore(p_score);

		if (p_popupPos != null && ::ProgressMgr.getGameMode() != GameMode.SpeedRun)
		{
			if (addedScore > 0)
			{
				::spawnEntity("ScorePopup", p_popupPos,
					{
						_color = p_sourceContainsVirus ? ColorRGBA(100, 255, 100, 255) :
						                                 ColorRGBA(255, 255, 255, 255),
						_score = addedScore
					});
			}
		}

		return addedScore;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	//

	function getAimInput()
	{
		return ::getRightStick();
	}

	_overriddenDirectionInput = null;
	function getDirectionInput()
	{
		return (_overriddenDirectionInput == null) ? ::getLeftStick() : _overriddenDirectionInput;
	}

	function hasDirectionInputOverride()
	{
		return _overriddenDirectionInput != null;
	}

	function setDirectionInputOverride(p_direction)
	{
		_overriddenDirectionInput = p_direction;
	}

	function resetDirectionInputOverride()
	{
		_overriddenDirectionInput = null;
	}

	function getWalkThreshold()
	{
		// Compensate for non perfect horizontal steering
		local yfactor = 1.0 - ::fabs(getDirectionInput().y) / 3.0;
		if (::getCurrentControllerType() == ControllerType_Keyboard && _overriddenDirectionInput == null)
		{
			return 0.5 * yfactor;
		}
		return 0.75 * yfactor;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	//

	function onCrushFilter(p_entity)
	{
		if ((p_entity instanceof ::Cockroach) == false)
		{
			return false;
		}

		return getSpeed().length() > 1.0;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	//

	function setThruster(p_thruster)
	{
		_activeThruster = p_thruster.weakref();
		foreach(t in _thrusters)
		{
			t.setSuspended(t.equals(p_thruster) == false);
		}
		_movementSettings.setDrag(_activeThruster._drag);
		_movementSettings.setCollisionDrag(_activeThruster._drag);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	//

	_lastAnimName = null;

	function startAnim(p_name)
	{
		if (p_name == _lastAnimName)
		{
			return;
		}

		_lastAnimName = p_name;

		_bodyPresentation.start(p_name, [], false, 0);
		_bodybackPresentation.start(p_name, [], false, 0);
	}

	function restartAnim()
	{
		stopAnim();
		if (_lastAnimName != null)
		{
			local anim = _lastAnimName;
			_lastAnimName = null;
			startAnim(anim);
		}
	}

	function stopAnim()
	{
		_bodyPresentation.stop();
		_bodybackPresentation.stop();
	}

	function addAnimTag(p_tag)
	{
		_bodyPresentation.addTag(p_tag);
		_bodybackPresentation.addTag(p_tag);
		restartAnim();
	}

	function removeAnimTag(p_tag)
	{
		_bodyPresentation.removeTag(p_tag);
		_bodybackPresentation.removeTag(p_tag);
		restartAnim();
	}

	function getSurroundingSpecificTags()
	{
		if (isInWater())             return ["water"];
		if (::Gravity.isZeroGravity()) return ["zerogravity"];
		if (_isOnFloor == false)     return ["air"];
		return ["floor"];
	}

	_carryParent = null;
	function onCarryBegin(p_parent)
	{
		_carryParent = p_parent.weakref();
	}

	function onCarryEnd()
	{
		if (_carryParent != null)
		{
			local speed = getSpeed();
			setSpeed(::Vector2(speed.x + _carryParent.getSpeed().x, speed.y));
		}

		_carryParent = null;
	}

	function onPlatformEnter()
	{
		_isOnPlatform = true;
		stopVirusUploading();
		stopFiringPrimary();
	}

	function onPlatformExit()
	{
		_isOnPlatform = false;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// UPDATE

	function preChildrenUpdate()
	{
		_movementSettings.setExtraForce(::g_physicsSettings.gravity); // reset the extra force by overwriting it with the global gravity
		_movementSettings.addToExtraForce(_extraForce);

		local controllerType = ::getCurrentControllerType();
		if (controllerType != _lastControllerType)
		{
			customCallback("onControllerSwitched", controllerType);
			_lastControllerType = controllerType;
		}

		// Reset again
		_extraForce = ::Vector2(0, 0);

		local wasOnFloor = _isOnFloor;
		_isOnFloor = isInWater() == false && hasSurveyResult(SurveyResult_StandOnSolid) &&
		             ::Gravity.isZeroGravity() == false;

		// Check to see if we need to reset the WaterThruster
		if (_isOnFloor || ::isInZeroGravity(this))
		{
			_thrusters.water.resetThrusterPower();
		}

		if (_isOnFloor != wasOnFloor && isInWater() == false)
		{
			// land
			if (_isOnFloor)
			{
				customCallback("onLand");

				if (_predictiveJump)
				{
					_predictiveJump = false;
					callOnChildren("onButtonJumpPressed");
				}
				else
				{
					_activeThruster.disableThruster();
				}
			}
			else
			// liftoff
			{
				customCallback("onLiftOff");
			}
		}

		// walk animations
		if (_isOnFloor && isInWater() == false)
		{
			local xInput        = getDirectionInput().x;
			local xSpeed        = getSpeed().x;
			local isControlling = moveControlsEnabled() && (xInput < 0 || xInput > 0);
			local isMoving      = xSpeed < -0.01 || xSpeed > 0.01;

			// Moving because of controller
			if (isMoving && isControlling)
			{
				local isMovingForward = isForwardLeft() ? (xInput < 0) : (xInput > 0);
				local threshold = getWalkThreshold();
				if (xInput > threshold || xInput < -threshold)
				{
					startAnim(isMovingForward ? "run" : "run_backwards");
				}
				else
				{
					startAnim(isMovingForward ? "walk" : "walk_backwards");
				}
			}
			// Moving because of external force?
			else if (isMoving && _carryParent == null && (xSpeed > 3.0 || xSpeed < -3.0))
			{
				local isMovingForward = isForwardLeft() ? (xSpeed < 0) : (xSpeed > 0);
				if (xSpeed > 10.0 || xSpeed < -10.0)
				{
					startAnim(isMovingForward ? "run" : "run_backwards");
				}
				else
				{
					startAnim(isMovingForward ? "walk" : "walk_backwards");
				}
			}
			else
			{
				// Not moving or being carried while not controlling entity
				startAnim("idle");
			}
		}

		updateAim();
	}

	function update(p_deltaTime)
	{
		preChildrenUpdate();

		base.update(p_deltaTime);

		startMovementInDirection(::Vector2(0, 0), _movementSettings);

		local speed = getSpeed().length();

		local maxSpeed = isInWater() ? _maxSpeedInWater : _maxSpeed;
		if (speed > maxSpeed)
		{
			local normalizedSpeed = getSpeed() / speed;
			setSpeed(normalizedSpeed * maxSpeed);
		}
	}

	_lookaheadOffset = ::Vector2(0, 0);
	function updateAim()
	{
		local hasKeyboardControls = ::getCurrentControllerType() == ControllerType_Keyboard;
		local lerpValue = 0.3;
		local rightStickLength = 0.0;
		if (aimControlsEnabled() == false)
		{
			// No lookahead when aiming is disabled
			_cameraOffsetEffect.setOffset(::Vector2(0, 0), 0.5, EasingType_Linear);
		}
		else
		{
			local rightStick = getAimInput();
			local leftStick = getDirectionInput();
			rightStickLength = rightStick.length();
			local leftStickLength = leftStick.length();
			local lookWithLeftStick = ::hasTwinstickControlScheme() && ::hasSwitchModeControlScheme() == false && leftStickLength > 0.25;

			local aimDeadzoneThreshold = _virusUploader._isActive ? 0.0 : 0.67;
			local lookAheadEasingTime = 0.5;
			local lookAheadEasingType = EasingType_Linear;

			if (hasKeyboardControls)
			{
				// Compute aim angle
				local mousePos = ::getMouseWorldPosition();
				local pos = getCenterPosition() + ::Vector2(0, 0.5);

				_targetAngle = ::wrapAngle(::atan2(mousePos.x - pos.x, mousePos.y - pos.y) * ::radToDeg);

				// Deadzone
				local distance = ::Camera.worldToScreen(mousePos).distanceTo(::Camera.worldToScreen(pos));
				if (distance > 0.12)
				{
					_lookaheadOffset = ::getVectorFromAngle(_targetAngle) * 5.0;
					// Scale offset based on FOV (tweaked for FOV 60)
					local fovScale = ::Camera.getTargetFOV() / 60;
					_lookaheadOffset *= fovScale;
				}
				lookAheadEasingTime = 1.0;
				lookAheadEasingType = EasingType_QuadraticOut;
			}
			else if (rightStickLength > aimDeadzoneThreshold || lookWithLeftStick)
			{
				local controllingStick = null;
				local offset = null;
				local lookaheadTimeout = null;
				if (rightStickLength > aimDeadzoneThreshold)
				{
					controllingStick = rightStick;
					_lookaheadOffset = controllingStick.normalize() * 5.0;
					lerpValue        = 0.6;
				}
				else
				{
					controllingStick = leftStick;
					_lookaheadOffset = controllingStick.normalize() * 5.0;
					lerpValue        = 0.17;
				}

				// Scale offset based on FOV (tweaked for FOV 60)
				local fovScale = ::Camera.getTargetFOV() / 60;
				_lookaheadOffset *= fovScale;

				// angle range is -180, 180 so we need to add 360 or the sensor shape starts complaining
				local angle = ::atan2(controllingStick.x, controllingStick.y) * ::radToDeg;
				_targetAngle= ::wrapAngle(angle + ::Camera.getCurrentRotation());
			}

			if (_lookAheadEnabled == false)
			{
				_lookaheadOffset = ::Vector2(0, 0);
			}

			_cameraOffsetEffect.setOffset(_lookaheadOffset, lookAheadEasingTime, lookAheadEasingType);
		}

		if (_lockAimAngle != null)
		{
			_targetAngle = _lockAimAngle;
		}

		// Check if primary weapon has auto rotation first
		local autoRotation = _primaryWeapon instanceof ::PlayerBotCustomGun ? _primaryWeapon.getAutoRotation() : null;
		if (autoRotation != null)
		{
			_aimAngle = ::wrapAngle(_aimAngle + autoRotation);
		}
		else if (hasKeyboardControls == false)
		{
			_aimAngle = ::angleLerp(_aimAngle, _targetAngle, lerpValue); // with smoothing
		}
		else
		{
			_aimAngle = _targetAngle; // instant
		}

		_primaryWeapon.setFiringAngle(_aimAngle);
		_secondaryWeapon.setFiringAngle(_aimAngle);
		_reticule.setAngle(_aimAngle);
		_spotLight.setAngle(_aimAngle);

		if (_virusUploader._isActive)
		{
			local range = VirusUploader.c_range;
			if (hasKeyboardControls == false)
			{
				range *= rightStickLength;
			}
			_virusUploaderLight.setRange(range, 0.075);
			_virusUploader.setRange(range);
		}
		_virusUploaderLight.setAngle(_aimAngle);
		_virusUploader.setAngle(_aimAngle);

		setForwardAsLeft(_aimAngle > 180);
	}

	function createPrimaryWeapon()
	{
		::killEntity(_primaryWeapon);
		_primaryWeapon = ::createGun(this, getChildrenOffset(), isInEasyMode() ?
			c_bulletPropertiesEasy : c_bulletPropertiesNormal);
	}

	function createButterflyWeapon(p_ammo)
	{
		::killEntity(_primaryWeapon);
		_primaryWeapon = ::createGun(this, getChildrenOffset(), c_bulletPropertiesButterfly);
		_primaryWeapon.setInfiniteAmmo(p_ammo == 0);
		_primaryWeapon.setAmmo(p_ammo);
	}

	function createCustomWeapon(p_props)
	{
		::killEntity(_primaryWeapon);
		local props = ::mergeTables(p_props, isInEasyMode() ? c_bulletPropertiesEasy : c_bulletPropertiesNormal);
		_primaryWeapon = ::createGun(this, getChildrenOffset(), props);
	}

	function onProjectileFired(p_projectile)
	{
		if (_butterflyMode)
		{
			return;
		}

		if (p_projectile.getclass() == ::PlayerBullet)
		{
			_scoreContainer.addPrimaryFired();
		}

		// HACK: Make rumble/recoil part of bullet or gun settings
		if (p_projectile.getclass() == ::ShotgunBullet)
		{
			_recoilCameraEffect.setOffsetInstant(-p_projectile._speed.getNormalized() * _recoilCameraScale * 10);
			_recoilCameraEffect.setOffset(::Vector2(0, 0), _recoilCameraDuration, EasingType_Linear);
			::ColorGrading.oneShot("playerbot_hit_bullet", 0.0, 0.03, 0.05);
			::Camera.shakeScreen(0.5, getCenterPosition(), CameraShakeFlag.DefaultPlayer);

			// recoil when standing on the floor looks odd, so don't do it
			if (_isOnFloor == false)
			{
				local direction = ::getVectorFromAngle(p_projectile._angle + 180);
				_externalForces.addForce(direction * 2000, _recoildDampening);
			}
			return;
		}

		if (::OptionsData.get(::OptionsKeys.ScreenShake))
		{
			_recoilCameraEffect.setOffsetInstant(-p_projectile._speed.getNormalized() * _recoilCameraScale);
			_recoilCameraEffect.setOffset(::Vector2(0, 0), _recoilCameraDuration, EasingType_Linear);
		}
	}

	function onEmptySecondaryFired(p_gun)
	{
		local radAngle = p_gun.getFiringAngle() * ::degToRad;
		local barrelLength = 1.35;	// FIXME: Shouldn't we get this from the p_gun?
		local pos = ::Vector2(
			::sin(radAngle) * barrelLength * (getForwardDir() == Direction_Left ? -1.0 : 1.0),
			::cos(radAngle) * barrelLength);
		local effect = spawnParticleOneShot("particles/no_ammo",
			pos + ::Vector2(0, 0.5), false, 0, false, ParticleLayer_UseLayerFromParticleEffect, 1.0);

		effect.setEmitterProperties(
			0,
			{
				velocity_x = ::sin(radAngle) * 2,
				velocity_y = ::cos(radAngle) * 2,
			}
		);
		effect.setEmitterProperties(
			1,
			{
				velocity_x = ::sin(radAngle) * 10,
				velocity_y = ::cos(radAngle) * 10,
			}
		);

		playSoundEffect("playerbot_out_of_ammo");
	}

	function onControllerSwitched(p_controller)
	{
		callOnChildren("onControllerSwitched", p_controller);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Easy Mode

	function onEasyModeEnabled(p_enabled)
	{
		if (p_enabled)
		{
			::Hud.destroyElement(_hardModePresentation);
			_hardModePresentation = null;
		}
		else if (_hardModePresentation == null)
		{
			_hardModePresentation = ::Hud.createElement(this, "presentation/hud_hardmode", HudAlignment.Left, HudLayer.Normal);
			_hardModePresentation.start("", [], false, 0);
		}
		// Reconfigure weapons (only if it doesn't have a custom gun)
		if ((_primaryWeapon instanceof ::PlayerBotCustomGun) == false)
		{
			createPrimaryWeapon();
		}
		_secondaryWeapon.select(_secondaryWeapon._type);
	}

	function isInEasyMode()
	{
		// Don't use the progressmgr, this check is quicker
		return _hardModePresentation == null;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Butterfly Mode

	function onButterflyModeEnter(p_ammo)
	{
		_butterflyMode = true;
		::Hud.showLayers(HudLayer.Ife);
		if (_reticule != null)
		{
			_reticule.hide();
		}
		setHackControlsEnabled(false);
		createButterflyWeapon(p_ammo);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// STATUS

	function onDamage(p_healthbar, p_amount)
	{
		local chance = ::min(p_amount * 0.10, 1.0);
		if (p_amount >= 5 || ::frnd() < chance)
		{
			::Audio.playGlobalSoundEffect("VoiceOver", "ROUGHSHOT_HIT");
		}

		if (_status != "nogun" && _status != "locked")
		{
			setStatus("agitated", 2.0);
		}
	}

	function onHeal(p_healthbar, p_amount)
	{
		local health = _healthBar.getNormalizedHealth();
		if (health > 0.5 && _status != "nogun" && _status != "locked")
		{
			setStatus("happy", 2.0);
		}
	}

	function resetStatus()
	{
		// Determine status based on health
		local level = _healthBar.getNormalizedHealth();
		if      (level < 0.25) _status = "health_low";
		else if (level < 0.50) _status = "health_medium";
		else _status = "normal"
	}

	function setStatus(p_status, p_timeout)
	{
		_status = p_status;

		if (p_timeout != null)
		{
			startCallbackTimer("resetStatus", p_timeout);
		}
	}

	function getStatus()
	{
		return _status;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Evaluate entity

	function hasHackedType(p_type)
	{
		return _virusUploader._lastHackedEntity != null ?
			_virusUploader._lastHackedEntity instanceof p_type : false;
	}

	function onEvaluateEntity(p_condition)
	{
		switch (p_condition)
		{
		case "hasHackedTurret":                 return hasHackedType(::Turret);
		case "hasHackedBumper":                 return hasHackedType(::BumperEnemy);
		case "hasHackedKamikaze":               return hasHackedType(::KamikazeEnemy);
		case "hasHackedHealthBot":              return hasHackedType(::HealthBot);
		case "hasSecondaryWeapon":              return _secondaryWeapon._unlockedTypes.len() > 0;
		case "hasNotUsedSecondariesInCampaign": return _scoreContainer._gameStats.campaign.secondaryFired == 0;
		case "hasNotShotDuringMission":         return _scoreContainer._gameStats.mission.secondaryFired == 0 && _scoreContainer._gameStats.mission.primaryFired == 0;
		case "hasTurretHack":                   return ::VirusHelpers.hasUnlockedVirus("Turret");
		case "hasKamikazeHack":                 return ::VirusHelpers.hasUnlockedVirus("KamikazeEnemy");
		case "hasBumperHack":                   return ::VirusHelpers.hasUnlockedVirus("BumperEnemy");
		case "hasHealthBotHack":                return ::VirusHelpers.hasUnlockedVirus("HealthBot");
		case "hasTrainHack":                    return ::VirusHelpers.hasUnlockedVirus("Train");
		case "isInHackMode":                    return _virusUploader != null && _virusUploader._isActive;
		default:
			::tt_panic("Unhandled condition '" + p_condition + "'");
			break
		}
		return false;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// FLUIDS

	// NOTE gravity is global, so this won't work in any multiplayer scenario or for multiple entities
	function onWaterTouchEnter()
	{
		base.onWaterTouchEnter();
		addAnimTag("touchwater");

		// FIX ME: Play sounds
	}

	function onWaterTouchExit()
	{
		base.onWaterTouchExit();
		removeAnimTag("touchwater");

		// FIX ME: Play sound effect & remove continues sound effect
	}

	function onWaterEnclosedEnter()
	{
		base.onWaterEnclosedEnter();

		// Set gravity to 0
		::Gravity.set(GravityType.Water, _waterTweenInDuration);

		setThruster(_thrusters.water);

		_colorGradingIDs["water"] <- ::ColorGrading.add(_colorGradingWater, 0.2);

		startAnim("fly");
		addAnimTag("submerged");

		::setRumble(RumbleStrength_Low, 0.0);

		// Initial drag when you enter water
		setSpeed(getSpeed() * 0.6);
	}

	function onWaterEnclosedExit()
	{
		base.onWaterEnclosedExit();

		// Reset gravity to default
		// FIXME: Not sure if the gravity is normal!!!
		::Gravity.set(GravityType.Normal, _waterTweenOutDuration);

		setThruster(_thrusters.normal);

		::setRumble(RumbleStrength_Low, 0.0);

		::ColorGrading.remove(_colorGradingIDs["water"], 0.2);
		delete _colorGradingIDs["water"];

		removeAnimTag("submerged");

		// Boost to lift you out of the water
		setSpeed(getSpeed() * 1.4 + ::Vector2(0, 6));
	}

	function onWaterfallEnclosedEnter()
	{
		addAnimTag("waterfall");
	}

	function onWaterfallEnclosedExit()
	{
		removeAnimTag("waterfall");
	}

	function onWaterfallTouchEnter()
	{
		startCallbackTimer("unlockSqueekyClean", 60.0);
	}

	function onWaterfallTouchExit()
	{
		stopTimer("unlockSqueekyClean");
	}

	function unlockSqueekyClean()
	{
		::Stats.unlockAchievement("squeeky_clean");
		::Stats.storeAchievements();
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// custom callbacks

	function onConversationCallback(p_name, p_value)
	{
		switch (p_name)
		{
		case "setEmotion": break; // Don't do anything
		default:
			::tt_panic("Unhandled callback '" + p_name + "'");
			break
		}
	}

	function onEMPHit(p_emp)
	{
		// FIXME: Implement something more useful here
		setSpeed(::Vector2(rnd_minmax(-100,100), rnd_minmax(0,100)));
		// Spawn bullet time for extra dramatic effect
		::spawnEntity("BulletTime", ::Vector2(0, 0),
			{
				_startDelay      = 0.02,
				_slowdownTime    = 0.01,
				_bulletTime      = 0.05,
				_speedupTime     = 0.45,
				_endScale        = 0.25,
			});
	}

	function onLand()
	{
		startAnim("idle");
		callOnChildren("onLand");

		::setRumble(RumbleStrength_Low, 0.0);
		playSoundEffect("playerbot_body_land");
		spawnParticleOneShot("particles/playerbot_land", ::Vector2(0,0), true, 0, false, ParticleLayer_UseLayerFromParticleEffect, 1.0);
	}

	function onLiftOff()
	{
		startAnim("fly");
		callOnChildren("onLiftOff");
	}

	function onZeroGravityEnter(p_source)
	{
		// Set zero G color grading
		if (p_source.hasProperty("colorGrading"))
		{
			_colorGradingIDs[p_source.getHandleValue()] <-
				::ColorGrading.add(p_source.getProperty("colorGrading"),
				                   p_source.getProperty("colorGradingFadeDuration"));
		}

		local refCount = getProperty("zeroGravityRefCount");
		if (refCount == 1)
		{
			callOnChildren("onZeroGravityEnter", p_source);

			// Set gravity to 0
			local duration = p_source instanceof ::ZeroGravityTrigger ?
				p_source.tweenInDuration : _zeroGravityTweenInDuration;

			::Gravity.set(GravityType.ZeroG, duration);

			addAnimTag("zerogravity");
			setThruster(_thrusters.zeroGravity);
		}
	}

	function onZeroGravityExit(p_source)
	{
		if (p_source.hasProperty("colorGrading"))
		{
			::ColorGrading.remove(_colorGradingIDs[p_source.getHandleValue()], p_source.getProperty("colorGradingFadeDuration"));
			delete _colorGradingIDs[p_source.getHandleValue()];
		}

		local refCount = getProperty("zeroGravityRefCount");
		if (refCount == 0)
		{
			callOnChildren("onZeroGravityExit", p_source);

			removeAnimTag("zerogravity");

			// FIXME: Add ref counting here?
			setThruster(_thrusters.normal);

			resetBoostCount();

			// FIXME: Whut? This is done to make sure the FlyDrag is set. Change this please
			_activeThruster.onLiftOff();

			if (_predictiveJump)
			{
				_predictiveJump = false;
				//The gravity mustn't be easing out at this point or the animations won't trigger
				::Gravity.set(GravityType.Normal);
				callOnChildren("onButtonJumpPressed");
			}
			else
			{
				// Set gravity to normal
				local duration = p_source instanceof ::ZeroGravityTrigger ?
					p_source.tweenOutDuration : _zeroGravityTweenOutDuration;

				::Gravity.set(GravityType.Normal, duration);
			}
		}
	}

	function onLavaTouchEnter()
	{
		_lastHitForce = (::Vector2(getSpeed().x / 3 ,60) / 1500);

		setSpeed(::Vector2(getSpeed().x, 60));
		_healthBar.doDamage(_healthBar._maxHealth / 2.0, "LAVA"); // HACK: pass a string here
		::Camera.shakeScreen(0.5, getCenterPosition(), CameraShakeFlag.DefaultPlayer);
		spawnParticleOneShot("particles/playerbot_lavahit", ::Vector2(0,0), true, 0, false, ParticleLayer_UseLayerFromParticleEffect, 1.0);
		::ColorGrading.oneShot("playerbot_hit_flame", 0.0, 0.2, 0.5);
		playSoundEffect("lava_hit");
	}

	function onLavafallTouchEnter()
	{
		_lastHitForce = (::Vector2(-getSpeed().x, 20) / 700);

		setSpeed(::Vector2(getSpeed().x * -1.5, 45));
		_healthBar.doDamage(_healthBar._maxHealth / 2.0, "LAVA"); // HACK: pass a string here
		::Camera.shakeScreen(1.0, getCenterPosition(), CameraShakeFlag.DefaultPlayer);
		::ColorGrading.oneShot("playerbot_hit_flame", 0.0, 0.2, 0.5);
		spawnParticleOneShot("particles/playerbot_lavahit", ::Vector2(0,0), true, 0, false, ParticleLayer_UseLayerFromParticleEffect, 1.0);
		playSoundEffect("lava_hit");
	}

	function onDeathRayHit()
	{
		_healthBar.doDamage(_healthBar._maxHealth / 20.0, "DEATHRAY"); // HACK: pass a string here
		::Camera.shakeScreen(0.3, getCenterPosition(), CameraShakeFlag.DefaultPlayer);
		spawnParticleOneShot("particles/playerbot_deathray_hit", ::Vector2(0,0), true, 0, false, ParticleLayer_UseLayerFromParticleEffect, 1.0);
		::ColorGrading.oneShot("hiteffect_deathray", 0.0, 0.1, 0.2);
		playSoundEffect("playerbot_deathray_hit");
		startCallbackTimer("onDeathRayHit", 0.2);
	}

	function onDeathRayEnter(p_deathRay)
	{
		onDeathRayHit();
	}

	function onDeathRayExit(p_deathRay)
	{
		stopTimer("onDeathRayHit");
	}

	function onTrainTrackTouch(p_trainTrack)
	{
		_lastHitForce = (::Vector2(getSpeed().x / 3 ,60) / 1500);

		setSpeed(::Vector2(getSpeed().x, 60));
		_healthBar.doDamage(_healthBar._maxHealth / 2.0, p_trainTrack);
		::Camera.shakeScreen(0.5, getCenterPosition(), CameraShakeFlag.DefaultPlayer);
		spawnParticleOneShot("particles/playerbot_electricityhit", ::Vector2(0,0), true, 0, false, ParticleLayer_UseLayerFromParticleEffect, 1.0);
		::ColorGrading.oneShot("playerbot_hit_electricity", 0.0, 0.1, 0.2);
		playSoundEffect("playerbot_traintrack_hit");
	}

	function onShredderTouch(p_shredder)
	{
		local direction = (p_shredder.getCenterPosition() - getCenterPosition()).normalize();
		_lastHitForce = -direction * 0.5;

		::Camera.shakeScreen(1.0, getCenterPosition(), CameraShakeFlag.DefaultPlayer);
		spawnParticleOneShot("particles/shredder_hit", ::Vector2(0,0), true, 0, false, ParticleLayer_UseLayerFromParticleEffect, 1.0);
		::ColorGrading.oneShot("playerbot_hit_flame", 0.0, 0.2, 0.5);
		_healthBar.doDamage(_healthBar._maxHealth / 4.0, p_shredder);
		applyImpulse(-direction * 10000);
	}

	function onInvisibleDamagerTouch(p_damager, p_type)
	{
		::Camera.shakeScreen(1.0, getCenterPosition(), CameraShakeFlag.DefaultPlayer);
		spawnParticleOneShot("particles/shredder_hit", ::Vector2(0,0), true, 0, false, ParticleLayer_UseLayerFromParticleEffect, 1.0);
		::ColorGrading.oneShot("playerbot_hit_flame", 0.0, 0.2, 0.5);
		_healthBar.doDamage(_healthBar._maxHealth / 8.0, p_damager);

		if (p_type == "circle")
		{
			local direction = (p_damager.getCenterPosition() - getCenterPosition()).normalize();
			_lastHitForce = -direction * 0.5;
			applyImpulse(-direction * 10000);
		}
		else
		{
			local xSpeed = getSpeed().x > 0 ? -60 : 60;
			_lastHitForce = (::Vector2(xSpeed * 30, getSpeed().y / 3) / 1500);
			setSpeed(::Vector2(xSpeed, getSpeed().y));
		}
	}

	function onLavaWaveTouch(p_lavawave)
	{
		_lastHitForce = (::Vector2(-getSpeed().x, 20));

		::Camera.shakeScreen(1.0, getCenterPosition(), CameraShakeFlag.DefaultPlayer);
		spawnParticleOneShot("particles/shredder_hit", ::Vector2(0,0), true, 0, false, ParticleLayer_UseLayerFromParticleEffect, 1.0);
		::ColorGrading.oneShot("playerbot_hit_flame", 0.0, 0.2, 0.5);
		_healthBar.doDamage(_healthBar._maxHealth / 6.0, p_lavawave);
		local direction = (p_lavawave.getCenterPosition() - getCenterPosition()).normalize();
		applyImpulse(-direction * 7000);
	}

	function onTrainGateTouch(p_trainGate)
	{
		::Camera.shakeScreen(0.34, getCenterPosition(), CameraShakeFlag.DefaultPlayer);
		::ColorGrading.oneShot("playerbot_hit_electricity", 0.0, 0.03, 0.05);
		_healthBar.doDamage(1, p_trainGate);
		local direction = getSpeed().x > 0 ? ::Vector2(-1, 0) : ::Vector2(1, 0);
		applyImpulse(direction * (_isOnFloor ? 10000 : 5000));
	}

	//////////////////////////////////////////////////////////////////////////////
	// Bullet Handling

	function onBulletHit(p_bullet)
	{
		_lastHitForce = -(p_bullet.getCenterPosition() - getCenterPosition()).normalize() * 0.4; // magic number based on average explosion force

		base.onBulletHit(p_bullet);

		if (p_bullet.getDamageValue() > 0)
		{
			{
				::Camera.shakeScreen(0.34, getCenterPosition(), CameraShakeFlag.DefaultPlayer);
				::ColorGrading.oneShot("playerbot_hit_bullet", 0.0, 0.03, 0.05);
			}

			if (::Gravity.isZeroGravity())
			{
				addHitDamageForces(p_bullet.getImpactForce() * _zeroGHitImpactFactor, ::Vector2(0, 0));
			}
			else
			{
				addHitDamageForces(p_bullet.getImpactForce(), _bulletHitExtraImpulse);
			}
		}
		// else do another force/shake when healed?
	}

	//////////////////////////////////////////////////////////////////////////////
	// Flame Handling

	function onFlameHit(p_flame)
	{
		if (isInWater() == false)
		{
			base.onFlameHit(p_flame);

			{
				::Camera.shakeScreen(0.25, getCenterPosition(), CameraShakeFlag.DefaultPlayer);
				::ColorGrading.oneShot("playerbot_hit_flame", 0.05, 0.2, 0.25);
			}

			// Reduce speed when hit by flame
			_isOnFloor ? setSpeed(getSpeed() * 0.4) : setSpeed(getSpeed() * 0.9);
		}
	}

	//////////////////////////////////////////////////////////////////////////////
	// Explosion Handling

	function onExplosionHit(p_explosion)
	{
		_lastHitForce = -(p_explosion.getCenterPosition() - getCenterPosition()).normalize() * 0.4;

		local intensity = p_explosion.getDamageValue(this);
		if (::Gravity.isZeroGravity())
		{
			intensity *= _zeroGExplosionImpactFactor;
		}
		applyExplosionForce(p_explosion, (0.75 * intensity) + 0.25, (0.75 * intensity) + 1.0);
		::Camera.shakeScreen(0.8, getCenterPosition(), CameraShakeFlag.DefaultPlayer);
		::ColorGrading.oneShot("playerbot_hit_explosion", 0.05, 0.2, 0.25);
		playSoundEffect("player_expl_layer");

		base.onExplosionHit(p_explosion);
	}

	//////////////////////////////////////////////////////////////////////////////
	// Laser Handling

	function onLaserEnter(p_laser)
	{
		_lastHitForce = -(p_laser.getPosition() - getCenterPosition()).normalize() * 0.1;

		::ColorGrading.oneShot("playerbot_hit_laser", 0.01, 0.1, 0.5);
	}

	function onLaserHit(p_laser)
	{
		base.onLaserHit(p_laser);

		_lastHitForce = -(p_laser.getPosition() - getCenterPosition()).normalize() * 0.4;

		// Reduce speed when hit by laser
		_isOnFloor ? setSpeed(::Vector2(0, 0)) : setSpeed(getSpeed() * 0.4);

		::Camera.shakeScreen(0.4, getCenterPosition(), CameraShakeFlag.DefaultPlayer);
	}

	//////////////////////////////////////////////////////////////////////////////

	function applyImpulse(p_impulse)
	{
		_externalForces.addImpulse(p_impulse);

		if (hasTimer("applyImpulseCooldown") == false)
		{
			setMoveControlsEnabled(false);
		}
		startTimer("applyImpulseCooldown", 0.125);
	}

	function applyExplosionForce(p_damager, p_forceScale, p_forceScaleFloor)
	{
		// FIXME: Scale this based on p_amount?
		::setRumble(RumbleStrength_High, 0.0);

		local factor = 1000.0;
		local impulse = -(p_damager.getCenterPosition() - getCenterPosition()).normalize();

		if (::Gravity.isZeroGravity())
		{
			factor *= p_forceScale;
		}
		else
		{
			if (_isOnFloor)
			{
				impulse.y = 1.0;
				factor *= p_forceScaleFloor;
			}
			else
			{
				// Make sure explosions tend to result in upwards motion
				impulse.y += (1.0 - (impulse.y + 1.0) / 2.0);
				factor *= p_forceScale;
			}
		}
		impulse *= factor;

		applyImpulse(impulse);
	}

	function applyTouchForce(p_damager, p_forceScale, p_forceScaleFloor)
	{
		// FIXME: Scale this based on p_amount?
		::setRumble(RumbleStrength_High, 0.0);

		local factor = 1000.0;
		local impulse = -(p_damager.getCenterPosition() - getCenterPosition()).normalize();

		if (::Gravity.isZeroGravity())
		{
			factor *= p_forceScale;
		}
		else
		{
			if (_isOnFloor)
			{
				impulse.y = 2.5;
				factor *= p_forceScaleFloor;
			}
			else
			{
				if (impulse.y > 0)
				{
					// Push extra against gravity
					impulse.y *= 8.0;
				}
				factor *= p_forceScale;
			}
		}
		impulse *= factor;

		applyImpulse(impulse);
	}

	function stopVirusUploading()
	{
		if (_virusUploader != null && _virusUploader._isActive)
		{
			_virusUploader.stop();
		}
	}

	function resetBoostCount()
	{
		_activeThruster.resetBoostCount();
	}

	function onCrushed(p_crusher)
	{
		addProperty("killer", p_crusher);

		applyTouchForce(p_crusher, 1.0, 1.0);
		::Camera.shakeScreen(0.8, getCenterPosition(), CameraShakeFlag.DefaultPlayer);
		::ColorGrading.oneShot("playerbot_hit_explosion", 0.05, 0.1, 0.25);
		_lastHitForce = -(p_crusher.getCenterPosition() - getCenterPosition()).normalize() / 1.5;

		_scoreContainer.addHealthLost(_healthBar._health);

		base.onCrushed(p_crusher);
	}

	function onTouchDamage(p_damager, p_amount)
	{
		_lastHitForce = -(p_damager.getCenterPosition() - getCenterPosition()).normalize() * 0.3; // magic number based on average explosion force

		local scale = ::Gravity.isZeroGravity() ? 2.5 : 1.0;
		applyTouchForce(p_damager, scale, 1.0);

		if (p_amount > 0)
		{
			::Camera.shakeScreen(0.2 * p_amount, getCenterPosition(), CameraShakeFlag.DefaultPlayer);
			::ColorGrading.oneShot("playerbot_hit_bullet", 0.0, 0.03, 0.05);
		}
	}

	//////////////////////////////////////////////////////////////////////////////
	// Spotlight

	function setDimmedSpotLight(p_isDimmed)
	{
		if (_spotLight != null)
		{
			_spotLight.setColor(p_isDimmed ? ColorRGBA(255, 255, 255, 50) :
			                                 ColorRGBA(255, 255, 255, 255));
		}
	}

	//////////////////////////////////////////////////////////////////////////////
	// Controls

	function moveControlsEnabled()
	{
		return _moveControlsEnabled.get();
	}

	function setMoveControlsEnabled(p_enabled)
	{
		// Reference counted bool
		return _moveControlsEnabled.set(p_enabled);
	}

	function aimControlsEnabled()
	{
		return _aimControlsEnabled.get();
	}

	function setAimControlsEnabled(p_enabled)
	{
		// Reference counted bool
		if (_aimControlsEnabled.set(p_enabled))
		{
			if (p_enabled == false)
			{
				stopFiringPrimary();
				if (_reticule != null) _reticule.hide();
			}
			else
			{
				if (_buttonPrimaryFirePressed)
				{
					startFiringPrimary();
				}
				if (_reticule != null) _reticule.show();
			}
			return true;
		}
		return false;
	}

	function setLockAimAngle(p_angle)
	{
		_lockAimAngle = p_angle;
	}

	function resetLockAimAngle()
	{
		_lockAimAngle = null;
	}

	function weaponsEnabled()
	{
		return _weaponsEnabled.get();
	}

	function setWeaponsEnabled(p_enabled)
	{
		// Reference counted bool
		if (_weaponsEnabled.set(p_enabled))
		{
			if (p_enabled == false)
			{
				stopFiringPrimary(false);
			}
			else if (_buttonPrimaryFirePressed)
			{
				startFiringPrimary();
			}
			return true;
		}
		return false;
	}

	function hackControlsEnabled()
	{
		return _hackControlsEnabled.get();
	}

	function setHackControlsEnabled(p_enabled)
	{
		if (p_enabled && _butterflyMode)
		{
			// Never allow hacking in butterfly mode
			return;
		}

		// Reference counted bool
		if (_hackControlsEnabled.set(p_enabled))
		{
			if (p_enabled == false) stopVirusUploading();
			return true;
		}
		return false;
	}

	function isLightEnabled()
	{
		return _lightEnabled;
	}

	function setLightEnabled(p_enabled)
	{
		_lightEnabled = p_enabled;
		p_enabled ? _spotLight.enableLight(0.075) : _spotLight.disableLight(0.075);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	//

	function addHitDamageForces(p_impulse, p_extraFloorImpulse)
	{
		stopMovement();
		_externalForces.clear();
		_externalForces.addImpulse(p_impulse);

		if (_isOnFloor)
		{
			_externalForces.addImpulse(p_extraFloorImpulse);
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	//

	function startFiringPrimary()
	{
		if (aimControlsEnabled() &&
		    _primaryWeapon.isFiring() == false && _virusUploader._isActive == false)
		{
			if (weaponsEnabled())
			{
				_primaryWeapon.startFiring();
				startTimer("unlockSprayAndPray", 120);
			}
			else
			{
				playSoundEffect("playerbot_gun_fire_jammed");
			}

			local hackedEntity = _virusUploader._lastHackedEntity;
			if (hackedEntity != null)
			{
				hackedEntity.customCallback("onParentStartFiring", this);
			}
		}
	}

	function stopFiringPrimary(p_stopHackEntity = true)
	{
		stopTimer("unlockSprayAndPray");
		if (_primaryWeapon.isFiring())
		{
			_primaryWeapon.stopFiring();
		}

		local hackedEntity = _virusUploader._lastHackedEntity;
		if (p_stopHackEntity && hackedEntity != null)
		{
			hackedEntity.customCallback("onParentStopFiring", this);
		}
	}

	function singleFiringSecondary()
	{
		if (aimControlsEnabled() && _secondaryWeapon.isFiring() == false)
		{
			local hackedEntityShouldFire = false;
			if (weaponsEnabled())
			{
				stopVirusUploading();
				hackedEntityShouldFire =_secondaryWeapon.fire();
				if (hackedEntityShouldFire)
				{
					_scoreContainer.addSecondaryFired();
				}
			}
			else
			{
				// Weapons disabled, yet
				hackedEntityShouldFire = _secondaryWeapon.isEmpty() == false;
			}
			local hackedEntity = _virusUploader._lastHackedEntity;
			if (hackedEntity != null && hackedEntityShouldFire)
			{
				hackedEntity.customCallback("onParentFiringSecondary", this, _secondaryWeapon);
				_secondaryWeapon.setAmmo(0);
			}
		}
	}

	function getUnlockedSecondaryWeapons()
	{
		return _secondaryWeapon._unlockedTypes;
	}

	function onButtonCancelPressed()
	{
		stopVirusUploading();
	}

	function onButtonPrimaryFirePressed()
	{
		if (_isOnPlatform)
		{
			return;
		}

		_buttonPrimaryFirePressed = true;

		if (::hasTwinstickControlScheme())
		{
			if (::getCurrentControllerType() != ControllerType_Keyboard && _lockAimAngle == null &&
			    aimControlsEnabled())
			{
				// Instantly set aim angle at correct position
				local stick = getAimInput();
				local angle = ::atan2(stick.x, stick.y) * ::radToDeg;
				_targetAngle= ::wrapAngle(angle + ::Camera.getCurrentRotation());
				_aimAngle = _targetAngle;
			}
		}
		else
		{
			_virusUploader.stop();
		}

		startFiringPrimary();
	}

	function onButtonPrimaryFireReleased(p_pressDuration)
	{
		_buttonPrimaryFirePressed = false;
		stopFiringPrimary();
	}

	function onButtonSecondaryFirePressed()
	{
		if (_butterflyMode == false && _isOnPlatform == false)
		{
			singleFiringSecondary();
		}
	}

	function selectSecondaryWeapon(p_type)
	{
		if (_butterflyMode == false && _isOnPlatform == false && hasTimer("selectCooldown") == false)
		{
			_secondaryWeapon.select(p_type);
		}
	}

	function onButtonSelectWeapon1Pressed()
	{
		selectSecondaryWeapon("homingmissile");
	}

	function onButtonSelectWeapon2Pressed()
	{
		selectSecondaryWeapon("shotgun");
	}

	function onButtonSelectWeapon3Pressed()
	{
		selectSecondaryWeapon("grenade");
	}

	function onButtonSelectWeapon4Pressed()
	{
		selectSecondaryWeapon("emp");
	}

	function onButtonToggleWeaponsPressed()
	{
		if (_butterflyMode == false || _isOnPlatform)
		{
			_secondaryWeapon.selectPrevious();
		}
	}

	_predictiveJump = false;
	function onButtonJumpPressed()
	{
		if (hasDirectionInputOverride() || _isOnPlatform)
		{
			return;
		}

		if (moveControlsEnabled())
		{
			if (::Gravity.isZeroGravity())
			{
				_predictiveJump = true;
				startTimer("predictiveJumpTimeout", c_predictiveJumpTimeoutZeroG);
			}
			else if (_isOnFloor == false)
			{
				_predictiveJump = true;
				startTimer("predictiveJumpTimeout", c_predictiveJumpTimeoutFalling);
			}

			callOnChildren("onButtonJumpPressed");
		}
	}

	function onButtonJumpReleased(p_pressDuration)
	{
		_predictiveJump = false;
		if (_isOnPlatform)
		{
			return;
		}

		if (moveControlsEnabled())
		{
			callOnChildren("onButtonJumpReleased", [p_pressDuration]);
		}
	}

	function onButtonVirusUploadPressed()
	{
		if (::VirusHelpers.hasUnlockedAnyVirus() == false || _isOnPlatform)
		{
			return;
		}

		if (_virusUploader != null && hackControlsEnabled() && aimControlsEnabled() && _virusUploader._isActive == false)
		{
			_virusUploader.start();
			stopFiringPrimary();
		}
		else
		{
			_virusUploader.stop();
		}

		::setRumble(RumbleStrength_Low, 0.0);
	}

	function onButtonDemoResetPressed()
	{
		::MenuScreen.pushScreen("MenuScreenDebug", { _activator = this.weakref() });
	}

	function onProgressRestored(p_id)
	{
		::createFade(this, "opaque_to_transparent");

		// Normal primary gun should not be shooting when restoring progress
		if ((_primaryWeapon instanceof ::PlayerBotCustomGun) == false)
		{
			stopFiringPrimary();
		}
		startTimer("selectCooldown", 0.5);
		if (::Level.isChallenge(::Level.getMissionID()) == false)
		{
			::g_pickupDropCounter += ::rnd_minmax(1, 8);
		}
		else
		{
			// Fixed drop counter
			::g_pickupDropCounter = 0;
		}

		::ProgressMgr.setEasyMode(::ProgressMgr.isEasyModeEnabled());

		if (::ProgressMgr.shouldOfferEasyMode())
		{
			// Add small delay, because there still might be other menu screens open
			// and they need to be killed first (which takes one frame)
			startCallbackTimer("onOfferEasyMode", 0.02);
		}
	}

	function onOfferEasyMode()
	{
		::MenuScreen.pushScreen("IngameEasyMode", { _activator = this.weakref() });
	}

	// Debug buttons
	function onButtonDebugCheatPressed()
	{
		local godmode = getRegistry().get("godmode");
		godmode = (godmode == null || godmode == false);
		getRegistry().set("godmode", godmode);

		_healthBar.setInvincible(godmode);
		if (godmode)
		{
			removeProperty("noticeCrush");
		}
		else
		{
			addProperty("noticeCrush");
		}

		::spawnEntity("Conversation", getCenterPosition(),
		{
			stickToEntity = this,
			_timeout      = 1.0,
			_textID       = godmode ? "GOD_MODE_ON" : "GOD_MODE_OFF"
		});
	}

	function onButtonDebugRestartReleased(p_duration)
	{
		callOnChildren("onDebugPrepareProgressRestore");
		::ProgressMgr.restoreCheckPoint()
	}

	function onDebugSpawnPointReached(p_player)
	{
		// Save this checkpoint
		::ProgressMgr.setLastCheckPoint("debugspawnpoint");
		::ProgressMgr.storeCheckPoint();
	}

	function _debugMoveToSpawnPoint(p_increment)
	{
		local sortedLocations = [];
		sortedLocations.push([0, this]);

		// This code is highly inefficient; but who cares, it's debug code
		local locations = ::getEntitiesByTag("PlayerBotSpawnPoint");
		foreach (spawnPoint in locations)
		{
			sortedLocations.push([spawnPoint.order, spawnPoint]);
		}
		sortedLocations.sort(@(a, b) a[0] <=> b[0]);

		local curIdx = getProperty("debugCurrentSpawnPointIndex");

		if (p_increment != null)
		{
			curIdx += p_increment;
			if (curIdx < 0) curIdx = sortedLocations.len() - 1;
			else if (curIdx >= sortedLocations.len()) curIdx = 0;
		}
		else
		{
			curIdx = sortedLocations.len() - 1;
		}
		local entity = sortedLocations[curIdx][1];
		if (entity instanceof ::PlayerBot)
		{
			setPosition(entity.getProperty("debugInitialSpawnLocation"));
		}
		else
		{
			setPosition(entity.getPosition());
		}
		entity.customCallback("onDebugSpawnPointReached", this);
		addProperty("debugCurrentSpawnPointIndex", curIdx);
	}

	function onButtonDebugPreviousSpawnPointPressed()
	{
		_debugMoveToSpawnPoint(-1);
	}

	function onButtonDebugNextSpawnPointPressed()
	{
		_debugMoveToSpawnPoint(1);
	}

	function onButtonDebugLastSpawnPointPressed()
	{
		_debugMoveToSpawnPoint(null);
	}

	function onButtonDebugPressed()
	{
		local unr = resurrectunreachable();

		if (unr != null)
		{
			foreach (member, val in unr)
			{
				::echo(val, typeof(val));
			}
		}
		else
		{
			::echo("nothing");
		}
	}

	function onReloadGame()
	{
		// This check should ALWAYS pass because onReloadGame only fires in
		// test builds; but added the check just to be 1000% sure
		if (::isTestBuild())
		{
			// Debug feature, disable warp plateau sequence when reloading
			warpPlateau = null;
		}
	}

	function onVirusUploaderStarted()
	{
	}

	function onVirusUploaderStopped()
	{
		if (_buttonPrimaryFirePressed)
		{
			startFiringPrimary();
		}
	}

	//////////////////////////////////////////////////////////////////////////////
	// IngameMenu Callbacks

	function onIngameMenuOpened(p_menu)
	{
		// HACK; kill all collect effects, as they appear in front of menu
		local effects = ::getEntitiesByTag("PickupCollectEffect");
		foreach (effect in effects)
		{
			::killEntity(effect);
		}

		stopVirusUploading();
		stopFiringPrimary();
	}

	function onIngameMenuClosed(p_menu)
	{
	}

	//////////////////////////////////////////////////////////////////////////////
	// Upgrade Callbacks

	function onUnlockWeapon(p_type)
	{
		spawnParticleOneShot("particles/upgradestation_playerglow", ::Vector2(0, 0), true, 0, false, ParticleLayer_UseLayerFromParticleEffect, 1.0);
		_secondaryWeapon.unlock(p_type);
	}

	function onUpgradeMagnet()
	{
		spawnParticleOneShot("particles/upgradestation_playerglow", ::Vector2(0, 0), true, 0, false, ParticleLayer_UseLayerFromParticleEffect, 1.0);
		_energyContainer.upgrade();
		spawnParticleOneShot("particles/upgradestation_magnet", ::Vector2(0, 0), true, 0, false, ParticleLayer_UseLayerFromParticleEffect, _energyContainer.getRange());
	}

	function onUpgradeHealthBar()
	{
		spawnParticleOneShot("particles/upgradestation_playerglow", ::Vector2(0, 0), true, 0, false, ParticleLayer_UseLayerFromParticleEffect, 1.0);
		_healthBar.upgrade();
	}

	//////////////////////////////////////////////////////////////////////////////
	// Pickup Callbacks

	function onPickupVirusCollected(p_pickup)
	{
		::MenuScreen.pushScreen("VirusPickupScreen",
		{
			_virusID   = p_pickup.type,
			_activator = this.weakref()
		});

		spawnParticleOneShot("particles/virus_pickup_collected", getCenterPosition(), false, 0, true, ParticleLayer_UseLayerFromParticleEffect, 1.0);
	}

	function onPickupSpecialCollected(p_pickup)
	{
		::MenuScreen.pushScreen("SpecialPickupScreen",
		{
			_specialID = p_pickup.type,
			_activator = this.weakref()
		});
	}

	function onPickupHealthCollected(p_pickup)
	{
		_healthBar.doHeal(isInEasyMode() ? 11 : 7);
	}

	function onPickupAmmoCollected(p_pickup)
	{
		_secondaryWeapon.addAmmo(1);
	}

	function onPickupShieldCollected(p_pickup)
	{
		if (_shield == null)
		{
			_shield = addChild("PlayerBotShield", getCenterOffset(),
			{
				_type  = ShieldType.Indestructible,
				_range = 2.5,
			});
		}
	}

	function onBulletTimeEnter()
	{
		callOnChildren("onBulletTimeEnter");
	}

	function onBulletTimeExit()
	{
		callOnChildren("onBulletTimeExit");
	}

	function onRestoreFromUpdate()
	{
		callOnChildren("onRestoreFromUpdate");
	}

	function onStartMission(p_missionID)
	{
		// All textballoons are available again
		::getRegistry().erasePersistent("displayedTextIDs");

		callOnChildren("onStartMission", p_missionID);
	}

	function onEndMission(p_missionID)
	{
		callOnChildren("onEndMission", p_missionID);
	}

	function onEndChallenge(p_missionID, p_medal, p_newMedal)
	{
		::MenuScreen.pushScreen("MenuScreenChallengeCompleted",
			{ _medal = p_medal, _newMedal = p_newMedal, _activator = this.weakref() });
	}

	function onPrepareEndMission()
	{
		setState("EndMission");
	}
}

class PlayerBot_State.EndMission
{
	function onEnterState()
	{
		local isFailed = ::Level.getMissionID() == "09_steelworks_lavarise";

		_endJingle = ::Audio.playGlobalSoundEffect("Music", isFailed ?
			"endmission_jingle_failed" : "endmission_jingle" );

		_fireworksCount = isFailed ? 1 : 4;
		::Audio.duckVolume(DuckingPreset.EndMission);

		// HACK: Reuse the camera offset trigger global
		if (::g_cameraOffsetTriggerEffect == null)
		{
			::g_cameraOffsetTriggerEffect = ::Camera.createCameraEffect();
		}
		::g_cameraOffsetTriggerEffect.setOffset(::Vector2(0, 8), 2.5, EasingType_QuadraticInOut);
		::g_cameraOffsetTriggerEffect.setFOV(55 - ::Camera.getTargetFOV(), 2.5, EasingType_QuadraticInOut);
		setLockAimAngle(0);
		setMoveControlsEnabled(false);
		setWeaponsEnabled(false);
		setHackControlsEnabled(false);
		setAimControlsEnabled(false);
		_targetAngle = 0;
		::killEntity(_primaryWeapon);
		_primaryWeapon = ::createGun(this, getChildrenOffset(), c_endMissionBulletProperties);
		setLookAheadEnabled(false);
		startCallbackTimer("onAim", 0.75);
	}

	function onAim()
	{
		local sign = (_lockAimAngle - 180) < 0 ? -1 : 1;
		setLockAimAngle(::wrapAngle(sign * ::frnd_minmax(10, 30)));
		_primaryWeapon.fire();

		--_fireworksCount;
		if (_fireworksCount > 0)
		{
			startCallbackTimer("onAim", 0.4);
		}
		else
		{
			if (::Level.getMissionID() == "09_steelworks_lavarise")
			{
				// Failed, so wait longer for ending tune
				startCallbackTimer("popState", 5.25);
			}
			else
			{
				startCallbackTimer("popState", 2.25);
			}
		}
	}

	function onExitState()
	{
		resetLockAimAngle();
		::g_cameraOffsetTriggerEffect.setOffset(::Vector2(0, 0), 1.0, EasingType_QuadraticInOut);
		::g_cameraOffsetTriggerEffect.setFOV(0, 1.0, EasingType_QuadraticInOut);
		setMoveControlsEnabled(true);
		setWeaponsEnabled(true);
		setHackControlsEnabled(true);
		setLookAheadEnabled(true);
		setAimControlsEnabled(true);
		::ProgressMgr.endMission();
		createPrimaryWeapon();
		::Audio.unduckVolume(DuckingPreset.EndMission);
	}
}

PlayerBot.setattributes("positionCulling", null);
PauseMenu.makePauseMenuOpener(PlayerBot);
Trigger.makeTriggerParentable(PlayerBot);
