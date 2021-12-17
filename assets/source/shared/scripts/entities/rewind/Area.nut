class Area extends EntityBase
</
	editorImage    = "editor.area"
	libraryImage   = "editor.library.area"
	placeable      = Placeable_Everyone
	collisionRect  = [ 0.0, 0.5, 1.0, 1.0 ]  // center X, center Y, width, height
	sizeShapeColor = Colors.Area
	stickToEntity  = true
/>

{
	</
		type        = "string"
		choice      = ["circle", "rectangle"]
		group       = "Appearance"
		order       = 0.0
	/>
	type = "rectangle";
	
	</
		type        = "integer"
		min         = 1
		max         = 100
		conditional = "type == rectangle"
		group       = "Appearance"
		order       = 0.1
	/>
	width = 6;
	
	</
		type        = "integer"
		min         = 1
		max         = 100
		conditional = "type == rectangle"
		group       = "Appearance"
		order       = 0.2
	/>
	height = 6;
	
	</
		type        = "integer"
		min         = 1
		max         = 50
		conditional = "type == circle"
		group       = "Appearance"
		order       = 0.3
	/>
	radius = 3;
	
	function onInvalidProperties(p_properties)
	{
		removeNonexistentProperties(this, p_properties);
		
		return p_properties;
	}
	
	function onInit()
	{
		::removeEntityFromWorld(this);
	}
	
	function onSpawn()
	{
		initStickToEntity();
	}
	
	function _typeof()
	{
		return "Area";
	}
	
	function getPointInArea(p_marginX = 0, p_marginY = 0)
	{
		if (type == "rectangle")
		{
			local w = ::max(0, (width * 0.5)  - p_marginX);
			local h = ::max(0, (height * 0.5) - p_marginY);
			
			return (getCenterPosition() + ::Vector2.frnd(-w, w, -h, h));
		}
		else if (type == "circle")
		{
			// taken from http://stackoverflow.com/questions/5837572/generate-a-random-point-within-a-circle-uniformly
			local t = 2.0 * ::PI * frnd();
			local u = frnd() + frnd();
			local r = (u > 1.0 ? 2.0 - u : u) * radius;
			local pos = getCenterPosition();
			return ::Vector2(r * ::cos(t) + pos.x, r * ::sin(t) + pos.y);
		}
		
		::tt_panic("Unhandled area type '" + type + "'");
		return ::Vector2(0, 0);
	}
	
	function update(p_deltaTime)
	{
		updateStickToEntity();
	}
}
