include_entity("rewind/bullets/Bullet");

class ShotgunBulletFragment extends Bullet
</
	placeable      = Placeable_Hidden
	collisionRect  = [ 0.0, 0.0, 0.005, 0.005 ]
/>
{
	static c_presentationFile = "presentation/shotgunbulletfragment";
	
	_group            = ::WeaponGroup.Enemy;
	_timeout          = 0.17; // should be tweaked on speed and range of shutgunbullet
	
	function onSpawn()
	{
		base.onSpawn();
		
		// Make this a ghost bullet
		makeEntityUndetectable(this);
		removeSensor(_touch);
	}
}
