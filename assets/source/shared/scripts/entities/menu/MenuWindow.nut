include_entity("menu/MenuCanvas");

class MenuWindow extends MenuCanvas
</
	placeable      = Placeable_Hidden
	collisionRect  = [ 0.0, 0.0, 6.0, 4.0 ]
/>
{
	static c_borderThickness = 10.0 / 1080;
	static c_margin          = 0.012;
	static c_titlebarHeight  = 0.05;
	
	_titleID  = null;
	_width    = null;
	_height   = null;
	_zOffset  = 0.0;
	_parent   = null;
	
	// Internal values
	_titlebarOffset = null;
	_xOffset        = 0;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		base.onInit();
		
		if (_parent == null)
		{
			::tt_panic("MenuWindow created without parent");
		}
		
		local totalHeight = _height + c_titlebarHeight;
		
		_titlebarOffset = (totalHeight - c_titlebarHeight) / 2.0;
		_xOffset        = (-_width / 2.0) + c_margin;
		
		local pres = createStoppedPresentation("window", "window");
		pres.setCustomZOffset(_zOffset);
		pres.addCustomValue("dropshadowWidth", _width * 1.43);
		pres.addCustomValue("dropshadowHeight", totalHeight * 1.8);
		pres.addCustomValue("borderWidth", _width + c_borderThickness);
		pres.addCustomValue("borderHeight", totalHeight + c_borderThickness);
		pres.addCustomValue("canvasWidth", _width);
		pres.addCustomValue("canvasHeight", totalHeight);
		pres.addCustomValue("titlebarOffset", _titlebarOffset);
		
		createTitle(_titleID);
		
		pres.start("fadein", [], false, 0);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onTextLabelTyped(p_id)
	{
		if (_parent != null)
		{
			_parent.customCallback("onWindowTextLabelTyped", this, p_id);
		}
	}
	
	function onDelayedPresentationShown(p_id)
	{
		if (_parent != null)
		{
			_parent.customCallback("onWindowDelayedPresentationShown", this, p_id);
		}
	}
	
	function onDelayedLabelShown(p_id)
	{
		if (_parent != null)
		{
			_parent.customCallback("onWindowDelayedLabelShown", this, p_id);
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Inherited Methods
	
	function createTitle(p_locID)
	{
		return createLabel("title", p_locID, ::Vector2(0.0, c_margin + (c_titlebarHeight / 2.0)), ::TextColors.Black,
			_width, 0.05, GlyphSetID_Text, HorizontalAlignment_Left);
	}
	
	function createStoppedLabel(p_id, p_locID, p_position, p_color, p_width, p_height, p_font,
	                            p_horizontalAlignment = HorizontalAlignment_Left,
	                            p_verticalAlignment   = VerticalAlignment_Center,
	                            p_extraTags = [])
	{
		// All labels are left aligned with x position at offset and y position
		p_horizontalAlignment = HorizontalAlignment_Left;
		p_position.x = _xOffset
		p_position.y += ((_height - c_titlebarHeight) / 2.0) - c_margin;
		
		// Valid area
		p_width  = ::min(p_width,  _width  - (c_margin * 2.0));
		p_height = ::min(p_height, _height - (c_margin * 2.0));
		return base.createStoppedLabel(p_id, p_locID, p_position, p_color, p_width, p_height, p_font,
			p_horizontalAlignment, p_verticalAlignment, p_extraTags);
	}
}
