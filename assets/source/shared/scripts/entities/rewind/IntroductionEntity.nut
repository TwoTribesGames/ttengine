include_entity("triggers/Trigger");
include_entity("rewind/RewindEntity");

class IntroductionEntity extends RewindEntity
</
	editorImage    = "editor.hamster"
	placeable      = Placeable_Hidden
	movementset    = "Static"
	collisionRect  = [ 0.0, 0.0, 1.5, 1.5 ]
/>
{
	positionCulling = false;
	
	// Constants
	static c_displayContinueTimeout = 14.5;
	static c_initialSkipTimeout     = 15.0;
	static c_normalSkipTimeout      = 1.0;
	
	_canBeSkipped = false;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		base.onInit();
		
		::ProgressMgr.setPlayMode(PlayMode.Invalid);
	}
	
	function onSpawn()
	{
		base.onSpawn();
		
		::Camera.setPrimaryFollowEntity(this);
		::Camera.setScrollingEnabled(false);
		::addButtonInputListeningEntity(this, InputPriority.Normal);
		
		// trigger possible child triggers first, so the level exits can override the "defaults"
		triggerChildTriggers(this);
		
		startTimer("skipAllowed", ::ProgressMgr.isIntroductionDisplayed() ?
			c_normalSkipTimeout : c_initialSkipTimeout);
		startCallbackTimer("displayContinue", c_displayContinueTimeout);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onTimer(p_name)
	{
		if (p_name == "skipAllowed")
		{
			_canBeSkipped = true;
		}
	}
	
	function onButtonRespawnPressed()
	{
		if (_canBeSkipped)
		{
			if (::notifyGame("onGamePastIntro"))
			{
				::ProgressMgr.setIsIntroductionDisplayed(true);
				::Level.fadeOutAndLoad("menu");
				::removeButtonInputListeningEntity(this);
			}
			::notifyGame("onGamePastIntro");
		}
	}
	
	function displayContinue()
	{
		_canBeSkipped = true;
		::Hud.createTextElement(this,
		{
			locID                = "INTRODUCTION_BUTTONPROMPT"
			presentation         = "hud_introduction"
			width                = 1.5,
			height               = 0.05,
			glyphset             = GlyphSetID_Text,
			hudalignment         = HudAlignment.Center,
			position             = ::Vector2(0.0, 0.0),
			color                = ColorRGBA(255,255,255,255),
			horizontalAlignment  = HorizontalAlignment_Center,
			verticalAlignment    = VerticalAlignment_Center,
			autostart            = true,
			layer                = HudLayer.Normal
		});
	}
}
IntroductionEntity.setattributes("positionCulling", null);
Trigger.makeTriggerParentable(IntroductionEntity);
