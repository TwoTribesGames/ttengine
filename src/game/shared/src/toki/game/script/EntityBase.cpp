#include <tt/code/bufferutils.h>

#include <toki/game/entity/Entity.h>
#include <toki/game/entity/EntityMgr.h>
#include <toki/game/script/wrappers/EventWrapper.h>
#include <toki/game/script/wrappers/PointerEventWrapper.h>
#include <toki/game/script/wrappers/TileSensorWrapper.h>
#include <toki/game/script/EntityBase.h>
#include <toki/game/script/EntityScriptClass.h>
#include <toki/game/script/sqbind_bindings.h>
#include <toki/game/Game.h>
#include <toki/script/serialization/SQSerializer.h>
#include <toki/script/serialization/SQUnserializer.h>
#include <toki/AppGlobal.h>
#include <toki/constants.h>


#ifdef SQBIND_NAMESPACE
namespace SQBIND_NAMESPACE {
#endif

s32 SqBind<toki::game::script::EntityBase>::typeTagDummy = 0;

#ifdef SQBIND_NAMESPACE
} // SqBind namespace end
#endif

namespace toki {
namespace game {
namespace script {

EntityScriptMgr* EntityBase::ms_mgr = 0;

//--------------------------------------------------------------------------------------------------
// Public member functions

EntityBasePtr EntityBase::create(const entity::EntityHandle& p_handle, const std::string& p_type, s32 p_id)
{
	EntityBasePtr instance(create(p_handle, p_type));
	
	if (instance != 0 && instance->onCreate(p_id))
	{
		return instance;
	}
	
	return EntityBasePtr();
}


EntityBase::~EntityBase()
{
	if (AppGlobal::hasGameAndEntityMgr())
	{
		callSqFun("destructor");
	}
	
	// Unregister all tags for this entity
	EntityScriptMgr& mgr = AppGlobal::getEntityScriptMgr();
	for (tt::str::Strings::const_iterator it = m_tags.begin();
	     it != m_tags.end(); ++it)
	{
		mgr.unregisterEntityByTag((*it), getHandle());
	}
	
	HSQUIRRELVM v = ms_mgr->getVM()->getVM();
	
	// Reset user pointer
	sq_pushobject(v, m_instance);
	sq_setinstanceup(v, -1, reinterpret_cast<SQUserPointer>(&getEmptyWrapper()));
	sq_poptop(v);
	
#if DEBUG_SQUIRREL_ENTITY_BASE_LIFETIME
	{
		SQRESULT result = sq_resurrectunreachable(v);
		TT_ASSERT(SQ_SUCCEEDED(result));
		if (sq_gettype(v, -1) != OT_NULL)
		{
			// Get debug info from script. -- START
			sq_poptop(v); // Remove array because this is now making the unreachables, reachable.
			ms_mgr->getVM()->callSqFun("checkAndPrintUnreachables");
			sq_pushnull(v); // Push something because the code wants to pop the array (but null is also fine.)
			//                             -- END
			
			TT_NONFATAL_PANIC("Found resurrectunreachable squirrel objects BEFORE releasing EntityBase's squirrel instance. Type: '%s'",
			                  getType().c_str());
			sq_collectgarbage(v);
		}
		sq_poptop(v);
	}
	
	TT_Printf("~EntityBase - type: '%s'\n", getType().c_str());
	
#endif
	
	// Release the instance
	if (sq_release(v, &m_instance) == SQFalse)
	{
		// We expect to be the last with a reference to this object.
		TT_NONFATAL_PANIC("Released EntityBase's squirrel instance but there were references left. Type: '%s'",
		                  getType().c_str());
		// If we are not it's not a big issue because we've updated the userpointer with the empty wrapper.
	}
	sq_resetobject(&m_instance);
	
#if DEBUG_SQUIRREL_ENTITY_BASE_LIFETIME
	{
		tt::script::SqTopRestorerHelper cleanup(v, true);
		
		SQRESULT result = sq_resurrectunreachable(v);
		TT_ASSERT(SQ_SUCCEEDED(result));
		if (sq_gettype(v, -1) != OT_NULL)
		{
#if 0 // Going through the array in C++ code. (not as good as the squirrel code)
			TT_ASSERT(sq_gettype(v, -1) == OT_ARRAY);
			
			// Process settings
			sq_pushnull(v); // null iterator
			while(SQ_SUCCEEDED(sq_next(v, -2)))
			{
				// Here -1 is the value and -2 is the key.
				SQInteger i = 0;
				sq_getinteger(v, -2, &i);
				
				SQObjectType sqType = sq_gettype(v, -1);
				std::string typeStr = tt::script::getType(v, sqType);
				
				if (sqType == OT_INSTANCE)
				{
					tt::script::SqTopRestorerHelper helper(v, true);
					
					sq_pushstring(v, "getType", -1);
					if (SQ_SUCCEEDED(sq_get(v, -2))) // get the function
					{
						const SQObjectType typeFound = sq_gettype(v, sq_gettop(v));
						if (typeFound == OT_CLOSURE ||
						    typeFound == OT_NATIVECLOSURE)
						{
							sq_push(v, -2); // push instance as 'this'.
							if (SQ_SUCCEEDED(sq_call(v, 1, SQTrue, SQTrue)))// SQFalse)))
							{
								const SQChar* sqTypeStr = 0;
								if (SQ_SUCCEEDED(sq_getstring(v, -1, &sqTypeStr)))
								{
									typeStr = sqTypeStr;
									typeStr += "\n";
								}
								sq_poptop(v); // pop string
							}
							sq_poptop(v); // pop this
						}
						else
						{
							sq_poptop(v);
						}
					}
				}
				TT_Printf("[%d] - %s", i, typeStr.c_str());
				
				sq_pop(v, 2); //pops key and val before the nex iteration
			}
			sq_poptop(v);
#endif
			// Get debug info from script. -- START
			sq_poptop(v); // Remove array because this is now making the unreachables, reachable.
			ms_mgr->getVM()->callSqFun("checkAndPrintUnreachables");
			sq_pushnull(v); // Push something because the code wants to pop the array (but null is also fine.)
			//                             -- END
			
			TT_NONFATAL_PANIC("Found resurrectunreachable squirrel objects after releasing EntityBase's squirrel instance. Type: '%s'",
			                  getType().c_str());
			sq_collectgarbage(v);
		}
		sq_poptop(v);
	}
#endif
}


void EntityBase::init(game::entity::EntityProperties& p_properties, bool p_gameReloaded)
{
	TT_ASSERT(sq_isnull(m_instance) == false);
	TT_ASSERT(m_currentState.isValid());
	TT_ASSERT(m_targetState.isValid() == false);
	
	using namespace level::entity;
	const entity::EntityLibrary& library    = AppGlobal::getEntityLibrary();
	const EntityInfo*            entityInfo = library.getEntityInfo(getType());
	
	if (entityInfo == 0)
	{
		TT_PANIC("Unsupported entity type: '%s' (no type information is available for it).", getType().c_str());
		return;
	}
	
	HSQUIRRELVM v = ms_mgr->getVM()->getVM();
	
	entity::EntityMgr& entityMgr = AppGlobal::getGame()->getEntityMgr();
	
	EntityInstance::Properties validProperties;
	EntityInstance::Properties invalidProperties;
	
	for (EntityInstance::Properties::const_iterator it = p_properties.begin();
	     it != p_properties.end(); ++it)
	{
		const EntityProperty& targetProperty = entityInfo->getProperty((*it).first);
		if (targetProperty.getType() == EntityProperty::Type_None || targetProperty.validate((*it).second) == false)
		{
			invalidProperties.insert(*it);
		}
		else
		{
			validProperties.insert(*it);
		}
	}
	
	if (invalidProperties.empty() == false)
	{
		updateInvalidLevelProperties(v, invalidProperties);
	}
	
	// invalidProperties should now be corrected, merge with validProperties
	validProperties.insert(invalidProperties.begin(), invalidProperties.end());
	p_properties = validProperties;
	
	// Override defaults based on instance specific properties
	sq_pushobject(v, m_instance);
	for (EntityInstance::Properties::const_iterator it = p_properties.begin();
	     it != p_properties.end(); ++it)
	{
		// Skip MISSION_ID property as it is only used in the editor
		if (it->first == "MISSION_ID")
		{
			continue;
		}
		
		const EntityProperty& targetProperty = entityInfo->getProperty((*it).first);
		if (targetProperty.getType() == EntityProperty::Type_None)
		{
			TT_NONFATAL_PANIC("An entity (ID: %d) of type '%s' contains an unsupported property: '%s' (value '%s').\n"
			                  "This property is not handled by the onInvalidProperties() callback.\nSkipping property\n",
			                  entityMgr.getEntityIDByHandle(getHandle()),
			                  getType().c_str(), (*it).first.c_str(), (*it).second.c_str());
			continue;
		}
		
		// Validate if this property is valid
		if (targetProperty.validate((*it).second) == false)
		{
			TT_NONFATAL_PANIC("An entity (ID: %d) of type '%s' contains property '%s' with invalid value '%s'.\n"
			                  "This property is not handled by the onInvalidProperties() callback.\nSkipping property\n",
			                  entityMgr.getEntityIDByHandle(getHandle()),
			                  getType().c_str(), (*it).first.c_str(), (*it).second.c_str());
			continue;
		}
		
		// get this member from the class
		sq_pushstring(v, (*it).first.c_str(), -1);
		
		switch (targetProperty.getType())
		{
		case EntityProperty::Type_Integer:         sq_pushinteger(v, tt::str::parseS32((*it).second,   0)); break;
		case EntityProperty::Type_Float:           sq_pushfloat  (v, tt::str::parseReal((*it).second,  0)); break;
		case EntityProperty::Type_Bool:            sq_pushbool   (v, tt::str::parseBool((*it).second,  0)); break;
		case EntityProperty::Type_String:          sq_pushstring (v, (*it).second.c_str(), -1);             break;
		case EntityProperty::Type_EntityID:        sq_pushinteger(v, tt::str::parseS32((*it).second,   0)); break;
		case EntityProperty::Type_DelayedEntityID: sq_pushinteger(v, tt::str::parseS32((*it).second,   0)); break;
		case EntityProperty::Type_Entity:
			{
				s32 id = tt::str::parseS32((*it).second, 0);
				const entity::EntityHandle handle = entityMgr.getEntityHandleByID(id);
				entity::Entity* targetEntity = entityMgr.getEntity(handle);
				if (targetEntity != 0)
				{
					EntityBase* base = targetEntity->getEntityScript().get();
					SqBind<EntityBase>::push(v, base);
					
					// Make weakref
					sq_weakref(v, -1);
					sq_remove(v, -2);
				}
				else
				{
#if !defined(TT_BUILD_FINAL)
					if (p_gameReloaded == false)
					{
						// When reloading the level, some properties can still contain references 
						// to just deleted entities, so don't panic.
						// However when the game is not reloaded, this should not ever happen.
						const entity::Entity* sourceEntity = entityMgr.getEntity(getHandle());
						TT_PANIC("%s Member '%s' points to an invalid entity: ID %s",
						         sourceEntity->getDebugInfo().c_str(),
						         (*it).first.c_str(),
						         (*it).second.c_str());
					}
#endif
					// Entity doesn't exist; so simply pretend this property is never set at all
					sq_poptop(v);
					continue;
				}
			}
			break;
			
		case EntityProperty::Type_ColorRGB:
			{
				const tt::str::Strings rgbComponents(tt::str::explode((*it).second, ","));
				if (rgbComponents.size() == 3)
				{
					const u8 r = tt::str::parseU8(rgbComponents[0], 0);
					const u8 g = tt::str::parseU8(rgbComponents[1], 0);
					const u8 b = tt::str::parseU8(rgbComponents[2], 0);
					tt::engine::renderer::ColorRGB color(r, g, b);
					SqBind<tt::engine::renderer::ColorRGB>::push(v, color);
				}
			}
			break;
			
		case EntityProperty::Type_ColorRGBA:
			{
				const tt::str::Strings rgbaComponents(tt::str::explode((*it).second, ","));
				// Allow implicit upgrades from color_rgb to color_rgba (if script type was changed)
				if (rgbaComponents.size() == 3 || rgbaComponents.size() == 4)
				{
					const u8 r = tt::str::parseU8(rgbaComponents[0], 0);
					const u8 g = tt::str::parseU8(rgbaComponents[1], 0);
					const u8 b = tt::str::parseU8(rgbaComponents[2], 0);
					const u8 a = (rgbaComponents.size() == 4) ? tt::str::parseU8(rgbaComponents[3], 0) : 255;
					tt::engine::renderer::ColorRGBA color(r, g, b, a);
					SqBind<tt::engine::renderer::ColorRGBA>::push(v, color);
				}
			}
			break;
			
		case EntityProperty::Type_IntegerArray:
		case EntityProperty::Type_FloatArray:
		case EntityProperty::Type_BoolArray:
		case EntityProperty::Type_StringArray:
		case EntityProperty::Type_EntityArray:
		case EntityProperty::Type_EntityIDArray:
		case EntityProperty::Type_DelayedEntityIDArray:
			{
				tt::str::Strings elements = tt::str::explode((*it).second, ",");
				sq_newarray(v, 0);
				
				for (tt::str::Strings::const_iterator elemIt = elements.begin();
				     elemIt != elements.end(); ++elemIt)
				{
					switch (targetProperty.getType())
					{
					case EntityProperty::Type_EntityIDArray:
					case EntityProperty::Type_DelayedEntityIDArray:
					case EntityProperty::Type_IntegerArray:
						sq_pushinteger(v, tt::str::parseS32((*elemIt), 0));
						break;
						
					case EntityProperty::Type_FloatArray:
						sq_pushfloat(v, tt::str::parseReal((*elemIt), 0));
						break;
						
					case EntityProperty::Type_BoolArray:
						sq_pushbool(v, tt::str::parseBool((*elemIt), 0)); 
						break;
						
					case EntityProperty::Type_StringArray:
						sq_pushstring(v, (*elemIt).c_str(), -1);
						break;
						
					case EntityProperty::Type_EntityArray:
						{
							// FIXME: Code duplication of the Type_Entity handling above!
							const s32 id = tt::str::parseS32(*elemIt, 0);
							const entity::EntityHandle handle = entityMgr.getEntityHandleByID(id);
							entity::Entity* targetEntity = entityMgr.getEntity(handle);
							if (targetEntity != 0)
							{
								EntityBase* base = targetEntity->getEntityScript().get();
								SqBind<EntityBase>::push(v, base);
								
								// Make weakref
								sq_weakref(v, -1);
								sq_remove(v, -2);
							}
							else
							{
#if !defined(TT_BUILD_FINAL)
								if (p_gameReloaded == false)
								{
									// When reloading the level, some properties can still contain references 
									// to just deleted entities, so don't panic.
									// However when the game is not reloaded, this should not ever happen.
									const entity::Entity* sourceEntity = entityMgr.getEntity(getHandle());
									TT_PANIC("%s Member '%s' points to an invalid entity: ID %s",
									         sourceEntity->getDebugInfo().c_str(),
									         (*it).first.c_str(), m_class->getName().c_str(),
									         (*elemIt).c_str(), handle.getValue());
								}
#endif
								
								// Non existing entity in array, simply don't do anything and continue
								continue;
							}
						}
						break;
						
					default:
						TT_PANIC("Unhandled array type '%s'",
						         EntityProperty::getTypeName(targetProperty.getType()));
						continue;
					}
					
					const SQRESULT result = sq_arrayappend(v, -2); 
					TT_ASSERT(SQ_SUCCEEDED(result));
				}
			}
			break;
			
		default:
			TT_NONFATAL_PANIC("Unhandled type '%s' for member '%s' in class '%s'", 
			                  EntityProperty::getTypeName(targetProperty.getType()),
			                  (*it).first.c_str(), m_class->getName().c_str());
			
			sq_poptop(v); // pop the member name (invalid)
			continue;
		}
		
		// set member in the instance
		sq_set(v, -3);
	}
	
	// Pop m_instance from stack
	sq_pop(v, 1);
	
	// Notify the script if the game is reloaded
	if (p_gameReloaded)
	{
		onReloadGame();
	}
}


void EntityBase::init(const HSQOBJECT& p_properties)
{
	TT_ASSERT(sq_isnull(m_instance) == false);
	TT_ASSERT(m_currentState.isValid());
	TT_ASSERT(m_targetState.isValid() == false);
	
	HSQUIRRELVM v = ms_mgr->getVM()->getVM();
	
	if (sq_isnull(p_properties) == false)
	{
		sq_pushobject(v, p_properties);
		TT_ASSERTMSG(sq_gettype(v, -1) == OT_TABLE, "Properties should be of type Table");
		
		sq_pushroottable(v);
		sq_pushobject(v, m_instance);
		
		sq_pushnull(v);
		// now iterate over the property table (should be at -4 now)
		while(SQ_SUCCEEDED(sq_next(v, -4)))
		{
			// now set this key/value in the instance (which is now at -4)
			if (SQ_FAILED(sq_set(v, -4)))
			{
				const SQChar* strPtr = 0;
				if (SQ_FAILED(sq_getstring(v, -2, &strPtr)))
				{
					strPtr = "";
				}
				TT_PANIC("init() cannot set value for property '%s'. "
					"Verify that the property exists in the base class '%s'",
					strPtr, m_class->getName().c_str());
				sq_pop(v, 5); // pop key value and error stuff
			}
		}
		sq_pop(v, 2);
	}
}


void EntityBase::deinit()
{
	TT_ASSERT(sq_isnull(m_instance) == false);
	TT_ASSERT(m_currentState.isValid());
}


bool EntityBase::onCreate(s32 p_id)
{
	// FIXME: Add support for calls that return a value in VirtualMachineMethods
	HSQUIRRELVM v = ms_mgr->getVM()->getVM();
	tt::script::SqTopRestorerHelper helper(v);
	sq_pushobject(v, m_class->getBaseClass()); // Push class here.
	sq_pushstring(v, "onCreate", -1);
	
	if (SQ_FAILED(sq_get(v, -2))) //get the function from the class
	{
		// If method doesn't exist in class, always return true
		return true;
	}
	
	const SQObjectType objType = sq_gettype(v, sq_gettop(v));
	if (objType != OT_CLOSURE &&
	    objType != OT_NATIVECLOSURE)
	{
		TT_PANIC("onCreate is not a (Native)Closure!");
		return true;
	}
	
	sq_pushobject(v, m_instance); // Push this instance
	sq_pushinteger(v, p_id);      // Push ID of this entity
	if (SQ_FAILED(sq_call(v, 2, SQTrue, TT_SCRIPT_RAISE_ERROR)))
	{
		TT_PANIC("Calling squirrel function 'onCreate' failed! Check if number of arguments match!");
		return true;
	}
	
	// If return value is false, the entity should not be created
	SQBool shouldCreate = true;
	if (SQ_SUCCEEDED(sq_getbool(v, -1, &shouldCreate)))
	{
		return shouldCreate == SQTrue;
	}
	
	return true;
}


void EntityBase::onDie()
{
	queueSqFun("onDie");
	
	// Process all pending callbacks (max update depth is 3) before calling onDie in script
	const s32 maxCallbackUpdates = 3;
	updateCallbacks(maxCallbackUpdates);
}


void EntityBase::onSound(const toki::game::event::Event& p_event) const
{
	queueSqFun("onSound", wrappers::EventWrapper(p_event));
}


void EntityBase::onVibration(const toki::game::event::Event& p_event) const
{
	queueSqFun("onVibration", wrappers::EventWrapper(p_event));
}


void EntityBase::onEventSpawned(const toki::game::event::Event& p_event, const std::string& p_userParam,
                                const EntityBaseCollection& p_entitiesFound)
{
	queueSqFun("onEventSpawned", wrappers::EventWrapper(p_event), p_userParam, p_entitiesFound);
}


void EntityBase::onCarryBegin(const entity::EntityHandle& p_carryingEntity)
{
	const EntityBase* carryingEntity = getEntityBase(p_carryingEntity);
	if (carryingEntity != 0)
	{
		queueSqFun("onCarryBegin", carryingEntity);
	}
}


void EntityBase::onCarryEnd()
{
	queueSqFun("onCarryEnd");
}


void EntityBase::setState(const std::string& p_newState)
{
	if (p_newState == m_currentState.getName()) // Same state
	{
		return;
	}
	
	// Check for called from onExitState
	const bool hadTargetState = m_targetState.isValid();
	
	m_targetState = m_class->getState(p_newState);
	TT_ASSERT(m_targetState.isValid());
	
	if (hadTargetState)
	{
		return;
	}
	
	if (m_currentState.isValid())
	{
		onExitState();
	}
	
	// update dirty sensors before state switching
	entity::Entity* entity = getHandle().getPtr();
	if (entity != 0)
	{
		entity->updateDirtySensors(AppGlobal::getGame()->getGameTimeInSeconds());
	}
	
	// Process all pending callbacks (max update depth is 3) before switching state
	const s32 maxCallbackUpdates = 3;
	updateCallbacks(maxCallbackUpdates);
	
	// Store name of previous state
	m_previousStateName = m_currentState.getName();
	
	m_currentState = m_targetState;
	m_targetState.reset();
	
	onEnterState();
	updateCallbacks(); // Flush the onEnterState callback.
}


const std::string& EntityBase::getType()  const
{
	TT_NULL_ASSERT(m_class);
	return m_class->getName();
}


void EntityBase::setScriptMgr(EntityScriptMgr* p_mgr)
{
	TT_ASSERT(ms_mgr == 0);
	TT_NULL_ASSERT(p_mgr);
	if (ms_mgr == 0 && p_mgr != 0)
	{
		ms_mgr = p_mgr;
	}
}


void EntityBase::resetScriptMgr()
{
	TT_ASSERT(ms_mgr != 0);
	ms_mgr = 0;
}


void EntityBase::addTag(const std::string& p_tag)
{
	tt::str::Strings::const_iterator it = std::find(m_tags.begin(), m_tags.end(), p_tag);
	if (it != m_tags.end())
	{
		return;
	}
	
	m_tags.push_back(p_tag);
	EntityScriptMgr& mgr = AppGlobal::getEntityScriptMgr();
	mgr.registerEntityByTag(p_tag, getHandle());
}


void EntityBase::removeTag(const std::string& p_tag)
{
	tt::str::Strings::iterator it = std::find(m_tags.begin(), m_tags.end(), p_tag);
	if (it == m_tags.end())
	{
		return;
	}
	
	m_tags.erase(it);
	EntityScriptMgr& mgr = AppGlobal::getEntityScriptMgr();
	mgr.unregisterEntityByTag(p_tag, getHandle());
}


void EntityBase::removeCallbacks()
{
	tt::code::helpers::freeContainer(m_callbacks);
}


void EntityBase::addObjectToSQSerializer(toki::script::serialization::SQSerializer& p_serializer) const
{
	p_serializer.addObject(ms_mgr->getVM()->getVM(), m_instance);
	
	for (Callbacks::const_iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it)
	{
		(*it)->addObjectToSQSerializer(p_serializer);
	}
}


void EntityBase::serialize(const toki::script::serialization::SQSerializer& p_serializer,
                           tt::code::BufferWriteContext* p_context) const
{
	(void)p_serializer;
	TT_NULL_ASSERT(p_context);
	TT_NULL_ASSERT(getHandle().getPtr());
	
	namespace bu = tt::code::bufferutils;
	
	bu::putHandle(getHandle(), p_context);
	bu::put(m_class->getName(), p_context);
	
	TT_ASSERTMSG(m_currentState.isValid(),
	             "Trying the serialize an EntityBase with an invalid currentState! "
	             "(Current state name: '%s'. entity class name: '%s'.)",
	             m_currentState.getName().c_str(), m_class->getName().c_str());
	
	// Current state
	bu::put(m_currentState.getName(), p_context);
	
	// Previous state
	bu::put(m_previousStateName, p_context);
	
	// Tags
	bu::put(static_cast<u32>(m_tags.size()), p_context);
	for (tt::str::Strings::const_iterator it = m_tags.begin(); it != m_tags.end(); ++it)
	{
		bu::put((*it), p_context);
	}
	
	// Callback is done later. (in: serializeSQSerializerObjects)
}


void EntityBase::serializeSQSerializerObjects(const toki::script::serialization::SQSerializer& p_serializer,
                                                tt::code::BufferWriteContext* p_context) const
{
	TT_NULL_ASSERT(p_context);
	
	namespace bu = tt::code::bufferutils;
	
	const u32 callbackCount = static_cast<u32>(m_callbacks.size());
	bu::put(callbackCount, p_context);
	
	// Callbacks
	for (Callbacks::const_iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it)
	{
		(*it)->serialize(p_serializer, p_context);
	}
}


EntityBasePtr EntityBase::unserialize(const toki::script::serialization::SQUnserializer& p_unserializer,
                                      tt::code::BufferReadContext* p_context)
{
	(void)p_unserializer;
	TT_NULL_ASSERT(p_context);
	
	namespace bu = tt::code::bufferutils;
	
	entity::EntityHandle handle = bu::getHandle<entity::Entity>(p_context);
	const std::string typeName  = bu::get<std::string>(         p_context);
	
	TT_ASSERT(handle.isEmpty() == false);
	TT_NULL_ASSERT(handle.getPtr());
	
	EntityBasePtr result(create(handle, typeName));
	
	TT_NULL_ASSERT(result);
	
	{
		const std::string state = bu::get<std::string>(p_context);
		result->m_currentState = result->getClass()->getState(state);
	}
	
	{
		const std::string state = bu::get<std::string>(p_context);
		result->m_previousStateName = state;
	}
	
	// Tags
	{
		u32 tagCount = bu::get<u32>(p_context);
		for (u32 i = 0; i < tagCount; ++i)
		{
			const std::string tagName(bu::get<std::string>(p_context));
			result->m_tags.push_back(tagName);
		}
	}
	
	handle.getPtr()->setEntityScriptFromSerialize(result);
	
	return result;
}


void EntityBase::unserializeSQSerializerObjects(const toki::script::serialization::SQUnserializer& p_unserializer,
                                                  tt::code::BufferReadContext* p_context)
{
	TT_NULL_ASSERT(p_context);
	
	namespace bu = tt::code::bufferutils;
	
	// Callbacks
	{
		const u32 callbackCount = bu::get<u32>(p_context);
		
		for (u32 i = 0; i < callbackCount; ++i)
		{
			CallbackPtr callback(Callback::unserialize(p_unserializer, p_context));
			m_callbacks.push_back(callback);
		}
	}
}


const EntityBase* EntityBase::getEntityBase(const entity::EntityHandle& p_handle)
{
	const entity::Entity* entity = p_handle.getPtr();
	return (entity != 0 && entity->isInitialized()) ? entity->getEntityScript().get() : 0;
}


//--------------------------------------------------------------------------------------------------
// Private member functions

EntityBasePtr EntityBase::create(const entity::EntityHandle& p_handle, const std::string& p_type)
{
	EntityScriptClassPtr entityScript(ms_mgr->getClass(p_type));
	
	if (entityScript == 0)
	{
		TT_NONFATAL_PANIC("Failed to create entity because (script) class for type '%s' was not found.",
		                  p_type.c_str());
		
		return EntityBasePtr();
	}
	
	EntityBasePtr instance(new EntityBase(p_handle, p_type, entityScript));
	instance->m_this = instance;
	
	return instance;
}


EntityBase::EntityBase(const entity::EntityHandle& p_handle, const std::string& p_type,
                       const EntityScriptClassPtr& p_class)
:
m_class(p_class),
m_currentState(m_class->getBaseState()),
m_targetState(),
m_previousStateName(),
m_wrapper(p_handle),
m_tags(),
m_callbacks(),
m_instance(),
m_this()
{
	TT_NULL_ASSERT(m_class);
	TT_ASSERT(m_currentState.isValid());
	TT_ASSERT(m_targetState.isValid() == false);
	
	TT_ASSERT(m_class->getName() == p_type);
	TT_ASSERT(getType()          == p_type);
	TT_ASSERT(getHandle()        == p_handle);
	
	sq_resetobject(&m_instance);
	
	HSQUIRRELVM v = ms_mgr->getVM()->getVM();
	
	sq_pushroottable(v);
	sq_pushobject(v, p_class->getBaseClass());
	
	// Create the Squirrel instance
	sq_createinstance(v, -1);
	sq_getstackobj(v, -1, &m_instance);
	sq_addref(v, &m_instance);
	sq_setinstanceup(v, -1, reinterpret_cast<SQUserPointer>(&m_wrapper));
	sq_pop(v, 3);
}


void EntityBase::updateCallbacks(s32 p_maxUpdates)
{
	TT_ASSERT(p_maxUpdates > 0);
	
	for (s32 i = 0; i < p_maxUpdates; ++i)
	{
		Callbacks copy;
		std::swap(copy, m_callbacks);
		
		m_callbacks.clear();
		
		for (Callbacks::const_iterator it = copy.begin(); it != copy.end(); ++it)
		{
			{
				// Don't use entityPreExecute after execute because it could have been destroyed! (Note: Get it again through the handle)
				entity::Entity* entityPreExecute = getHandle().getPtr();
				TT_NULL_ASSERT(entityPreExecute);
				if(entityPreExecute != 0 &&
				   entityPreExecute->isInitialized())
				{
					(*it)->execute(m_instance);
				}
			}
			
			// Get entity again because it could have been destroyed by the execute done above.
			entity::Entity* entity = getHandle().getPtr();
			
			if (entity == 0 ||
			    entity->isInitialized() == false)
			{
				// Stop callbacks.
				m_callbacks.clear();
				break;
			}
		}
		
		if (m_callbacks.empty())
		{
			break;
		}
	}
	
	TT_ASSERTMSG(p_maxUpdates == 1 || m_callbacks.empty(),
		"Possible infinite callback loop detected!\n"
		"updateCallbacks exceeded p_maxUpdates ('%d'). There are '%d' new callbacks left after updating.",
		p_maxUpdates, m_callbacks.size());
}


void EntityBase::updateInvalidLevelProperties(HSQUIRRELVM p_vm,
                                              level::entity::EntityInstance::Properties& p_properties) const
{
	tt::script::SqTopRestorerHelper helper(p_vm);
	
	sq_pushobject(p_vm, m_instance);
	sq_pushstring(p_vm, "onInvalidProperties", -1);
	sq_get(p_vm, -2);
	if (sq_gettype(p_vm, -1) != OT_CLOSURE)
	{
		// No need to fire a panic here as that will be done in the calling code (EntityBase::init)
		return;
	}
	
	// Calling instance
	sq_pushobject(p_vm, m_instance);
	// First argument
	sq_newtable(p_vm);
	
	// Push all invalid properties in table
	for (level::entity::EntityInstance::Properties::const_iterator it = p_properties.begin();
	     it != p_properties.end(); ++it)
	{
		sq_pushstring(p_vm, (*it).first.c_str(), -1);
		sq_pushstring(p_vm, (*it).second.c_str(), -1);
		if (SQ_FAILED(sq_newslot(p_vm, -3, SQFalse)))
		{
			TT_PANIC("Entity '%s': Error creating invalid properties for onInvalidProperties. "
			         "Cannot create slot for key/value '%s' / '%s'.",
			         getType().c_str(), (*it).first.c_str(), (*it).second.c_str());
			return;
		}
	}
	
	if (SQ_FAILED(sq_call(p_vm, 2, SQTrue, TT_SCRIPT_RAISE_ERROR)))
	{
		TT_PANIC("Entity '%s': Error calling onInvalidProperties().", getType().c_str());
		return;
	}
	
	if (sq_gettype(p_vm, -1) != OT_TABLE)
	{
		TT_PANIC("Entity '%s': onInvalidProperties() callback doesn't return a table containing "
		         "the corrected properties", getType().c_str());
		return;
	}
	
	// Fill p_properties with corrected values
	p_properties.clear();
	sq_pushnull(p_vm); //null iterator
	while(SQ_SUCCEEDED(sq_next(p_vm, -2)))
	{
		const SQChar* strPtr = 0;
		if (SQ_FAILED(sq_getstring(p_vm, -2, &strPtr)))
		{
			TT_PANIC("onInvalidProperties() should return a table with keys of type string");
			return;
		}
		const std::string key(strPtr);
		
		if (SQ_FAILED(sq_getstring(p_vm, -1, &strPtr)))
		{
			TT_PANIC("onInvalidProperties() should return a table with values of type string");
			return;
		}
		const std::string value(strPtr);
		
		p_properties.insert(std::make_pair(key, value));
		
		// Next element
		sq_pop(p_vm, 2);
	}
	sq_pop(p_vm, 4);
}

// Namespace end
}
}
}
