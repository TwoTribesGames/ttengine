include_entity("rewind/RewindEntity");

class MenuEntity extends RewindEntity
</
	placeable      = Placeable_Hidden
	movementset    = "Static"
	collisionRect  = [ 0.0, 0.0, 1.5, 1.5 ]
/>
{
	positionCulling = false;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		::MenuScreen.pushScreen("MainMenu");
	}
}
