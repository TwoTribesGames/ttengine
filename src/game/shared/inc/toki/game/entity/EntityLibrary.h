#if !defined(INC_TOKI_GAME_ENTITY_ENTITYLIBRARY_H)
#define INC_TOKI_GAME_ENTITY_ENTITYLIBRARY_H


#include <map>
#include <string>

#include <toki/level/entity/EntityInfo.h>


namespace toki {
namespace game {
namespace entity {

/*! \brief Back-end part of the entity library: scans for available entities and provides information about them. */
class EntityLibrary
{
public:
	typedef std::map<std::string, level::entity::EntityInfo>::const_iterator const_iterator;
	
	
	EntityLibrary();
	~EntityLibrary();
	
	void rescan();
	
	const level::entity::EntityInfo* getEntityInfo(const std::string& p_typeName) const;
	
	inline const_iterator begin() const { return m_types.begin(); }
	inline const_iterator end()   const { return m_types.end();   }
	
private:
	// Mapping of type name -> type info (containing properties etc)
	typedef std::map<std::string, level::entity::EntityInfo> EntityTypes;
	
	
	EntityTypes m_types;
};

// Namespace end
}
}
}


#endif  // !defined(INC_TOKI_GAME_ENTITY_ENTITYLIBRARY_H)
