include_entity("rewind/RewindEntity");

::g_toasterInstance <- null;

class AchievementToaster extends RewindEntity
</
	placeable      = Placeable_Hidden
	collisionRect  = [ 0.0, 0.5, 1.0, 1.0 ]  // center X, center Y, width, height
/>
{
	_name = null;
	_presentation = null;
	_textPresentation = null;
	
	_queue = [];
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		base.onInit();
		
		if (::g_toasterInstance != null)
		{
			_queue.push(_name);
			::killEntity(this);
			return;
		}
		::g_toasterInstance = this.weakref();
		
		setCanBePaused(false);
		::removeEntityFromWorld(this);
		makeScreenSpaceEntity();
		
		_presentation = ::Hud.createElement(this, "presentation/hud_achievement_unlocked", HudAlignment.Right, HudLayer.All);
		local id = ::achievementTable[_name].id;
		_presentation.addCustomValue("id", id);
		_presentation.startEx("", [], false, 0, getPresentationCallback("remove"));
		
		local headerLabel = addTextLabel("ACHIEVEMENT_UNLOCKED", 11.0, 0.6, GlyphSetID_Text);
		headerLabel.setHorizontalAlignment(HorizontalAlignment_Left);
		headerLabel.setVerticalAlignment(VerticalAlignment_Center);
		headerLabel.setColor(::TextColors.Black);
		
		local textLabel = addTextLabel("", 6.8, 2.2, GlyphSetID_Text);
		textLabel.setHorizontalAlignment(HorizontalAlignment_Left);
		textLabel.setVerticalAlignment(VerticalAlignment_Center);
		textLabel.setColor(::TextColors.Light);
		local locID = ::achievementTable[_name].locid + "_NAME";
		textLabel.setTextUTF8(::getLocalizedAchievementStringUTF8(locID));
		
		_textPresentation = ::Hud.createElement(this, "presentation/hud_achievement_unlocked_text", HudAlignment.Right, HudLayer.All);
		_textPresentation.addTextLabel(headerLabel, ::Vector2(0.038, 0.091));
		_textPresentation.addTextLabel(textLabel, ::Vector2(0.07, 0.0));
		_textPresentation.start("", [], false, 0);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onPresentationObjectEnded(p_object, p_name)
	{
		::g_toasterInstance = null;
		if (_queue.len() > 0)
		{
			local name = _queue.pop();
			::spawnEntity("AchievementToaster", ::Vector2(0, 0), { _name = name });
		}
		::killEntity(this);
	}
	
	function onDie()
	{
		::Hud.destroyElement(_presentation);
		::Hud.destroyElement(_textPresentation);
	}
}
