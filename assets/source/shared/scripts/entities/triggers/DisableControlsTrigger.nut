include_entity("triggers/Trigger");

class DisableControlsTrigger extends Trigger
</ 
	editorImage    = "editor.disablecontrolstrigger"
	libraryImage   = "editor.library.disablecontrolstrigger"
	placeable      = Placeable_Developer
	sizeShapeColor = Colors.Trigger
	movementset    = "StaticIdle"
	displayName    = "Trigger - Disable Controls"
	group          = "04.4 Player Triggers"
	stickToEntity  = true
/>
{
	</
		type   = "bool"
		group  = "Specific Settings"
		order  = 0.0
	/>
	disableMoveControls = true;
	
	</
		type   = "bool"
		group  = "Specific Settings"
		order  = 0.1
	/>
	disableAimControls = false;
	
	</
		type   = "bool"
		group  = "Specific Settings"
		order  = 0.2
	/>
	disableHackControls = false;
	
	</
		type   = "bool"
		group  = "Specific Settings"
		order  = 0.3
	/>
	disableWeapons = false;
	
	</
		type   = "bool"
		group  = "Specific Settings"
		order  = 0.4
	/>
	lockAimControls = false;
	
	</
		type   = "bool"
		group  = "Specific Settings"
		order  = 0.5
	/>
	disableReticule = false;
	
	function onTriggerEnterFirst(p_entity)
	{
		local playerBot = ::getFirstEntityByTag("PlayerBot");
		if (playerBot == null)
		{
			return;
		}
		
		if (playerBot.setMoveControlsEnabled(disableMoveControls == false))
		{
			local speed = playerBot.getSpeed();
			speed.x = 0.0;
			p_entity.setSpeed(speed);
		}
		
		playerBot.setAimControlsEnabled(disableAimControls == false);
		playerBot.setWeaponsEnabled(disableWeapons == false);
		if (disableWeapons)
		{
			playerBot.setStatus("nogun", null);
		}
		playerBot.setHackControlsEnabled(disableHackControls == false);
		if (lockAimControls)
		{
			playerBot.setLockAimAngle(90);
			playerBot.setStatus("locked", null);
		}
		
		if (disableReticule)
		{
			if (playerBot._reticule != null)
			{
				playerBot._reticule.hide();
			}
		}
	}
	
	function onTriggerExitLast(p_entity)
	{
		local playerBot = ::getFirstEntityByTag("PlayerBot");
		if (playerBot == null)
		{
			return;
		}
		
		playerBot.setMoveControlsEnabled(true)
		playerBot.setAimControlsEnabled(true);
		playerBot.setWeaponsEnabled(true);
		playerBot.setHackControlsEnabled(true);
		if (disableWeapons)
		{
			playerBot.resetStatus();
		}
		
		if (lockAimControls)
		{
			playerBot.resetLockAimAngle();
			playerBot.resetStatus();
		}
		
		if (disableReticule)
		{
			if (playerBot._reticule != null)
			{
				playerBot._reticule.show();
			}
		}
	}
}
