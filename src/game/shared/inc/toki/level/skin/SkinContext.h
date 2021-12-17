#if !defined(INC_TOKI_LEVEL_SKIN_SKINCONTEXT_H)
#define INC_TOKI_LEVEL_SKIN_SKINCONTEXT_H


#include <toki/level/skin/BlobData.h>
#include <toki/level/skin/EdgeCache.h>
#include <toki/level/skin/functions.h>
#include <toki/level/skin/fwd.h>
#include <toki/level/skin/MaterialCache.h>
#include <toki/level/skin/types.h>


namespace toki {
namespace level {
namespace skin {

class SkinContext
{
public:
	static SkinContextPtr create(s32 p_levelWidth, s32 p_levelHeight);
	~SkinContext();
	
	void handleLevelResized(s32 p_newLevelWidth, s32 p_newLevelHeight);
	
	
	MaterialCache     tileMaterial;
	EdgeCache         edgeCache;
	impl::GrowEdge*   scratchGrowEdge;
	s32               scratchGrowEdgeSize;
	BlobData          blobData;
	
private:
	SkinContext(s32 p_levelWidth, s32 p_levelHeight);
	
	// No copying
	SkinContext(const SkinContext&);
	SkinContext& operator=(const SkinContext&);
};

// Namespace end
}
}
}


#endif  // !defined(INC_TOKI_LEVEL_SKIN_SKINCONTEXT_H)
