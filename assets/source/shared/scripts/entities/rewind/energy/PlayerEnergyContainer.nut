include_entity("rewind/EntityChild");

class PlayerEnergyContainer extends EnergyContainer
</
	placeable      = Placeable_Hidden
	movementset    = "Static"
	collisionRect  = [ 0.0, 0.0, 0.1, 0.1 ]
	group          = "Rewind"
/>
{
	// Constants
	static c_rangePerUpgrade = [4.0, 7.5, 15.0];
	
	// Store / Restore Constants
	static c_internalsStoredOnLevelExit = ["_energy", "_upgradeLevel"];
	static c_internalsStoredForUpdate   = ["_energy", "_upgradeLevel"];
	
	_energyLabel        = null;
	_collectSensor      = null;
	_upgradeLevel       = 0;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		_energyLabel = ::Hud.createTextElement(this,
		{
			locID               = "",
			width               = 0.10,
			height              = 0.03,
			glyphset            = GlyphSetID_Text,
			hudalignment        = HudAlignment.Left,
			position            = Vector2(0.066, -0.44),
			color               = ColorRGBA(255,255,255,255),
			presentation        = "hud_energy_container",
			horizontalAlignment = HorizontalAlignment_Left,
			autostart           = true
		});
		
		base.onInit();
		
		_createOrUpdateSensor();
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onCollectedEnergy(p_energy)
	{
		::setRumble(RumbleStrength_Low, 0.0);
		_parent.addEnergyScore(p_energy); 
		setEnergy(_energy + p_energy);
	}
	
	function onCollectSightEnter(p_entity, p_sensor)
	{
		p_entity.startCollection(this);
	}
	
	function onCollectFilter(p_entity)
	{
		return p_entity instanceof ::FreeEnergy;
	}
	
	function onDie()
	{
		base.onDie();
		
		::Hud.destroyTextElement(_energyLabel);
	}
	
	function onRestoreFromUpdate()
	{
		base.onRestoreFromUpdate();
		
		_createOrUpdateSensor();
		setEnergy(getEnergy());
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Inherited Methods
	
	function setEnergy(p_energy)
	{
		// Don't call base here
		local previousEnergy = getEnergy();
		
		// Make sure _energy is an integer
		_energy = ::ceil(p_energy);
		_energyLabel.label.setText(::format("%d", _energy));
	}
	
	function releaseEnergy()
	{
		// Don't ever do this
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods
	
	function upgrade()
	{
		if (hasMaximumUpgradeLevel() == false)
		{
			++_upgradeLevel;
			_createOrUpdateSensor();
		}
	}
	
	function getUpgradeLevel()
	{
		return _upgradeLevel;
	}
	
	function getRange()
	{
		return c_rangePerUpgrade[_upgradeLevel];
	}
	
	function hasMaximumUpgradeLevel()
	{
		return _upgradeLevel == c_rangePerUpgrade.len()-1;
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Private Methods
	
	function _createOrUpdateSensor()
	{
		local range = c_rangePerUpgrade[_upgradeLevel];
		if (_collectSensor == null)
		{
			_collectSensor = addSightSensor(::CircleShape(0, range), null, ::Vector2(0, 0));
			_collectSensor.setEnterCallback("onCollectSightEnter");
			_collectSensor.setFilterCallback("onCollectFilter");
		}
		else
		{
			_collectSensor.setShape(::CircleShape(0, range));
		}
	}
}
