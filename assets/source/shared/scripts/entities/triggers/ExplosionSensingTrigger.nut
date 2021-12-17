include_entity("triggers/Trigger");
include_entity("rewind/bullets/Bullet");

class ExplosionSensingTrigger extends Trigger
</
	editorImage    = "editor.explosionsensingtrigger"
	libraryImage   = "editor.library.explosionsensingtrigger"
	placeable      = Placeable_Developer
	sizeShapeColor = Colors.Trigger
	displayName    = "Trigger - Explosion Sensing"
	group          = "04. Input Triggers"
	stickToEntity  = true
/>
{
	</
		type         = "integer"
		min          = 1
		max          = 100
		order        = 1
		description  = "The amount of explosions that need to hit before the trigger works"
		group        = "Specific Settings"
	/>
	minExplosions = 1;
	_explosionsHit = 0;
	_disableTileRegistration = false;
	
	function onInit()
	{
		base.onInit();
		
		addProperty("noticeExplosions");
		setDetectableByTouch(true);
		
		local pos    = getPosition();
		local offset = getCenterOffset();
		
		if (width  == 0) 
		{
			width  = Level.getWidth() + ::fabs((2 * pos.x) - Level.getWidth());
		}
		if (height == 0)
		{
			height = Level.getHeight() + ::fabs((2 * pos.y) - Level.getHeight());
		}
		
		setCollisionRect(offset, width, height);
		setTouchShape(::BoxShape(width * 0.9, height * 0.9), offset);
		
		// if the parentTrigger is set don't create touch sensors etc. (those will be handled by the parent)
		if (parentTrigger != null && ("addChildTrigger" in parentTrigger))
		{
			//::assert("addChildTrigger" in parentTrigger, entityIDString(this) + " tries to set non-triggerparent " + entityIDString(parentTrigger) + " as parent");
			parentTrigger.addChildTrigger(this);
		}
	}
	
	function createTouchSensor()
	{
	}
	
	function onExplosionHit(p_entity)
	{
		if (isEnabled() == false)
		{
			return;
		}
		_explosionsHit += 1;
		
		if (_explosionsHit >= minExplosions)
		{
			_explosionsHit = 0;
			_triggerEnter(p_entity, this);
			_triggerExit(p_entity, this);
		}
	}
}
ExplosionSensingTrigger.setattributes("triggerOnTouch", null);
ExplosionSensingTrigger.setattributes("triggerOnUncull", null);
ExplosionSensingTrigger.setattributes("parentTrigger", null);
ExplosionSensingTrigger.setattributes("triggerFilter", null);
ExplosionSensingTrigger.setattributes("filterAllEntitiesOfSameType", null);
