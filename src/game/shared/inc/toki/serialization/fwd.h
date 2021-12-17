#if !defined(INC_TOKI_SERIALIZATION_FWD_H)
#define INC_TOKI_SERIALIZATION_FWD_H


#include <tt/platform/tt_types.h>


namespace toki {
namespace serialization {

class Serializer;
typedef tt_ptr<Serializer>::shared SerializerPtr;

class SerializationMgr;
typedef tt_ptr<SerializationMgr>::shared SerializationMgrPtr;

// Namespace end
}
}


#endif  // !defined(INC_TOKI_SERIALIZATION_FWD_H)
