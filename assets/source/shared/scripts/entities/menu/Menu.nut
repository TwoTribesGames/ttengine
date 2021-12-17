include_entity("menu/MenuScreen");

::g_menuInstance <- null;

class Menu extends MenuScreen
</
	placeable      = Placeable_Hidden
	collisionRect  = [ 0.0, 0.5, 1.0, 1.0 ]  // center X, center Y, width, height
/>
{
	static c_colorGradingTexture = null; // Required
	static c_musicTrack          = null; // Optional
	static c_hasCRTbackground    = true;
	
	_visibleHudLayers           = HudLayer.Permanent;
	_colorGradingID             = null;
	_colorGradingEffectsEnabled = false;
	_playingMusicTrack          = null;
	_wasPointerAllowed          = false;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		// Only one instance can exist
		if (::g_menuInstance != null)
		{
			close();
		}
		
		_wasPointerAllowed = ::isPointerAllowed();
		::setPointerAllowed(true);
		
		::g_menuInstance = this.weakref();
		
		_colorGradingID = ::ColorGrading.add(getColorGradingTexture(), 0.0, ColorGradingPriority.Highest);
		_colorGradingEffectsEnabled = ::areColorGradingEffectsEnabled();
		::setColorGradingEffectsEnabled(false);
		::setColorGradingAfterHud(true);
		_visibleHudLayers = ::Hud.getLayers();
		::Game.pause();
		::Hud.setOffset(::Vector2(0, 0));
		::Hud.setOffsetLocked(true);
		
		base.onInit();
	}
	
	function onSpawn()
	{
		base.onSpawn();
		
		::Hud.showLayers(HudLayer.Menu);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onDie()
	{
		base.onDie();
		
		if (_playingMusicTrack != null)
		{
			::destroyMusicTrack(_playingMusicTrack);
		}
		
		::setColorGradingAfterHud(false);
		::ColorGrading.remove(_colorGradingID);
		::setColorGradingEffectsEnabled(_colorGradingEffectsEnabled);
		
		::Game.unpause();
		::setPointerAllowed(_wasPointerAllowed);
		
		::Hud.showLayers(_visibleHudLayers);
		::g_menuInstance = null;
		::Hud.setOffsetLocked(false);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods
	
	function getColorGradingTexture()
	{
		return c_colorGradingTexture;
	}
	
	function create()
	{
		base.create();
		
		if (c_hasCRTbackground)
		{
			createPresentation("grid", "hud_grid");
			createPresentation("scanlines", "hud_scanlines");
			createPresentation("reflection_light", "hud_reflection_light");
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Helpers Methods
	
	function createMusic()
	{
		if (c_musicTrack != null)
		{
			_playingMusicTrack = ::createMusicTrack(c_musicTrack);
			_playingMusicTrack.play();
		}
	}
}
