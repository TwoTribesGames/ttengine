#include <toki/game/entity/EntityLibrary.h>
#include <toki/game/script/EntityScriptMgr.h>
#include <toki/AppGlobal.h>


namespace toki {
namespace game {
namespace entity {

//--------------------------------------------------------------------------------------------------
// Public member functions

EntityLibrary::EntityLibrary()
:
m_types()
{
}


EntityLibrary::~EntityLibrary()
{
}


void EntityLibrary::rescan()
{
	// FIXME: Perhaps make this less brute-force? Only add EntityInfo that is new, remove what was removed
	m_types.clear();
	
	using namespace game::script;
	EntityScriptMgr& mgr = AppGlobal::getEntityScriptMgr();
	const EntityScriptMgr::ScriptClasses& classes = mgr.getScriptClasses();
	for (EntityScriptMgr::ScriptClasses::const_iterator it = classes.begin(); it != classes.end(); ++it)
	{
		if ((*it).second == 0)
		{
			continue;
		}
		level::entity::EntityInfo info(level::entity::EntityInfo::create((*it).second));
		
		if (info.isValid() == false)
		{
			continue;
		}
		
		m_types[info.getName()] = info;
	}
}


const level::entity::EntityInfo* EntityLibrary::getEntityInfo(const std::string& p_typeName) const
{
	EntityTypes::const_iterator it = m_types.find(p_typeName);
	return (it != m_types.end()) ? &(*it).second : 0;
}

// Namespace end
}
}
}
