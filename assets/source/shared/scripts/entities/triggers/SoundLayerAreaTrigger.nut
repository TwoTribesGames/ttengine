include_entity("triggers/Trigger");

class SoundLayerAreaTrigger extends Trigger
</
	editorImage    = "editor.soundlayerareatrigger"
	libraryImage   = "editor.library.soundlayerareatrigger"
	placeable      = Placeable_Developer
	collisionRect  = [ 0.0, 1.0, 2.0, 2.0 ]
	sizeShapeColor = Colors.AudioTrigger
	displayName    = "Trigger - SoundLayer"
	group          = "06. Sound"
	stickToEntity  = true
/>
{
	</
		type   = "string"
		choice = ::g_soundLayerActions
		order  = 0
		group  = "Specific Settings"
	/>
	enterAction = "Enable";
	
	</
		type   = "string"
		choice = ::g_soundLayerActions
		order  = 1
		group  = "Specific Settings"
	/>
	exitAction = "Disable";
	
	</
		type        = "bool"
		description = "Override default fade duration set in the sound sources themselves when fading in"
		order       = 2
		group       = "Specific Settings"
	/>
	useFadeInDuration = true;
	
	</
		type        = "float"
		min         = 0.0
		max         = 10.0
		description = "Fade duration for when the mode is set to Enable. Note: this duration is not used when the mode is 'Set'"
		order       = 2.1
		group       = "Specific Settings"
	/>
	fadeInDuration = 0.5;
	
	</
		type        = "bool"
		description = "Override default fade duration set in the sound sources themselves when fading out"
		order       = 3
		group       = "Specific Settings"
	/>
	useFadeOutDuration = true;
	
	</
		type        = "float"
		min         = 0.0
		max         = 10.0
		description = "Fade duration for when the mode is set to Disable. Note: this duration is not used when the mode is 'Set'"
		order       = 3.1
		group       = "Specific Settings"
	/>
	fadeOutDuration = 0.5;
	
	</
		type        = "bool"
		description = "Automatically perform exitAction after a set delay, turn 'once' on to make this a one time event"
		order       = 3.2
		group       = "Specific Settings"
	/>
	autoExit = false;
	
	</
		type        = "float"
		min         = 0.5
		max         = 100.0
		description = "Delay for when autoExit is used"
		order       = 3.3
		conditional = "autoExit == true"
		group       = "Specific Settings"
	/>
	autoExitDelay = 10.0;
	
	function onInvalidProperties(p_properties)
	{
		local propertyFixes = {}
		propertyFixes.autoDisable <- { result = "autoExit" };
		propertyFixes.autoDisableDelay <- { result = "autoExitDelay" };
		
		fixInvalidProperties(p_properties, propertyFixes); 
		
		return base.onInvalidProperties(p_properties);
	}
	
	function getActiveLayers()
	{
		foreach (soundLayer in ::g_soundLayers.getLayerNames())
		{
			if (this[soundLayer]) yield soundLayer;
		}
	}
	
	function performAction(p_action)
	{
		switch (p_action)
		{
			case "Enable":
				foreach (layer in getActiveLayers())
				{
					::g_soundLayers.unmuteLayer(layer, useFadeInDuration ? fadeInDuration : null);
				}
				break;
			case "Disable":
				foreach (layer in getActiveLayers())
				{
					::g_soundLayers.muteLayer(layer, useFadeOutDuration ? fadeOutDuration : null);
				}
				break;
			case "Enable on Loop":
				foreach (layer in getActiveLayers())
				{
					::g_soundLayers.queueUnmuteLayer(layer, useFadeInDuration ? fadeInDuration : null);
				}
				break;
			case "Disable on Loop":
				foreach (layer in getActiveLayers())
				{
					::g_soundLayers.queueMuteLayer(layer, useFadeOutDuration ? fadeOutDuration : null);
				}
				break;
			case "Set":
				// set the soundlayer init flag to true so the soundlayer settings won't undo the 
				// changes made by this trigger if it is connected to a spawn/landingstone/door
				::g_soundLayersInitialized = true;
				foreach (layer in ::g_soundLayers.getLayerNames())
				{
					::g_soundLayers.setLayerMuted(layer, this[layer] == false);
				}
				break;
		}
	}
	
	function onSpawn()
	{
		base.onSpawn();
		
		if (::g_showSoundDebug)
		{
			local halfsize = ::Vector2(width * 0.5, height * 0.5);
			DebugView.setColor(200, 200, 255, 196);
			DebugView.drawRect(getCenterPosition() - halfsize, getCenterPosition() + halfsize, 10000);
			
			local text = "";
			foreach (soundLayer in getActiveLayers())
			{
				text += "[" + enterAction + "] " + soundLayer + " [" + exitAction + "]\n";
			}
			DebugView.drawTextInWorld(getCenterPosition() - halfsize, text, 10000);
		}
	}
	
	function onTriggerEnterFirst(p_entity)
	{
		performAction(enterAction);
		
		if (autoExit)
		{
			startTimer("autoExit", autoExitDelay);
		}
	}
	
	function onTimer(p_name)
	{
		if (p_name == "autoExit")
		{
			doExitAction();
		}
	}
	
	function onTriggerExit(p_entity)
	{
		doExitAction();
	}
	
	function doExitAction()
	{
		performAction(exitAction);
	}
}

::g_soundLayers.addLayerCheckboxesToClass(SoundLayerAreaTrigger);