include_entity("rewind/enemies/Asteroid");

class AsteroidBig extends Asteroid
</
	editorImage         = "editor.asteroidbig"
	libraryImage        = "editor.library.asteroidbig"
	placeable           = Placeable_Everyone
	movementset         = "Static"
	collisionRect       = [ 0.0, 9.0, 18.0, 18.0 ]
	group               = "01. Enemies"
	stickToEntity       = false
/>
{
	// Constants
	static c_mass             = 5;
	static c_maxHealth        = 32;
	static c_maxRotationSpeed = 20;
	static c_score            = 200;
	
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
		::Camera.shakeScreen(0.35, pos);
		
		for (local i = 0; i < 3; i++)
		{
			local rot   = ::frnd_minmax(0, 360);
			local speed = ::frnd_minmax(0, 3);
			::spawnEntity("AsteroidMedium", ::Vector2(pos.x + ::frnd_minmax(-spawnRange, spawnRange),
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
