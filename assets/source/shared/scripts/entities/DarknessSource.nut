class DarknessSource extends EntityBase
</ 
	editorImage    = "editor.darknesssource"
	libraryImage   = "editor.library.darknesssource"
	placeable      = Placeable_Everyone
	collisionRect  = [ 0.0, 1.0, 2.0, 2.0 ]  // center X, center Y, width, height
	sizeShapeColor = Colors.Darkness
	displayName    = "Light - Darkness Source"
	stickToEntity  = true
/>
{
	</
		type  = "integer"
		min   = 0
		max   = 250
		order = 0
	/>
	width = 2;
	
	</
		type  = "integer"
		min   = 0
		max   = 250
		order = 1
	/>
	height = 2;
	
	</
		type  = "bool"
		order = 3
	/>
	enabled = true;
	
	_darkness = null;
	
	function onInit()
	{
		::removeEntityFromWorld(this);
		
		local pos = getPosition();
		if (width  == 0) 
		{
			width  = Level.getWidth() + ::fabs((2 * pos.x) - Level.getWidth());
		}
		if (height == 0)
		{
			height = Level.getHeight() + ::fabs((2 * pos.y) - Level.getHeight());
		}
		
		_darkness = addDarkness(width, height);
		setEnabled(enabled);
	}
	
	function onSpawn()
	{
		initStickToEntity();
	}
	
	function setEnabled(p_enabled)
	{
		_darkness.setEnabled(p_enabled);
	}
	
	function isEnabled()
	{
		return _darkness.isEnabled();
	}
	
	function update(p_deltaTime)
	{
		updateStickToEntity();
	}
}
