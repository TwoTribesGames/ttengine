include_entity("rewind/conversation/BaseConversation");

class MissionBriefing extends BaseConversation
</
	placeable      = Placeable_Hidden
	movementset    = "Static"
	collisionRect  = [ 0.0, 0.0, 0.1, 0.1 ]
	group          = "Rewind"
/>
{
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		base.onInit();
	}
	
	function onSpawn()
	{
		base.onSpawn();
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onTimer(p_name)
	{
		base.onTimer(p_name);
		
		if (p_name == "killMe")
		{
			::killEntity(this);
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Inherited methods
	
	function removeMe()
	{
		base.removeMe();
		
		stopAllTimers();
		
		// Since the MissionBriefing doesn't use a background anymore; simply kill this one after some seconds
		startTimer("killMe", 3.0);
	}
	
	function createBackground()
	{
		return null;
	}
	
	function destroyBackground()
	{
	}
	
	function createLabels()
	{
		_labels = [];
		
		local missionID = ::Level.getMissionID();
		
		local header = "";
		local text   = "";
		if (::ProgressMgr.getPlayMode() == PlayMode.Challenge)
		{
			local nextMedal = ::ProgressMgr.getChallengeNextMedal(missionID);
			local medalName = ::ProgressMgr.getChallengeMedalName(nextMedal).toupper();
			header = "CHALLENGE_GOAL_" + medalName;
			text = header + "_" + missionID.toupper();
		}
		else
		{
			text   = "MISSION_BRIEFING_" + missionID.toupper();
			header = text + "_HEADER";
		}
		
		// Header
		{
			local pres = createPresentationObjectInLayer("presentation/briefing_text", ParticleLayer_Hud);
			local label = addTextLabel("", 14, 1.5, GlyphSetID_Text);
			label.setHorizontalAlignment(HorizontalAlignment_Center);
			label.setVerticalAlignment(VerticalAlignment_Center);
			label.setColor(ColorRGBA(255,255,255,255));
			label.addDropShadow(::Vector2(0.002, -0.002), ::ColorRGBA(0,0,0,96));
			pres.addTextLabel(label, ::Vector2(0, 0.0));
			// Always all caps
			label.setTextUTF8(::getLocalizedStringUTF8(header).toupper());
			
			_labels.push({ label = label, presentation = pres });
		}
		
		// Text
		{
			local pres = createPresentationObjectInLayer("presentation/briefing_text", ParticleLayer_Hud);
			local label = addTextLabel("", 20, 1.5, GlyphSetID_Header);
			label.setHorizontalAlignment(HorizontalAlignment_Center);
			label.setVerticalAlignment(VerticalAlignment_Center);
			label.setColor(ColorRGBA(255,255,255,255));
			label.addDropShadow(::Vector2(0.002, -0.002), ::ColorRGBA(0,0,0,96));
			pres.addTextLabel(label, ::Vector2(0, -0.04));
			// Always all caps
			label.setTextUTF8(::getLocalizedStringUTF8(text).toupper());
			
			_labels.push({ label = label, presentation = pres });
		}
	}
	
	function destroyLabels()
	{
		// Cleaned up automatically when entity dies
	}
}
