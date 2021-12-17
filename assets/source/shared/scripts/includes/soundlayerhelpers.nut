tt_include("includes/math");

::g_showSoundDebug <- false;
::g_soundLayerActions <- ["Enable", "Disable", "Set", "Enable on Loop", "Disable on Loop", "Do nothing"];

class SoundLayer
{
	activations = 0;
	name        = null;
	
	constructor (p_name)
	{
		name = p_name;
	}
	
	function isMuted()
	{//::echo("isMuted:" activations name);
		return activations <= 0;
	}
	
	function normalize()
	{
		activations = activations <= 0 ? 0 : 1;
	}
	
	function setMute(p_mute, p_fadeDuration = null)
	{//::echo("set muted:" p_mute name);
		activations = p_mute ? 0 : 1;
		_setMuteOnSoundSources(p_mute, p_fadeDuration);
	}
	
	function queueMute(p_mute, p_fadeDuration = null)
	{//::echo("queue muted:" p_mute name);
		activations = p_mute ? 0 : 1;
		_queueMuteOnSoundSources(p_mute, p_fadeDuration);
	}
	
	function setMuteInstant(p_mute)
	{//::echo("set muted INSTANT:" p_mute name);
		activations = p_mute ? 0 : 1;
		_setMuteInstantOnSoundSources(p_mute);
	}
	
	function setInitialMute(p_mute)
	{//::echo("set muted INItial:" p_mute name);
		activations = p_mute ? 0 : 1;
		_setInitialMuteOnSoundSources(p_mute);
	}
	
	
	function _setMuteOnSoundSources(p_mute, p_fadeDuration = null)
	{
		local entities = ::getEntitiesByTag(name);
		foreach (e in entities)
		{
			//::assert("setMute" in e, entityIDString(e) + " was registered with tag [" + name + "] and it is not (un)mutable.");
			e.setMute(p_mute, p_fadeDuration);
		}
	}
	
	function _setInitialMuteOnSoundSources(p_mute)
	{
		local entities = ::getEntitiesByTag(name);
		foreach (e in entities)
		{
			//::assert("setInitialMute" in e, entityIDString(e) + " was registered with tag [" + name + "] and it is not (un)mutable.");
			e.setInitialMute(p_mute);
		}
	}
	
	function _setMuteInstantOnSoundSources(p_mute)
	{
		local entities = ::getEntitiesByTag(name);
		foreach (e in entities)
		{
			//::assert("setMuteInstant" in e, entityIDString(e) + " was registered with tag [" + name + "] and it is not (un)mutable.");
			e.setMuteInstant(p_mute);
		}
	}
	
	function _queueMuteOnSoundSources(p_mute, p_fadeDuration = null)
	{
		local entities = ::getEntitiesByTag(name);
		foreach (e in entities)
		{
			//::assert("queueMute" in e, entityIDString(e) + " was registered with tag [" + name + "] and it is not queueMuteable.");
			e.queueMute(p_mute, p_fadeDuration);
		}
	}
}

class SoundLayers
{
	_soundLayerName = null;
	_layers         = null;
	_layerNames     = null;
	
	constructor (p_soundLayerName = "SoundLayer", p_numLayers = 0)
	{
		_soundLayerName = p_soundLayerName;
		_layers     = {};
		_layerNames = [];
		
		if (p_numLayers > 0) createLayers(p_numLayers);
	}
	
	// creates a table with layers and an array with the (sorted) layer names
	function createLayers(p_numLayers)
	{
		foreach (i in ::range(1, p_numLayers))
		{
			local layerName = _soundLayerName + i;
			
			createLayer(layerName);
		}
	}
	
	function createLayer(p_name)
	{
		_layers[p_name] <- ::SoundLayer(p_name);
		_layerNames.append(p_name);
	}
	
	// returns true if a layer is muted
	function isLayerMuted(p_layer)
	{
		return _layers[p_layer].isMuted();
	}
	
	// set the mute for a layer and update its activations count to 1 or 0
	function setLayerMuted(p_layer, p_mute)
	{
		_layers[p_layer].setMute(p_mute);
	}
	
	// set the mute for a layer and update its activations count to 1 or 0
	function setLayerMutedInstant(p_layer, p_mute)
	{
		_layers[p_layer].setMuteInstant(p_mute);
	}
	
	// set the mute for a layer and update its activations count to 1 or 0
	function setLayerInitalMuted(p_layer, p_mute)
	{
		_layers[p_layer].setInitialMute(p_mute);
	}
	
	function unmuteLayer(p_layer, p_fadeDuration = null)
	{
		//_layers[p_layer].activations++;
		//echo(_layers[p_layer].activations, p_layer);
		
		// only update the layer muting when we cross the threshold
		//if (_layers[p_layer].activations == 1)
		{
			_layers[p_layer].setMute(false, p_fadeDuration);
		}
	}
	
	function muteLayer(p_layer, p_fadeDuration = null)
	{
		//_layers[p_layer].activations--;
		//echo(_layers[p_layer].activations, p_layer);
		
		// only update the layer muting when we cross the threshold
		//if (_layers[p_layer].activations == 0)
		{
			_layers[p_layer].setMute(true, p_fadeDuration);
		}
	}
	
	function queueUnmuteLayer(p_layer, p_fadeDuration = null)
	{	
		//_layers[p_layer].activations++; // this means a sound can not be playing but the activation count can already be > 0...
		//echo(_layers[p_layer].activations, p_layer);
		
		// only update the layer muting when we cross the threshold
		//if (_layers[p_layer].activations == 1)
		{
			_layers[p_layer].queueMute(false, p_fadeDuration);
		}
	}
	
	function queueMuteLayer(p_layer, p_fadeDuration = null)
	{
		//_layers[p_layer].activations--; // this means a sound can still be playing but the activation count can be 0...
		//echo(_layers[p_layer].activations, p_layer);
		
		// only update the layer muting when we cross the threshold
		//if (_layers[p_layer].activations == 0)
		{
			_layers[p_layer].queueMute(true, p_fadeDuration);
		}
	}
	
	// set the activations to count for each layer to either 1 or 0
	function normalizeLayers()
	{
		foreach (layerName, layer in _layers)
		{
			local before = layer.activations;
			layer.normalize();
//echo("\t\t", before, layer.activations, layerName);
		}
	}
	
	// mute all music layers at once
	function muteAllLayers(p_fadeDuration)
	{
		foreach (layerName, layer in _layers)
		{
			layer.setMute(true, p_fadeDuration);
		}
	}
	
	function getLayerNames()
	{
		return _layerNames;
	}
	
	// add a bool checkbox to an entity class for each sound layer to an entity class 
	function addLayerCheckboxesToClass(p_class, p_startOrder = 4)
	{
		foreach (soundLayer in _layerNames) 
		{
			p_class[soundLayer] <- false;
			p_class.setattributes(soundLayer, {type = "bool", order = p_startOrder});
			p_startOrder++;
		}
	}
}

// create general sound layers (can be used for additional music)
::g_soundLayers <- SoundLayers("SoundLayer", 10);
// create sound layers for special events (e.g. being carried by the bird)
::g_specialSoundLayers <- SoundLayers();
