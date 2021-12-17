#if !defined(INC_TOKI_GAME_SCRIPT_ENTITYSCRIPTCLASS_H)
#define INC_TOKI_GAME_SCRIPT_ENTITYSCRIPTCLASS_H

#include <map>

#include <squirrel/squirrel.h>

#include <tt/script/VirtualMachine.h>

#include <toki/game/script/EntityState.h>
#include <toki/game/script/fwd.h>
#include <toki/script/attributes/ClassAttributes.h>


namespace toki {
namespace game {
namespace script {


class EntityScriptClass
{
public:
	static EntityScriptClassPtr create(const std::string& p_path, const std::string& p_name);
	EntityState getState(const std::string& p_stateName) const;
	
	inline const std::string& getName() const { return m_name; }
	inline HSQOBJECT getBaseClass() const { return m_baseClass; }
	inline EntityState getBaseState() const { return getState(""); }
	
	inline bool hasState(const std::string& p_stateName) const
	{
		return m_stateMap.find(p_stateName) != m_stateMap.end();
	}
	
	inline const toki::script::attributes::ClassAttributes& getAttributes() const
	{
		return m_attributes;
	}
	
	void registerAttributes(const tt::script::VirtualMachinePtr& p_vm);
	
private:
	typedef std::map<std::string, EntityState> StateMap;
	
	EntityScriptClass(const std::string& p_name, const HSQOBJECT& p_baseClass,
	                  const StateMap& p_rawStateMap, const StateMap& p_stateMap)
	:
	m_name(p_name),
	m_baseClass(p_baseClass),
	m_rawStateMap(p_rawStateMap),
	m_stateMap(p_stateMap),
	m_attributes()
	{}
	
	inline const StateMap& getRawStateMap() const { return m_rawStateMap; }
	
	static void registerAttribute(const std::string& p_memberName,
	                              const std::string& p_attributeName,
	                              const tt::str::Strings& p_attributeValue,
	                              const std::string& p_attributeType);
	
	std::string m_name;
	HSQOBJECT m_baseClass;
	
	StateMap m_rawStateMap; // State classes 'clean' from script
	StateMap m_stateMap;    // State classes with inheritence magic applied.
	
	toki::script::attributes::ClassAttributes m_attributes;
	static toki::script::attributes::ClassAttributes ms_classAttributesScratch;
	
	friend class EntityScriptMgr;
};

// Namespace end
}
}
}


#endif  // !defined(INC_TOKI_GAME_SCRIPT_ENTITYSCRIPTCLASS_H)
