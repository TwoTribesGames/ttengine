include_entity("rewind/shields/Shield");

class EnemyShield extends Shield
</
	placeable      = Placeable_Hidden
	movementset    = "Static"
	collisionRect  = [ 0.0, 0.0, 0.1, 0.1 ]
	group          = "Rewind"
/>
{
	static c_weaponGroup = WeaponGroup.Enemy;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onDie()
	{
		base.onDie();
		
		if (_type == ShieldType.Destructible && _parent.hasProperty("dieQuietly") == false)
		{
			local effect = spawnParticleOneShot("particles/shield_enemy_destruct", ::Vector2(0, 0), false, 0, false,
				ParticleLayer_UseLayerFromParticleEffect, _range);
			effect.setInitialExternalImpulse(-_angle, 1.0);
			playSoundEffect("shield_destroyed_enemy");
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods
	
	function createPresentation()
	{
		local name = _type == ShieldType.Indestructible ? "shield_enemy_indestructible" : "shield_enemy_destructible";
		local pres = createPresentationObjectInLayer("presentation/" + name, ParticleLayer_InFrontOfEntities);
		pres.addTag(_range > 10 ? "large" : "normal");
		
		return pres;
	}
}
