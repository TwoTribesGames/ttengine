class Url extends EntityBase
</ 
	editorImage    = "editor.url"
	libraryImage   = "editor.library.url"
	placeable      = Placeable_Developer
	collisionRect  = [ 0.0, 1.0, 2.0, 2.0 ]  // center X, center Y, width, height
	sizeShapeColor = Colors.GUI
	group          = "99. Misc."
/>
{
	</
		type  = "string"
		order = 0
	/>
	url = "http://www.foo.bar";
	
	</
		type = "float"
		min  = 1.0
		max  = 50.0
	/>
	width = 3.0;
	
	</
		type = "float"
		min  = 1.0
		max  = 50.0
	/>
	height = 3.0;
	
	function onInit()
	{
		setCollisionRect(::Vector2(0, 1), width, height);
		
		if (url.slice(0,7) != "http://" || url.slice(0,8) != "https://")
		{
			url = "http://" + url;
		}
	}
	
	function onPointerReleased(p_event)
	{
		if (Level.isUserLevel() == false)
		{
			openURL(url);
		}
	}
}
