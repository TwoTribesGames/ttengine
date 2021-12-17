include_entity("SoundSource");

class GlobalSoundSource extends SoundSource
</
	editorImage    = "editor.globalsoundsource"
	libraryImage   = "editor.library.globalsoundsource"
	placeable      = Placeable_Everyone
	collisionRect  = [ 0.0, 0.0, 0.01, 0.01 ]  // center X, center Y, width, height
	group          = "06. Sound"
	displayName    = "Sound - Sound Source Ambient"
/>
{
	function onSpawn()
	{
		base.onSpawn();
		
		if (::g_showSoundDebug)
		{
			makeScreenSpaceEntity();
			
			// "transform" the position into screenspace
			local pos = getPosition();
			pos.y = Level.getHeight() - pos.y;
			
			pos.x *= 0.05;
			pos.y *= 0.025;
			
			pos.x -= 0.7; // assume the sound sources are one tile right of the level border
			pos.y = (1 - pos.y) - 0.5;
			
			setPosition(pos);
		}
	}
	
	function playSound()
	{
		return ::Audio.playGlobalSoundEffect(soundbank, _cueName);
	}
}

// hide inner and outer radius settings
GlobalSoundSource.setattributes("innerRadius", null);
GlobalSoundSource.setattributes("radius", null);