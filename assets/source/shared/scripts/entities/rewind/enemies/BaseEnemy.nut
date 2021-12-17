include_entity("rewind/shields/EnemyShield");
include_entity("rewind/RewindEntity");
include_entity("triggers/Trigger");
include_entity("ColorGrading");

::g_pickupDropCounter <- 0;
::g_weaponDropFactor  <- 1.0;
::g_healthDropFactor  <- 1.0;
::g_dropRate          <- 1.0;

class BaseEnemy extends RewindEntity
</
	movementset         = "Static"
	placeable           = Placeable_Hidden
	collisionRect       = [ 0.0, 0.5, 1.0, 1.0 ]
	group               = "Rewind"
/>
{
	</
		type  = "bool"
		order = 100
		group = "Misc."
	/>
	enabled = true;
	
	</
		type           = "entity"
		filter         = ::getTriggerTypes()
		group          = "Misc."
		order          = 101
		referenceColor = ReferenceColors.Enable
		description    = "When set, this entity will be enabled when an incoming signal fires."
	/>
	enableSignal = null;
	
	// Constants
	static c_maxHealth                           = null; // null is invincible
	static c_mass                                = null;
	static c_dazedByEMPTimeout                   = null;
	static c_pickupDropCount                     = 8;
	static c_healthMultiplierWhenHacked          = 3.0;
	static c_isAffectedByVirus                   = false;
	static c_virusLightRadius                    = 3.0;
	static c_lavaDamage                          = 5;
	static c_energy                              = null;
	static c_score                               = 100;
	static c_rumbleStrength                      = RumbleStrength_Medium;
	
	_healthBar       = null;
	_energyContainer = null;
	_shield          = null;
	
	// The main presentation
	_presentation = null;
	
	_virusUploadedEvent = null;
	_virusLight         = null;
	_virusHealth        = null;
	
	_wasEnabled = null;
	_enabledSet = false;
	
	// Effect played if the entity has a virus uploaded to it
	_hackedIndicator = null;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		base.onInit();
		
		if (c_isAffectedByVirus)
		{
			_virusUploadedEvent = ::EventPublisher("virusuploadedevent");
			addProperty("noticeVirusUploader");
		}
		
		setCanBePushed(true);
		createMovementController(false);
		addProperty("weaponGroup", ::WeaponGroup.Enemy);
		addProperty("attractHomingMissiles");
		addProperty("noticeExplosions");
		addProperty("noticeBullets");
		addProperty("noticeCrush");
		addProperty("noticeShredders");
		addProperty("noticeTrainTracks");
		addProperty("noticeForceFields");
		addProperty("noticeFlames");
		addProperty("noticeEMP");
		addProperty("opensPipeGates");
		addProperty("openDoors");
		addProperty("hasCorpse", ::CorpseParams("Corpse", { _rumbleStrength = c_rumbleStrength }));
		
		if (c_pickupDropCount == 0)
		{
			addProperty("noRandomPickupsOnDie");
		}
		
		setDefaultSightDetectionPoints();
		enableDefaultWaterInteraction();
		enableDefaultLavaInteraction();
		
		if (enableSignal != null)
		{
			enableSignal.addChildTrigger(this);
		}
		
		_healthBar       = createHealthBar();
		_shield          = createShield();
		_energyContainer = createEnergyContainer();
	}
	
	function onSpawn()
	{
		base.onSpawn();
		
		registerZeroGravityAffectedEntity(this);
		setEnabled(enabled);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onTimer(p_name)
	{
	}
	
	function onEnabled()
	{
		resumeAllTimers();
		
		startAllPresentationObjects("idle", []);
	}
	
	function onDisabled()
	{
		// Stop all movement and timers immediately
		stopMovement();
		suspendAllTimers();
		
		startAllPresentationObjects("idle", []);
	}
	
	function onWaterTouchEnter()
	{
		if (hasProperty("reduceSpeedWhenEnteringWater"))
		{
			setSpeed(getSpeed() / 5.0);
		}
	}
	
	function onLavafallTouchEnter()
	{
		local offset = getCollisionRect().getWidth() / 2.0;
		if (getSpeed().x < 0) offset = -offset;
		
		spawnParticleOneShot("particles/lavafall_splash_enemy", getCenterPosition() + ::Vector2(offset, 0),
			false, 0, true, ParticleLayer_UseLayerFromParticleEffect, getBoundingRadius());
		
		customCallback("onLavaTouchEnter");
	}
	
	function onLavafallTouchExit()
	{
		local offset = getCollisionRect().getWidth() / 2.0;
		if (getSpeed().x > 0) offset = -offset;
		
		spawnParticleOneShot("particles/lavafall_splash_enemy", getCenterPosition() + ::Vector2(offset, 0),
			false, 0, true, ParticleLayer_UseLayerFromParticleEffect, getBoundingRadius());
	}
	
	function onLavaTouchEnter()
	{
		if (_healthBar != null && c_lavaDamage > 0)
		{
			_healthBar.doDamage(c_lavaDamage, null);
		}
	}
	
	function onLavaTouchExit()
	{
	}
	
	function onDazedEnter()
	{
		setSpeed(::Vector2(0, 0));
		
		startAllPresentationObjects("dazed", []);
	}
	
	function onDazedExit()
	{
		startAllPresentationObjects("idle", []);
	}
	
	function onScreenEnter()
	{
		if (c_isAffectedByVirus)
		{
			VirusUploader.registerVisibleEntity(this);
		}
	}
	
	function onScreenExit()
	{
		if (c_isAffectedByVirus)
		{
			VirusUploader.unregisterVisibleEntity(this);
		}
	}
	
	function onCulled()
	{
		base.onCulled();
		
		if (enabled)
		{
			setEnabled(false);
		}
	}
	
	function onUnculled()
	{
		base.onUnculled();
		
		if (_wasEnabled)
		{
			setEnabled(true);
		}
	}
	
	function onDropped(p_source)
	{
		if (p_source instanceof ::Dropship)
		{
			// Set position based on the location of the dropship's hatch in all size variations.
			local sourceHeight = p_source.getCollisionRect().getHeight();
			local sourcePosition = p_source.getCenterPosition();
			setPosition(::Vector2(sourcePosition.x, sourcePosition.y - (sourceHeight / 3)));
			
			local speedFactor = (sourceHeight / 7.0);
			
			setSpeed(p_source.getSpeed() + ::Vector2(0, -20 * speedFactor));
		}
	}
	
	function onEvaluateEntity(p_condition)
	{
		switch (p_condition)
		{
		case "isHacked": return containsVirus();
		default:
			::tt_panic("Unhandled condition '" + p_condition + "'");
			break
		}
		return false;
	}
	
	function onVirusUploaded(p_entity)
	{
		_virusLight = addLight(::Vector2(0, 0), 0.0, 1.0);
		_virusLight.setRadius(c_virusLightRadius, 0.1);
		_virusLight.setTexture("empbomb");
		_virusLight.setColor(ColorRGBA(90, 255, 90, 255));
		
		// Enforce a clean state
		setState("");
		updateState();
		
		if (_virusHealth == null)
		{
			// First time hacked
			_virusHealth = 1.0;
		}
		
		local hackedCount = getProperty("hackedCount");
		hackedCount = hackedCount == null ? 1 : hackedCount + 1;
		addProperty("hackedCount", hackedCount);
		
		if (_shield != null)
		{
			_shield.disableShield();
		}
		
		// No more stickToEntity when hacked
		stickToEntity = null;
		
		_hackedIndicator = ::VirusUploader.createHackedIndicator(this);
		
		setEnabled(true);
		
		// Boost health
		if (_healthBar != null)
		{
			_healthBar.setMaxHealth(_healthBar.getMaxHealth() * c_healthMultiplierWhenHacked);
			_healthBar.setNormalizedHealth(1.0);
		}
		
		::Stats.incrementStat("hacked_enemies");
		if (hackedCount == 10)
		{
			::Stats.unlockAchievement("hack_same");
			::Stats.storeAchievements();
		}
		
		_virusUploadedEvent.publish(this);
		
		// Don't cull when containing virus
		setPositionCullingEnabled(false);
	}
	
	function onVirusRemoved()
	{
		removeLight(_virusLight);
		_virusLight = null;
		
		::VirusUploader.destroyHackedIndicator(this, _hackedIndicator);
		_hackedIndicator = null;
		
		if (_shield != null)
		{
			_shield.enableShield();
		}
		
		// Reduce max health
		if (_healthBar != null)
		{
			local health = _healthBar.getNormalizedHealth();
			_healthBar.setMaxHealth(_healthBar.getMaxHealth() / c_healthMultiplierWhenHacked);
			_healthBar.setNormalizedHealth(health);
		}
		
		// Restore positionCulling
		setPositionCullingEnabled(positionCulling);
	}
	
	function onCrushed(p_crusher)
	{
		base.onCrushed(p_crusher);
	}
	
	function onBulletHit(p_bullet)
	{
		if (_shield != null && _shield.isIndestructible()) return;
		
		base.onBulletHit(p_bullet);
	}
	
	function onExplosionHit(p_explosion)
	{
		if (_shield != null && _shield.isIndestructible()) return;
		
		base.onExplosionHit(p_explosion);
	}
	
	function onEMPHit()
	{
		if (_shield != null && _shield.isIndestructible()) return;
		
		::ColorGrading.oneShot("emp", 0.0, 0.00, 0.05);
		::Camera.shakeScreen(0.15, getCenterPosition());
		
		// Never kill the entity
		if (_healthBar.getHealth() > 1)
		{
			_healthBar.doDamage(1, this);
		}
		spawnParticleOneShot("particles/emp_explosion", getCenterPosition(), false, 0, true,
			ParticleLayer_UseLayerFromParticleEffect, getBoundingRadius());
		
		callOnChildren("onEMPHit");
		makeDazed(c_dazedByEMPTimeout);
	}
	
	function onLaserHit(p_laser)
	{
		// FIXME: Collin added this; this needs to be moved to Shield and we need to add proper shield reject and bounce code
		if (_shield != null && _shield.isIndestructible()) return;
		
		base.onLaserHit(p_laser);
	}
	
	function onLavaWaveTouch(p_lavawave)
	{
		local direction = (p_lavawave.getCenterPosition() - getCenterPosition()).normalize();
		setSpeed(getSpeed() + -direction * 60);
		spawnParticleOneShot("particles/kamikaze_lavawavehit", ::Vector2(0,0), true, 0, false, ParticleLayer_UseLayerFromParticleEffect, 1.0);
	}
	
	function onHealthBarEmpty(p_healthBar, p_killer)
	{
		// Don't add score if killer has died
		if (::isValidEntity(p_killer))
		{
			p_killer = ("_shooter" in p_killer) ? p_killer._shooter : p_killer;
			if (p_killer instanceof ::PlayerBot && ::isValidEntity(p_killer))
			{
				p_killer.addKillScore(c_score, getCenterPosition() - getCenterOffset());
			}
			else
			{
				if (p_killer instanceof ::BaseEnemy && p_killer.containsVirus())
				{
					// Killer is an hacked enemy; add score
					local playerBot = ::getFirstEntityByTag("PlayerBot");
					if (playerBot != null)
					{
						playerBot.addKillScore(c_score, getCenterPosition() - getCenterOffset(), true);
						if (p_killer.getclass() == ::Turret && playerBot.isInWater())
						{
							::Stats.incrementStat("killed_enemies_underwater");
						}
					}
				}
				else
				{
					addProperty("noRandomPickupsOnDie");
				}
			}
		}
		
		base.onHealthBarEmpty(p_healthBar, p_killer);
	}
	
	function onDie()
	{
		if (c_isAffectedByVirus)
		{
			VirusUploader.unregisterVisibleEntity(this);
		}
		
		::VirusUploader.destroyHackedIndicator(this, _hackedIndicator);
		_hackedIndicator = null;
		
		if (hasProperty("dieQuietly") == false)
		{
			if (hasProperty("noRandomPickupsOnDie") == false)
			{
				handleRandomPickupDrop();
			}
			
			if (hasProperty("noCameraShakeOnDie") == false)
			{
				local power = ::clamp(getBoundingRadius() / 6, 0.25, 1.0);
				::Camera.shakeScreen(power, getCenterPosition());
			}
		}
		
		local prop = getProperty("hasCorpse");
		if (prop != null && ::isInZeroGravity(this))
		{
			prop.addPresentationTags("zerog");
		}
		
		unregisterZeroGravityAffectedEntity(this);
		
		base.onDie();
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
		
		// Default behavior
		setEnabled(true);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods
	
	function getVirusHealthForIndicator()
	{
		// 5 chunks in total
		return ::ceil(_virusHealth * 5.0).tointeger();
	}
	
	function decreaseVirusHealth(p_amount)
	{
		if (p_amount <= 0)
		{
			return;
		}
		
		local prevHealth = getVirusHealthForIndicator();
		_virusHealth -= p_amount;
		local newHealth = getVirusHealthForIndicator();
		
		if (prevHealth != newHealth)
		{
			_hackedIndicator.presentation.addCustomValue("health", newHealth);
			_hackedIndicator.presentation.stop();
			_hackedIndicator.presentation.start("", [getType(), newHealth.tostring()], false, 10);
		}
		
		if (_virusHealth <= 0.0)
		{
			customCallback("onVirusHealthEmpty");
		}
	}
	
	function setWeaponDropFactor(p_factor)
	{
		::g_weaponDropFactor = p_factor;
	}
	
	function setHealthDropFactor(p_factor)
	{
		::g_healthDropFactor = p_factor;
	}
	
	function setDropRate(p_rate)
	{
		::g_dropRate = p_rate;
	}
	
	function handleRandomPickupDrop()
	{
		local player = ::getFirstEntityByTag("PlayerBot");
		if (player == null)
		{
			return;
		}
		
		::g_pickupDropCounter += ::g_dropRate;
		if (::g_pickupDropCounter < c_pickupDropCount)
		{
			// No pickup; exit early
			return;
		}
		::g_pickupDropCounter = 0;
		
		local playerHealth = player._healthBar.getNormalizedHealth();
		
		// Prefer health pickups if health of player is low
		local healthThreshold = 0.0;
		if (playerHealth < 0.5)
		{
			healthThreshold = (1.0 - playerHealth) * 0.9;
		}
		else if (playerHealth < 1.0)
		{
			healthThreshold = 0.075;
		}
		healthThreshold *= g_healthDropFactor;
		
		// Redetermine healthThreshold based on g_weaponDropFactor
		local weaponThreshold = (1.0 - healthThreshold) * ::g_weaponDropFactor;
		if (::g_weaponDropFactor < 1.0 && player._secondaryWeapon.isEmpty() == false)
		{
			// Low probablity mode; only drop weapon when player doesn't have ammo
			weaponThreshold = 0.0;
		}
		
		if (healthThreshold > 0.0)
		{
			healthThreshold = 1.0 - weaponThreshold;
		}
		
		if (weaponThreshold == 0.0 && healthThreshold == 0.0)
		{
			// No pickup either; restore drop counter so that it'll retry the next time
			::g_pickupDropCounter = c_pickupDropCount - 1;
			return;
		}
		
		// No gun spawning when player doesn't own a gun yet
		local collectedGuns = player.getUnlockedSecondaryWeapons();
		local type = collectedGuns.len() > 0 ? "PickupAmmo" : "PickupHealth";
		local typeRnd = ::frnd();
		if (typeRnd < healthThreshold) type = "PickupHealth";
		
		local pickup = ::spawnEntity(type, getCenterPosition(), { _timeout = 20 });
		if (pickup != null)
		{
			pickup.setSpeed(getSpeed());
		}
	}
	
	function createHealthBar()
	{
		return (c_maxHealth == null) ? null : addChild("HealthBar", getCenterOffset(), { _maxHealth = c_maxHealth });
	}
	
	function createShield()
	{
		return null;
	}
	
	function createEnergyContainer()
	{
		return (c_energy != null) ?
			addChild("EnergyContainer", getCenterOffset(), { _energy = c_energy }) : null;
	}
	
	function setEnabled(p_enabled)
	{
		// FIXME: Not sure if this is the most elegant way to know whether this entity was enabled before
		_wasEnabled = enabled;
		
		if (_enabledSet && p_enabled == enabled)
		{
			return;
		}
		
		_enabledSet = true;
		enabled = p_enabled;
		if (enabled)
		{
			customCallback("onEnabled");
			callOnChildren("onParentEnabled"); // let children decide whether they need to deactivate as well
		}
		else
		{
			customCallback("onDisabled");
			callOnChildren("onParentDisabled");
		}
	}
	
	function isEnabled()
	{
		return enabled;
	}
	
	function wasEnabled()
	{
		return _wasEnabled;
	}
	
	function makeDazed(p_timeout)
	{
		if (p_timeout <= 0.0)
		{
			return;
		}
		
		if (getState() == "Dazed")
		{
			if (p_timeout > getTimerTimeout("dazed"))
			{
				// Restart timer
				startTimer("dazed", p_timeout);
			}
		}
		else
		{
			startTimer("dazed", p_timeout);
			setState("Dazed", StatePriority.High); // High prio; we don't want to miss this
		}
	}
	
	function isDazed()
	{
		return getState() == "Dazed";
	}
	
	function containsVirus()
	{
		return _virusLight != null;
	}
	
	function getMass()
	{
		return c_mass;
	}
}

class BaseEnemy_State.Dazed
{
	function onEnterState()
	{
		setEnabled(false);
		// All timers are suspended, but resume the "dazed" timer which has been
		// set in makeDazed
		resumeTimer("dazed");
		
		customCallback("onDazedEnter");
	}
	
	function onExitState()
	{
		setEnabled(wasEnabled());
		
		customCallback("onDazedExit");
	}
	
	function onTimer(p_name)
	{
		if (p_name == "dazed")
		{
			popState();
		}
	}
}
Trigger.makeTriggerTarget(BaseEnemy);

function BaseEnemy::incrementPickupDropCounter()
{
	++::g_pickupDropCounter;
}
