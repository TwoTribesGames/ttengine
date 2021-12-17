class PlayerBotSpawnPoint extends EntityBase
</
	editorImage    = "editor.playerbotspawnpoint"
	libraryImage   = "editor.library.playerbotspawnpoint"
	placeable      = Placeable_Everyone
	movementset    = "Static"
	collisionRect  = [ 0.0, 1.0, 2.0, 2.0 ]  // note the large collision rect use to make sure it keeps empty space for the entities in the elevator
	displayName    = "PlayerBot Spawn Point"
/>
{
	</
		type = "integer"
		min  = 1
		max  = 10
	/>
	order = 1;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		if (isTestBuild() == false)
		{
			::killEntity(this);
			return;
		}
		
		registerEntityByTag("PlayerBotSpawnPoint");
		
		::removeEntityFromWorld(this);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onDebugSpawnPointReached(p_player)
	{
		// Save this checkpoint
		::ProgressMgr.setLastCheckPoint("debugspawnpoint");
		::ProgressMgr.storeCheckPoint();
	}
}
