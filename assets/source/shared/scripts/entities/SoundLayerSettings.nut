class SoundLayerSettings extends EntityBase
</
	editorImage    = "editor.soundlayersettings"
	libraryImage   = "editor.library.soundlayersettings"
	placeable      =  Placeable_Developer
	collisionRect  = [ 0.0, 1.0, 1.0, 2.0 ]
	group          = "06. Sound"
	displayName    = "Sound - Sound Layer Settings"
/>
{
	</
		type  = "bool"
	/>
	showSoundDebug = false;
	
	_specialSoundMusic = []; // this array is dynamically filled by the "addSoundLayerMember" below
	
	function onInit()
	{
		::g_showSoundDebug = showSoundDebug;
		::g_soundLayersInitialized = false;
		
		registerEntityByTag("soundlayersettings");
		
		local pos  = getPosition() + ::Vector2(0, 3);
		local step = ::Vector2(0, 1);
		
		foreach (special in _specialSoundMusic)
		{
			if (this[special.musicMember] != null)
			{
				pos += step;
				::spawnEntity("MusicSource", pos, { musicTrack = this[special.musicMember], soundLayer = special.layer } );
				::g_specialSoundLayers.setLayerMuted(special.layer, true);
			}
		}
		
		::removeEntityFromWorld(this);
	}
	
	function onSpawn()
	{
		if (::g_soundLayersInitialized == false)
		{
			foreach (soundLayer in ::g_soundLayers.getLayerNames())
			{
				::g_soundLayers.setLayerInitalMuted(soundLayer, this[soundLayer] == false);
			}
			//::g_soundLayers.normalizeLayers();
			
			::g_soundLayersInitialized = true;
		}
	}
}

function addSoundLayerMember(p_class, p_memberName, p_soundLayerName, p_order = 0, p_description = null)
{
	p_class[p_memberName] <- null;
	
	local attributes = {type = "string", choice = getMusicTrackNames(), order = p_order};
	if (p_description != null)
	{
		attributes.description <- p_description;
	}
	p_class.setattributes(p_memberName, attributes);
	
	::g_specialSoundLayers.createLayer(p_soundLayerName);
	p_class._specialSoundMusic.append( { musicMember = p_memberName, layer = p_soundLayerName } );
}

//addSoundLayerMember(SoundLayerSettings, "birdMusic", "SoundLayerBird", 0,"Music played when Toki gets carried by a bird");
//addSoundLayerMember(SoundLayerSettings, "fallMusic", "SoundLayerTokiFall", 0, "Music played when Toki falls down");
::g_soundLayers.addLayerCheckboxesToClass(SoundLayerSettings, 1);

::g_soundLayersInitialized <- false;