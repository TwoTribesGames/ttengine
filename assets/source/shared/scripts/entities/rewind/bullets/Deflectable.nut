include_entity("rewind/bullets/Bullet");

class Deflectable extends Bullet
</
	placeable      = Placeable_Hidden
	movementset    = "Static"
	collisionRect  = [ 0.0, 0.125, 0.25, 0.25 ]
	group          = "Rewind"
/>
{
	// Constants
	static c_deflectionDamage = 1;
	static c_score            = 100;
	
	// Creation members
	_bulletHitDeflectAngle = 25;
	_health                = 10;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onSpawn()
	{
		base.onSpawn();
		
		addProperty("noticeBullets");
		
		// Deflectables can be seen
		setSightDetectionPoints([::Vector2(0, 0)]);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onBulletHit(p_bullet)
	{
		--_health;
		
		// Deflect 
		local perpDot = ::Vector2.perpDot(_speed, p_bullet._speed);
		_speed = _speed.rotate(_bulletHitDeflectAngle * (perpDot < 0 ? -1.0 : 1.0));
		// Don't call updateAngle() here as it will be called in update() anyway
		
		_presentation.start("idle", [_group, "deflected"], false, 0);
		if (_health <= 0)
		{
			impact();
			if (c_score > 0 && p_bullet._shooter instanceof ::PlayerBot)
			{
				p_bullet._shooter.addKillScore(c_score, getCenterPosition());
			}
		}
	}
	
	function impact()
	{
		removeBullet();
	}
}
