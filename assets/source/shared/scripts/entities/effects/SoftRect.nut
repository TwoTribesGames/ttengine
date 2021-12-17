class SoftRect extends EntityBase
</
	editorImage    = "editor.softrect"
	libraryImage   = "editor.library.softrect"
	placeable      = Placeable_Everyone
	collisionRect  = [ 0.0, 1.0, 2.0, 2.0 ]
	sizeShapeColor = Colors.Effect
	displayName    = "Effect - Soft Rect"
	group          = "08. Soft Rect Effects"
	stickToEntity  = true
/>
{
	</
		type   = "string"
		choice = ["Controlling Entity", "Camera"]
		order  = 0
	/>
	targetType = "Controlling Entity";
	
	</
		type  = "float"
		min   = 0.0
		max   = 1.0
		order = 1
	/>
	baseStrength = 1.0;
	
	</
		type    = "integer"
		min     = 0
		max     = 100
		order   = 2
	/>
	leftBorder = 5;
	
	</
		type    = "integer"
		min     = 0
		max     = 100
		order   = 2.1
	/>
	rightBorder = 5;
	
	</
		type    = "integer"
		min     = 0
		max     = 100
		order   = 2.2
	/>
	topBorder = 5;
	
	</
		type    = "integer"
		min     = 0
		max     = 100
		order   = 2.3
	/>
	bottomBorder = 5;
	
	</
		type = "integer"
		min  = 0
		max  = 250
		description = "Trigger width, use 0 to make it as wide as the level"
	/>
	width = 6;
	
	</
		type = "integer"
		min  = 0
		max  = 250
		description = "Trigger height, use 0 to make it as tall as the level"
	/>
	height = 6;
	
	</
		type         = "bool"
		order        = 101
	/>
	enabled = true;
	
	_effectRect = null;
	
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
		
		local target = EffectRectTarget_ControllingEntityPos;
		switch (targetType)
		{
			case "Camera":
				target = EffectRectTarget_CameraPos;
				break;
			case "Controlling Entity":
				target = EffectRectTarget_ControllingEntityPos;
				break;
			default:
				target = EffectRectTarget_ControllingEntityPos;
				break;
		}
		
		_effectRect = addEffectRect(target);
		_effectRect.setSize(width, height);
		
		_effectRect.setLeftBorder  (leftBorder);
		_effectRect.setRightBorder (rightBorder);
		_effectRect.setTopBorder   (topBorder);
		_effectRect.setBottomBorder(bottomBorder);
		
		setEnabled(enabled);
	}
	
	function onSpawn()
	{
		initStickToEntity();
	}
	
	function getEffectArea()
	{
		return _effectRect;
	}
	
	function setHeight(p_height)
	{
		height = p_height;
		_effectRect.setSize(width, height);
	}
	
	function setWidth(p_width)
	{
		width = p_width;
		_effectRect.setSize(width, height);
	}
	
	function setEnabled(p_enabled)
	{
		//if (_triggerSensor != null) _triggerSensor.setEnabled(p_enabled);
		_effectRect.setBaseStrengthInstant(p_enabled ? baseStrength : 0);
		
		enabled = p_enabled;
	}
	
	function isEnabled()
	{
		return enabled;
	}
	
	function _typeof()
	{
		return "SoftRect";
	}
	
	function onTimer(p_name)
	{
		// FIXME: use updateStickToEntity here, but that won't work with the effect rects
		if (p_name == "remove")
		{
			::killEntity(this);
		}
	}
	
	function update(p_deltaTime)
	{
		// FIXME: use updateStickToEntity here, but that won't work with the effect rects
		if (stickToEntity != null)
		{
			if (::isValidEntity(stickToEntity))
			{
				// Set in center
				if (stickToEntityOffset == null)
				{
					setPosition(stickToEntity.getCenterPosition() - getCenterOffset());
				}
				else
				{
					local pos = stickToEntity.getCenterPosition() + stickToEntityOffset;
					setPosition(pos - getCenterOffset());
				}
			}
			else
			{
				if (hasTimer("remove") == false)
				{
					setPosition(::Vector2(-10000, -10000));
					startTimer("remove", 2.0);
				}
			}
		}
	}
}
