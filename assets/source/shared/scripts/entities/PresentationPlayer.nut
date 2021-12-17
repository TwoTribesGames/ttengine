include_entity("rewind/RewindEntity");

class PresentationPlayer extends RewindEntity
</ 
	editorImage    = "editor.presentationplayer"
	libraryImage   = "editor.library.presentationplayer"
	placeable      = Placeable_Developer
	collisionRect  = [ 0.0, 0.5, 1.0, 1.0 ]  // center X, center Y, width, height
	stickToEntity  = true
	group          = "99. Misc."
/>
{
	</
		type        = "string"
		choice      = getPresentationNames()
		order       = 1
	/>
	presentation = null;
	
	</
		type        = "string"
		choice      = getLayerNames()
		description = "The layer to load the presentation file in, leave empty to load the presentation in the default layer"
		order       = 2
	/>
	presentationLayer = null;
	
	</
		type        = "string"
		order       = 3
		description = "The presentation name to play"
	/>
	name = "";
	
	</
		type        = "string"
		order       = 4
		description = "A comma separated list of tags to add e.g: bloated, falling, scared"
	/>
	tags = "";
	
	</
		type  = "float"
		min   = -2
		max   = 2
		order = 5
	/>
	screenSpaceXPos = 0;
	
	</
		type  = "float"
		min   = -2
		max   = 2
		order = 5
	/>
	screenSpaceYPos = 0;
	
	</
		type = "bool"
		order = 7
	/>
	flipX = false;
	
	</
		type = "bool"
		order = 8
	/>
	flipY = false;
	
	</
		type  = "float"
		min   = 0.1
		max   = 10.0
		order = 9
	/>
	scale = 1.0;
	
	</
		type         = "bool"
		order        = 101
	/>
	enabled = true;
	
	_layer = ParticleLayer_InFrontOfEntities; // default to the entities layer
	_presentation = null;
	
	function onInit()
	{
		base.onInit();
		
		::removeEntityFromWorld(this);
		
		if (presentation != null)
		{
			if (presentationLayer != null)
			{
				_layer = getLayerFromName(presentationLayer);
				
				if (presentationLayer == "hud" || presentationLayer == "in_front_of_hud")
				{
					setPosition(::Vector2(screenSpaceXPos, screenSpaceYPos));
					makeScreenSpaceEntity();
				}
			}
			
			_presentation = createPresentationObjectInLayer("presentation/" + presentation, _layer);
			
			if (tags != "")
			{
				local _tags = split(tags, " ,");
				foreach (tag in _tags)
				{
					_presentation.addTag(tag);
				}
			}
			
			// Add region as tag
			local region = ::getRegion();
			if (region.len() > 0)
			{
				_presentation.addTag(region);
			}
			
			_presentation.addTag(::getPlatformString());
			
			if (flipX)
			{
				_presentation.flipX();
			}
			
			if (flipY)
			{
				_presentation.flipY();
			}
			
			_presentation.setCustomUniformScale(scale);
		}
		
		setEnabled(enabled);
	}
	
	function isEnabled()
	{
		return enabled;
	}
	
	function setEnabled(p_enabled)
	{
		enabled = p_enabled;
		
		_presentation.stop();
		if (enabled && _presentation != null)
		{
			_presentation.start(name, [], false, 1);
		}
	}
}
