::g_musicSystem <- null;

function getMusicSystemTracks()
{
	local allTracks = ::getMusicTrackNames();

	return allTracks;
}


class MusicSystem
{
	_layers = null;
	_tracks = null;
	_level  = null;

	constructor ()
	{
		_layers = ::SoundLayers("MusicLayer", 3);
		_level  = 0;
	}

	function init(p_track, p_startPlaying)
	{
		_tracks = [];
		if (p_track != null)
		{
			// Create slow/medium and fast music tracks
			_tracks.push(::spawnEntity("MusicSource", ::Vector2(0.0, 0.0),
				{ musicTrack = p_track + "_slow",   soundLayer = "MusicLayer1", enabled = p_startPlaying } ));
			_tracks.push(::spawnEntity("MusicSource", ::Vector2(0.0, 0.5),
				{ musicTrack = p_track + "_medium", soundLayer = "MusicLayer2", enabled = p_startPlaying } ));
			_tracks.push(::spawnEntity("MusicSource", ::Vector2(0.0, 1.0),
				{ musicTrack = p_track + "_fast",   soundLayer = "MusicLayer3", enabled = p_startPlaying } ));
		}
		_layers.muteAllLayers(0.0);
	}

	function isPlaying()
	{
		if (_tracks == null || _tracks.len() == 0)
		{
			return false;
		}

		return _tracks[0].isEnabled();
	}

	function play()
	{
		if (_tracks != null)
		{
			foreach (track in _tracks)
			{
				track.setEnabled(true);
			}
		}
	}

	function stop()
	{
		if (_tracks != null)
		{
			foreach (track in _tracks)
			{
				track.setEnabled(false);
			}
		}
	}

	function getIntensityLevel()
	{
		return _level;
	}

	function setIntensityLevel(p_level, p_fadeDuration = 1.0)
	{
		_level = p_level;
		switch (_level)
		{
		case 0:
			_layers.muteAllLayers(p_fadeDuration);
			break;

		case 1:
			_layers.unmuteLayer("MusicLayer1", p_fadeDuration);
			_layers.muteLayer  ("MusicLayer2", p_fadeDuration);
			_layers.muteLayer  ("MusicLayer3", p_fadeDuration);
			break;

		case 2:
			_layers.muteLayer  ("MusicLayer1", p_fadeDuration);
			_layers.unmuteLayer("MusicLayer2", p_fadeDuration);
			_layers.muteLayer  ("MusicLayer3", p_fadeDuration);
			break;

		case 3:
			_layers.muteLayer  ("MusicLayer1", p_fadeDuration);
			_layers.muteLayer  ("MusicLayer2", p_fadeDuration);
			_layers.unmuteLayer("MusicLayer3", p_fadeDuration);
			break;

		default:
			::tt_panic("Unhandled music intensity level '" + p_level + "'");
			break;
		}
	}
}

function MusicSystem::getInstance()
{
	if (::g_musicSystem == null)
	{
		::g_musicSystem = ::MusicSystem();
	}

	return ::g_musicSystem;
}
