include_entity("rewind/RewindEntity");

class DestructibleObjectBulletCatcher extends RewindEntity
</
	placeable      = Placeable_Hidden
	movementset    = "Static"
	collisionRect  = [ 0.0, 1.0, 2.0, 2.0 ]
/>
{
	_parent = null;
	
	function onInit()
	{
		base.onInit();
		
		createMovementController(false);
		makeBackgroundEntity(this);
		setDetectableByTouch(true);
		setDetectableBySight(true);
		
		addProperty("noticeBullets");
		addProperty("noticeExplosions");
		addProperty("noRandomPickupsOnDie");
		addProperty("dieQuietly");
		addProperty("ignoreLasers");
		addProperty("weaponGroup", ::WeaponGroup.Neutral);
	}
	
	function attach(p_parent, p_width, p_height)
	{
		_parent = p_parent;
		
		setParentEntityWithOffset(p_parent, ::Vector2(0, 0));
		setCollisionRect(::Vector2(0, p_height * 0.5), p_width, p_height);
		
		local width  = p_width  * 0.99;
		local height = p_height * 0.99;
		local rect = _parent.getCollisionRect();
		setTouchShape(::BoxShape(width, height), rect.getPosition());
		
		// Ensure entity is seen
		local halfWidth  = (width / 2.0);
		local halfHeight = (height / 2.0);
		
		setSightDetectionPoints(
			[
				::Vector2(-halfWidth,  halfHeight),
				::Vector2( halfWidth,  halfHeight),
				::Vector2(-halfWidth, -halfHeight),
				::Vector2( halfWidth, -halfHeight)
			]
		);
	}
	
	function onExplosionHit(p_explosion)
	{
		if (_parent != null)
		{
			_parent.customCallback("onExplosionHit", p_explosion);
		}
	}
	
	function onBulletHit(p_bullet)
	{
		if (_parent != null)
		{
			_parent.customCallback("onBulletHit", p_bullet);
		}
	}
}
