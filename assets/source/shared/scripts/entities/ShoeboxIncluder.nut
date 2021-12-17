function getFilteredShoeboxIncludes()
{
	local names = ::getShoeboxIncludeNames();
	local result = [];
	foreach (name in names)
	{
		if (::stringEndsWith(name, "_lightmask") == false)
		{
			result.push(name);
		}
	}
	return result;
}


class ShoeboxIncluder extends EntityBase
</
	editorImage         = "editor.shoeboxincluder"
	libraryImage        = "editor.library.shoeboxincluder"
	placeable           = Placeable_Developer
	collisionRect       = [ 0.0, 1.0, 2.0, 2.0 ]  // center X, center Y, width, height
	ignoreSpawnSections = true
/>
{
	</
		type        = "string"
		choice      = ::getFilteredShoeboxIncludes()
		order       = 0
		description = "Path to the shoebox in the levels folder"
	/>
	shoeboxInclude = null;
	
	</
		type        = "float"
		order       = 1
		min         = 0
		max         = 10
	/>
	scale = 1;
	
	</
		type        = "integer"
		order       = 2
		min         = -100
		max         = 100
	/>
	priority = 0;
	
	</
		type        = "float"
		order       = 3
		min         = -100
		max         = 100
	/>
	zPosition = 0;
	
	function onInit()
	{
		if (shoeboxInclude == null)
		{
			editorWarning("Please select an include");
			return;
		}
	}
	
	function onSpawn()
	{
		if (shoeboxInclude != null)
		{
			local region = ::getRegion();
			if (region.len() > 0)
			{
				local names = ::getShoeboxIncludeNames();
				if (names.find(region + "/" + shoeboxInclude) != null)
				{
					shoeboxInclude = region + "/" + shoeboxInclude;
				}
			}
			addShoeboxInclude("levels/shoebox_includes/" + shoeboxInclude, getShoeboxSpaceFromWorldPos(getPosition()), zPosition, priority, scale);
		}
		::killEntity(this);
	}
}
