class PickupCollectEffect extends EntityBase
</
	placeable      = Placeable_Hidden
	movementset    = "Static"
	collisionRect  = [0.0, 0.0, 0.1, 0.1] // center X, center Y, width, height
/>
{
	_presentation   = null;
	_targetPosition = null;
	_type           = null;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		makeScreenSpaceEntity();
		::removeEntityFromWorld(this);
		registerEntityByTag("PickupCollectEffect");
		
		local pos    = getCenterPosition();
		local diff   = ::Vector2((_targetPosition.x * ::getAspectRatio()) - pos.x, _targetPosition.y - pos.y);
		
		_presentation = createPresentationObjectInLayer("presentation/pickup_collecteffect_" + _type , ParticleLayer_Hud);
		_presentation.addCustomValue("diff_x", diff.x);
		_presentation.addCustomValue("diff_y", diff.y);
		_presentation.setCustomRotation(-::getAngleFromVector(diff));
		_presentation.start("idle", [], false, 0);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onProgressRestored(p_id)
	{
		::killEntity(this);
	}
	
	function onPresentationObjectCallback(p_object, p_name)
	{
		if (p_name == "kill_parent")
		{
			::killEntity(this);
		}
	}
}
