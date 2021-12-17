include_entity("menu/MenuScreen");

::g_popupMenuInstance <- null;

enum PopupMenuExitState
{
	Cancel,
	Accept
};

class PopupMenu extends MenuScreen
</
	placeable      = Placeable_Hidden
	collisionRect  = [ 0.0, 0.0, 1.0, 1.0 ]  // center X, center Y, width, height
/>
{
	static c_buttonScale = 0.9;
	
	_titleID           = "POPUPMENU_HEADER";
	_subTextID         = null;
	_subTextPosition   = null;
	_caller            = null;
	_fadeAndCloseAll   = false;
	
	// Internal values
	_exitState         = PopupMenuExitState.Cancel;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		// Only one instance can exist
		if (::g_popupMenuInstance != null)
		{
			::killEntity(::g_popupMenuInstance);
		}
		::g_popupMenuInstance = this.weakref();
		
		::Audio.playGlobalSoundEffect("Effects", "menu_next");
		::setRumble(RumbleStrength_Low, 0.0);
		
		base.onInit();
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onFadeEnded(p_fade, p_animation)
	{
		closeAll();
	}
	
	function onAcceptSelected()
	{
		_exitState = PopupMenuExitState.Accept;
		if (_fadeAndCloseAll)
		{
			createPresentation("static_end", "hud_static");
			startTimer("fade", 0.25);
		}
		else
		{
			close();
		}
	}
	
	function onTimer(p_name)
	{
		if (p_name == "fade")
		{
			// Leaving state; kill all soundeffects
			::Audio.stopCategory("Music");
			::Audio.stopCategory("Effects");
			::Audio.stopCategory("Ambient");
			::Audio.stopCategory("VoiceOver");
			::MusicSource.stopAll();
			
			::createPersistentFade(this, "transparent_to_opaque");
			return;
		}
		
		base.onTimer(p_name);
	}
	
	function onDie()
	{
		base.onDie();
		
		if (::isValidEntity(_caller))
		{
			_caller.customCallback(_exitState == PopupMenuExitState.Cancel ?
				"onPopupMenuCanceled" : "onPopupMenuAccepted", this);
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods
	
	function create()
	{
		createTitle(_titleID);
		if (_subTextID != null && _subTextPosition != null)
		{
			createLabel("subtext", _subTextID, _subTextPosition, ::TextColors.Light, 1.2, 0.1, GlyphSetID_Text);
		}
		createPanelButtons(::Vector2(0.0, 0.04), ::MainMenu.c_buttonsSpacing, 1,
		[
			["btn_accept", "GUIButton", ["POPUPMENU_ACCEPT"], onAcceptSelected],
			["btn_cancel", "GUIButton", ["POPUPMENU_CANCEL"], close]
		]);
	}
}
