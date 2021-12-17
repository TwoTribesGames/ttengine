include_entity("rewind/enemies/Asteroid");

class AsteroidMedium extends Asteroid
</
	editorImage         = "editor.asteroidmedium"
	libraryImage        = "editor.library.asteroidmedium"
	placeable           = Placeable_Everyone
	movementset         = "Static"
	collisionRect       = [ 0.0, 5.0, 10.0, 10.0 ]
	group               = "01. Enemies"
	sizeShapeColor      = Colors.ForceField
	stickToEntity       = false
/>
{
	// Constants
	static c_mass             = 3;
	static c_maxHealth        = 16;
	static c_maxRotationSpeed = 40;
	static c_score            = 100;
	
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
		
		// Spawn smaller fragments
		// Spawnrange must never be > than differences between radii
		local spawnRange = _radius / 1.5;
		local pos = getCenterPosition();
		::Camera.shakeScreen(0.2, pos);
		
		for (local i = 0; i < 4; i++)
		{
			local rot   = ::frnd_minmax(0, 360);
			local speed = ::frnd_minmax(0, 6);
			::spawnEntity("AsteroidSmall", ::Vector2(pos.x + ::frnd_minmax(-spawnRange, spawnRange),
			                                         pos.y + ::frnd_minmax(-spawnRange, spawnRange)),
			{
				rotation           = rot,
				speed              = speed,
				zeroGravity        = zeroGravity,
				_spawnedByAsteroid = true
			});
		}
	}
}
