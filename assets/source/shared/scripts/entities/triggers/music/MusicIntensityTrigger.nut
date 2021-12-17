include_entity("triggers/Trigger");

class MusicIntensityTrigger extends Trigger
</
	placeable      = Placeable_Hidden
	sizeShapeColor = Colors.Trigger
	displayName    = "Trigger - Music Intensity"
	group          = "06. Sound"
	stickToEntity  = true
/>
{
	</
		type        = "bool"
		order       = 1
		group       = "Specific Settings"
	/>
	useDefaultFade = true;
	
	</
		type        = "float"
		min         = 0.0
		max         = 10.0
		order       = 2
		group       = "Specific Settings"
		conditional = "useDefaultFade == false"
	/>
	fadeDuration = 5.0;
	
	// Constants
	static c_intensityLevel        = null;
	static c_defaultFadeSlowToFast = 1.0;
	static c_defaultFadeFastToSlow = 10.0;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onTriggerEnterFirst(p_entity)
	{
		local ms = ::MusicSystem.getInstance();
		if (ms.isPlaying() == false)
		{
			ms.play();
		}
		
		local curLevel = ms.getIntensityLevel();
		if (curLevel == c_intensityLevel)
		{
			return;
		}
		
		if (useDefaultFade == false)
		{
			ms.setIntensityLevel(c_intensityLevel, fadeDuration);
		}
		else
		{
			ms.setIntensityLevel(c_intensityLevel,
				c_intensityLevel > curLevel ? c_defaultFadeSlowToFast : c_defaultFadeFastToSlow);
		}
	}
}
