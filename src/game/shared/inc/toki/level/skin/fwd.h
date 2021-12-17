#if !defined(INC_TOKI_LEVEL_SKIN_FWD_H)
#define INC_TOKI_LEVEL_SKIN_FWD_H


#include <tt/platform/tt_types.h>


namespace toki {
namespace level {
namespace skin {


struct BlobData;
class EdgeCache;
class MaterialCache;
class SkinConfig;
class SkinContext;
class TileMaterial;

namespace impl
{
	struct EdgeShape;
	struct GrowEdge;
}

typedef tt_ptr<SkinConfig >::shared SkinConfigPtr;
typedef tt_ptr<SkinContext>::shared SkinContextPtr;

// Namespace end
}
}
}


#endif  // !defined(INC_TOKI_LEVEL_SKIN_FWD_H)
