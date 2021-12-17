include_entity("rewind/enemies/destructibleobjects/DestructibleObject");

class DestructibleObjectCrateCorpse02 extends DestructibleObject
</
	editorImage         = "editor.destructibleobjectcratecorpse02"
	libraryImage        = "editor.library.destructibleobjectcratecorpse02"
	placeable           = Placeable_Everyone
	movementset         = "Static"
	collisionRect       = [ 0.0, 1.0, 3.0, 2.0 ]
	group               = "01.2 Destructible Objects"
	displayName         = "Crate Side Corpse"
	stickToEntity       = true
/>
{
	// Misc.
	static c_maxHealth                = 5;
	static c_pickupDropCount          = 8;
	static c_score                    = 10;
	static c_hasPermanentCorpse       = true;
	
	// General settings
	static c_damagedByBullets         = true;
	static c_hasCollisionTiles        = true;
	static c_cameraShakeOnDie         = true;
	static c_rumbleStrength           = RumbleStrength_Low;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		base.onInit();
		
		removeProperty("noRandomPickupsOnDie");
	}
}
