include_entity("rewind/EntityChild");

class GunBarrel
{
	_angle  = null;
};

class BaseGun extends EntityChild
</
	placeable      = Placeable_Hidden
	movementset    = "Static"
	collisionRect  = [ 0.0, 0.0, 0.1, 0.1 ]
/>
{
	_isFiring = false;
	_gunType  = null;
	
	// Presentation variables
	_gunPresentationName = null;
	_presentation        = null;
	
	// Barrels
	_barrelCount         = 1;
	_barrelLength        = 0.0; // How far from the center of our gun we will spawn
	_barrelSpread        = 360.0;
	_barrelRandomSpread  = 0.0;
	_barrels             = null;
	
	// Fire rate
	_firingRate          = 5.0;
	_firingRateInterval  = 0;
	
	// Firing patterns
	_firingPattern           = "0";
	_firingPatternIdx        = 0;
	_firingPatternStartIdx   = 0;
	
	// Gun Preset Flags
	_flags                   = null;
	
	// Ammo variables
	_ammo = 0;
	_maxAmmo = 1000000;
	
	</
		autoGetSet = true
	/>
	_infiniteAmmo = false;
	
	// Angle variables
	_firingAngle = 0;
	
	</
		autoGetSet = true
	/>
	_firingAngleTarget = 0;
	
	</
		autoGetSet = true
	/>
	_firingAngleTargetOffset = 0;
	
	</
		autoGetSet = true
	/>
	_autoAim          = true;
	_autoAimSpeed      = 0.2; // Time to sway to the new firing angle
	_canFire           = true;
	_firingNextTimeout = 0.0;
	_weaponGroup       = null; //Player or Enemy: If null the group will be copied from the parent's weaponGroup property
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		base.onInit();
		
		if (_barrelCount < 1)
		{
			::tt_panic("Number of barrels should be > 0");
			_barrelCount = 1;
		}
		
		if (_gunPresentationName != null)
		{
			::assert(_gunPresentationName.len() > 0, "_gunPresentationName should have a valid name, or should be null");
			_presentation = createPresentationObject("presentation/" + _gunPresentationName);
			_presentation.setCustomRotation(-_firingAngle);
			_presentation.addTag("barrelcount_" + _barrelCount);
			if ("c_isAffectedByVirus" in _parent && _parent.c_isAffectedByVirus)
			{
				_presentation.addTag("ishackable");
			}
		}
		
		if (_weaponGroup == null)
		{
			_weaponGroup = _parent.getProperty("weaponGroup");
			::null_assert(_weaponGroup);
		}
		
		setFiringPattern(_firingPattern);
		
		_barrels = [];
		for (local i = 0; i < _barrelCount; ++i)
		{
			_barrels.push(GunBarrel())
		}
		
		setFiringAngle(0);
		updateBarrelAngles();
		setFiringRate(_firingRate);
	}
	
	function onSpawn()
	{
		base.onSpawn();
		
		if (_presentation != null)
		{
			_presentation.start("idle", [], false, 0);
		}
	}
	
	function onDie()
	{
		stopFiring();
		
		base.onDie();
	}
	
	function hasExpired()
	{
		return ((_ammo <= 0) && (_infiniteAmmo == false));
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	
	function setFiringPattern(p_pattern)
	{
		// Default to always shoot pattern if no pattern is specified
		_firingPattern = p_pattern.len() == 0 ? "0" : p_pattern;
	}
	
	function getFiringPattern()
	{
		return _firingPattern;
	}
	
	function getCurrentFiringSymbol()
	{
		return _firingPattern[_firingPatternIdx];
	}
	
	function fire()
	{
		if (_canFire == false)
		{
			return false;
		}
		
		local currentFiringSymbol = getCurrentFiringSymbol();
		// Don't call assert in an inner loop (which fire() might be)
		//::assert(currentFiringSymbol != '?', "Cannot have ? as a symbol; it can only follow a non-? character");
		
		local nextIdx = _firingPatternIdx + 1;
		if (nextIdx >= _firingPattern.len())
		{
			nextIdx = 0;
		}
		
		// Do ? (once) logic
		if (_firingPattern[nextIdx] == '?')
		{
			local startPattern = _firingPattern.slice(0, _firingPatternIdx);
			local endPattern = (_firingPatternIdx + 1 < _firingPattern.len()) ?
				_firingPattern.slice(_firingPatternIdx + 2, _firingPattern.len()) : "";
			setFiringPattern(startPattern + endPattern);
			
			nextIdx = _firingPatternIdx;
		}
		
		handleFiringSymbol(currentFiringSymbol, _firingPattern[nextIdx]);
		
		_firingPatternIdx = nextIdx;
		
		if (currentFiringSymbol != '-')
		{
			if (_infiniteAmmo == false && _ammo <= 0)
			{
				return false;
			}
			
			foreach (barrel in _barrels)
			{
				handleBarrelFiring(barrel._angle);
			}
			
			if (_infiniteAmmo == false)
			{
				--_ammo;
			}
			
			if (_presentation != null && _parent.isInWater() == false)
			{
				_presentation.stop();
				_presentation.start("fire", [_weaponGroup], false, 0);
			}
		}
		
		_canFire = false;
		_firingNextTimeout = _firingRateInterval;
		return true;
	}
	
	function handleFiringSymbol(p_currentSymbol, p_nextSymbol)
	{
		// Needs to be implemented by derived classes
	}
	
	function handleBarrelFiring(p_angle)
	{
		// Needs to be implemented by derived classes
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	
	function getAmmo()
	{
		return _ammo;
	}
	
	function getMaxAmmo()
	{
		return _maxAmmo;
	}
	
	function hasFullAmmo()
	{
		return (_ammo >= _maxAmmo);
	}
	
	function setAmmo(p_amount)
	{
		_ammo = p_amount > _maxAmmo ? _maxAmmo : p_amount;
	}
	
	function addAmmo(p_amount)
	{
		_ammo = ::min(_ammo + p_amount, _maxAmmo);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	
	function setFiringRate(p_rate)
	{
		_firingRate = p_rate;
		if (_firingRate > 0)
		{
			_firingRateInterval = 1.0 / _firingRate;
		}
	}
	
	function setFiringAngle(p_angle, p_direct = true)
	{
		_firingAngle = p_angle;
		if (p_direct)
		{
			_firingAngleTarget = p_angle;
			updateBarrelAngles();
		}
		
		if (_presentation != null) _presentation.setCustomRotation(-_firingAngle);
	}
	
	function rotateFiringAngle(p_delta)
	{
		_firingAngleTarget= ::wrapAngle(_firingAngleTarget + p_delta);
	}
	
	function updateFiringAngle()
	{
		local targetAngle = _firingAngleTarget + _firingAngleTargetOffset;
		if (::fabs(_firingAngle - targetAngle) < 0.01)
		{
			updateBarrelAngles();
			return false;
		}
		
		_firingAngle = (::angleLerp(_firingAngle, targetAngle, _autoAimSpeed));
		setFiringAngle(_firingAngle, false);
		
		updateBarrelAngles();
		return true;
	}
	
	function getFiringAngle()
	{
		return _firingAngle;
	}
	
	function updateBarrelAngles()
	{
		local baseAngle = getFiringAngle();
		
		// Early exit
		if (_barrelCount == 1)
		{
			if (_barrelRandomSpread > 0.0)
			{
				local spreadRange = _barrelRandomSpread / 2.0;
				baseAngle += ::frnd_minmax(-spreadRange, spreadRange);
			}
			_barrels[0]._angle = baseAngle;
			return
		}
		
		local startAngle = _barrelSpread == 360.0 ? 0 : - (_barrelSpread / 2.0);
		local separation = _barrelSpread == 360.0 ? 
			_barrelSpread / _barrelCount : _barrelSpread / (_barrelCount - 1);
		
		for (local i = 0; i < _barrelCount; ++i)
		{
			local angle = baseAngle + (i * separation) + startAngle;
			if (_barrelRandomSpread > 0.0)
			{
				local spreadRange = _barrelRandomSpread / 2.0;
				angle += ::frnd_minmax(-spreadRange, spreadRange);
			}
			
			_barrels[i]._angle = angle;
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	
	// single use aim at entity
	function aimAtEntity(p_entity, p_direct = false)
	{
		local delta = p_entity.getCenterPosition() - getCenterPosition();
		local angle = ::getAngleFromVector(delta);
		
		setFiringAngleTarget(angle);
		if (p_direct)
		{
			setFiringAngle(angle, true);
		}
	}
	
	// set a target to keep the gun aimed at
	_targetEntity = null;
	function setTargetEntity(p_entity)
	{
		_targetEntity = p_entity == null ? null : p_entity.weakref();
		if (_targetEntity instanceof ::RewindEntity)
		{
			_targetEntity.subscribeDeathEvent(this, "onTargetDied");
		}
	}
	
	function getTargetEntity()
	{
		return _targetEntity;
	}
	
	function removeTargetEntity()
	{
		_targetEntity = null;
	}
	
	function onTargetDied(p_target)
	{
		removeTargetEntity();
	}
	
	function onDisabled()
	{
		stopFiring();
		setTargetEntity(null);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	
	function removePresentation()
	{
		if (_presentation != null)
		{
			_presentation.stop();
			destroyPresentationObject(_presentation);
			_presentation = null;
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Child Update
	
	function childUpdate(p_deltaTime)
	{
		if (_autoAim && ::isValidEntity(_targetEntity))
		{
			aimAtEntity(_targetEntity);
		}
		updateFiringAngle();
		
		// Don't use timers for firing, as they may cause significant slowdown if there are a lot of guns
		// active at the same time
		if (_firingNextTimeout > 0)
		{
			_firingNextTimeout -= p_deltaTime;
			if (_firingNextTimeout <= 0)
			{
				_canFire = true;
				if (_isFiring)
				{
					fire();
				}
			}
		}
	}
	
	function startFiring()
	{
		_isFiring = true;
		
		if (_firingRate > 0)
		{
			_firingPatternIdx = _firingPatternStartIdx;
			
			// FIXME: Culling happens AFTER the onSpawn; so make sure we don't fire a bullet in the onSpawn but a little later
			// This needs to be properly fixed. E.g., no onSpawn yet for culled entities on startup?
			if (_firingNextTimeout <= 0)
			{
				_firingNextTimeout = 0.01;
			}
			
			if (_presentation != null)
			{
				_presentation.addTag("firing");
			}
		}
	}
	
	function stopFiring()
	{
		_isFiring = false;
		
		if (_presentation != null)
		{
			_presentation.removeTag("firing");
		}
	}
	
	function isFiring()
	{
		return _isFiring;
	}
	
	function onPresentationObjectEnded(p_object, p_name)
	{
		if (_presentation != null) 
		{
			_presentation.stop();
			_presentation.start("idle", [], false, 0);
		}
	}
	
	function onZeroGravityEnter(p_source)
	{
		if (_presentation != null)
		{
			_presentation.addTag("zerogravity");
		}
	}
	
	function onZeroGravityExit(p_source)
	{
		if (_presentation != null)
		{
			_presentation.removeTag("zerogravity");
		}
	}
	
	function onWaterEnter()
	{
		if (_presentation != null)
		{
			_presentation.addTag("submerged");
		}
	}
	
	function onWaterExit()
	{
		if (_presentation != null)
		{
			_presentation.removeTag("submerged");
		}
	}
}
