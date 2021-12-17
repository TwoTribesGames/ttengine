include_entity("rewind/EntityChild");

class VirusUploadSensor extends EntityChild
</
	placeable      = Placeable_Hidden
	movementset    = "Static"
	collisionRect  = [ 0.0, 0.0, 0.1, 0.1 ]
	group          = "Rewind"
/>
{
	// Creation members
	_mimickedType = null;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		base.onInit();
		
		addProperty("noticeVirusUploader");
		
		setSightDetectionPoints([::Vector2(0, 0)]);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onVirusUploaded(p_entity)
	{
		spawnParticleOneShot("particles/virusuploader_success", ::Vector2(0, 0), true, 0, false, ParticleLayer_UseLayerFromParticleEffect, 1.0);
		_parent.onVirusUploaded(p_entity);
	}
	
	function onVirusRemoved()
	{
		_parent.onVirusRemoved();
	}
	
	function onScreenEnter()
	{
		VirusUploader.registerVisibleEntity(this);
	}
	
	function onScreenExit()
	{
		VirusUploader.unregisterVisibleEntity(this);
	}
	
	function onDie()
	{
		base.onDie();
		
		VirusUploader.unregisterVisibleEntity(this);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Deferred methods
	
	function containsVirus()
	{
		return _parent.containsVirus();
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Inherited methods
	
	function getType()
	{
		// Mimicked type (this is required for the VirusUploader, but is otherwise dangerous)
		return _mimickedType;
	}
}
