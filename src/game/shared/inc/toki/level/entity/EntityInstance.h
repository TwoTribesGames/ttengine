#if !defined(INC_TOKI_LEVEL_ENTITY_ENTITYINSTANCE_H)
#define INC_TOKI_LEVEL_ENTITY_ENTITYINSTANCE_H


#include <map>
#include <string>
#include <vector>

#include <tt/math/Vector2.h>
#include <tt/platform/tt_types.h>
#include <tt/str/str.h>

#include <toki/level/entity/editor/fwd.h>
#include <toki/level/entity/fwd.h>


namespace toki {
namespace level {
namespace entity {

/*! \brief An entity as it is placed in the level, with all properties that were defined for the specific instance. */
class EntityInstance
{
public:
	typedef std::map<std::string, std::string> Properties;
	
	
	static EntityInstancePtr create(const std::string&       p_type,
	                                s32                      p_id,
	                                const tt::math::Vector2& p_position = tt::math::Vector2::zero);
	~EntityInstance();
	
	EntityInstancePtr clone()                     const;
	EntityInstancePtr cloneWithNewID(s32 p_newID) const;
	
	inline bool isValid() const { return m_type.empty() == false && m_id >= 0; }
	
	inline const std::string& getType() const { return m_type; }
	inline s32                getID()   const { return m_id;   }
	inline s32                getSpawnSectionID() const          { return m_spawnSectionID;        }
	inline void               setSpawnSectionID(s32 p_sectionID) { m_spawnSectionID = p_sectionID; }
	
	inline const tt::math::Vector2& getPosition() const { return m_position; }
	void                            setPosition(const tt::math::Vector2& p_pos);
	
	inline const Properties& getProperties() const { return m_properties; }
	void                     setProperties(const Properties& p_properties);
	inline void              setPropertiesUpdatedByScript(bool p_updated)
	{
		m_propertiesUpdatedByScript = p_updated;
	}
	
	inline bool              getPropertiesUpdatedByScript() const
	{
		return m_propertiesUpdatedByScript;
	}
	
	bool hasProperty(const std::string& p_name) const;
	const std::string& getPropertyValue(const std::string& p_name) const;
	void setPropertyValue(const std::string& p_name, const std::string& p_value);
	void removeProperty(const std::string& p_name);
	
	// returns true if property exists and isn't hidden by conditional
	bool isPropertyVisible(const std::string& p_name) const;
	
	void registerObserver  (const EntityInstanceObserverWeakPtr& p_observer);
	void unregisterObserver(const EntityInstanceObserverWeakPtr& p_observer);
	inline void clearObservers() { m_observers.clear(); }
	
	editor::EntityInstanceEditorRepresentation* getOrCreateEditorRepresentation();
	inline editor::EntityInstanceEditorRepresentation* getEditorRepresentation() { return m_editorRepresentation; }
	void destroyEditorRepresentation();
	
	bool matchesFilter(const tt::str::StringSet& p_filter) const;
	
	static bool sortOrder(const EntityInstancePtr& p_left, const EntityInstancePtr& p_right);
	
private:
	typedef std::vector<EntityInstanceObserverWeakPtr> Observers;
	
	
	EntityInstance(const std::string&       p_type,
	               s32                      p_id,
	               const tt::math::Vector2& p_position);
	EntityInstance(const EntityInstance& p_rhs);
	
	void notifyPositionChanged();
	void notifyPropertiesChanged();
	
	// No assignment
	EntityInstance& operator=(const EntityInstance&);
	
	
	tt_ptr<EntityInstance>::weak m_this;
	
	std::string       m_type;            //!< Name of this entity's type (is also the name of the script class)
	s32               m_id;              //!< Unique (per level) identifier for this entity.
	s32               m_spawnSectionID;  //!< If not >= 0, this entity is not spawned at start but can be spawned using the section spawner
	tt::math::Vector2 m_position;        //!< World position.
	Properties        m_properties;      // All the custom properties defined for this instance (overridden from the type defaults)
	Observers         m_observers;
	bool              m_propertiesUpdatedByScript;
	
	editor::EntityInstanceEditorRepresentation* m_editorRepresentation;
};

// Namespace end
}
}
}


#endif  // !defined(INC_TOKI_LEVEL_ENTITY_ENTITYINSTANCE_H)
