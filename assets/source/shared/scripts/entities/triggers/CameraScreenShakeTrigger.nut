include_entity("triggers/Trigger");

class CameraScreenShakeTrigger extends Trigger
</
	editorImage    = "editor.camerascreenshaketrigger"
	libraryImage   = "editor.library.camerascreenshaketrigger"
	placeable      = Placeable_Developer
	sizeShapeColor = Colors.Trigger
	displayName    = "Trigger - Camera Screen Shake"
	group          = "05. Camera"
	stickToEntity  = true
/>
{
	</
		type   = "float"
		min    = 0.01
		max    = 5.0
		order  = 0
		group  = "Specific Settings"
	/>
	power = 0.7;
	
	</
		type    = "bool"
		order   = 1
		group   = "Specific Settings"
	/>
	shakeFlagPosition = true;
	
	</
		type    = "bool"
		order   = 2
		group   = "Specific Settings"
	/>
	shakeFlagRumble = true;
	
	</
		type   = "bool"
		order  = 3
		group  = "Specific Settings"
	/>
	shakeFlagRotation = true;
	
	</
		type   = "bool"
		order  = 4
		group  = "Specific Settings"
	/>
	shakeFlagFOV = true;
	
	</
		type        = "bool"
		order       = 5
		conditional = "shakeFlagPosition == true"
		group       = "Specific Settings"
	/>
	shakeFlagHUD = true;
	
	function onTriggerEnterFirst(p_entity)
	{
		local flags = 0;
		if (shakeFlagPosition) flags = flags | CameraShakeFlag.Position;
		if (shakeFlagRumble)   flags = flags | CameraShakeFlag.RumbleNoPanning;
		if (shakeFlagRotation) flags = flags | CameraShakeFlag.Rotation;
		if (shakeFlagFOV)      flags = flags | CameraShakeFlag.FOV;
		if (shakeFlagHUD)      flags = flags | CameraShakeFlag.HUD;
		
		::Camera.shakeScreen(power, getCenterPosition(), flags, false);
	}
}
