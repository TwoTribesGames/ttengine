include_entity("triggers/Trigger");

class BulletTimeTrigger extends Trigger
</ 
	editorImage    = "editor.bullettimetrigger"
	libraryImage   = "editor.library.bullettimetrigger"
	placeable      = Placeable_Developer
	sizeShapeColor = Colors.Trigger
	displayName    = "Trigger - Bullet Time"
	group          = "04.1 Action Triggers"
	stickToEntity  = true
/>
{
	</
		type   = "float"
		min    = 0.0
		max    = 10.0
		order  = 1
		group  = "Specific Settings"
	/>
	startDelay = 0.15;

	</
		type   = "float"
		min    = 0.0
		max    = 10.0
		order  = 2
		group  = "Specific Settings"
	/>
	slowdownTime = 0.2;

	</
		type   = "float"
		min    = 0.0
		max    = 10.0
		order  = 3
		group  = "Specific Settings"
	/>
	bulletTime = 0.075;

	</
		type   = "float"
		min    = 0.0
		max    = 10.0
		order  = 4
		group  = "Specific Settings"
	/>
	speedupTime = 0.2;

	</
		type   = "float"
		min    = 0.0
		max    = 10.0
		order  = 5
		group  = "Specific Settings"
	/>
	endScale = 0.05;

	function onTriggerEnterFirst(p_entity)
	{
		// Spawn bullettime
		::spawnEntity("BulletTime", ::Vector2(0, 0),
		{
			_startDelay      = startDelay,
			_slowdownTime    = slowdownTime,
			_bulletTime      = bulletTime,
			_speedupTime     = speedupTime,
			_endScale        = endScale,
		});
	}
}
