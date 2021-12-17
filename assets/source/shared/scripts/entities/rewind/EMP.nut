include_entity("ColorGrading");

class EMP extends EntityBase
</
	placeable      = Placeable_Hidden
	collisionRect  = [ 0.0, 0.0, 0.95, 0.95 ]
	group          = "Rewind"
/>
{
	_duration                    = 0.5;
	_colorgradingFadeInDuration  = 0.2;
	_colorgradingFadeOutDuration = 0.8;
	_colorGradingTextureName     = "emp";
	</
		autoGetSet = true
	/>
	_radius         = 2;
	_shooter        = null; // the entity that caused the EMP
	_colorGradingID = null;
	
	function onInit()
	{
		local touch = addTouchSensor(::CircleShape(0, _radius), null, ::Vector2(0,0));
		touch.setEnterCallback("onEMPTouchEnter");
		touch.setFilterCallback("onEMPFilter");
		
		setCanBePushed(false);
		
		makeBackgroundEntity(this);
	}
	
	function onSpawn()
	{
		spawnParticleOneShot("particles/emp_explosion", ::Vector2(0, 0), false, 0, false, ParticleLayer_UseLayerFromParticleEffect, _radius / 10.0);
		
		startTimer("remove", _duration);
		startTimer("empColorgradingFadeInDuration", _colorgradingFadeInDuration);
		
		_colorGradingID = ::ColorGrading.add(_colorGradingTextureName, _colorgradingFadeInDuration);
		
		::Camera.shakeScreen(0.5, getCenterPosition());
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	
	function onEMPFilter(p_entity)
	{
		return p_entity != _shooter && p_entity.hasProperty("noticeEMP") && p_entity.containsVirus() == false;
	}
	
	function onEMPTouchEnter(p_entity, p_sensor)
	{
		p_entity.customCallback("onEMPHit");
	}
	
	function onTimer(p_name)
	{
		if (p_name == "remove")
		{
			::killEntity(this);
		}
		else if (p_name == "empColorgradingFadeInDuration")
		{
			::ColorGrading.remove(_colorGradingID, _colorgradingFadeOutDuration);
		}
	}
}

function createEMP(p_position, p_radius, p_shooter, p_spawnoptions = {})
{
	local emp = ::spawnEntity("EMP", p_position,
		::mergeTables({ _radius = p_radius, _shooter = p_shooter.weakref() }, p_spawnoptions));
	
	return emp;
}
