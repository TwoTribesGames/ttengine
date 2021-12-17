include_entity("rewind/enemies/Asteroid");

class AsteroidSmall extends Asteroid
</
	editorImage         = "editor.asteroidsmall"
	libraryImage        = "editor.library.asteroidsmall"
	placeable           = Placeable_Everyone
	movementset         = "Static"
	collisionRect       = [ 0.0, 2.0, 4.0, 4.0 ]
	group               = "01. Enemies"
	sizeShapeColor      = Colors.ForceField
	stickToEntity       = false
/>
{
	// Constants
	static c_mass             = 2;
	static c_maxHealth        = 4;
	static c_maxRotationSpeed = 100;
	static c_score            = 50;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		base.onInit();
		
		_presentation.addTag("small");
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onDie()
	{
		base.onDie();
		
		if (hasProperty("dieQuietly"))
		{
			// Early exit
			return;
		}
		
		::Camera.shakeScreen(0.1, getCenterPosition());
	}
	
	function onTouchDamageDealt(p_source)
	{
		if (p_source instanceof ::PlayerBot)
		{
			_healthBar.doDamage(2, p_source);
		}
	}
}
