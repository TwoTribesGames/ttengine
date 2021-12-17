class Hud extends EntityBase
{
	_elements         = null;
	_hudAligners      = null;
	// hardcode 16:9 ratio for now.
	// TODO: make it support aspect ratios dynamically
	_screenWidth      = 1.7777777;
	_isHidden         = false;
	_offset           = null;
	_offsetLocked     = false;
	_visibleLayers    = HudLayer.Permanent;
	
	function onInit()
	{
		_elements = [];
		setPosition(::Vector2(0,0));
		_offset = ::Vector2(0, 0);
		makeScreenSpaceEntity();
		::removeEntityFromWorld(this);
		setCanBePaused(false);
		
		_hudAligners =
		[
			::spawnEntity("HudAligner", ::Vector2(-_screenWidth / 2.0, 0.0), { _id = "left" }),
			::spawnEntity("HudAligner", ::Vector2(-_screenWidth / 8.0, 0.0), { _id = "centerleft" }),
			::spawnEntity("HudAligner", ::Vector2(                0.0, 0.0), { _id = "center" }),
			::spawnEntity("HudAligner", ::Vector2( _screenWidth / 8.0, 0.0), { _id = "centerright" }),
			::spawnEntity("HudAligner", ::Vector2( _screenWidth / 2.0, 0.0), { _id = "right" })
		];
	}
	
	function onSpawn()
	{
		updateLayout();
	}
	
	function onPresentationObjectCallback(p_object, p_name)
	{
		if (p_name == "remove")
		{
			p_object.stop();
			destroyElement(p_object);
		}
	}
	
	function updateVisibility(p_element, p_visibleLayers)
	{
		local visible = p_element.layer == HudLayer.Permanent || ((p_element.layer & p_visibleLayers) != 0);
		p_element.presentation.setVisible(visible);
	}
	
	function updateVisibleLayers(p_visibleLayers)
	{
		local hud = ::Hud.getInstance();
		foreach (elem in hud._elements)
		{
			updateVisibility(elem, p_visibleLayers);
		}
	}
	
	function createElementInternal(p_parent, p_filename, p_hudalignment, p_layer, p_renderLayer)
	{
		if (p_hudalignment < 0 || p_hudalignment >= HudAlignment.Count)
		{
			::tt_panic("Hud alignment '" + p_hudalignment + "' is out of range");
			p_hudalignment = HudAlignment.Center;
		}
		
		local elem =
		{
			layer        = p_layer,
			presentation = _hudAligners[p_hudalignment].createPresentation(p_parent, p_filename, p_renderLayer)
		};
		
		local hud = ::Hud.getInstance();
		updateVisibility(elem, hud._visibleLayers);
		
		_elements.push(elem);
		return elem;
	}
	
	function destroyElementInternal(p_element)
	{
		// Find presentation object in elements
		local removedFromElements = false;
		for (local i = 0; i < _elements.len(); ++i)
		{
			if (_elements[i].presentation.equals(p_element))
			{
				_elements.remove(i);
				removedFromElements = true;
				break;
			}
		}
		
		if (removedFromElements == false)
		{
			::tt_panic("Cannot find element '" + p_element + "' in _elements");
		}
		
		// Find presentation object in aligners
		foreach (hudalignment in _hudAligners)
		{
			if (hudalignment.hasPresentationObject(p_element))
			{
				hudalignment.destroyPresentation(p_element);
				return;
			}
		}
		
		::tt_panic("Cannot find element '" + p_element + "' in _hudAligners");
	}
	
	function createTextElementInternal(p_parent, p_creationParams)
	{
		// FIXME: Get rid of this hackery and replace it with a 1080p rendertarget which
		// gets scaled based on the screenheight. Note: text is not correctly scaled in non 1080p
		// screensizes.
		local screenHeight = getScreenHeight().tofloat();
		
		// Original pixel sizes are based on 1080 screen width
		local pixelWidth  = p_creationParams.width  * 1080;
		local pixelHeight = p_creationParams.height * 1080;
		
		local label = addTextLabel(
			p_creationParams.locID,
			pixelWidth / 64,
			pixelHeight / 64,
			p_creationParams.glyphset
		);
		label.setHorizontalAlignment(p_creationParams.horizontalAlignment);
		label.setVerticalAlignment(p_creationParams.verticalAlignment);
		label.setColor(p_creationParams.color);
		
		// Uncomment this line to easily position/debug HUD space textlabels
		//label.setShowTextBorders(true);
		
		local scale = p_creationParams.scale * (screenHeight / 1080.0);
		local pos   = p_creationParams.position;
		local origin = ::Vector2(0, 0);
		{
			// Correctly offset quad based on text alignment
			if (p_creationParams.horizontalAlignment == HorizontalAlignment_Left)
			{
				origin.x = -scale * (pixelWidth / (screenHeight * 2));
			}
			else if (p_creationParams.horizontalAlignment == HorizontalAlignment_Right)
			{
				origin.x = scale * (pixelWidth / (screenHeight * 2));
			}
			
			if (p_creationParams.verticalAlignment == VerticalAlignment_Top)
			{
				origin.y = scale * (pixelHeight / (screenHeight * 2));
			}
			else if (p_creationParams.verticalAlignment == VerticalAlignment_Bottom)
			{
				origin.y = -scale * (pixelHeight / (screenHeight * 2));
			}
		}
		pos -= origin;
		
		local elem = createElementInternal(p_parent,
		                                   "presentation/" + p_creationParams.presentation,
		                                   p_creationParams.hudalignment, p_creationParams.layer,
		                                   p_creationParams.renderLayer);
		elem.presentation.addTextLabel(label, ::Vector2(0, 0));
		// FIXME: Unfortunately we cannot set a custom origin in presentations, so we have to solve
		// this with custom values
		elem.presentation.addCustomValue("origin_x", origin.x);
		elem.presentation.addCustomValue("origin_y", origin.y);
		elem.presentation.setCustomTranslation(pos);
		elem.presentation.setCustomZOffset(p_creationParams.zOffset);
		elem.presentation.setCustomUniformScale(p_creationParams.scale);
		
		if (p_creationParams.autostart)
		{
			elem.presentation.start("", [], false, 1);
		}
		
		local hud = ::Hud.getInstance();
		updateVisibility(elem, hud._visibleLayers);
		
		return { label = label, presentation = elem.presentation };
	}
	
	function destroyTextElementInternal(p_element)
	{
		p_element.presentation.removeTextLabel(p_element.label);
		destroyElementInternal(p_element.presentation);
		removeTextLabel(p_element.label);
	}
	
	function getInstance()
	{
		if (::isValidEntity(::g_hud) == false)
		{
			::g_hud = ::spawnEntity("Hud", ::Vector2(0, 0));
		}
		return ::g_hud;
	}
	
	function hasInstance()
	{
		return ::g_hud != null;
	}
	
	function update(p_deltaTime)
	{
		updateAspectRatio();
	}
}

