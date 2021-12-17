include_entity("rewind/healthbars/HudHealthBar");

class PlayerBotHealthBar extends HudHealthBar
</
	placeable      = Placeable_Hidden
	movementset    = "Static"
	collisionRect  = [ 0.0, 0.0, 0.1, 0.1 ]
/>
{
	// Constants
	static c_healthPerUpgradeNormal = [20, 23, 26];
	static c_healthPerUpgradeBrutal = [12, 14, 16];
	static c_hudChunksAlignment     = HorizontalAlignment_Left;

	// Store / Restore Constants
	static c_internalsStoredOnLevelExit = ["_health", "_upgradeLevel"];
	static c_internalsStoredForUpdate   = ["_health", "_upgradeLevel"];

	// Creation params
	_maxHealth                  = null;

	// Internals
	_dangerPresentationLeft     = null;
	_dangerPresentationRight    = null;
	_dangerSound                = null;
	_firstFrame                 = true;
	_upgradeLevel               = 0;

	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn

	function onInit()
	{
		base.onInit();

		_maxHealth = getMaxHealthForUpgrade();

		_dangerPresentationLeft = ::Hud.createElement(this, "presentation/hud_danger", HudAlignment.Left);
		_dangerPresentationLeft.addTag("left");
		_dangerPresentationRight = ::Hud.createElement(this, "presentation/hud_danger", HudAlignment.Right);
		_dangerPresentationRight.addTag("right");
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks

	function onHUDShow()
	{
		handleDangerSound();
	}

	function onHUDHide()
	{
		stopDangerSound();
	}

	function onExplosionHit(p_explosion)
	{
		// Override this for custom behavior; don't call base
		local damage = p_explosion.getDamageValue(_parent);

		// If explosion is caused by the shooter itself, reduce the damage a bit
		local shooter = p_explosion._shooter;
		if (::isValidEntity(shooter))
		{
			if (_parent.equals(shooter) || (shooter instanceof ::BaseEnemy && shooter.containsVirus()))
			{
				damage /= 3.0;
			}
		}

		if (damage > 0)
		{
			doDamage(damage, p_explosion);
		}
	}

	function onDie()
	{
		base.onDie();

		stopDangerSound();

		::Hud.destroyElement(_dangerPresentationLeft);
		::Hud.destroyElement(_dangerPresentationRight);
	}

	function onProgressRestored(p_id)
	{
		base.onProgressRestored(p_id);

		local deathCount = ::ProgressMgr.getLastCheckPointDeathCount();
		if (deathCount > 0)
		{
			local addedHealth = ::min(10, (deathCount - 1) + 5);
			initHealth(getHealth() + addedHealth);
		}
	}

	function onRestoreFromUpdate()
	{
		base.onRestoreFromUpdate();

		_maxHealth = getMaxHealthForUpgrade();
		if (_health == null)
		{
			_health = _maxHealth;
		}
		initHealth(_health);
	}

	function onReloadRequested()
	{
		// This check should ALWAYS pass because onReloadGame only fires in
		// test builds; but added the check just to be 1000% sure
		if (::isTestBuild())
		{
			setHealth(_maxHealth);
		}
		base.onReloadRequested();
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Inherited Methods

	function initHealth(p_health)
	{
		_hudChunks = _maxHealth;

		base.initHealth(p_health);
	}

	function initHud()
	{
		base.initHud();

		_dangerPresentationLeft.stop();
		_dangerPresentationRight.stop();

		if (_health <= c_dangerChunk)
		{
			_dangerPresentationLeft.start("danger", [], false, 0);
			_dangerPresentationRight.start("danger", [], false, 0);
		}
		handleDangerSound();
	}

	function startHitPresentation(p_name, p_extraTag = null)
	{
		// Override base; custom code to match c_dangerChunk
		if (_hitPresentation != null && _hudChunks != null)
		{
			local chunk = ::ceil(getNormalizedHealth() * _hudChunks.tofloat());
			local tags = [];
			if      (p_extraTag != null)         tags.push(p_extraTag);
			if      (chunk <= c_dangerChunk)     tags.push("high_damage");
			else if (chunk <= c_dangerChunk * 2) tags.push("medium_damage");
			else if (chunk <= c_dangerChunk * 3) tags.push("low_damage");

			_hitPresentation.start(p_name, tags, false, 0);
		}
	}

	function updateBar(p_isDamaged, p_newChunk, p_oldChunk)
	{
		base.updateBar(p_isDamaged, p_newChunk, p_oldChunk);

		if (_isInitializing)
		{
			return;
		}

		local movedIntoDanger  = (p_newChunk <= c_dangerChunk && p_oldChunk > c_dangerChunk);
		// Update danger indicators
		if (movedIntoDanger)
		{
			_dangerPresentationLeft.start("danger", [], false, 0);
			_dangerPresentationRight.start("danger", [], false, 0);
		}
		else if (p_newChunk > c_dangerChunk)
		{
			_dangerPresentationLeft.stop();
			_dangerPresentationRight.stop();
			local tags = [p_isDamaged ? "damage" : "heal"];

			_dangerPresentationLeft.start("hit", tags, false, 0);
			_dangerPresentationRight.start("hit", tags, false, 0);
		}
		handleDangerSound();
	}

	function doTouchDamage(p_amount, p_damager)
	{
		if (p_damager.hasProperty("touchDamageDoesntKillPlayer"))
		{
			local damage = (_health - p_amount > 0) ? p_amount : ::max(0, _health - 1.0);
			return doDamage(damage, p_damager);
		}
		return doDamage(p_amount, p_damager);
	}

	function doDamage(p_amount, p_damager, p_extraTag = null)
	{
		if (_parent._shield != null)
		{
			return;
		}

		base.doDamage(p_amount, p_damager, p_extraTag);

		_parent._scoreContainer.addHealthLost(p_amount);

		if (p_extraTag == "flame")
		{
			return;
		}

		local debrisEffect = _parent.spawnParticleOneShot("particles/playerbot_hit_chunks", ::Vector2(0,0), false, 0, false, ParticleLayer_UseLayerFromParticleEffect, 1.0);

		if (_parent._lastHitForce != null)
		{
			local force = _parent._lastHitForce * 20 * p_amount;

			debrisEffect.setEmitterProperties(0, { velocity_x = -force.x, velocity_y = -force.y + 10.0 });
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods

	function stopDangerSound()
	{
		if (_dangerSound != null)
		{
			_dangerSound.stop();
			_dangerSound = null;
		}
	}

	function handleDangerSound()
	{
		if (_health <= c_dangerChunk && _dangerSound == null)
		{
			_dangerSound = ::Audio.playGlobalSoundEffect("Effects", "playerbot_almost_dead_alarm");
		}
		else if (_health > c_dangerChunk && _dangerSound != null)
		{
			_dangerSound.stop();
			_dangerSound = null;
		}
	}

	function upgrade()
	{
		if (hasMaximumUpgradeLevel() == false)
		{
			++_upgradeLevel;
			_maxHealth = getMaxHealthForUpgrade();
			doHeal(_maxHealth);
			initHealth(getHealth());

			for (local i = 0; i < _hudChunks; ++i)
			{
				_hudChunksPresentations[i].start("heal", [], false, 0);
			}
		}
	}

	function getUpgradeLevel()
	{
		return _upgradeLevel;
	}

	function hasMaximumUpgradeLevel()
	{
		return _upgradeLevel == c_healthPerUpgradeNormal.len()-1;
	}

	function getMaxHealthForUpgrade()
	{
		return ::ProgressMgr.getGameMode() == GameMode.Brutal ?
			c_healthPerUpgradeBrutal[_upgradeLevel] : c_healthPerUpgradeNormal[_upgradeLevel];
	}
}
