include_entity("triggers/Trigger");

class SoundSource extends EntityBase
</
	editorImage    = "editor.soundsource"
	libraryImage   = "editor.library.soundsource"
	placeable      = Placeable_Everyone
	collisionRect  = [ 0.0, 0.5, 1.0, 1.0 ]  // center X, center Y, width, height
	group          = "06. Sound"
	sizeShapeColor = Colors.Sound
	displayName    = "Sound - Sound Source Positional"
	stickToEntity  = true
/>
{
	</
		type   = "string"
		choice = ["Ambient", "Effects", "Music"]
		order  = 0.0
	/>
	soundbank = "Effects";
	
	</
		type   = "string"
		choice = ::getSoundCueNamesInBanks(["Ambient"])
		conditional = "soundbank == Ambient"
		order  = 0.1
	/>
	ambientCue = null;
	
	</
		type   = "string"
		choice = ::getSoundCueNamesInBanks(["Effects"])
		conditional = "soundbank == Effects"
		order  = 0.1
	/>
	effectsCue = null;
	
	</
		type   = "string"
		choice = ::getSoundCueNamesInBanks(["Music"])
		conditional = "soundbank == Music"
		order  = 0.1
	/>
	musicCue = null;
	
	</
		type  = "float"
		min   = 0.0
		max   = 100.0
		order = 1
		description = "Default volume for this sound, the fades will fade to this volume (only works if the volume variable RPC is set correctly)"
	/>
	volume = 100.0;
	
	</
		type  = "float"
		min   = 0.0
		max   = 10.0
		order = 2
		description = "Duration of the fade when the sound gets faded in (i.e. when used as layers)"
	/>
	fadeDuration = 0.5;
	
	</
		type  = "integer"
		min   = 1
		max   = 100
		order = 2.1
		description = "The min radius in tiles where the sound can be heard when used as positional audio"
	/>
	innerRadius = 10;
	
		</
		type  = "integer"
		min   = 1
		max   = 100
		order = 2.2
		description = "The max radius in tiles where the sound can be heard when used as positional audio"
	/>
	radius = 25;
	
	</
		type   = "string"
		choice = ::g_soundLayers.getLayerNames()
		order = 3
		description = "If set, the sound can be used as a sound layer"
	/>
	soundLayer = null;
	
	</
		type           = "bool"
		order          = 100
		group          = "Misc."
	/>
	enabled = true;
	
	</
		type           = "entity"
		filter         = ::getTriggerTypes()
		group          = "Misc."
		order          = 101
		referenceColor = ReferenceColors.Enable
		description    = "When set, this entity will be enabled when an incoming signal fires."
	/>
	enableSignal = null;
	
	</
		type  = "bool"
		order = 99999998
		group = "Misc."
	/>
	positionCulling = true;
	
	_playingCue = null;
	_cueName    = null;
	_muted      = false;
	_enabledSet = false;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		if ((ambientCue != null && effectsCue != null) || (ambientCue != null && musicCue != null) ||
		    (musicCue != null && effectsCue != null))
		{
			editorWarning("Multiple cues selected!");
			::killEntity(this);
			return;
		}
		
		if (ambientCue != null)
		{
			soundbank = "Ambient";
		}
		else if (effectsCue != null)
		{
			soundbank = "Effects";
		}
		else if (musicCue != null)
		{
			soundbank = "Music";
		}
		
		switch (soundbank)
		{
		case "Ambient": _cueName = ambientCue; break;
		case "Effects": _cueName = effectsCue; break;
		case "Music":   _cueName = musicCue; break;
		default:
			::tt_panic("Unhandled soundbank '" + soundbank + "'");
			break;
		}
		
		if (_cueName == null)
		{
			editorWarning("No sound cue set, please select a sound cue to play!");
			::killEntity(this);
			return;
		}
		
		::removeEntityFromWorld(this);
		
		if (soundLayer != null)
		{
			registerEntityByTag(soundLayer);
		}
		
		if (enableSignal != null)
		{
			enableSignal.addChildTrigger(this);
		}
	}
	
	function onSpawn()
	{
		setEnabled(enabled);
		
		if (::g_showSoundDebug)
		{
			updateDebugText();
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	_wasEnabled = null;
	function onCulled()
	{
		_wasEnabled = enabled;
		setEnabled(false);
	}
	
	function onUnculled()
	{
		setEnabled(_wasEnabled == null ? enabled : _wasEnabled);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Inherited methods
	
	function _triggerEnter(p_entity, p_parent)
	{
		// Enabled trigger fired; enable this entity
		setEnabled(true);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods
	
	function updateDebugText()
	{
		setDebugText((soundLayer == null ? "" : soundLayer) + " - cue: " + cue + (_muted ? " (muted)" : ""));
	}
	
	function setEnabled(p_enabled)
	{
		if (_enabledSet && p_enabled == enabled)
		{
			return;
		}
		
		_enabledSet = true;
		enabled = p_enabled;
		
		if (enabled)
		{
			_playingCue = playSound();
			setVolumeInstant(_muted ? 0 : volume);
		}
		else
		{
			if (_playingCue != null && _playingCue.isPlaying())
			{
				_playingCue.stop();
			}
		}
	}
	
	function setMute(p_mute, p_fadeDuration = null)
	{
		_muted = p_mute;
		setVolume(_muted ? 0 : volume, p_fadeDuration);
		updateDebugText();
	}
	
	function setMuteInstant(p_mute)
	{
		setMute(p_mute, 0);
	}
	
	function setVolume(p_volume, p_fadeDuration = null)
	{
		if (_playingCue != null)
		{
			_playingCue.setFadingVariable("Volume", p_volume, p_fadeDuration == null ? fadeDuration : p_fadeDuration);
		}
	}
	
	function setVolumeInstant(p_volume)
	{
		if (_playingCue != null)
		{
			_playingCue.setVariable("Volume", p_volume);
		}
	}
	
	// This can be overridden to play other types of sound effects (like global sounds)
	// should return a SoundCue object.
	function playSound()
	{
		local effect = playSoundEffectFromSoundbank(soundbank, _cueName);
		effect.setRadius(innerRadius, radius);
		
		if (::g_showSoundDebug)
		{
			DebugView.setColor(250, 250, 50, 255);
			DebugView.drawCircle(getPosition(), innerRadius, 10000);
			DebugView.setColor(150, 150, 50, 255);
			DebugView.drawCircle(getPosition(), radius, 10000);
		}
		return effect; 
	}
}
Trigger.makeTriggerTarget(SoundSource);
