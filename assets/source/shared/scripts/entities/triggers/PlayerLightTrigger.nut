include_entity("triggers/Trigger");

class PlayerLightTrigger extends Trigger
</ 
	editorImage    = "editor.playerlighttrigger"
	libraryImage   = "editor.library.playerlighttrigger"
	placeable      = Placeable_Developer
	collisionRect  = [ 0.0, 1.0, 2.0, 2.0 ]
	sizeShapeColor = Colors.Trigger
	displayName    = "Trigger - Player Light"
	group          = "04.4 Player Triggers"
	stickToEntity  = true
/>
{
	</
		type    = "string"
		choice  = ["On","Off", "Do nothing"]
		order   = 1
		group   = "Specific Settings"
	/>
	onEnter = "On";
	
	</
		type    = "string"
		choice  = ["On","Off", "Do nothing"]
		order   = 2
		group   = "Specific Settings"
	/>
	onExit = "Do nothing";
	
	function setLight(p_action)
	{
		local player = ::getFirstEntityByTag("PlayerBot");
		if (player == null)
		{
			return;
		}
		
		switch (p_action)
		{
			case "On":
				player.setLightEnabled(true);
				break;
			case "Off":
				player.setLightEnabled(false);
				break;
			case "Do nothing":
				break;
			
			default:
				break;
		}
	}

	function onTriggerEnter(p_entity)
	{
		base.onTriggerEnter(p_entity);
		setLight(onEnter);
	}
	
	function onTriggerExit(p_entity)
	{
		base.onTriggerExit(p_entity);
		setLight(onExit);
	}
}