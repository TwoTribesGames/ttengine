include_entity("rewind/enemies/destructibleobjects/DestructibleObject");

class DestructibleObjectCable extends DestructibleObject
</
	editorImage         = "editor.destructibleobjectcable"
	libraryImage        = "editor.library.destructibleobjectcable"
	placeable           = Placeable_Everyone
	movementset         = "Static"
	collisionRect       = [ 0.0, 1.0, 1.0, 2.0 ]
	group               = "01.2 Destructible Objects"
	displayName         = "Cable"
	stickToEntity       = true
/>
{
	// Visuals
	static c_presentationLayer        = ParticleLayer_InFrontOfShoeboxZeroOne;
	
	// Misc.
	static c_maxHealth                = 5;
	static c_score                    = 10;
	static c_hasPermanentCorpse       = true;
	
	// General settings
	static c_damagedByBullets         = true;
	static c_rumbleStrength           = RumbleStrength_Low;
}
