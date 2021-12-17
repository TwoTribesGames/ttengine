include_entity("rewind/enemies/BaseEnemy");

class DestructibleObject extends BaseEnemy
</
	editorImage         = "editor.destructible"
	libraryImage        = "editor.library.destructible"
	placeable           = Placeable_Hidden
	movementset         = "Static"
	collisionRect       = [ 0.0, 0.5, 1.0, 1.0 ]
	group               = "01.2 Destructible Objects"
	stickToEntity       = true
/>
{
	// Visuals
	</
		type        = "string"
		choice      = ["left", "right"]
		order       = 0.0
		group       = "Appearance"
	/>
	orientation       = "right";
	
	</
		type        = "string"
		choice      = ["s", "p"] 
		order       = 0.1
		group       = "Appearance"
	/>
	tileType = "s";
	
	
	// Constants
	// Visuals
	static c_hasPresentation         = true;
	static c_presentationLayer       = ParticleLayer_InFrontOfEntities;
	static c_hasPermanentCorpse      = false;
	
	// Misc.
	static c_maxHealth               = null; // Use null for invincible
	static c_score                   = 0;    // Score points for destructing this object
	
	// Touch related values
	static c_playerTouchDamage       = null; // The amount of damage done to the player when touched.
	static c_playerExtraTouchForce   = null; // The amount of force added to the player when touched
	static c_touchSound              = null; // The soundeffect played when touched
	static c_wallTouchSound          = null; // The soundeffect played when object hits wall
	static c_floorTouchSound         = null; // The soundeffect played when object hits floor (has higher prio than wall; if null wall will be played)
	static c_rumbleStrength          = RumbleStrength_Medium; // The rumble effect played 
	
	// General settings
	static c_damagedByTouch          = false; // Damage from touch
	static c_damagedByBullets        = false; // Damage from bullets
	static c_cameraShakeOnDie        = false; // Shake camera when this object dies
	static c_attractHomingMissiles   = false; // Attracts homing missiles
	static c_hasCollisionTiles       = false; // Creates entity collision tiles when set to true
	static c_isLightBlocking         = false; // Should this entity cast a shadow?
	static c_energy                  = null;  // Energy that is dropped when killed; use null for no energy
	static c_pickupDropCount         = 0;
	
	// Physics settings
	static c_hasPhysicsMovement      = false;  // Use physics movement
	static c_hasCollisionResponse    = false;  // Response when impacted with bullets or player (requires c_hasPhysicsMovement!)
	static c_mass                    = null;   // Mass, used for physics movement only
	static c_drag                    = null;   // Drag, for physics movement only
	
	// Collision response settings
	static c_bulletMass              = 0.3;
	
	// Internal values. Do not change
	_moveSettings                    = null;
	_moveSettingsZeroG               = null;
	_bulletCatcher                   = null;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		base.onInit();
		
		if (c_hasCollisionResponse && c_hasPhysicsMovement == false)
		{
			editorWarning("Object with collision response should also have physics movement");
		}
		
		setCanBeCarried(true);
		
		if (c_maxHealth == null)
		{
			addInvincibilityProperties();
		}
		
		addProperty("doNotHeal");
		addProperty("noticeBullets");
		removeProperty("noticeEMP");
		removeProperty("openDoors");
		removeProperty("opensPipeGates");
		addProperty("hasCorpse", ::CorpseParams("Corpse", { _isPermanent = c_hasPermanentCorpse, _rumbleStrength = c_rumbleStrength } ));
		
		if (c_hasCollisionTiles)
		{
			local rect = getCollisionRect();
			local width  = ::ceil(rect.getWidth());
			local height = ::ceil(rect.getHeight());
			setEntityCollisionTiles(createCollisionBorderTileString(width, height, tileType));
			setEntityCollisionTilesActive(true);
			
			// Entity has collision tiles, sight detection is useless
			setDetectableBySight(false);
			setSightDetectionPoints([]);
			
			// Also create a bullet catcher
			if (c_damagedByBullets)
			{
				_bulletCatcher = ::spawnEntity("DestructibleObjectBulletCatcher", ::Vector2(0, 0));
				_bulletCatcher.attach(this, width + 0.8, height);
			}
		}
		
		if (c_cameraShakeOnDie == false)
		{
			addProperty("noCameraShakeOnDie");
		}
		
		if (c_playerTouchDamage != null || c_playerExtraTouchForce != null)
		{
			addProperty("touchDamage", c_playerTouchDamage);
		}
		
		if (c_attractHomingMissiles == false)
		{
			// default is attraction, so remove property
			removeProperty("attractHomingMissiles");
		}
		
		// Touch sensor
		if (c_hasPhysicsMovement || c_damagedByTouch)
		{
			local rect = getCollisionRect();
			// Make touch rect a little bit bigger so bigger entities can touch it
			local sensor = addTouchSensor(::BoxShape(rect.getWidth() + 0.2, rect.getHeight() + 0.2));
			sensor.setEnterCallback("onTouchEnter");
			sensor.setFilterCallback("onTouchFilter");
		}
		
		if (c_hasPresentation)
		{
			local presentationFile = "presentation/" + getType().tolower();
			if (c_presentationLayer == null)
			{
				_presentation = createPresentationObject(presentationFile);
			}
			else
			{
				_presentation = createPresentationObjectInLayer(presentationFile, c_presentationLayer);
			}
			_presentation.setAffectedByMovement(true);
		}
		
		if (c_hasPhysicsMovement)
		{
			_moveSettings      = PhysicsSettings(c_mass, c_drag, 0, 0.0, -1.0);
			_moveSettingsZeroG = PhysicsSettings(c_mass, c_drag * 16.0, 0, 0.0, -1.0);
			
			_moveSettings.setExtraForce(::Gravity.getVector(GravityType.Normal));
			_moveSettings.setBouncyness(::frnd_minmax(0.1, 0.2));
			_moveSettings.setCollisionDrag(c_drag * 200.0);
			_moveSettingsZeroG.setBouncyness(::frnd_minmax(0.6, 0.8));
			
			startMoving();
		}
		
		setLightBlocking(c_isLightBlocking);
		
		setForwardFromString(this, orientation);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onEnabled()
	{
		// Don't call base.onEnabled()
		startAllPresentationObjects("idle", []);
	}
	
	function onDisabled()
	{
		// Don't call base.onDisabled() as that stops the movement
		startAllPresentationObjects("idle", []);
	}
	
	function onBulletHit(p_bullet)
	{
		if (c_damagedByBullets)
		{
			base.onBulletHit(p_bullet);
		}
		
		if (c_hasCollisionResponse)
		{
			handleImpact(c_bulletMass, p_bullet._speed * 60);
		}
	}
	
	function onTouchDamageDealt(p_source)
	{
		if (c_playerExtraTouchForce != null && p_source instanceof ::PlayerBot)
		{
			p_source.applyTouchForce(this, c_playerExtraTouchForce, c_playerExtraTouchForce);
		}
	}
	
	function onTouchEnter(p_entity, p_sensor)
	{
		if (c_hasCollisionResponse)
		{
			handleImpact(p_entity.getMass(), p_entity.getSpeed());
			::setRumble(RumbleStrength_Low, 0.0);
		}
		
		if (c_touchSound != null)
		{
			playSoundEffect(c_touchSound);
		}
		
		if (c_damagedByTouch)
		{
			_healthBar.doDamage(1, p_entity);
		}
	}
	
	function onTouchFilter(p_entity)
	{
		return p_entity instanceof ::PlayerBot;
	}
	
	function onDazedEnter()
	{
		// Do nothing
	}
	
	function onDazedExit()
	{
		// Do nothing
	}
	
	function onSolidCollision(p_collisionNormal, p_speed)
	{
		if (c_floorTouchSound != null && p_collisionNormal.y != 0)
		{
			playSoundEffect(c_floorTouchSound);
		}
		else if (c_wallTouchSound != null)
		{
			playSoundEffect(c_wallTouchSound);
		}
	}
	
	function onWaterEnclosedEnter()
	{
		// HACK: Reused onZeroGravityEnter
		setSpeed(getSpeed() / 4.0);
		onZeroGravityEnter(null);
	}
	
	function onWaterEnclosedExit()
	{
		// HACK: Reused onZeroGravityEnter
		onZeroGravityExit(null);
	}
	
	function onZeroGravityEnter(p_source)
	{
		if (c_hasPhysicsMovement)
		{
			startMovementInDirection(getSpeed().normalize(), _moveSettingsZeroG);
		}
	}
	
	function onZeroGravityExit(p_source)
	{
		if (c_hasPhysicsMovement)
		{
			startMovementInDirection(::Vector2(0, 0), _moveSettings);
		}
	}
	
	function onDie()
	{
		base.onDie();
		
		::killEntity(_bulletCatcher);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods
	
	function startMoving()
	{
		if (::isInZeroGravity(this) || isInWater())
		{
			startMovementInDirection(getSpeed().normalize(), _moveSettingsZeroG);
		}
		else
		{
			startMovementInDirection(::Vector2(0, 0), _moveSettings);
		}
	}
	
	function handleImpact(p_impactorMass, p_impactorSpeed)
	{
		if (::isInZeroGravity(this))
		{
			// Newton's law (1 is impactor, 2 is this object)
			// v1 = (v1*(m1-m2) + 2*m2*v2)/(m1+m2)
			// v2 = (v2*(m2-m1) + 2*m1*v1)/(m1+m2)
			local m1 = p_impactorMass;
			local m2 = c_mass;
			local v1 = p_impactorSpeed;
			local v2 = getSpeed();
			setSpeed((v2 * (m2 - m1 ) + v1 * 2.0 *m1 ) / (m1 + m2));
		}
		else
		{
			// All objects behave the same on the ground
			local extraSpeed = (p_impactorSpeed.x > 0) ? ::Vector2(10, 10) : ::Vector2(-10, 10);
			setSpeed(getSpeed() + extraSpeed);
		}
	}
}
