include_entity("rewind/shields/Shield");

class PlayerBotShield extends Shield
</
	placeable      = Placeable_Hidden
	movementset    = "Static"
	collisionRect  = [ 0.0, 0.0, 0.1, 0.1 ]
	group          = "Rewind"
/>
{
	static c_weaponGroup = WeaponGroup.Player;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		base.onInit();
		
		startCallbackTimer("warning", 7.0);
		startCallbackTimer("removeMe", 10.0);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onDie()
	{
		base.onDie();
		
		local effect = spawnParticleOneShot("particles/shield_player_destruct", ::Vector2(0, 0), false, 0, false,
		                                    ParticleLayer_UseLayerFromParticleEffect, _range);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods
	
	function removeMe()
	{
		::killEntity(this);
	}
	
	function warning()
	{
		_presentation.addTag("warning");
		_presentation.start("idle", [], false, 0);
	}
	
	function createPresentation()
	{
		return createPresentationObjectInLayer("presentation/shield_player", ParticleLayer_InFrontOfEntities);
	}
}
