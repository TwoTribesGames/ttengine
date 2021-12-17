#if !defined(INC_TOKI_SCRIPT_SERIALIZATION_FWD_H)
#define INC_TOKI_SCRIPT_SERIALIZATION_FWD_H

#include <tt/platform/tt_types.h>

namespace toki {
namespace script {
namespace serialization {

class SQCache;
typedef tt_ptr<SQCache>::shared SQCachePtr;

class SQSerializer;
class SQUnserializer;
class UserType;

// Namespace end
}
}
}

#endif  // !defined(INC_TOKI_SCRIPT_SERIALIZATION_FWD_H)
