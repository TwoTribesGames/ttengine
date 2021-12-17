::g_activeMusicTracks <- [];

class MusicSource extends EntityBase
</
	editorImage    = "editor.musicsource"
	libraryImage   = "editor.library.musicsource"
	placeable      = Placeable_Developer
	collisionRect  = [ 0.0, 0.5, 1.0, 1.0 ]  // center X, center Y, width, height
	group          = "06. Sound"
	displayName    = "Sound - Music Track"
/>
{
	</
		type   = "string"
		choice = ::getMusicTrackNames()
		order  = 0
	/>
	musicTrack = null;
	
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
		type   = "string"
		choice = ::g_soundLayers.getLayerNames()
		order = 3
		description = "If set, the sound can be used as a sound layer"
	/>
	soundLayer = null;
	
	_muted             = false;
	_queuedMute        = false;
	_queudFadeDuration = 0.5;
	_hasMuteQueued     = false;
	
	</
		type = "bool"
		order = 5
	/>
	enabled = true;
	_enabledSet = false;
	
	_playingMusicTrack = null;
	_loopEvent = null;
	
	function onValidateScriptState()
	{
		if (musicTrack == null)
		{
			editorWarning("No music track set, please select a music track!");
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		//::assert(musicTrack != null, entityIDString(this) + " has no track set! Can't play nothing!");
		
		::removeEntityFromWorld(this);
		
		if (soundLayer != null)
		{
			registerEntityByTag(soundLayer);
		}
		
		// Create the music track early on (so that the data can be preloaded)
		// NOTE: Creating a track does not automatically *play* it, so this is fine
		_playingMusicTrack = ::createMusicTrack(musicTrack);
		if (_playingMusicTrack != null)
		{
			_playingMusicTrack.setCallbackEntity(this);
			::g_activeMusicTracks.push(_playingMusicTrack);
		}
	}
	
	function onSpawn()
	{
		if (::g_showSoundDebug)
		{
			setShowPresentationTags(true);
			updateDebugText();
		}
		
		_noMuteSetYet = true; // from here on it doesn't matter if a mute was set yet
		setEnabled(enabled);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onMusicTrackLooped(p_track)
	{
		if (_hasMuteQueued)
		{
			_setMute(_queuedMute, _queudFadeDuration);
		}
		
		if (_loopEvent != null)
		{
			_loopEvent.publish(p_track);
		}
	}
	
	function onProgressRestored(p_id)
	{
		if (_playingMusicTrack != null)
		{
			local volume = _playingMusicTrack.getVolume();
			setVolumeInstant(0);
			setVolume(volume, 1.0);
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods
	
	function destructor()
	{
		if (_playingMusicTrack != null)
		{
			destroyMusicTrack (_playingMusicTrack);
			
			// find this track in global ::g_activeMusicTracks
			local idx = ::g_activeMusicTracks.find(_playingMusicTrack);
			::assert(idx != null, "Destroyed music track not in ::g_activeMusicTracks");
			::g_activeMusicTracks.remove(idx);
		}
	}
	
	function updateDebugText()
	{
		setDebugText((soundLayer == null ? "" : soundLayer) + " - track: " + musicTrack + (_muted ? " (muted)" : ""));
	}
	
	function setEnabled(p_enabled)
	{
		_enabledSet = true;
		
		if (p_enabled)
		{
			setState("Enabled");
		}
		else
		{
			setState("Disabled");
		}
	}
	
	function isEnabled()
	{
		return (_enabledSet && enabled);
	}
	
	_noMuteSetYet = true;
	function setInitialMute(p_mute)
	{
		if (_noMuteSetYet)
		{
			_setMute(p_mute, 0);
		}
	}
	
	function setMute(p_mute, p_fadeDuration = null)
	{
		_setMute(p_mute, p_fadeDuration);
	}
	
	function setMuteInstant(p_mute)
	{
		_setMute(p_mute, 0);
	}
	
	function queueMute(p_mute, p_fadeDuration = null)
	{
		_queuedMute        = p_mute;
		_queudFadeDuration = p_fadeDuration;
		_hasMuteQueued     = true;
	}
	
	function _setMute(p_mute, p_fadeDuration = null)
	{
		_muted = p_mute;
		setVolume(_muted ? 0 : volume, p_fadeDuration);
		updateDebugText();
		
		_hasMuteQueued = false;
		_noMuteSetYet  = false;
	}
	
	function setVolume(p_volume, p_fadeDuration = null)
	{
		if (_playingMusicTrack != null)
		{
			_playingMusicTrack.setVolume(p_volume, p_fadeDuration == null ? fadeDuration : p_fadeDuration, false);
		}
	}
	
	function setVolumeInstant(p_volume)
	{
		if (_playingMusicTrack != null)
		{
			_playingMusicTrack.setVolume(p_volume, 0, false);
		}
	}
	
	function subscribeLoopEventListener(p_listener)
	{
		if (_loopEvent == null)
		{
			_loopEvent = ::EventPublisher("loopevent");
		}
		_loopEvent.subscribe(p_listener, "onMusicTrackLooped");
	}
	
	function unsubscribeLoopEventListener(p_listener)
	{
		if (_loopEvent == null)
		{
			return;
		}
		_loopEvent.unsubscribe(p_listener, "onMusicTrackLooped");
	}
	
	// 'Static' methods
	function stopAll()
	{
		foreach (source in ::g_activeMusicTracks)
		{
			source.stop();
		}
	}
	
	function pauseAll()
	{
		foreach (source in ::g_activeMusicTracks)
		{
			source.pause();
		}
	}
	
	function resumeAll()
	{
		foreach (source in ::g_activeMusicTracks)
		{
			source.unpause();
		}
	}
	
	function setVolumeAll(p_volume, p_fadeDuration = null)
	{
		foreach (source in ::g_activeMusicTracks)
		{
			source.setVolume(p_volume, p_fadeDuration == null ? fadeDuration : p_fadeDuration, false);
		}
	}
}

class MusicSource_State.Enabled
{
	function onEnterState()
	{
		enabled = true;
		_playingMusicTrack.play();
		setVolumeInstant(_muted ? 0 : volume);
	}
}

class MusicSource_State.Disabled
{
	function onEnterState()
	{
		enabled = false;
		if (_playingMusicTrack != null &&
		    _playingMusicTrack.isPlaying())
		{
			_playingMusicTrack.stop();
		}
	}
}
