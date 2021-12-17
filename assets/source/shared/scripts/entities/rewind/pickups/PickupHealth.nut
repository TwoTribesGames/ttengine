include_entity("rewind/pickups/Pickup");

class PickupHealth extends Pickup
</ 
	editorImage    = "editor.pickuphealth"
	libraryImage   = "editor.library.pickuphealth"
	placeable      = Placeable_Everyone
	movementset    = "Static"
	displayName    = "Pickup - Health"
	collisionRect  = [ 0.0, 0.5, 1.0, 1.0 ]  // center X, center Y, width, height
	workshopTags   = ["Collectibles"]
	stickToEntity  = true
	group          = "03. Pickups"
/>
{
	// Constants
	static c_presentationName = "pickup_health";
	static c_callbackName     = "onPickupHealthCollected";
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Inherited methods
	
	function collect(p_entity)
	{
		// Don't pickup health if healthbar is full
		if (p_entity._healthBar.isFull())
		{
			_presentation.start("full", [], false, 0);
			return;
		}
		
		if (hasProperty("killedByPlayer"))
		{
			::Stats.incrementStat("plundered_healbots");
		}
		
		::spawnEntity("PickupCollectEffect", ::Camera.worldToScreen(getCenterPosition()),
		{
			_targetPosition = ::Vector2(-0.43, 0.4505),
			_type           = "health"
		});
		
		base.collect(p_entity);
	}
	
	function onPresentationObjectCallback(p_object, p_name)
	{
		base.onPresentationObjectCallback(p_object, p_name);
		
		if (p_name == "gotoidle")
		{
			_presentation.start("idle", [], false, 0);
		}
	}
}
