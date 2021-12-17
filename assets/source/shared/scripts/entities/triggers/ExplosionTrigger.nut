include_entity("triggers/Trigger");

class ExplosionTrigger extends Trigger
</ 
	editorImage    = "editor.explosiontrigger"
	libraryImage   = "editor.library.explosiontrigger"
	placeable      = Placeable_Developer
	collisionRect  = [ 0.0, 1.0, 2.0, 2.0 ]
	sizeShapeColor = Colors.Trigger
	displayName    = "Trigger - Explosion"
	group          = "04.1 Action Triggers"
	stickToEntity  = true
/>
{
	</
		type   = "float"
		min    = 1
		max    = 20
		order  = 0
		group  = "Specific Settings"
	/>
	explosionRadius = 3;
	
	</
		type   = "integer"
		min    = 0
		max    = 25
		order  = 1
		group  = "Specific Settings"
	/>
	damage = 3;
	
	</
		type    = "bool"
		order   = 2
		group   = "Specific Settings"
	/>
	playExplosionSound = true;
	
	function onTriggerEnterFirst(p_entity)
	{
		base.onTriggerEnterFirst(p_entity);
		
		local pos = getCenterPosition();
		
		for (local x = pos.x - width * 0.5; x < pos.x + width * 0.5; x += explosionRadius * 2)
		{
			for (local y = pos.y - height * 0.5; y < pos.y + height * 0.5; y += explosionRadius * 2)
			{
				createExplosion(::Vector2(x, y), explosionRadius, this,
					{
						_damageValue = damage
					}
				);
				
				if (playExplosionSound)
				{
					playSoundEffect("explosion");
				}
			}
		}
	}
	
	function containsVirus()
	{
		return false;
	}
}
