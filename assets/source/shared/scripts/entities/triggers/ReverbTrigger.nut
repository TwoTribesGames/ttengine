include_entity("triggers/Trigger");

class ReverbTrigger extends Trigger
</
	editorImage    = "editor.reverbtrigger"
	libraryImage   = "editor.library.reverbtrigger"
	placeable      = Placeable_Hidden
	sizeShapeColor = Colors.AudioTrigger
	displayName    = "Trigger - Reverb"
	group          = "04.1 Action Triggers"
	stickToEntity  = true
/>
{
	entities = [];
	
	</
		type   = "string"
		choice = getReverbEffectNames()
		order  = 0
		group  = "Specific Settings"
	/>
	reverbEffect = null;
	
	</
		type   = "string"
		choice = getAudioCategoryNames()
		order  = 2
		group  = "Specific Settings"
	/>
	category = null;
	
	</
		type  = "integer"
		min   = 0
		max   = 100
		order = 3
		group = "Specific Settings"
	/>
	reverbVolume = 100;
	
	function onTriggerEnterFirst(p_entity)
	{
		if (reverbEffect != null)
		{
			// HACK: Reverb volume in forest should always be 0.
			//       The forest reverb is only used for the whistle,
			//       which sets its own reverb volume.
			if (reverbEffect == "forest")
			{
				reverbVolume = 0.0;
			}
			
			::Audio.setReverbVolumeForCategory(category, reverbVolume);
			::Audio.setReverbEffect(reverbEffect);
		}
	}
}
