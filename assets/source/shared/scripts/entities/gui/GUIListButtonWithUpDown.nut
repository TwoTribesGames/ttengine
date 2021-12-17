include_entity("gui/GUIListButton");

class GUIListButtonWithUpDown extends GUIListButton
</
	placeable      = Placeable_Hidden
	collisionRect  = [ 0.0, 0.0, 1.085, 0.05 ]
/>
{
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onMoveLeft()
	{
		if (isEnabled() == false)
		{
			return false;
		}
		
		moveLeft();
		select(-1);
		
		::Audio.playGlobalSoundEffect("Effects", "menu_switch");
		::setRumble(RumbleStrength_Low, 0.0);
		return true; // Button is handled
	}
	
	function onMoveRight()
	{
		if (isEnabled() == false)
		{
			return false;
		}
		
		moveRight();
		select(1);
		
		::Audio.playGlobalSoundEffect("Effects", "menu_switch");
		::setRumble(RumbleStrength_Low, 0.0);
		return true; // Button is handled
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Inherited Methods
	
	function select(p_diff)
	{
		// call button delegate
		if (_delegate != null)
		{
			local args = clone _args;
			args.push(p_diff);
			_delegate.acall(args);
		}
	}
}
