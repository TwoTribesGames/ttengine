#if !defined(INC_TOKI_LEVEL_ENTITY_ENTITYINFO_H)
#define INC_TOKI_LEVEL_ENTITY_ENTITYINFO_H


#include <string>

#include <tt/engine/renderer/ColorRGBA.h>
#include <tt/math/Rect.h>
#include <tt/str/str_types.h>

#include <toki/game/movement/fwd.h>
#include <toki/game/script/fwd.h>
#include <toki/game/fwd.h>
#include <toki/level/entity/EntityProperty.h>
#include <toki/level/entity/fwd.h>
#include <toki/level/types.h>


namespace toki {
namespace level {
namespace entity {

/*! \brief Encapsulates the information related to an entity type (based on a script file). */
class EntityInfo
{
public:
	EntityInfo();
	
	static EntityInfo create(const game::script::EntityScriptClassPtr& p_entityClass);
	
	inline bool               isValid()                   const { return m_name.empty() == false; }
	inline const std::string& getName()                   const { return m_name;                  }
	inline const std::string& getDisplayName()            const { return m_displayName;           }
	inline Placeable          getPlaceable()              const { return m_placeable;             }
	inline const std::string& getEditorImage()            const { return m_editorImage;           }
	inline const std::string& getLibraryImage()           const { return m_libraryImage;          }
	inline const std::string& getMovementSetName()        const { return m_movementSetName;       }
	inline s32                getOrder()                  const { return m_order;                 }
	inline bool               ignoreSpawnSections()       const { return m_ignoreSpawnSections;   }
	inline const tt::math::VectorRect& getCollisionRect() const { return m_collisionRect;         }
	inline real               getPathFindAgentRadius()    const { return m_pathFindAgentRadius;   }
	inline bool               hasPathCrowdSeparation()    const { return m_pathCrowdSeparation;   }
	inline s32                getMaxEntityCount()         const { return m_maxEntityCount;        }
	
	/*! \return The Steam Workshop tags to add for this entity type when publishing a level. */
	inline const tt::str::Strings& getWorkshopTags() const { return m_workshopTags; }
	
	inline bool                                   hasFixedSizeShapeCircle()     const { return m_hasFixedSizeShapeCircle;    }
	inline bool                                   hasFixedSizeShapeRectangle()  const { return m_hasFixedSizeShapeRectangle; }
	inline real                                   getFixedSizeShapeRadius()     const { return m_fixedSizeShapeRadius;       }
	inline const tt::math::Vector2&               getFixedSizeShapeRectangle()  const { return m_fixedSizeShapeRectangle;    }
	inline const tt::engine::renderer::ColorRGBA& getSizeShapeColor()           const { return m_sizeShapeColor;             }
	inline bool                                   isSizeShapeFromEntityCenter() const { return m_sizeShapeFromEntityCenter;  }
	
	inline const std::string& getGroup() const { return m_group; }
	
	inline const EntityProperties& getProperties() const { return m_properties; }
	
	bool                  hasProperty(const std::string& p_name) const;
	const EntityProperty& getProperty(const std::string& p_name) const;
	
private:
	std::string m_name;
	std::string m_displayName;
	Placeable   m_placeable;  //!< Who can place this entity in the editor.
	std::string m_editorImage;
	std::string m_libraryImage;
	std::string m_movementSetName;
	s32         m_order;
	bool        m_ignoreSpawnSections;
	
	tt::math::VectorRect m_collisionRect;
	
	real m_pathFindAgentRadius;
	bool m_pathCrowdSeparation;
	s32  m_maxEntityCount; // maximum number of entities that are allowed of this type in one level (-1 if no limit)
	
	tt::str::Strings m_workshopTags;
	
	bool                            m_hasFixedSizeShapeRectangle;
	bool                            m_hasFixedSizeShapeCircle;
	real                            m_fixedSizeShapeRadius;
	tt::math::Vector2               m_fixedSizeShapeRectangle;
	tt::engine::renderer::ColorRGBA m_sizeShapeColor;
	bool                            m_sizeShapeFromEntityCenter;
	
	std::string m_group;
	
	EntityProperties m_properties; // All the properties available for this type
	
	static const EntityInfo invalid;
};

// Namespace end
}
}
}


#endif  // !defined(INC_TOKI_LEVEL_ENTITY_ENTITYINFO_H)
