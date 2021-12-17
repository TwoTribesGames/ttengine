#include <squirrel/sqbind.h>

#include <tt/code/helpers.h>
#include <tt/code/bufferutils.h>
#include <tt/fs/utils/utils.h>
#include <tt/fs/Dir.h>
#include <tt/fs/DirEntry.h>

#include <toki/game/entity/Entity.h>
#include <toki/game/entity/EntityMgr.h>
#include <toki/game/entity/PresStartSettings.h>
#include <toki/game/entity/types.h>
#include <toki/game/fluid/types.h>
#include <toki/game/script/wrappers/EventWrapper.h>
#include <toki/game/script/wrappers/PointerEventWrapper.h>
#include <toki/game/script/EntityScriptClass.h>
#include <toki/game/script/EntityScriptMgr.h>
#include <toki/game/script/sqbind_bindings.h>
#include <toki/game/script/TimerMgr.h>
#include <toki/game/Game.h>
#include <toki/game/types.h>
#include <toki/script/ScriptMgr.h>
#include <toki/AppGlobal.h>


namespace toki {
namespace game {
namespace script {


//--------------------------------------------------------------------------------------------------
// Helper functions

class nocaseCompare : public std::less<std::string>
{
	bool operator()(const std::string& p_first, const std::string& p_second) const
	{
		return tt::str::toLower(p_first) < tt::str::toLower(p_second);
	}
};


//--------------------------------------------------------------------------------------------------
// Public member functions

EntityScriptMgr::EntityScriptMgr()
:
m_collectGarbage(false)
{
}


EntityScriptClassPtr EntityScriptMgr::getClass(const std::string& p_name) const
{
	if (p_name.empty())
	{
		return EntityScriptClassPtr();
	}
	
	ScriptClasses::const_iterator it = m_scriptClasses.find(p_name);
	if (it != m_scriptClasses.end())
	{
		return (*it).second;
	}
	
	return EntityScriptClassPtr();
}


EntityScriptClassPtr EntityScriptMgr::getParentClass(const HSQOBJECT& p_class) const
{
	if (sq_isnull(p_class))
	{
		return EntityScriptClassPtr();
	}
	
	HSQUIRRELVM v = m_vm->getVM();
	tt::script::SqTopRestorerHelper helper(v);
	
	sq_pushobject(v, p_class);
	sq_getbase(v, -1);
	HSQOBJECT parentClass;
	sq_resetobject(&parentClass);
	sq_getstackobj(v, -1, &parentClass);
	
	for (ScriptClasses::const_iterator it = m_scriptClasses.begin(); it != m_scriptClasses.end(); ++it)
	{
		if ((*it).second == 0)
		{
			continue;
		}
		
		// push this class on the stack
		sq_pushobject(v, (*it).second->getBaseClass());
		
		// compare it
		if (sq_cmp(v) == 0)
		{
			sq_pop(v, 3);
			return (*it).second;
		}
		
		sq_poptop(v);
	}
	
	sq_pop(v, 2);
	return EntityScriptClassPtr();
}


void EntityScriptMgr::registerForCallbacksUpdate(const EntityBasePtr& p_entity)
{
	m_updateCallbacks[p_entity->getHandle()] = p_entity;
}


void EntityScriptMgr::collectGarbage() const
{
	TT_NULL_ASSERT(m_vm);
	HSQUIRRELVM v = m_vm->getVM();
	sq_collectgarbage(v);
}


void EntityScriptMgr::init(const std::string& p_libraryPath)
{
	if (m_scriptClasses.empty() == false)
	{
		TT_PANIC("EntityScriptMgr already initialized");
		return;
	}
	
	m_vm = toki::script::ScriptMgr::getVM();
	TT_ASSERT(m_vm->isInitialized());
	
	m_rootPath = m_vm->getRoot() + p_libraryPath + "/";
	
	m_vm->loadAndRunScript("attributes");
	
	TT_SQBIND_SETVM(m_vm);
	TT_SQBIND_FUNCTION_NAME(EntityScriptMgr::include_entity, "include_entity");
	TT_SQBIND_FUNCTION_NAME(EntityScriptClass::registerAttribute, "registerAttribute");
	
	// Initialize the timer manager
	TimerMgr::init();
	
	// Set the script manager for EntityBase
	EntityBase::setScriptMgr(this);
	
	processNutFilesInDirectory(m_rootPath);
}


void EntityScriptMgr::deinit()
{
	EntityBase::resetScriptMgr();
	
	collectGarbage();
	
	m_vm.reset();
	
	TimerMgr::deinit();
	
	tt::code::helpers::freeContainer(m_scriptClasses);
	tt::code::helpers::freeContainer(m_taggedEntities);
	tt::code::helpers::freeContainer(m_updateCallbacks);
}


void EntityScriptMgr::update(real p_elapsedTime)
{
	updateEntityCallbacks();
	TimerMgr::update(p_elapsedTime);
	if (m_collectGarbage)
	{
		collectGarbage();
		m_collectGarbage = false;
	}
}


void EntityScriptMgr::registerAttributes() const
{
	if (m_vm == 0)
	{
		TT_PANIC("m_vm should not be 0. registerAttributes should be called after initialization.");
		return;
	}
	
	for (ScriptClasses::const_iterator it = m_scriptClasses.begin(); it != m_scriptClasses.end(); ++it)
	{
		if ((*it).second == 0)
		{
			continue;
		}
		
		(*it).second->registerAttributes(m_vm);
	}
}


void EntityScriptMgr::registerEntityByTag(const std::string& p_tag, const entity::EntityHandle& p_handle)
{
	entity::EntityHandles& handles = m_taggedEntities[p_tag];
	
	entity::EntityHandles::const_iterator it = std::find(handles.begin(), handles.end(), p_handle);
	if (it != handles.end())
	{
		TT_PANIC("Tag '%s' for entity with handle '%p' is already tagged",
			p_tag.c_str(), p_handle.getValue());
		return;
	}
	
	handles.push_back(p_handle);
}


void EntityScriptMgr::unregisterEntityByTag(const std::string& p_tag, const entity::EntityHandle& p_handle)
{
	TaggedEntities::iterator tagIt = m_taggedEntities.find(p_tag);
	if (tagIt == m_taggedEntities.end())
	{
		TT_PANIC("Tag '%s' not found", p_tag.c_str());
		return;
	}
	
	entity::EntityHandles& handles = (*tagIt).second;
	entity::EntityHandles::iterator it = std::find(handles.begin(), handles.end(), p_handle);
	if (it == handles.end())
	{
		TT_PANIC("Tag '%s' for entity with handle '%p' doesn't exist, so it cannot be removed",
			p_tag.c_str(), p_handle.getValue());
		return;
	}
	
	handles.erase(it);
	
	if (handles.empty())
	{
		m_taggedEntities.erase(tagIt);
	}
}


EntityBaseCollection EntityScriptMgr::getEntitiesByTag(const std::string& p_tag)
{
	TaggedEntities::iterator tagIt = m_taggedEntities.find(p_tag);
	if (tagIt != m_taggedEntities.end())
	{
		EntityBaseCollection result;
		entity::EntityMgr& mgr(AppGlobal::getGame()->getEntityMgr());
		entity::EntityHandles& handles = (*tagIt).second;
		
		for (entity::EntityHandles::iterator it = handles.begin(); it != handles.end();)
		{
			entity::Entity* entity = mgr.getEntity(*it);
			if (entity != 0)
			{
				result.push_back(entity->getEntityScript().get());
				++it;
			}
			else
			{
				TT_PANIC("Found handle to non-existing entity");
				it = handles.erase(it);
			}
		}
		return result;
	}
	
	return EntityBaseCollection();
}


bool EntityScriptMgr::classHasBaseClass(const std::string& p_class, const std::string& p_baseClass) const
{
	// Early exit
	if (p_class == p_baseClass)
	{
		return true;
	}
	
	if (p_class.empty() || p_baseClass.empty())
	{
		TT_PANIC("p_class '%s' or p_baseClass '%s' is empty", p_class.c_str(), p_baseClass.c_str());
		return false;
	}
	HSQUIRRELVM v = m_vm->getVM();
	tt::script::SqTopRestorerHelper helper(v);
	
	sq_pushroottable(v);
	
	// Fetch the two classes
	sq_pushstring(v, p_baseClass.c_str(), p_baseClass.size());
	sq_get(v, -2);
	sq_pushstring(v, p_class.c_str(), p_class.size());
	sq_get(v, -3);
	
	// Check if class contains base
	SQBool result = sq_containsbase(v);
	
	// Cleanup
	sq_pop(v, 3);
	
	return result == SQTrue;
}


void EntityScriptMgr::serialize(tt::code::BufferWriteContext* p_context) const
{
	TT_NULL_ASSERT(p_context);
	
	namespace bu = tt::code::bufferutils;
	
	bu::put(m_rootPath, p_context);	// std::string
	
	// Tags
	{
		const u32 tagCount = static_cast<u32>(m_taggedEntities.size());
		bu::put(tagCount, p_context);
		for (TaggedEntities::const_iterator tagIt = m_taggedEntities.begin(); tagIt != m_taggedEntities.end(); ++tagIt)
		{
			bu::put      (tagIt->first , p_context);
			
			const entity::EntityHandles& handles = tagIt->second;
			const u32 handleCount = static_cast<u32>(handles.size());
			bu::put(handleCount, p_context);
			for (entity::EntityHandles::const_iterator handleIt = handles.begin(); handleIt != handles.end(); ++handleIt)
			{
				bu::putHandle((*handleIt), p_context);
			}
		}
	}
	
	// updateCallbacks
	{
		const u32 callbackCount = static_cast<u32>(m_updateCallbacks.size());
		bu::put(callbackCount, p_context);
		for (EntityBaseWeakPtrs::const_iterator it = m_updateCallbacks.begin(); it != m_updateCallbacks.end(); ++it)
		{
			EntityBasePtr ptr((*it).second.lock());
			bu::putHandle( (ptr != 0) ? ptr->getHandle() : entity::EntityHandle(), p_context);
		}
	}
	
	// collectGarbage
	bu::put(m_collectGarbage, p_context);
}


void EntityScriptMgr::unserialize(tt::code::BufferReadContext*  p_context)
{
	// Reset old data.
	m_taggedEntities.clear();
	m_updateCallbacks.clear();
	m_collectGarbage = false;
	
	// Unserialize
	TT_NULL_ASSERT(p_context);
	
	namespace bu = tt::code::bufferutils;
	
	m_rootPath = bu::get<std::string>(p_context);
	
	// Tags
	{
		const u32 tagCount = bu::get<u32>(p_context);
		for (u32 tagIdx = 0; tagIdx < tagCount; ++tagIdx)
		{
			const std::string tag = bu::get<std::string>(p_context);
			const u32 handleCount = bu::get<u32        >(p_context);
			
			for (u32 handleIdx = 0; handleIdx < handleCount; ++handleIdx)
			{
				entity::EntityHandle handle = bu::getHandle<entity::Entity>(p_context);
				registerEntityByTag(tag, handle);
			}
		}
	}
	
	// updateCallbacks
	{
		const u32 callbackCount = bu::get<u32>(p_context);
		for (u32 i = 0; i < callbackCount; ++i)
		{
			const entity::EntityHandle handle = bu::getHandle<entity::Entity>(p_context);
			entity::Entity*            entity = handle.getPtr();
			if (entity != 0)
			{
				registerForCallbacksUpdate(entity->getEntityScript());
			}
		}
	}
	
	// collectGarbage
	m_collectGarbage = bu::get<bool>(p_context);
}


//--------------------------------------------------------------------------------------------------
// Private member functions

void EntityScriptMgr::include_entity(const std::string& p_path)
{
	EntityScriptMgr& mgr = AppGlobal::getEntityScriptMgr();
	const std::string name(tt::fs::utils::getFileTitle(p_path));
	const std::string path(tt::fs::utils::getDirectory(p_path));
	const std::string filePath(mgr.getRootPath() + path);
	if (tt::fs::fileExists(filePath + name + ".nut" ) ||
	    tt::fs::fileExists(filePath + name + ".bnut") )
	{
		mgr.createScriptClass(mgr.getRootPath() + path, name);
	}
	else
	{
		TT_PANIC("Trying to include entity '%s' from directory '%s', but file doesn't exists",
			name.c_str(), filePath.c_str());
	}
}


void EntityScriptMgr::updateEntityCallbacks()
{
	if (m_updateCallbacks.empty())
	{
		return;
	}
	
	EntityBasePtrs copy;
	copy.reserve(m_updateCallbacks.size());
	for (EntityBaseWeakPtrs::const_iterator it = m_updateCallbacks.begin(); it != m_updateCallbacks.end(); ++it)
	{
		EntityBasePtr entity((*it).second.lock());
		if (entity != 0)
		{
			copy.push_back(entity);
		}
	}
	
	m_updateCallbacks.clear();
	
	for (EntityBasePtrs::iterator it = copy.begin(); it != copy.end(); ++it)
	{
		(*it)->updateCallbacks();
	}
}


void EntityScriptMgr::createScriptClass(const std::string& p_path, const std::string& p_name)
{
	if (hasClass(p_name))
	{
		return;
	}
	
	m_scriptClasses[p_name] = EntityScriptClass::create(p_path, p_name);
}


void EntityScriptMgr::processNutFilesInDirectory(const std::string& p_path)
{
	if (tt::fs::dirExists(p_path) == false)
	{
		return;
	}
	
	tt::fs::DirPtr dir(tt::fs::openDir(p_path));
	if (dir == 0)
	{
		return;
	}
	
	// Use a set so we can sort the script names as we insert them
	nocaseCompare cmp;
	tt::str::StringSet file_list(cmp);
	
	tt::fs::DirEntry entry;
	while (dir->read(entry))
	{
		const std::string& fileName(entry.getName());
		
		if (entry.isDirectory())
		{
			if (fileName != "." && fileName != "..")
			{
				file_list.insert(p_path + fileName + "/");
			}
			continue;
		}
		
		if (tt::fs::utils::getExtension(fileName) != "nut"  &&
		    tt::fs::utils::getExtension(fileName) != "bnut" )
		{
			continue;
		}
		file_list.insert(fileName);
	}
	
	for (tt::str::StringSet::const_iterator it = file_list.begin(); it != file_list.end(); ++it)
	{
		const std::string& fileName = *it;
		
		if (fileName[fileName.length() - 1] == '/')
		{
			processNutFilesInDirectory(fileName);
		}
		else
		{
			const std::string className(tt::fs::utils::getFileTitle(fileName));
			createScriptClass(p_path, className);
		}
	}
}

// Namespace end
}
}
}
