include_entity("gui/GUIElement");

class GUITable extends GUIElement
</
	placeable      = Placeable_Hidden
	collisionRect  = [ 0.0, 0.0, 6.0, 4.0 ]
/>
{
	static c_presentationFile = "guitable";
	static c_rowHeight        = 0.04;
	static c_margin           = 0.02;
	
	// Creation params
	_width            = null;
	_rowCount         = null;
	_columnData       = null;
	_columnProperties = null;
	_glyphSetID       = GlyphSetID_Text;
	_zOffset          = 0.0;
	_showPanel        = true;
	
	// Internal values
	_columnCount      = null;
	_rowPresentations = null;
	_rows             = null;
	_selectedIndex    = null;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		base.onInit();
		
		_rowPresentations = [];
		_rows = [];
		
		local pos = getPosition();
		local yOffset = pos.y;
		
		if (_showPanel)
		{
			local height = _rowCount * c_rowHeight;
			local panel = createPresentationObjectInLayer("presentation/menu/elements/crtglow", ParticleLayer_Hud);
			panel.addCustomValue("height", height);
			panel.setCustomTranslation(::Vector2(0, yOffset - ((height - c_rowHeight) * 0.5)));
			panel.start("fadein", [], false, 0);
		}
		
		local type = "row_a";
		local isEmptyTable = _columnData == null;
		_columnCount = _columnProperties.len();
		if (_columnProperties.len() != _columnCount)
		{
			::tt_panic("Column count doesn't match widths count");
			return;
		}
		
		// Verify width (should be 100%)
		local width = 0.0;
		for (local i = 0; i < _columnCount; ++i)
		{
			width += _columnProperties[i].width;
		}
		
		if (::fabs(width - 1.0) > 0.001)
		{
			::tt_panic("_columnProperties should have total of 1.0 width, width = " + width);
			return;
		}
		
		// Text will be rendered on this presentation
		_presentation.setCustomZOffset(_zOffset + 0.002);
		
		for (local y = 0; y < _rowCount; ++y)
		{
			local pres = //::Hud.createElement(this, "presentation/menu/elements/guitable", HudAlignment.Center, HudLayer.Menu);
				createPresentationObjectInLayer("presentation/menu/elements/guitable", ParticleLayer_Hud);
			pres.setCustomScale(::Vector2(_width, c_rowHeight));
			pres.setCustomTranslation(::Vector2(pos.x, yOffset));
			pres.setCustomZOffset(_zOffset + 0.001); // just behind the text presentation
			pres.start(type, [], false, 0);
			local labels = [];
			local xOffset = (-_width / 2.0) + pos.x;
			for (local x = 0; x < _columnCount; ++x)
			{
				local cellWidth = _columnProperties[x].width * _width;
				local label = addTextLabel("", (cellWidth - c_margin) / 0.06, c_rowHeight / 0.06, _glyphSetID);
				label.setHorizontalAlignment(_columnProperties[x].alignment);
				label.setVerticalAlignment(VerticalAlignment_Center);
				label.setColor(::TextColors.Light);
				_presentation.addTextLabel(label, ::Vector2(xOffset + cellWidth / 2.0, yOffset));
				labels.push(label);
				xOffset += cellWidth;
			}
			_rows.push(labels);
			_rowPresentations.push(pres);
			yOffset -= c_rowHeight;
			type = type == "row_a" ? "row_b" : "row_a";
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods
	
	function clear()
	{
		setSelectedIndex(null);
		foreach (row in _rows)
		{
			foreach (column in row)
			{
				column.setText("");
			}
		}
	}
	
	function setData(p_data)
	{
		clear();
		local rowCount = ::min(_rowCount, p_data.len());
		for (local y = 0; y < rowCount; ++y)
		{
			for (local x = 0; x < _columnCount; ++x)
			{
				_rows[y][x].setTextUTF8(p_data[y][x].tostring());
			}
		}
	}
	
	function getSelectedIndex()
	{
		return _selectedIndex;
	}
	
	function setSelectedIndex(p_index)
	{
		if (_selectedIndex != null)
		{
			// Unselect first
			_rowPresentations[_selectedIndex].start((_selectedIndex % 2) == 0 ? "row_a" : "row_b", [], false, 0);
		}
		
		_selectedIndex = p_index;
		
		if (_selectedIndex != null)
		{
			_rowPresentations[_selectedIndex].start("row_selected", [], false, 0);
		}
	}
}
