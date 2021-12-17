class Explosion extends EntityBase
</
	placeable      = Placeable_Hidden
	collisionRect  = [ 0.0, 0.0, 0.95, 0.95 ]
	group          = "Rewind"
/>
{
	_damageValue  = 3;
	
	_shooter          = null; // the entity that shot something that caused this explosion
	_parentVesselType = null; // the type of vessel that exploded (e.g., ::Missile)
	
	</
		autoGetSet = true
	/>
	_radius = 2;
	
	_duration = 0.1;
	
	function onInit()
	{
		local touch = addTouchSensor(::CircleShape(0, _radius), null, ::Vector2(0,0));
		touch.setEnterCallback("onExplosionTouchEnter");
		touch.setFilterCallback("onExplosionFilter");
		
		makeBackgroundEntity(this);
	}
	
	_particleRadius = 1.5;
	function onSpawn()
	{
		startTimer("remove", _duration);
		::Camera.shakeScreen(_radius * 0.02, getPosition(), CameraShakeFlag.NoRotation);
	}
	
	function onTimer(p_name)
	{
		if (p_name == "remove")
		{
			spawnParticleOneShot("particles/explosion", ::Vector2(0, 0), false, 0, false, ParticleLayer_UseLayerFromParticleEffect, _radius);
			::killEntity(this);
		}
	}
	
	function onExplosionTouchEnter(p_entity, p_sensor)
	{
		if (p_entity != _shooter || _shooter instanceof ::PlayerBot)
		{
			p_entity.customCallback("onExplosionHit", this);
		}
	}
	
	function onExplosionFilter(p_entity)
	{
		return p_entity.hasProperty("noticeExplosions");
	}
	
	function isFromPlayer()
	{
		return( _shooter instanceof ::PlayerBot);
	}
	
	function _typeof()
	{
		return "Explosion";
	}
	
	function getDamageValue(p_target)
	{
		local dist = getCenterPosition().distanceToRect(p_target.getCollisionRectWorldSpace());
		
		if (dist >= 0 && dist <= _radius)
		{
			return _damageValue;
		}
		
		return 0;
	}
}

function createExplosion(p_position, p_radius, p_shooter, p_spawnoptions = {})
{
	local explosion = ::spawnEntity("Explosion", p_position, 
		::mergeTables({ _radius = p_radius, _shooter = p_shooter }, p_spawnoptions));
	
	return explosion;
}
