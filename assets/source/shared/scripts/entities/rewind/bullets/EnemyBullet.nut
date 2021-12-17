include_entity("rewind/bullets/Bullet");

class EnemyBullet extends Bullet
</
	placeable      = Placeable_Hidden
	collisionRect  = [ 0.0, 0.0, 0.005, 0.005 ]
/>
{
	_group            = ::WeaponGroup.Enemy;
	_timeout          = 3;
	_hitsDeflectables = true;
	
	function handleEntityImpact(p_entity)
	{
		playHitAnim("bullet_enemy_hit_entity", -_speed.x * 10, -_speed.y * 10);
		
		p_entity.customCallback("onBulletHit", this);
		removeBullet();
	}
}
