include_entity("rewind/enemies/Turret");

class LaserTurret extends Turret
</
	editorImage         = "editor.laserturret"
	libraryImage        = "editor.library.laserturret"
	placeable           = Placeable_Everyone
	movementset         = "StaticIdle"
	collisionRect       = [ 0.0, 0.75, 1.5, 1.5 ]
	group               = "01. Enemies"
	pathFindAgentRadius = 0.666
	pathCrowdSeparation = true
	stickToEntity       = true
/>
{
	</
		type        = "string"
		choice      = ::getLaserPresetNames(::g_turretGunPresets)
		order       = 0.0
		group       = "Firing"
	/>
	preset = "laser";
	
	// Constants
	static c_maxHealth            = 5;
	static c_score                = 50;
	static c_shieldRange          = 2.2;
	static c_shieldHealth         = 20;
	static c_gunOffset            = ::Vector2(0, 0.3);
	static c_isRemovedFromWorld   = false;
	static c_dazedByEMPTimeout    = 3.0;
	static c_pickupDropCount      = 12;
	static c_killOnOutOfAmmo      = false;
	static c_isAffectedByVirus    = false;
	static c_energy               = 4;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		base.onInit();
	}
	
	function onSpawn()
	{
		base.onSpawn();
	}
}

