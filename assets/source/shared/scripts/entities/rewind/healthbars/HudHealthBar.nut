include_entity("rewind/healthbars/HealthBar");

class HudHealthBar extends HealthBar
</
	placeable      = Placeable_Hidden
	movementset    = "Static"
	collisionRect  = [ 0.0, 0.0, 0.1, 0.1 ]
/>
{
	// Constants
	static c_hudChunksAlignment = null; // derived class needs to specify this
	static c_position           = ::Vector2(0.08, 0.44);
	static c_hudChunksDistance  = 0.017;
	static c_dangerChunk        = 5;
	
	// Creation params
	_hudChunks = null; // derived class needs to specify this. Cannot be a static as it might be variable per entity
	
	// Internals
	_displayedHealth        = 0.0;
	_hudChunksPresentations = null;
	_isInitializing         = true;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		base.onInit();
		
		_hudChunksPresentations = [];
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onDie()
	{
		base.onDie();
		
		destroyHud();
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods
	
	function updateBar(p_isDamaged, p_newChunk, p_oldChunk)
	{
		// Update chunks
		if (p_isDamaged)
		{
			// Play hit anim on all
			local healthyChunk = ::min(p_newChunk, p_oldChunk);
			for (local i = 0; i < healthyChunk; ++i)
			{
				local tags = healthyChunk <= c_dangerChunk && i <= c_dangerChunk ? ["danger"] : [];
				_hudChunksPresentations[i].stop();
				_hudChunksPresentations[i].start("hit", tags, false, 0);
			}
		}
		
		if (_isInitializing)
		{
			for (local i = 0; i < _hudChunks; ++i)
			{
				_hudChunksPresentations[i].stop();
				if (i < p_newChunk)
				{
					local tags = p_newChunk <= c_dangerChunk && i <= c_dangerChunk ? ["danger"] : [];
					_hudChunksPresentations[i].start(i < p_newChunk ? "full" : "empty", tags, false, 0);
				}
				else
				{
					_hudChunksPresentations[i].start("empty", [], false, 0);
				}
			}
		}
		else if (p_newChunk > p_oldChunk)
		{
			// Healing
			for (local i = 0; i < p_newChunk; ++i)
			{
				local tags = p_newChunk < c_dangerChunk && i < c_dangerChunk ? ["danger"] : [];
				_hudChunksPresentations[i].stop();
				_hudChunksPresentations[i].start(i < p_oldChunk ? "full" : "heal", tags, false, 0);
			}
		}
		else if (p_newChunk < p_oldChunk)
		{
			for (local i = p_oldChunk; i > p_newChunk; --i)
			{
				_hudChunksPresentations[i-1].start("damage", [], false, 0);
			}
		}
	}
	
	function destroyHud()
	{
		foreach (pres in _hudChunksPresentations)
		{
			::Hud.destroyElement(pres);
		}
		_hudChunksPresentations.clear();
	}
	
	function initHud()
	{
		if (_hudChunksPresentations.len() > 0)
		{
			destroyHud();
		}
		
		local flipped = (c_hudChunksAlignment == HorizontalAlignment_Right);
		
		local distance = flipped ? c_hudChunksDistance : c_hudChunksDistance;
		local hudalignment  = flipped ? HudAlignment.Right : HudAlignment.Left;
		
		for (local i = 0; i < _hudChunks; ++i)
		{
			local chunk = ::Hud.createElement(this,
					"presentation/hud_health_chunk",
					hudalignment,
					HudLayer.Normal | HudLayer.VirusUploadMode
			);
			local offset = (i * distance);
			if (flipped) chunk.flipX();
			chunk.setCustomTranslation(c_position + ::Vector2(offset, 0));
			_hudChunksPresentations.push(chunk);
		}
		
		_displayedHealth = 0.0;
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Inherited methods
	
	function initHealth(p_health)
	{
		_isInitializing = true;
		
		base.initHealth(p_health);
		
		initHud();
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Update
	
	function childUpdate(p_deltaTime)
	{
		local newHealth = getNormalizedHealth();
		if (newHealth != _displayedHealth)
		{
			local newChunk = ::ceil(newHealth        * _hudChunks.tofloat());
			local oldChunk = ::ceil(_displayedHealth * _hudChunks.tofloat());
			
			updateBar(newHealth < _displayedHealth, newChunk, oldChunk);
			_displayedHealth = newHealth;
			_isInitializing = false;
		}
	}
}
