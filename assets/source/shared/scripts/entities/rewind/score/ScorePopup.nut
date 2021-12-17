class ScorePopup extends EntityBase
</
	placeable      = Placeable_Hidden
	movementset    = "Static"
	collisionRect  = [ 0.0, 0.0, 0.1, 0.1 ]
	group          = "Rewind"
/>
{
	_color = null;
	_score = null;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		::removeEntityFromWorld(this);
		
		local label = addTextLabel("", 3.0, 0.5, GlyphSetID_Text);
		label.setText("+" + _score);
		label.setColor(_color);
		label.setHorizontalAlignment(HorizontalAlignment_Center);
		label.setVerticalAlignment(VerticalAlignment_Center);
		
		local presentation = createPresentationObjectInLayer("presentation/hud_score_popup", ParticleLayer_BehindHud);
		presentation.addTextLabel(label, ::Vector2(0.0, 0.0));
		presentation.setAffectedByFloorDirection(false);
		presentation.setAffectedByOrientation(false)
		presentation.start("", [], false, 1);
		
		// Not too fond of kill_parent in presentation, so start a timer here manually
		// to ensure object is removed
		startTimer("killme", 1.2);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onTimer(p_name)
	{
		::killEntity(this);
	}
}
