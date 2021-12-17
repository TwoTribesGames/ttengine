include_entity("triggers/Trigger");

class VirusUploadTrigger extends Trigger
</
	editorImage    = "editor.virusuploadtrigger"
	libraryImage   = "editor.library.virusuploadtrigger"
	placeable      = Placeable_Everyone
	collisionRect  = [ 0.0, 0.0, 1.0, 1.0 ]
	sizeShapeColor = Colors.Trigger
	displayName    = "Trigger - Virus Upload"
	group          = "04. Input Triggers"
	stickToEntity  = true
/>
{
	// Redefine here because conditional cannot be evaluated if 'type' attribs are null
	</
		type = "float"
		min  = 1
		max  = 10
		order = 1
		group = "Generic Settings"
	/>
	radius = 1;
	
	</
		type           = "entity"
		filter         = ::getTriggerTypes()
		group          = "Specific Settings"
		order          = 3
		referenceColor = ReferenceColors.Enable
		description    = "Virus will be unloaded when receiving this signal"
	/>
	resetSignal = null;
	
	// Only circular triggers allowed
	type = "circle";
	
	// This trigger should never be used once
	once = false;
	
	// Internals
	_isVirusUploaded = false;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		addProperty("noticeVirusUploader");
		
		setCollisionRect(::Vector2(0, 0), radius * 2, radius * 2);
		setSightDetectionPoints([::Vector2(0, 0)]);
		
		// if the parentTrigger is set don't create touch sensors etc. (those will be handled by the parent)
		if (parentTrigger != null && ("addChildTrigger" in parentTrigger))
		{
			//::assert("addChildTrigger" in parentTrigger, entityIDString(this) + " tries to set non-triggerparent " + entityIDString(parentTrigger) + " as parent");
			parentTrigger.addChildTrigger(this);
		}
		
		if (resetSignal != null)
		{
			resetSignal.addChildTrigger(this);
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onVirusUploaded(p_entity)
	{
		_triggerEnter(p_entity, null);
		_triggerExit(p_entity, null);
		
		_isVirusUploaded = true;
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
		VirusUploader.unregisterVisibleEntity(this);
		
		base.onDie();
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Inherited methods
	
	function _triggerEnter(p_entity, p_parent)
	{
		if (p_parent != null && p_parent.equals(resetSignal))
		{
			_isVirusUploaded = false;
			return;
		}
		
		base._triggerEnter(p_entity, p_parent);
	}
	
	function createTouchSensor()
	{
		// No need for a touch sensor. This trigger is controller by the VirusUploader
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods
	
	function containsVirus()
	{
		return _isVirusUploaded || (isEnabled() == false);
	}
}
VirusUploadTrigger.setattributes("type", null);
VirusUploadTrigger.setattributes("width", null);
VirusUploadTrigger.setattributes("height", null);

VirusUploadTrigger.setattributes("once", null);
VirusUploadTrigger.setattributes("triggerOnTouch", null);
VirusUploadTrigger.setattributes("triggerOnUncull", null);
VirusUploadTrigger.setattributes("parentTrigger", null);
VirusUploadTrigger.setattributes("triggerFilter", null);
VirusUploadTrigger.setattributes("filterAllEntitiesOfSameType", null);
