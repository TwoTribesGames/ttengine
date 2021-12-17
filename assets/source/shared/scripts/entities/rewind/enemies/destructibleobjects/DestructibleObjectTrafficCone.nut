include_entity("rewind/enemies/destructibleobjects/DestructibleObject");

class DestructibleObjectTrafficCone extends DestructibleObject
</
	editorImage         = "editor.destructibleobjecttrafficcone"
	libraryImage        = "editor.library.destructibleobjecttrafficcone"
	placeable           = Placeable_Everyone
	movementset         = "Static"
	collisionRect       = [ 0.0, 0.5, 1.0, 1.0 ]
	group               = "01.2 Destructible Objects"
	displayName         = "Traffic Cone"
	stickToEntity       = true
/>
{
	// Misc.
	static c_maxHealth                = 1;
	static c_score                    = 1;
	
	// Touch related values
	static c_touchSound               = "trafficcone_touch";
	
	// General settings
	static c_damagedByBullets         = true;
	static c_rumbleStrength           = null;
	
	// Physics settings
	static c_hasPhysicsMovement       = true;
	static c_hasCollisionResponse     = true;
	static c_mass                     = 3.0;
	static c_drag                     = 0.15;
}
