#if !defined(INC_TOKI_GAME_SCRIPT_ENTITYSCRIPTMGR_H)
#define INC_TOKI_GAME_SCRIPT_ENTITYSCRIPTMGR_H

#include <tt/code/fwd.h>
#include <tt/script/VirtualMachine.h>

#include <toki/game/entity/fwd.h>
#include <toki/game/script/fwd.h>
#include <toki/game/script/TimerMgr.h>
#include <toki/game/movement/fwd.h>


namespace toki {
namespace game {
namespace script {

class EntityScriptMgr
{
public:
	typedef std::map<std::string, EntityScriptClassPtr> ScriptClasses;
	EntityScriptMgr();
	
	void init(const std::string& p_libraryPath);
	inline void reset() { m_taggedEntities.clear(); m_updateState.clear(); TimerMgr::reset(); }
	void deinit();
	
	void update(real p_elapsedTime);
	
	void registerAttributes() const;
	
	inline bool hasClass(const std::string& p_name) const
	{
		return m_scriptClasses.find(p_name) != m_scriptClasses.end();
	}
	EntityScriptClassPtr getClass(const std::string& p_name) const;
	EntityScriptClassPtr getParentClass(const HSQOBJECT& p_class) const;
	inline const tt::script::VirtualMachinePtr& getVM() const { return m_vm; }
	
	inline const ScriptClasses& getScriptClasses() const { return m_scriptClasses; }
	
	void registerForCallbacksUpdate(const EntityBasePtr& p_entity);
	
	inline void scheduleGarbageCollection() { m_collectGarbage = true; }
	void collectGarbage() const;
	
	void registerEntityByTag(const std::string& p_tag, const entity::EntityHandle& p_handle);
	void unregisterEntityByTag(const std::string& p_tag, const entity::EntityHandle& p_handle);
	EntityBaseCollection getEntitiesByTag(const std::string& p_tag);
	
	inline const std::string& getRootPath() const { return m_rootPath; }
	
	bool classHasBaseClass(const std::string& p_class, const std::string& p_baseClass) const;
	
	void serialize  (tt::code::BufferWriteContext* p_context) const;
	void unserialize(tt::code::BufferReadContext*  p_context);
	
private:
	typedef std::map<std::string, entity::EntityHandles> TaggedEntities;
	typedef std::map<entity::EntityHandle, EntityBaseWeakPtr> EntityBaseWeakPtrs;
	typedef std::vector<EntityBasePtr> EntityBasePtrs;
	
	static void include_entity(const std::string& p_path);
	void updateEntityCallbacks();
	void createScriptClass(const std::string& p_path, const std::string& p_name);
	void processNutFilesInDirectory(const std::string& p_path);
	
	std::string        m_rootPath;
	TaggedEntities     m_taggedEntities;
	EntityBaseWeakPtrs m_updateState;
	EntityBasePtrs     m_updateStateCopy;
	EntityBaseWeakPtrs m_updateCallbacks;
	bool               m_collectGarbage;
	
	tt::script::VirtualMachinePtr m_vm;
	ScriptClasses m_scriptClasses;
};

// Namespace end
}
}
}


#endif  // !defined(INC_TOKI_GAME_SCRIPT_ENTITYSCRIPTMGR_H)
