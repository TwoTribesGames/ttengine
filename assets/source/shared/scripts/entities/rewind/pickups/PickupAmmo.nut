include_entity("rewind/pickups/Pickup");

class PickupAmmo extends Pickup
</
	editorImage    = "editor.pickupammo"
	libraryImage   = "editor.library.pickupammo"
	placeable      = Placeable_Everyone
	movementset    = "Static"
	displayName    = "Pickup - Ammo"
	collisionRect  = [ 0.0, 0.5, 1.0, 1.0 ] // center X, center Y, width, height
	workshopTags   = ["Collectibles"]
	stickToEntity  = true
	group          = "03. Pickups"
/>
{
	</
		type = "bool"
	/>
	canBeCarried = true;
	
	// Constants
	static c_presentationName = "pickup_ammo";
	static c_callbackName     = "onPickupAmmoCollected";
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Inherited methods
	
	function onInit()
	{
		base.onInit();
		
		setCanBeCarried(canBeCarried);
	}
	
	function collect(p_entity)
	{
		if (p_entity._secondaryWeapon.isEmpty() == false || p_entity._secondaryWeapon._type == null)
		{
			return;
		}
		
		_presentation.addTag(p_entity._secondaryWeapon._type);
		::spawnEntity("PickupCollectEffect", ::Camera.worldToScreen(getCenterPosition()),
		{
			_targetPosition = ::Vector2(0.41, -0.35)
			_type           = "gun"
		});
		
		base.collect(p_entity);
	}
}
