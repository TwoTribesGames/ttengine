include_entity("triggers/Trigger");

class ParticlePlayer extends EntityBase
</
	editorImage    = "editor.particleplayer"
	libraryImage   = "editor.library.particleplayer"
	placeable      = Placeable_Developer
	collisionRect  = [ 0.0, 0.0, 1.0, 1.0 ]  // center X, center Y, width, height
	group          = "99. Misc."
	stickToEntity  = true
/>
{
	</
		type   = "string"
		choice = getParticleEffectNames()
		group  = "Appearance"
		order  = 0.0
	/>
	particleEffect = "dazed";
	
	</
		type   = "string"
		choice = getLayerNames()
		group  = "Appearance"
		order  = 0.2
	/>
	layer = "behind_shoebox_zero";
	
	</
		type  = "bool"
		group = "Appearance"
		order = 0.3
	/>
	keepParticlesOnDisable = true;
	
	</
		type  = "float"
		min   = 0.0
		max   = 25.0
		group = "Appearance"
		order = 0.4
	/>
	spawnDelay = 0.0;
	
	</
		type  = "bool"
		group = "Appearance"
		order = 0.5
	/>
	flipX = false;
	
	</
		type           = "bool"
		group          = "Appearance"
		order          = 0.6
	/>
	flipY = false;
	
	</
		type  = "float"
		min   = 0.1
		max   = 10.0
		group = "Appearance"
		order = 0.7
	/>
	scale = 1.0;
	
	</
		type  = "bool"
		order = 101
		group = "Misc."
	/>
	enabled = true;
	
	</
		type           = "entity"
		filter         = ::getTriggerTypes()
		group          = "Misc."
		order          = 102
		referenceColor = ReferenceColors.Enable
		description    = "When set, this entity will be enabled when an incoming signal fires."
	/>
	enableSignal = null;
	
	_effect = null;
	_layer  = null;
	
	function onInvalidProperties(p_properties)
	{
		p_properties = removeNonexistentProperties(this, p_properties);
		
		return p_properties;
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		// Do validation
		if (particleEffect == null)
		{
			editorWarning("No particle effect selected for this ParticlePlayer!");
			return;
		}
		
		if (enableSignal != null)
		{
			enableSignal.addChildTrigger(this);
		}
		
		::removeEntityFromWorld(this);
		
		_layer = getLayerFromName(layer);
	}
	
	function onSpawn()
	{
		initStickToEntity();
		setEnabled(enabled);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onEnabled()
	{
		_effect = spawnParticleContinuous("particles/" + particleEffect, ::Vector2(0, 0), true, spawnDelay, false, _layer, scale);
		handleFlipping(_effect);
	}
	
	function onDisabled()
	{
		if (_effect != null)
		{
			_effect.stop(keepParticlesOnDisable);
			_effect = null;
		}
	}
	
	function onDie()
	{
		if (_effect != null)
		{
			_effect.stop(keepParticlesOnDisable);
			_effect = null;
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Inherited methods
	
	function _triggerEnter(p_entity, p_parent)
	{
		if (p_parent == null)
		{
			::tt_panic("_triggerEnter should have parent != null");
			return;
		}
		
		// Default behavior
		setEnabled(true);
	}
	
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods
	
	function handleFlipping(p_effect)
	{
		if (flipX || flipY)
		{
			// Effect is IMMEDIATELY started so pregenerated particles miss the flip X and Y flags.
			// Kill the effect and restart it
			p_effect.stop(false);
			if (flipX) p_effect.flipX();
			if (flipY) p_effect.flipY();
			p_effect.play();
		}
	}
	
	function setEnabled(p_enabled)
	{
		enabled = p_enabled;
		
		if (enabled)
		{
			onEnabled();
		}
		else
		{
			onDisabled();
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Update
	
	function update(p_deltaTime)
	{
		updateStickToEntity();
		if (_effect != null && _effect.isActive() == false)
		{
			_effect = null;
		}
	}
}
Trigger.makeTriggerTarget(ParticlePlayer);
