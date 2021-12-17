#if !defined(INC_TOKI_LEVEL_ENTITY_HELPERS_H)
#define INC_TOKI_LEVEL_ENTITY_HELPERS_H


#include <string>
#include <utility>
#include <vector>

#include <tt/code/ErrorStatus.h>
#include <tt/engine/renderer/fwd.h>
#include <tt/platform/tt_types.h>

#include <toki/level/entity/fwd.h>
#include <toki/level/fwd.h>


namespace toki {
namespace level {
namespace entity {

typedef std::pair<s32, s32>   OldNewID;  // mapping of an entity's old ID to its new ID
typedef std::vector<OldNewID> OldNewIDs;

/*! \brief Removes all invalid entity-to-entity references from a level. */
void removeUnreferencedEntityReferences(const LevelDataPtr& p_targetLevel);

/*! \brief Updates entity-to-entity references in the specified entity instances. */
void updateInternalEntityReferences(const EntityInstances& p_instances,
                                    const OldNewIDs&       p_oldAndNewIDs,
                                    const LevelDataPtr&    p_targetLevel);


/*! \brief Turns an EntityInstance color property string value into a ColorRGB. */
tt::engine::renderer::ColorRGB parseColorRGBProperty(const std::string&     p_propertyValue,
                                                     tt::code::ErrorStatus* p_errStatus = 0);

/*! \brief Turns an EntityInstance color property string value into a ColorRGB. */
tt::engine::renderer::ColorRGBA parseColorRGBAProperty(const std::string&     p_propertyValue,
                                                       tt::code::ErrorStatus* p_errStatus = 0);

/*! \brief Turns a ColorRGB into an EntityInstance color property string value. */
std::string makePropertyString(const tt::engine::renderer::ColorRGB& p_color);

/*! \brief Turns a ColorRGBA into an EntityInstance color property string value. */
std::string makePropertyString(const tt::engine::renderer::ColorRGBA& p_color);

// Namespace end
}
}
}


#endif  // !defined(INC_TOKI_LEVEL_ENTITY_HELPERS_H)
