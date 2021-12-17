#include <tt/code/helpers.h>
#include <tt/platform/tt_error.h>

#include <toki/level/skin/GrowEdge.h>
#include <toki/level/skin/SkinContext.h>


namespace toki {
namespace level {
namespace skin {

//--------------------------------------------------------------------------------------------------
// Public member functions

SkinContextPtr SkinContext::create(s32 p_levelWidth, s32 p_levelHeight)
{
	if (p_levelWidth <= 0 || p_levelHeight <= 0)
	{
		TT_PANIC("Invalid level size: %d x %d. Both width and height need to be greater than 0.",
		         p_levelWidth, p_levelHeight);
		return SkinContextPtr();
	}
	
	return SkinContextPtr(new SkinContext(p_levelWidth, p_levelHeight));
}


SkinContext::~SkinContext()
{
	tt::code::helpers::safeDeleteArray(scratchGrowEdge);
}


void SkinContext::handleLevelResized(s32 p_newLevelWidth, s32 p_newLevelHeight)
{
	TT_ASSERT(p_newLevelWidth  > 0);
	TT_ASSERT(p_newLevelHeight > 0);
	
	tileMaterial.handleLevelResized(p_newLevelWidth, p_newLevelHeight);
	edgeCache   .handleLevelResized(p_newLevelWidth, p_newLevelHeight);
	
	// Did the width change?
	if (p_newLevelWidth != scratchGrowEdgeSize)
	{
		// Grow edge scratch
		tt::code::helpers::safeDeleteArray(scratchGrowEdge);
		
		scratchGrowEdgeSize   = p_newLevelWidth;
		scratchGrowEdge       = new impl::GrowEdge  [scratchGrowEdgeSize];
		
		// Blob
		blobData.reset(p_newLevelWidth);
	}
}


//--------------------------------------------------------------------------------------------------
// Private member functions

SkinContext::SkinContext(s32 p_levelWidth, s32 p_levelHeight)
:
tileMaterial         (p_levelWidth, p_levelHeight),
edgeCache            (tt::math::Point2(p_levelWidth, p_levelHeight)),
scratchGrowEdge      (new impl::GrowEdge[p_levelWidth]),
scratchGrowEdgeSize  (p_levelWidth),
blobData()
{
	blobData.reset(p_levelWidth);
}

// Namespace end
}
}
}
