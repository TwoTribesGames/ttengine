#if !defined(INC_TT_COM_TYPES_H)
#define INC_TT_COM_TYPES_H


#include <map>
#include <set>

#include <tt/platform/tt_types.h>
#include <tt/math/hash/NamedHash.h>
#include <tt/code/Identifier.h>


namespace tt {
namespace com {

typedef math::hash::NamedHash<32> ObjectID;

class ComponentBase;

typedef tt_ptr<ComponentBase>::shared ComponentBasePtr;
typedef std::set<ComponentBasePtr>    ComponentSet;

typedef std::map<ObjectID, ComponentBasePtr> ComponentMap;

class Object;
typedef tt_ptr<Object>::shared ObjectPtr;


enum ComponentIDDummy { };
enum InterfaceIDDummy { };
enum MessageIDDummy { };

/*! \brief Type-safe component identifier. */
typedef code::Identifier<ComponentIDDummy> ComponentID;
typedef std::set<ComponentID>              ComponentIDSet;

/*! \brief Type-safe interface identifier. */
typedef code::Identifier<InterfaceIDDummy> InterfaceID;
typedef std::set<InterfaceID>              InterfaceIDSet;

/*! \brief Type-safe message identifier. */
typedef code::Identifier<MessageIDDummy> MessageID;


// Namespace end
}
}


#endif  // !defined(INC_TT_COM_TYPES_H)
