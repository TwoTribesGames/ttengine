include_entity("rewind/RewindEntity");

class CollisionMaker extends RewindEntity
</
	editorImage    = "editor.collisionmaker"
	libraryImage   = "editor.library.collisionmaker"
	placeable      = Placeable_Developer
	sizeShapeColor = Colors.Area
	displayName    = "Collision Maker"
	stickToEntity  = true
/>
{
	</
		type  = "bool"
		order = 0
	/>
	enabled = true;
	
	</
		type = "integer"
		min  = 1
		max  = 200
		order = 1
	/>
	width  = 4;
	
	</
		type  = "integer"
		min   = 1
		max   = 200
		order = 2
	/>
	height = 4;
	
	</
		type   = "string"
		choice = ["Solid", "Crystal", "Solid_Allow_Pathfinding"] // FIXME: Allow for water/lava sources? Currently unsupported by code
		order  = 3
	/>
	tileType = "Solid";
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		setCollisionRect(::Vector2(0, 1), width, height);
		
		base.onInit();
		
		setPositionCullingEnabled(false);
		setCanBePushed(false);
		setUpdateSurvey(false);
		addInvincibilityProperties();
		addProperty("weaponGroup", ::WeaponGroup.Neutral);
		setDetectableByTouchOnly();
		
		setEnabled(enabled);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onProgressRestored(p_id)
	{
		// something fishy is going on with restoring CollisionMakers causing water tiles to
		// appear when lava falls are on the same Y location and are both blocked by CollisionMakers.
		// This hack will work around that.
		removeEntityCollisionTiles();
		startTimer("HACK_setEnabled", 0.01);
	}
	
	function onTimer(p_name)
	{
		if (p_name == "HACK_setEnabled")
		{
			setEnabled(enabled);
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods
	
	function setEnabled(p_enabled)
	{
		enabled = p_enabled;
		
		if (enabled)
		{
			addProperty("noticeBullets");
			setDetectableByTouch(true);
			setEntityCollisionTiles(createCollisionTileString(width, height, getTileCharByName(tileType)));
			setLightBlocking(tileType != "Crystal");
		}
		else
		{
			removeProperty("noticeBullets");
			setDetectableByTouch(false);
			removeEntityCollisionTiles();
			setLightBlocking(false);
		}
	}
}
