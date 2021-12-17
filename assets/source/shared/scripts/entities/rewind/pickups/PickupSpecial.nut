include_entity("rewind/pickups/Pickup");

class PickupSpecial extends Pickup
</
	editorImage    = "editor.pickupspecial"
	libraryImage   = "editor.library.pickupspecial"
	placeable      = Placeable_Everyone
	movementset    = "Static"
	displayName    = "Pickup - Special"
	collisionRect  = [ 0.0, 0.5, 1.0, 1.0 ]  // center X, center Y, width, height
	workshopTags   = ["Collectibles"]
	stickToEntity  = true
	group          = "03. Pickups"
/>
{
	</
		type   = "string"
		choice = ["special_1", "special_2", "special_3", "special_4", "special_5"]
		order  = 2
	/>
	type = null;
	
	// Constants
	static c_registryKey       = "collected_specials";
	static c_touchSensorRadius = 1.75;
	static c_presentationName  = "pickup_special";
	static c_callbackName      = "onPickupSpecialCollected";
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		base.onInit();
		
		if (type == null)
		{
			editorWarning("type not set");
		}
		
		local table = ::getRegistry().getPersistent(c_registryKey);
		if (table != null && (type in table))
		{
			::killEntity(this);
			return;
		}
		
		_presentation.addTag(type);
		_presentation.addTag("pc"); // FIX AFTER SONY APPROVAL ::getPlatformString());
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onScreenEnter()
	{
		// do nothing
	}
	
	function onCrushed(p_crusher)
	{
		// do nothing
	}
	
	function onLavaWaveTouch(p_lavawave)
	{
		// do nothing
	}
	
	function onLavaTouchEnter()
	{
		// do nothing
	}
	
	function onLavafallEnclosedEnter()
	{
		// do nothing
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Inherited methods
	
	function collect(p_entity)
	{
		local table = ::getRegistry().getPersistent(c_registryKey);
		if (table == null)
		{
			table = {};
		}
		
		if ((type in table) == false)
		{
			table[type] <- true;
			::getRegistry().setPersistent(c_registryKey, table);
			::Stats.incrementStat("collected_specials");
		}
		else
		{
			::tt_panic("Special '" + type + "' was already in table");
		}
		
		base.collect(p_entity);
	}
	
}
