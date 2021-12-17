include_entity("triggers/Trigger");
include_entity("rewind/bullets/Bullet");

class BulletSensingTrigger extends Trigger
</
	editorImage    = "editor.bulletsensingtrigger"
	libraryImage   = "editor.library.bulletsensingtrigger"
	placeable      = Placeable_Developer
	sizeShapeColor = Colors.Trigger
	displayName    = "Trigger - Bullet Sensing"
	group          = "04. Input Triggers"
	stickToEntity  = true
/>
{
	</
		type   = "string"
		choice = ["player", "enemy"]
		order = 0
		group   = "Specific Settings"
	/>
	weaponGroup = "player";
	
	</
		type = "integer"
		min = 1
		max = 300
		order = 1
		description = "The amount of bullets that need to hit before the trigger works"
		group   = "Specific Settings"
	/>
	minBullets = 1;
	_bulletsHit = 0;
	
	function onSpawn()
	{
		base.onSpawn();
		
		// Make sure the target optimization doesn't work for this trigger
		_triggerSensor.setTarget(null);
	}
	
	function createTouchSensor()
	{
		base.createTouchSensor();
	}
	
	function onTriggerTouchFilter(p_entity)
	{
		return (p_entity instanceof ::Bullet) && p_entity._group == weaponGroup;
	}
	
	function onTouchEnter(p_entity, p_sensor)
	{
		_bulletsHit += 1;
		
		if (_bulletsHit >= minBullets)
		{
			base.onTouchEnter(p_entity, p_sensor);
		}
	}
	
	function onTouchExit(p_entity, p_sensor)
	{
		base.onTouchExit(p_entity, p_sensor);
		
		// Do this check AFTER the base.onTouchExit otherwise the trigger could never fire again
		if (_bulletsHit >= minBullets)
		{
			_bulletsHit = 0;
		}
	}
}
