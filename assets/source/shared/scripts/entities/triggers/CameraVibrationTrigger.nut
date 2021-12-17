include_entity("CameraPositionShaker");
include_entity("triggers/Trigger");

class CameraVibrationTrigger extends Trigger
</
	editorImage    = "editor.cameravibrationtrigger"
	libraryImage   = "editor.library.cameravibrationtrigger"
	placeable      = Placeable_Developer
	sizeShapeColor = Colors.Trigger
	displayName    = "Trigger - Camera Vibration"
	group          = "05. Camera"
	stickToEntity  = true
/>
{
	</
		type  = "float"
		min   = 0.0
		max   = 0.6
		order = 0
		group = "Specific Settings"
	/>
	amplitudeX = 0.05;
	
	</
		type   = "float"
		min    = 0.0
		max    = 0.6
		order  = 1
		group  = "Specific Settings"
	/>
	amplitudeY = 0.05;
	
	</
		type    = "float"
		min     = 1
		max     = 60
		order   = 2
		group   = "Specific Settings"
	/>
	frequency = 40;
	
	</
		type        = "bool"
		description = "Automatically perform exitAction after a set delay, turn 'once' on to make this a one time event"
		order       = 3.2
		group       = "Specific Settings"
	/>
	autoExit = false;
	
	</
		type        = "float"
		min         = 0.5
		max         = 100.0
		description = "Delay for when autoExit is used"
		order       = 3.3
		conditional = "autoExit == true"
		group       = "Specific Settings"
	/>
	autoExitDelay = 5.0;
	
	</	
		type    = "bool"
		order   = 3.4
		group   = "Specific Settings"
	/>
	shakeHudAsWell = true;
	
	_cameraShaker = null;
	
	function onTriggerEnterFirst(p_entity)
	{
		_cameraShaker = ::Camera.shakePosition(
			getPosition(), -1, ::Vector2(amplitudeX, amplitudeY), frequency, shakeHudAsWell, false);
		
		_cameraShaker._deathEvent.subscribe(this, "onShakerDied");
		
		if (autoExit)
		{
			startTimer("autoExit", autoExitDelay);
		}
	}
	
	function onTriggerExitLast(p_entity)
	{
		if (_cameraShaker != null)
		{
			_cameraShaker.stopShaking();
		}
	}
	
	function onTimer(p_name)
	{
		if (p_name == "autoExit" && _cameraShaker != null)
		{
			_cameraShaker.stopShaking();
		}
	}
	
	function onShakerDied(p_shaker)
	{
		::killEntity(this);
	}
	
	function onDie()
	{
		base.onDie();
		
		if (_cameraShaker != null)
		{
			_cameraShaker._deathEvent.unsubscribe(this, "onShakerDied");
		}
	}
}
