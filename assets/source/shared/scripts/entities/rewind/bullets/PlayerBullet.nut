include_entity("rewind/bullets/Bullet");

class PlayerBullet extends Bullet
</
	placeable      = Placeable_Hidden
	collisionRect  = [ 0.0, 0.0, 0.005, 0.005 ]
/>
{
	_group            = ::WeaponGroup.Player;
	_timeout          = 3;
	_radius           = 1.0;
	_hitsDeflectables = true;
	
	static c_positionCulling = true;
	
	function onBulletTouchEnter(p_entity, p_sensor)
	{
		if (p_entity instanceof ::BaseEnemy)
		{
			// Use global for fast access
			++::_perm_table.bulletHitCount;
			local entityShield = p_entity._shield;
			if (entityShield != null && entityShield.isIndestructible())
			{
				return;
			}
		}
		
		handleEntityImpact(p_entity);
	}
	
	function handleEntityImpact(p_entity)
	{
		playHitAnim("bullet_hit_entity", -_speed.x * 10, -_speed.y * 10);
		
		p_entity.customCallback("onBulletHit", this);
		removeBullet();
	}
}
