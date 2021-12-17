/*
*/

function getFirstEntityByTag(p_tag)
{
	local entities = ::getEntitiesByTag(p_tag);
	
	if (entities.len() > 0)
	{
		return entities[0];
	}
	
	return null;
}

function isEntityAlive(p_entity)
{
	return p_entity != null && p_entity.isValid() && p_entity.isInitialized() &&
	       p_entity.isDying() == false;
}

function isValidEntity(p_entity)
{
	return p_entity != null && p_entity.isValid();
}

function isValidAndInitializedEntity(p_entity)
{
	return ::isValidEntity(p_entity) && p_entity.isInitialized();
}

function EntityBase::_tostring()
{
	return ::entityString(this);
}

function EntityBase::getBoundingRadius()
{
	local rect = getCollisionRect();
	return ::max(rect.getWidth(), rect.getHeight()) / 2.0;
}

function EntityBase::initStickToEntity()
{
	if (::isValidEntity(stickToEntity))
	{
		stickToEntity.customCallback("onStickToEntityReceived", this);
		if (stickToEntityWithOffset)
		{
			// check if need to make a clone of stickToEntity
			stickToEntityOffset = getCenterPosition() - stickToEntity.getCenterPosition();
		}
	}
}

function EntityBase::updateStickToEntity()
{
	if (stickToEntity != null)
	{
		stickToEntity = updateStickToPosition(stickToEntity, stickToEntityOffset);
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////
// Properties

EntityBase.stickToEntity <- null;
EntityBase._properties <- null;

// adds (or overwrites) a property
function EntityBase::addProperty(p_property, p_value = true)
{
	if (_properties == null)
	{
		_properties = {};
	}
	
	_properties[p_property] <- p_value;
}

function EntityBase::removeProperty(p_property)
{
	if (_properties == null)
	{
		return;
	}
	
	if (hasProperty(p_property))
	{
		delete _properties[p_property];
	}
	
	if (_properties.len() == 0)
	{
		// Remove table, so that it doesn't appear in serialization
		_properties = null;
	}
}

function EntityBase::hasProperty(p_prop)
{
	return p_prop in _properties;
}

// returns the value for the property by reference (so it can be easily updated)
// or null if the property is not defined
function EntityBase::getProperty(p_prop)
{
	if (hasProperty(p_prop))
	{
		return _properties[p_prop];
	}
	return null;
}

/** /
function EntityBase::dumpAllProperties()
{
	echo("*** Dumping Properties for '" + entityString(this) + "' ***");
	foreach (key, prop in _properties)
	{
		echo(key + " -> " + prop);
	}
	echo("*** End Dumping Properties");
}

EntityBase._label <- null
::origSpawn <- ::spawnEntity;
function ::spawnEntity(p_type, p_pos, p_props)
{
	local spawn = ::origSpawn(p_type, p_pos, p_props);
	
	spawn._label = spawn.addTextLabel("", 8, 1, GlyphSetID_Text);
	spawn._label.setText(spawn.getType() + " " + spawn.getHandleValue());
	spawn._label.setColor(ColorRGBA(255, 255, 255, 255));
	return spawn;
}
/**/


////////////////////////////////////////////////////////////////////////////////////////////////
// functions to alter detection

// removes an entity from as much world related systems as possible (use this for waypoints, sound sources, etc.)
function removeEntityFromWorld(p_entity)
{
	p_entity.disableTileRegistration();
	p_entity.setUpdateSurvey(false);
	
	makeEntityUndetectable(p_entity);
	
	p_entity.customCallback("onRemovedFromWorld");
}

// makes an entity undetectable for sensors
function makeEntityUndetectable(p_entity)
{
	p_entity.removeAllSightDetectionPoints();
	p_entity.removeAllLightDetectionPoints();
	p_entity.removeTouchShape();
	
	p_entity.setDetectableByTouch(false);
	p_entity.setDetectableBySight(false);
	p_entity.setDetectableByLight(false);
}

// Zero Gravity helpers

function registerZeroGravityAffectedEntity(p_entity)
{
	registerEntityByTag("noticeZeroGravity");
	addProperty("noticeZeroGravity");
	addProperty("zeroGravityRefCount", 0);
	
	if (ZeroGravityTrigger.hasActiveTrigger())
	{
		handleZeroGravityEnter(p_entity, ZeroGravityTrigger.getActiveTrigger());
	}
}

function unregisterZeroGravityAffectedEntity(p_entity)
{
	unregisterEntityByTag("noticeZeroGravity");
	removeProperty("noticeZeroGravity");
	removeProperty("zeroGravityRefCount");
}

function handleZeroGravityEnter(p_entity, p_source)
{
	local refCount = p_entity.getProperty("zeroGravityRefCount");
	if (refCount == null)
	{
		// unregisterZeroGravityAffectedEntity might have been called just before this
		return null;
	}
	
	++refCount;
	p_entity.addProperty("zeroGravityRefCount", refCount);
	p_entity.customCallback("onZeroGravityEnter", p_source);
	return refCount;
}

function handleZeroGravityExit(p_entity, p_source)
{
	local refCount = p_entity.getProperty("zeroGravityRefCount");
	if (refCount == null)
	{
		// unregisterZeroGravityAffectedEntity might have been called just before this
		return null;
	}
	
	--refCount;
	p_entity.addProperty("zeroGravityRefCount", refCount);
	p_entity.customCallback("onZeroGravityExit", p_source);
	return refCount;
}

function isInZeroGravity(p_entity)
{
	return p_entity.getProperty("zeroGravityRefCount") > 0;
}

// use this for enties that are in the world but not interact with normal creatures on the 0 layer (e.g. for the checkpoint, warppowersource)
// note that this does not disable tile registration because that would not make it carryable, register stomps etc.
function makeBackgroundEntity(p_entity)
{
	makeEntityUndetectable(p_entity);
	p_entity.customCallback("onMadeBackgroundEntity");
}

function makeEntityClassStickToEntity(p_class, p_shouldAddStickToEntity)
{
	try
	{
		if (p_shouldAddStickToEntity)
		{
			p_class.newmember("stickToEntityOffset", null);
			p_class.newmember("stickToEntityWithOffset", false);
			p_class.setattributes("stickToEntity",           {type = "entity", referenceColor = ReferenceColors.Stick, order = 1000, group = "StickTo" });
			p_class.setattributes("stickToEntityWithOffset", {type = "bool",   order = 1001, group = "StickTo" });
		}
	}
	catch(e)
	{
		// Cannot add members to built-in C++ classes
	}
}
