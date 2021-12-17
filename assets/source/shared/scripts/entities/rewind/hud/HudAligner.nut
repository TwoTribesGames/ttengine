class HudAligner extends EntityBase
</
	placeable      = Placeable_Hidden
	collisionRect  = [ 0.0, 0.0, 6.0, 4.0 ]
/>
{
	// Creation params
	_id = null;
	
	// Internals
	_parentEntities = null;
	_debugNames = null;
	
	// Checks all presentations for valid entities (only works in test builds)
	static c_validateTimeout = 4.0;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		makeScreenSpaceEntity();
		::removeEntityFromWorld(this);
		
		_parentEntities = {};
		
		if (::isTestBuild())
		{
			startCallbackTimer("onValidate", c_validateTimeout);
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods
	
	function createPresentation(p_parent, p_filename, p_layer = ParticleLayer_Hud)
	{
		local pres = createPresentationObjectInLayer(p_filename, p_layer);
		
		// Store weakref to parent to be able to reroute the presentation callbacks
		_parentEntities[pres.getHandleValue()] <- p_parent.weakref();
		if (::isTestBuild())
		{
			if (_debugNames == null) _debugNames = {};
			_debugNames[pres.getHandleValue()] <- p_filename;
		}
		return pres;
	}
	
	function destroyPresentation(p_presentation)
	{
		// Remove parent from table
		delete _parentEntities[p_presentation.getHandleValue()];
		if (::isTestBuild())
		{
			delete _debugNames[p_presentation.getHandleValue()];
		}
		destroyPresentationObject(p_presentation);
	}
	
	function getParent(p_object)
	{
		local id = p_object.getHandleValue();
		if ((id in _parentEntities) == false)
		{
			return null;
		}
		return _parentEntities[id];
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onPresentationObjectCallback(p_object, p_name)
	{
		local parent = getParent(p_object);
		if (parent == null)
		{
			::tt_panic("HudAligner '" + _id + "': Presentation callback '" + p_name +
			         "' fired, but parent entity doesn't exist anymore.");
			destroyPresentation(p_object);
			return;
		}
		
		// Reroute to parent
		parent.customCallback("onPresentationObjectCallback", p_object, p_name);
	}
	
	function onPresentationObjectEnded(p_object, p_name)
	{
		local parent = getParent(p_object);
		if (parent == null)
		{
			::tt_panic("HudAligner '" + _id + "': Presentation ended callback '" + p_name + 
			         "' fired, but parent entity doesn't exist anymore.");
			destroyPresentation(p_object);
			return;
		}
		
		// Reroute to parent
		parent.customCallback("onPresentationObjectEnded", p_object, p_name);
	}
	
	function onPresentationObjectCanceled(p_object, p_name)
	{
		local parent = getParent(p_object);
		if (parent == null)
		{
			::tt_panic("HudAligner '" + _id + "': Presentation canceled callback '" + p_name + 
			         "' fired, but parent entity doesn't exist anymore.");
			destroyPresentation(p_object);
			return;
		}
		
		// Reroute to parent
		parent.customCallback("onPresentationObjectCanceled", p_object, p_name);
	}
	
	function onValidate()
	{
		if (::isTestBuild() == false)
		{
			// Shouldn't happen as onValidate can only be triggered in testbuilds,
			// but better safe than sorry
			return;
		}
		
		foreach (handle, entity in _parentEntities)
		{
			if (entity == null)
			{
				::tt_panic("HudAligner '" + _id + "' contains a presentation '" + _debugNames[handle] + "' with handle '" + handle +
				         "' that points to a non-existing entity. Did entity forget to cleanup its hud elements?");
			}
		}
		
		startCallbackTimer("onValidate", c_validateTimeout);
	}
}
