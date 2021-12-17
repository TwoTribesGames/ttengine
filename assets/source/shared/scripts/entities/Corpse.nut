include_entity("rewind/RewindEntity");

class Corpse extends RewindEntity
</
	placeable      = Placeable_Hidden
	movementset    = "Corpse"
/>
{
	positionCulling           = false;
	
	_isPermanent              = false;
	_presentationName         = "corpse";
	_presentation             = null;
	_presentationFile         = null;
	_collisionRect            = null;
	_presentationTags         = null;
	_presentationCustomValues = null;
	_presentationLayer        = null;
	_floorDir                 = Direction_Down;
	_isForwardLeft            = false;
	_rumbleStrength           = null;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		base.onInit();
		
		// if the _presentationTags isn't set, set it to the default (if we'd set it above it would be a static member)
		if (_presentationTags == null)
		{
			_presentationTags = [];
		}
		
		if (_presentationCustomValues == null)
		{
			_presentationCustomValues = [];
		}
		
		::assert(_presentationFile != null, "Trying to spawn a corpse with no presentation file given.");
		
		addInvincibilityProperties();
		addProperty("noticeCrush");
		addProperty("noticeShredders");
		
		// Corpses cannot be pushed
		setCanBePushed(false);
		
		if (_presentationLayer != null)
		{
			// Presentation layer explicitly set; use that
			_presentation = createPresentationObjectInLayer("presentation/" + _presentationFile, _presentationLayer);
		}
		else
		{
			_presentation = createPresentationObject("presentation/" + _presentationFile);
		}
		
		if (_collisionRect != null)
		{
			setCollisionRectWithVectorRect(_collisionRect);
		}
		
		// Make sure corpses cannot be detected
		makeEntityUndetectable(this);
	}
	
	function onSpawn()
	{
		base.onSpawn();
		
		setFloorDirection(_floorDir);
		setForwardAsLeft(_isForwardLeft);
		setCanBePushed(true);
		
		foreach (tag in _presentationTags)
		{
			_presentation.addTag(tag);
		}
		
		foreach (valuePair in _presentationCustomValues)
		{
			_presentation.addCustomValue(valuePair[0], valuePair[1]);
		}
		
		if (_isPermanent == false)
		{
			startCallbackTimer("onTimeout", 10.0);
		}
		_presentation.startEx(_presentationName, [], false, 1, getPresentationCallback("remove"));
		
		if (_rumbleStrength != null)
		{
			::setRumble(_rumbleStrength, ::getRumblePanning(_rumbleStrength, getCenterPosition()));
		}
		
		startMovement(Direction_None, 0);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onTimeout()
	{
		::tt_panic("Corpse anim is still not ended for presentation '" + _presentationFile + "'");
		::killEntity(this);
	}
	
	function onPresentationObjectEnded(p_object, p_name)
	{
		if (_isPermanent)
		{
			_presentationName = "corpse_permanent";
			_presentation.start(_presentationName, [], false, 1);
		}
		else
		{
			::killEntity(this);
		}
	}
	
	function onCrushed(p_crusher)
	{
		// don't call base.onCrushed because that will try to create anohter corpse
		
		// only crush the corpse if it wasn't already crushed
		if (_presentationTags.find("crushed") == null)
		{
			_presentation.addTag("crushed");
			_presentation.startEx(_presentationName, _presentationTags, _removeAfterAnimation, 1, getPresentationCallback("corpse"));
		}
	}
}
