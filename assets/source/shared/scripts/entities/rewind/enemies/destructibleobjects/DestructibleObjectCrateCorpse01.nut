include_entity("rewind/enemies/destructibleobjects/DestructibleObject");

class DestructibleObjectCrateCorpse01 extends DestructibleObject
</
	editorImage         = "editor.destructibleobjectcratecorpse01"
	libraryImage        = "editor.library.destructibleobjectcratecorpse01"
	placeable           = Placeable_Everyone
	movementset         = "Static"
	collisionRect       = [ 0.0, 1.0, 3.0, 2.0 ]
	group               = "01.2 Destructible Objects"
	displayName         = "Crate Front Corpse"
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