function Hud::createElement(p_parent, p_presFile, p_hudAlignment, p_layer = HudLayer.Normal)
{
	return Hud.getInstance().createElementInternal(p_parent, p_presFile, p_hudAlignment, p_layer, ParticleLayer_Hud).presentation;
}

function Hud::createElementInRenderLayer(p_parent, p_presFile, p_hudAlignment, p_renderLayer, p_layer = HudLayer.Normal)
{
	return Hud.getInstance().createElementInternal(p_parent, p_presFile, p_hudAlignment, p_layer, p_renderLayer).presentation;
}

function Hud::destroyElement(p_element)
{
	if (p_element != null)
	{
		Hud.getInstance().destroyElementInternal(p_element);
	}
}

function Hud::createTextElement(p_parent, p_creationParams)
{
	// Add defaults
	local params = ::mergeTables(p_creationParams,
		{
			scale               = 1.0
			horizontalAlignment = HorizontalAlignment_Center,
			verticalAlignment   = VerticalAlignment_Center,
			presentation        = "emptity",
			autostart           = true,
			zOffset             = 0.0,
			layer               = HudLayer.Normal,
			renderLayer         = ParticleLayer_Hud
		}
	);
	return Hud.getInstance().createTextElementInternal(p_parent, params);
}

function Hud::destroyTextElement(p_element)
{
	if (p_element != null)
	{
		Hud.getInstance().destroyTextElementInternal(p_element);
	}
}

function Hud::hide()
{
	local hud = ::Hud.getInstance();
	hud._isHidden = true;
	::Hud.updateLayout();
	local player = ::getFirstEntityByTag("PlayerBot");
	if (player != null)
	{
		player.customCallback("onHUDHide");
	}
}

function Hud::show()
{
	local hud = ::Hud.getInstance();
	hud._isHidden = false;
	::Hud.updateLayout();
	local player = ::getFirstEntityByTag("PlayerBot");
	if (player != null)
	{
		player.customCallback("onHUDShow");
	}
}

function Hud::isVisible()
{
	local hud = ::Hud.getInstance();
	return hud._isHidden == false;
}

function Hud::showLayers(p_layers)
{
	if (p_layers != null)
	{
		local hud = ::Hud.getInstance();
		hud._visibleLayers = p_layers;
		hud._isHidden = false;
		::Hud.updateLayout();
	}
}

function Hud::getLayers()
{
	local hud = ::Hud.getInstance();
	return hud._visibleLayers;
}

function Hud::setOffset(p_offset)
{
	local hud = ::Hud.getInstance();
	if (hud._offsetLocked == false)
	{
		hud._offset = p_offset;
		Hud.updateLayout();
	}
}

function Hud::setOffsetLocked(p_locked)
{
	local hud = ::Hud.getInstance();
	hud._offsetLocked = p_locked;
}

function Hud::updateLayout()
{
	local hud = ::Hud.getInstance();
	
	//local hideOffset = (hud._isHidden ? ::Vector2(1000.0, 1000.0) : Vector2(0.0, 0.0));
	
	hud._hudAligners[HudAlignment.Left]       .setPosition(::Vector2(-hud._screenWidth / 2.0, 0.0) + hud._offset);
	hud._hudAligners[HudAlignment.CenterLeft] .setPosition(::Vector2(-hud._screenWidth / 8.0, 0.0) + hud._offset);
	hud._hudAligners[HudAlignment.Center]     .setPosition(::Vector2(                    0.0, 0.0) + hud._offset);
	hud._hudAligners[HudAlignment.CenterRight].setPosition(::Vector2( hud._screenWidth / 8.0, 0.0) + hud._offset);
	hud._hudAligners[HudAlignment.Right]      .setPosition(::Vector2( hud._screenWidth / 2.0, 0.0) + hud._offset);
	
	updateVisibleLayers(hud._isHidden ? HudLayer.Permanent : hud._visibleLayers);
}

function Hud::updateAspectRatio()
{
	local hud = ::Hud.getInstance();
	local aspect = ::getAspectRatio();
	if (hud._screenWidth != aspect)
	{
		hud._screenWidth = aspect;
		::Hud.updateLayout();
	}
}

::g_hud <- null;
