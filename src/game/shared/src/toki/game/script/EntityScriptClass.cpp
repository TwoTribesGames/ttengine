#include <tt/code/helpers.h>

#include <toki/game/script/EntityBase.h>
#include <toki/game/script/EntityScriptClass.h>
#include <toki/game/script/EntityScriptMgr.h>
#include <toki/script/ScriptMgr.h>
#include <toki/AppGlobal.h>


namespace toki {
namespace game {
namespace script {

toki::script::attributes::ClassAttributes  EntityScriptClass::ms_classAttributesScratch;


/*! \brief Helper function which creates a copy of a squirrel class.
    \note  It expects the source class and base class on the squirrel stack.
    \param p_v Squirrel VM to use.
    \param p_namespaceTable stack index to 'namespace' table to which to add the new class.
    \param p_name name to use for the new class. (It's added with this name to the table p_namespaceTable). */
static void copySQClass(HSQUIRRELVM p_v, SQInteger p_namespaceTable, const std::string& p_name)
{
	tt::script::SqTopRestorerHelper helper(p_v, true, -1); // We consume 2 objects from stack and add 1 new object.
	
	// Create new class with baseClass as base
	TT_ASSERT(sq_gettype(p_v, -1) == OT_CLASS); // We expect the base class at -1.
	sq_newclass(p_v, true);
	const SQInteger newClassIndex = sq_gettop(p_v);
	
	const SQInteger srcClassIndex = sq_gettop(p_v) - 1; // We expect the source class at -2. (Yes, top - 1 is correct to get -2.)
	TT_ASSERT(sq_gettype(p_v, srcClassIndex) == OT_CLASS); 
	
	// Add new state (class) to roottable (inside of the impl namespace table).
	// NOTE: We do this our new class is found by SQCache so it can be serialized.
	//       and it adds a reference to our class so it stays alive after getting popped from stack.
	sq_pushstring(p_v, p_name.c_str(), -1);
	sq_push(p_v, newClassIndex); // Duplicate, so we keep the table on the stack.
	SQRESULT result = sq_newslot(p_v, p_namespaceTable, SQFalse); // Add to table
	TT_ASSERT(SQ_SUCCEEDED(result));
	
	// Iterate over the source class and add its members to the new state
	sq_pushnull(p_v); // iterator
	while (SQ_SUCCEEDED(sq_next_getweakrefs(p_v, srcClassIndex))) // Iterate over raw state's members.
	{
		// Here -1 is the value and -2 is the key
		/*
		std::string keyStr = tt::script::getType(p_v, sq_gettype(p_v, -2), p_name.c_str(), -2);
		TT_Printf("copySQClass - %s", keyStr.c_str());
		// */
		
		// Check if the values is static in the raw state.
		bool isStatic = (sq_isfield(p_v, srcClassIndex) == false);
		
		sq_push(p_v, -2); // duplicate key
		sq_getattributes(p_v, srcClassIndex); // Get attributes from raw state. (pops key)
		// Expects key, value, attributes on stack.
		result = sq_rawnewmember(p_v, newClassIndex, isStatic);
		TT_ASSERT(SQ_SUCCEEDED(result));
		
#if !defined(TT_BUILD_FINAL) // DEBUG TEST CODE -- START
		sq_pop(p_v, 2); // remove attribute and value.
		
		sq_push(p_v, -1); // duplicate key.
		result = sq_get(p_v, newClassIndex); // Test that we can get key from new class.
		TT_ASSERT(SQ_SUCCEEDED(result)); // Failed to get the key/value we just added!
		
		sq_pushnull(p_v); // Add dummy value on stack so cleanup code below keeps working.
#endif // DEBUG TEST CODE -- END
		
		sq_pop(p_v, 3); // remove attribute, value and key.
	}
	
	sq_poptop(p_v); // pop iterator.
	sq_remove(p_v, srcClassIndex); // Remove source class. (Keep new class.)
}




//--------------------------------------------------------------------------------------------------
// Public member functions

EntityScriptClassPtr EntityScriptClass::create(const std::string& p_path, const std::string& p_name)
{
	EntityScriptMgr& mgr = AppGlobal::getEntityScriptMgr();
	tt::script::VirtualMachinePtr vmPtr = mgr.getVM();
	
	HSQUIRRELVM v = vmPtr->getVM();
	
	tt::script::SqTopRestorerHelper helper(v, true);
	
	HSQOBJECT stateTable;
	sq_resetobject(&stateTable);
	
	const std::string stateID(p_name + "_State");
	
	// Save initial roottable
	HSQOBJECT initRootTable;
	sq_resetobject(&initRootTable);
	sq_pushroottable(v);
	const SQInteger roottableIndex = sq_gettop(v);
	SQRESULT result = sq_clone(v, roottableIndex);
	TT_ASSERT(SQ_SUCCEEDED(result));
	sq_getstackobj(v, -1, &initRootTable);
	sq_addref(v, &initRootTable);
	sq_poptop(v);
	
	// First create the state table for this class
	sq_pushstring(v, stateID.c_str(), -1);
	sq_newtable(v);
	sq_getstackobj(v, -1, &stateTable);
	sq_newslot(v, roottableIndex, SQFalse);
	
	const std::string fullPath(p_path + p_name);
	bool loadSuccess = vmPtr->loadAndRunScriptNoRootPath(fullPath);
	
	if (loadSuccess)
	{
		// Find entity in roottable
		sq_pushstring(v, p_name.c_str(), -1);
		
		if (SQ_FAILED(sq_get(v, roottableIndex)))
		{
			TT_PANIC("Entity script '%s.nut' should contain class '%s', but doesn't.",
			          fullPath.c_str(), p_name.c_str());
			loadSuccess = false;
		}
	}
	
	if (loadSuccess == false)
	{
		// Clear old root table.
		sq_pushroottable(v);
		result = sq_clear(v, -1);
		TT_ASSERT(SQ_SUCCEEDED(result));
		
		// Restore initial root table
		sq_pushobject(v, initRootTable);
		sq_pushnull(v);
		while(SQ_SUCCEEDED(sq_next(v, -2)))
		{
			result = sq_newslot(v, -5, SQFalse);
			TT_ASSERT(SQ_SUCCEEDED(result));
		}
		sq_pop(v, 4); // Pop iterator, init roottable, state table and roottable.
		
		sq_release(v, &initRootTable);
		return EntityScriptClassPtr();
	}
	
	StateMap rawStates;
	// get baseclass
	HSQOBJECT baseClass;
	sq_resetobject(&baseClass);
	sq_getstackobj(v, -1, &baseClass);
	result = sq_settypetag(v, -1, SqBind<EntityBase>::get_typetag());
	TT_ASSERT(SQ_SUCCEEDED(result));
	
	EntityScriptClassPtr parent(mgr.getParentClass(baseClass));
	if (parent != 0)
	{
		const StateMap& parentStates = parent->getRawStateMap();
		rawStates.insert(parentStates.begin(), parentStates.end());
	}
	
	// Pop the class
	sq_poptop(v);
	
	// get all states from stateTable
	sq_pushobject(v, stateTable);
	
	sq_pushnull(v); //null iterator
	while(SQ_SUCCEEDED(sq_next(v, -2)))
	{
		//here -1 is the value and -2 is the key
		if (sq_gettype(v,-2) == OT_STRING) 
		{
			const SQChar* str;
			if (SQ_FAILED(sq_getstring(v, -2, &str)))
			{
				TT_PANIC("sq_getstring failed");
				str = "";
			}
			
			HSQOBJECT stackObj;
			sq_resetobject(&stackObj);
			sq_getstackobj(v, -1, &stackObj);
			std::string name(str);
			rawStates[name] = EntityState(name, stackObj);
		}
		else
		{
			TT_PANIC("Entity '%s has invalid State table", p_name.c_str());
			// FIXME: Roottable contains invalid entries at this point
		}
			
		sq_pop(v, 2); //pops key and val before the next iteration
	}
	
	sq_pop(v, 2); //pops the null iterator and the statetable. (leave the roottable.)
	
	// Release (clone of) the initial roottable
	sq_release(v, &initRootTable);
	
	// --------------------------------------------------------------------------------------------
	// Create new (impl) states based on the (raw) states.
	const std::string stateIDImpl(stateID + "_impl");
	
	// Create new state table (namespace) for this new states.
	sq_newtable(v); // Create table
	const SQInteger implNSTable = sq_gettop(v); // impl namescapetable.
	sq_pushstring(v, stateIDImpl.c_str(), -1);
	sq_push(v, implNSTable); // Duplicate, so we keep the table on the stack.
	result = sq_newslot(v, roottableIndex, SQFalse); // Add to roottable
	TT_ASSERT(SQ_SUCCEEDED(result));
	
	StateMap states;
	for (StateMap::iterator it = rawStates.begin(); it != rawStates.end(); ++it)
	{
		tt::script::SqTopRestorerHelper preStateHelper(v, true);
		
		const std::string& stateName = it->first;
		
		{
			// Push all base classes of our the raw state to the stack.
			HSQOBJECT rawSqState = it->second.getSqState();
			sq_pushobject(v, rawSqState);
			const SQInteger rawStateIndex = sq_gettop(v);
			do
			{
				result = sq_getbase(v, -1); // Pushes base class or null;
				TT_ASSERT(SQ_SUCCEEDED(result));
			}
			while (sq_gettype(v, -1) != OT_NULL);
			
			sq_poptop(v); // Remove null.
			sq_pushobject(v, baseClass); // Add base class
			
			// FIXME: Instead of copying the class each time we could check if we already created the base class for a previous state.
			do
			{
				const SQInteger depth = sq_gettop(v) - rawStateIndex - 1;
				copySQClass(v, implNSTable, stateName + ((depth > 0) ? "_" + tt::str::toStr(depth) : ""));
			}
			while (sq_gettop(v) > rawStateIndex);
		}
		
		// Get handle to new class which should now be at the top of the stack.
		HSQOBJECT newSqState;
		sq_resetobject(&newSqState);
		sq_getstackobj(v, -1, &newSqState);
		
		states[stateName] = EntityState(stateName, newSqState);
		
		/*
		TT_Printf("EntityScriptClass::create - Adding state '%s'::'%s' %p\n", 
		          p_name.c_str(), stateName.c_str(), newSqState._unVal.pClass);
		// */
		
		sq_poptop(v); // pop newState.
	}
	
	sq_pop(v, 2); // pop root table and impl (namespace) table
	
	// insert baseclass 'state'
	states[""] = EntityState("", baseClass); // FIXME: Store in explicitly named m_baseState varible instead of hiding it in the data with some magic string.
	
	return EntityScriptClassPtr(new EntityScriptClass(p_name, baseClass, rawStates, states));
}


EntityState EntityScriptClass::getState(const std::string& p_stateName) const
{
	StateMap::const_iterator it = m_stateMap.find(p_stateName);
	if (it != m_stateMap.end())
	{
		return (*it).second;
	}
	
	TT_PANIC("Trying to get invalid state: '%s'! (entityType: '%s')", p_stateName.c_str(), m_name.c_str());
	
	return EntityState(); // FIXME: Maybe return base class here?
}


void EntityScriptClass::registerAttributes(const tt::script::VirtualMachinePtr& p_vm)
{
	TT_NULL_ASSERT(p_vm);
	
	// The called script will fill ms_classAttributesScratch
	tt::code::helpers::freeContainer(ms_classAttributesScratch);
	
	p_vm->callSqFun("__registerAttributes", m_name);
	
	// All attributes for the script class should now be on the scratch
	m_attributes = ms_classAttributesScratch;
}


//--------------------------------------------------------------------------------------------------
// Private member functions

void EntityScriptClass::registerAttribute(const std::string& p_memberName,
                                          const std::string& p_attributeName,
                                          const tt::str::Strings& p_attributeValue,
                                          const std::string& p_attributeType)
{
	using namespace toki::script::attributes;
	Attribute attribute(p_attributeName, p_attributeValue, p_attributeType);
	
	// now find member
	MemberAttributesCollection& members = ms_classAttributesScratch.getMemberAttributesCollection();
	for (MemberAttributesCollection::iterator it = members.begin();
	     it != members.end(); ++it)
	{
		if ((*it).first == p_memberName)
		{
			(*it).second.insert(attribute);
			return;
		}
	}
	
	// not found add new member
	AttributeCollection attributes;
	attributes.insert(attribute);
	members.emplace_back(p_memberName, attributes);
}

// Namespace end
}
}
}
