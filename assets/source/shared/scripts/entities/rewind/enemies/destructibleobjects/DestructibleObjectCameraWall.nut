include_entity("rewind/enemies/destructibleobjects/DestructibleObject");

class DestructibleObjectCameraWall extends DestructibleObject
</
	editorImage         = "editor.destructibleobjectcamerawall"
	libraryImage        = "editor.library.destructibleobjectcamerawall"
	placeable           = Placeable_Everyone
	movementset         = "Static"
	collisionRect       = [ 0.0, 1.0, 2.0, 2.0 ]
	group               = "01.2 Destructible Objects"
	displayName         = "Camera on Wall"
	stickToEntity       = true
/>
{
	// Misc.
	static c_maxHealth                = 5;
	static c_score                    = 10;
	static c_hasPermanentCorpse       = true;
	
	// General settings
	static c_damagedByBullets         = true;
}
