enum TriggerState
{
	Idle,
	Activated
};

class Trigger extends EntityBase
</
	editorImage    = "editor.trigger"
	libraryImage   = "editor.library.trigger"
	placeable      = Placeable_Everyone
	collisionRect  = [ 0.0, 1.0, 2.0, 2.0 ]
	sizeShapeColor = Colors.Trigger
	group          = "04. Input Triggers"
	stickToEntity  = true
/>
{
	</
		type = "string"
		choice = ["circle", "rectangle"]
		group = "Generic Settings"
		order = -100
	/>
	type = "rectangle";
	
	</
		type = "integer"
		min  = 0
		max  = 300
		order = -99
		group = "Generic Settings"
		conditional = "type == rectangle"
		description = "Trigger width, use 0 to make it as wide as the level"
	/>
	width = 2;
	
	</
		type = "integer"
		min  = 0
		max  = 250
		order = -98
		group = "Generic Settings"
		conditional = "type == rectangle"
		description = "Trigger height, use 0 to make it as tall as the level"
	/>
	height = 2;
	
	</
		type = "integer"
		min  = 1
		max  = 100 
		order = -97
		group = "Generic Settings"
		conditional = "type == circle"
	/>
	radius = 4;
	
	</
		type        = "bool"
		order       = -96
		group = "Generic Settings"
		description = "When set to false the trigger needs to contain the entire entity before it triggers"
		conditional = "type == rectangle"
	/>
	triggerOnTouch = true;
	
	</
		type   = "bool"
		order  = -95
		group = "Generic Settings"
		description = "Only fires this trigger when it is being unculled. Note: touch will NOT fire this trigger anymore."
	/>
	triggerOnUncull = false;
	
	</
		type        = "bool"
		order       = -94
		group       = "Generic Settings"
		description = "Only use this trigger once, disable it after the first exit"
	/>
	once = false;
	
	</
		type  = "bool"
		order = -93
		group = "Generic Settings"
	/>
	enabled = true;
	_enabledSet = false;
	
	</
		type           = "entity"
		filter         = ::getTriggerTypes()
		order          = -92
		group          = "Generic Settings"
		referenceColor = ReferenceColors.Parent
	/>
	parentTrigger = null;
	
	</
		type           = "entityid_array"
		filter         = ["BaseEnemy", "PlayerBot", "Elevator", "Pickup"]
		order          = 101
		group          = "Filter"
		referenceColor = ReferenceColors.List
		description    = "If empty it filters the PlayerBot"
	/>
	triggerFilter = null;
	
	</
		type           = "bool"
		order          = 102
		group          = "Filter"
		description    = "When set to true the triggers is triggered by all entities of the same type, instead of by a single instance."
	/>
	filterAllEntitiesOfSameType = false;
	
	_triggerSensor  = null;
	_touchers      = null;
	_childTriggers = null;
	
	_disableTileRegistration = true;
	_triggerState = TriggerState.Idle;
	_triggeredCount = 0;
	
	function onInvalidProperties(p_properties)
	{
		removeNonexistentProperties(this, p_properties);
		
		return p_properties;
	}
	
	function onCreate(p_id)
	{
		_childTriggers = ::EntityCollection();
		return true;
	}
	
	function onValidateScriptState()
	{
		if (parentTrigger != null && ("addChildTrigger" in parentTrigger) == false)
		{
			editorWarning("Selected parent trigger entity can't function as a trigger parent!");
		}
	}
	
	function onInit()
	{
		if (_disableTileRegistration)
		{
			::removeEntityFromWorld(this);
		}
		else
		{
			makeEntityUndetectable(this);
		}
		
		setCanBePushed(false);
		setUpdateSurvey(false);
		
		if (filterAllEntitiesOfSameType && triggerFilter == null)
		{
			editorWarning("Cannot filter of same type if triggerFilter is empty");
			filterAllEntitiesOfSameType = false;
		}
		
		if (filterAllEntitiesOfSameType)
		{
			local filter = [];
			foreach (id in triggerFilter)
			{
				local c = ::getEntityClassByID(id);
				if (c != null)
				{
					if (filter.find(c) == null)
					{
						filter.push(c);
					}
					else
					{
						editorWarning("Entity of type '" + entity.getType() + "' is already part of triggerFilter, remove one.");
					}
				}
			}
			triggerFilter = filter;
		}
		else if (triggerFilter == null)
		{
			local player = ::getFirstEntityByTag("PlayerBot");
			if (player != null)
			{
				triggerFilter = [player.getID()];
			}
		}
		
		local pos    = getPosition();
		local offset = getCenterOffset();
		
		if (width  == 0) 
		{
			width  = Level.getWidth() + ::fabs((2 * pos.x) - Level.getWidth());
		}
		if (height == 0)
		{
			height = Level.getHeight() + ::fabs((2 * pos.y) - Level.getHeight());
		}
		
		// if the parentTrigger is set don't create touch sensors etc. (those will be handled by the parent)
		if (parentTrigger != null && ("addChildTrigger" in parentTrigger))
		{
			//::assert("addChildTrigger" in parentTrigger, entityIDString(this) + " tries to set non-triggerparent " + entityIDString(parentTrigger) + " as parent");
			parentTrigger.addChildTrigger(this);
		}
		
		// Only enable position culling if this trigger triggers onUncull
		setPositionCullingEnabled(triggerOnUncull);
	}
	
	function onSpawn()
	{
		initStickToEntity();
		
		if (parentTrigger == null)
		{
			// Only create touch sensor if this trigger doesn't fire on uncull
			if (triggerOnUncull == false)
			{
				createTouchSensor();
				
				// Optimization; in case of a single filtered entity, set touchsensor target
				if (filterAllEntitiesOfSameType == false && _triggerSensor != null &&
				    triggerFilter != null && triggerFilter.len() == 1)
				{
					// Check if entity even exists (perhaps it has not been spawned yet?)
					local target = ::getEntityByID(triggerFilter[0]);
					_triggerSensor.setTarget(target);
				}
			}
			_touchers = ::EntityCollection();
		}
		
		setEnabled(enabled);
	}
	
	function onDie()
	{
		if (_triggerSensor != null)
		{
			_triggerSensor.setEnabled(false);
		}
		
		foreach (trigger in _childTriggers.getAllEntities())
		{
			::killEntity(trigger);
		}
	}
	
	function getTouchSensorShape()
	{
		local shape = null;
		
		switch (type)
		{
		case "circle":
			shape = ::CircleShape(0, radius);
			break;
			
		case "rectangle":
			shape = ::BoxShape(width, height);
			if (triggerOnTouch == false)
			{
				shape.doContains();
			}
			break;
			
		default:
			::tt_panic("Unhandled fieldType '" + fieldType + "'");
			break;
		}
		
		return shape;
	}
	
	function createTouchSensor()
	{
		local shape = getTouchSensorShape();
		
		_triggerSensor = addTouchSensor(shape);
		_triggerSensor.setDefaultEnterAndExitCallback();
		_triggerSensor.setFilterCallback("onTriggerTouchFilter");
	}
	
	function onTriggerTouchFilter(p_entity)
	{
		return isEntityEligible(p_entity);
	}
	
	function getTouchers()
	{
		//::echo("getTouchers", _touchers, "parent:", parentTrigger, (parentTrigger != null) ? parentTrigger.getType() : "", this.getID(), this.getPosition());
		if (parentTrigger != null && ("getTouchers" in parentTrigger))
		{
			return parentTrigger.getTouchers();
		}
		
		return _touchers;
	}
	
	function addChildTrigger(p_trigger)
	{
		_childTriggers.addEntity(p_trigger);
	}
	
	function removeChildTrigger(p_trigger)
	{
		_childTriggers.removeEntity(p_trigger);
	}
	
	function setHeight(p_height)
	{
		height = p_height;
		_triggerSensor.setShape(::BoxShape(width, height));
	}
	
	function setWidth(p_width)
	{
		width = p_width;
		_triggerSensor.setShape(::BoxShape(width, height));
	}
	
	function isEntityEligible(p_entity)
	{
		if (parentTrigger != null && ("isEntityEligible" in parentTrigger))
		{
			return parentTrigger.isEntityEligible(p_entity);
		}
		
		if (::isValidEntity(p_entity) == false)
		{
			return false;
		}
		
		if (filterAllEntitiesOfSameType)
		{
			foreach (c in triggerFilter)
			{
				if (c == p_entity.getclass())
				{
					return true;
				}
			}
		}
		else
		{
			local entityID = p_entity.getID();
			foreach (id in triggerFilter)
			{
				if (entityID == id)
				{
					return true;
				}
			}
		}
		
		return false;
	}
	
	// "private" functions to facilitate calling methods
	function _triggerEnterFirst(p_entity, p_parent)
	{
		onTriggerEnterFirst(p_entity);
	}
	
	function _triggerEnter(p_entity, p_parent)
	{
		if (_triggerState == TriggerState.Activated) return; // Already entered
		if (isEnabled() == false) return;
		
		_triggerState = TriggerState.Activated;
		++_triggeredCount;
		
		getTouchers().addEntity(p_entity);
		
		if (getTouchers().len() == 1)
		{
			_triggerEnterFirst(p_entity, this);
		}
		
		foreach(trigger in _childTriggers.getAllEntities())
		{
			trigger._triggerEnter(p_entity, this);
		}
		onTriggerEnter(p_entity);
	}
	
	function _triggerExitLast(p_entity, p_parent)
	{
		onTriggerExitLast(p_entity);
	}
	
	function _triggerExit(p_entity, p_parent)
	{
		if (_triggerState != TriggerState.Activated) return; // Only register exit when start is Activated
		if (isEnabled() == false) return;
		
		_triggerState = TriggerState.Idle;
		
		getTouchers().removeEntity(p_entity);
		if (getTouchers().len() == 0)
		{
			_triggerExitLast(p_entity, this);
		}
		
		foreach(trigger in _childTriggers.getAllEntities())
		{
			trigger._triggerExit(p_entity, this);
		}
		onTriggerExit(p_entity);
		
		if (once)
		{
			setEnabled(false);
		}
	}
	// end of "private" functions
	
	function onTouchEnter(p_entity, p_sensor)
	{
		_triggerEnter(p_entity, this);
	}
	
	function onTouchExit(p_entity, p_sensor)
	{
		_triggerExit(p_entity, this);
	}
	
	function onCulled()
	{
		if (triggerOnUncull)
		{
			// FIXME: Remove hardcoded playerbot dependency
			local player = ::getFirstEntityByTag("PlayerBot");
			if (player != null)
			{
				onTouchExit(player, null);
			}
		}
	}
	
	function onUnculled()
	{
		if (triggerOnUncull)
		{
			// FIXME: Remove hardcoded playerbot dependency
			local player = ::getFirstEntityByTag("PlayerBot");
			if (player != null)
			{
				onTouchEnter(player, null);
			}
		}
	}
	
	function setEnabled(p_enabled)
	{
		if (_enabledSet && p_enabled == enabled)
		{
			return;
		}
		
		_enabledSet = true;
		
		if (_triggeredCount > 0 && once && p_enabled)
		{
			// Make sure a 'once' trigger cannot be re-enabled after it has been triggered
			return;
		}
		
		if (_triggerSensor != null && _triggerSensor.isEnabled() != p_enabled)
		{
			if (p_enabled)
			{
				_triggerSensor.removeAllSensedEntities();
			}
			_triggerSensor.setEnabled(p_enabled);
		}
		
		// Check if parent trigger
		if (parentTrigger != null && p_enabled != enabled)
		{
			// Make sure the trigger is enabled so onTouchEnter/Exit will be properly handled
			enabled = true
			
			// Check trigger status. In case _triggerState doesn't exist it is
			// assumed the parent is always activated
			local parentActivated = (("_triggerState" in parentTrigger) == false) ||
			                        parentTrigger._triggerState != TriggerState.Idle;
			
			if (parentActivated)
			{
				// No touchers, simulate touch
				if (p_enabled)
				{
					onTouchEnter(parentTrigger, null);
				}
				else
				{
					onTouchExit(parentTrigger, null);
				}
			}
		}
		
		// do this last so _triggerExit and _triggerExitLast are able to do the last callback
		enabled = p_enabled;
		
		// Finally fire the callbacks so derived triggers can handle
		if (enabled)
		{
			customCallback("onEnabled");
		}
		else
		{
			customCallback("onDisabled");
		}
	}
	
	function isEnabled()
	{
		return enabled;
	}
	
	// override these
	function onTriggerEnter(p_entity)
	{
	}
	
	function onTriggerExit(p_entity)
	{
	}
	
	function onTriggerEnterFirst(p_entity)
	{
	}
	
	function onTriggerExitLast(p_entity)
	{
	}
	
	function onEnabled()
	{
	}
	
	function onDisabled()
	{
	}
	
	function _typeof()
	{
		return "Trigger";
	}
	
	function update(p_deltaTime)
	{
		updateStickToEntity();
	}
	
	// makes a class act as a trigger parent
	function makeTriggerParentable(p_class)
	{
		p_class._childTriggers <- null;
		p_class._triggerTouchers <- null;
		
		p_class.addChildTrigger <- function (p_trigger)
			{
				if (_childTriggers == null) _childTriggers = ::EntityCollection();
				_childTriggers.addEntity(p_trigger);
			};
			
		p_class.triggerChildTriggers <- function (p_entity)
			{
				// create the trigger touchers the first time the child triggers are triggered
				if (_triggerTouchers == null)
				{
					_triggerTouchers = ::EntityCollection();
					_triggerTouchers.addEntity(p_entity);
				}
				
				if (_childTriggers == null) return;
				
				foreach (trigger in _childTriggers.getAllEntities())
				{
					trigger._triggerEnter(p_entity, this);
					//trigger._triggerEnterFirst(p_entity);
				}
			};
		
		p_class.triggerEnterChildTriggers <- function (p_entity)
		{
			triggerChildTriggers(p_entity);
		}
		
		p_class.triggerExitChildTriggers <- function (p_entity)
			{
				// create the trigger touchers the first time the child triggers are triggered
				if (_triggerTouchers == null)
				{
					_triggerTouchers = ::EntityCollection();
					_triggerTouchers.addEntity(p_entity);
				}
				
				if (_childTriggers == null) return;
				
				foreach (trigger in _childTriggers.getAllEntities())
				{
					trigger._triggerExit(p_entity, this);
				}
			};
			
		if (("getTouchers" in p_class) == false)
		{
			p_class.getTouchers <- function ()
				{
					return _triggerTouchers;
				}
		}
	}
	
	function makeTriggerTarget(p_class)
	{
		if (("_triggerEnter" in p_class) == false)
		{
			p_class.newmember("_triggerEnter", function(p_entity, p_parent) {});
		}
		
		if (("_triggerEnterFirst" in p_class) == false)
		{
			p_class.newmember("_triggerEnterFirst", function(p_entity, p_parent) {});
		}
		
		if (("_triggerExit" in p_class) == false)
		{
			p_class.newmember("_triggerExit", function(p_entity, p_parent) {});
		}
		
		if (("_triggerExitLast" in p_class) == false)
		{
			p_class.newmember("_triggerExitLast", function(p_entity, p_parent) {});
		}
	}
}
