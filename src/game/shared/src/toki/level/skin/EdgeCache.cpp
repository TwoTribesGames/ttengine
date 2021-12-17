#include <tt/code/helpers.h>

#include <toki/level/skin/EdgeCache.h>
#include <toki/level/skin/MaterialCache.h>


namespace toki {
namespace level {
namespace skin {

EdgeCache::EdgeCache(const tt::math::Point2& p_levelSize)
:
m_horizontalSize(p_levelSize.x - 1, p_levelSize.y    ),
m_verticalSize(  p_levelSize.x    , p_levelSize.y - 1),
m_horizontalEdges(0),
m_verticalEdges(  0)
{
	TT_ASSERT(m_horizontalSize.x >= 0);
	TT_ASSERT(m_horizontalSize.y >= 0);
	TT_ASSERT(m_verticalSize.x   >= 0);
	TT_ASSERT(m_verticalSize.y   >= 0);
	
	m_horizontalEdges = new EdgeType[m_horizontalSize.x * m_horizontalSize.y];
	m_verticalEdges   = new EdgeType[m_verticalSize.x   * m_verticalSize.y  ];
}


EdgeCache::~EdgeCache()
{
	tt::code::helpers::safeDelete(m_horizontalEdges);
	tt::code::helpers::safeDelete(m_verticalEdges);
}


void EdgeCache::update(const MaterialCache& p_tiles)
{
	TT_ASSERT(m_verticalSize.x   == p_tiles.getSize().x);
	TT_ASSERT(m_horizontalSize.y == p_tiles.getSize().y);
	const tt::math::Point2& levelSize = p_tiles.getSize();
	
	const TileMaterial* tilePtr = p_tiles.getRawTiles();
	EdgeType* edgePtrHorizontal = m_horizontalEdges;
	
	const TileMaterial* tileToLeftPtr    = tilePtr;
	const TileMaterial* startPreviousRow = tilePtr;
	++tilePtr;
	tt::math::Point2 pos(1, 0);
	// Go over the bottom row. (only for the horizontal edges.)
	for (; pos.x < levelSize.x; ++pos.x, ++tilePtr, ++edgePtrHorizontal)
	{
#if defined(TT_BUILD_DEV) // Asserts to make sure this usage for the raw data is correct.
		TT_ASSERT(tilePtr           == &p_tiles.getTileMaterial(pos));
		TT_ASSERT(edgePtrHorizontal == &getLeftEdge(pos));
#endif
		*edgePtrHorizontal = getEdgeForTiles(*tileToLeftPtr, *tilePtr);
		
		tileToLeftPtr = tilePtr;
	}
	
	EdgeType* edgePtrVertical = m_verticalEdges;
	++pos.y;
	for (; pos.y < levelSize.y; ++pos.y)
	{
		// Start new row
		const TileMaterial* tileBelowPtr = startPreviousRow;
		startPreviousRow                 = tilePtr;
		pos.x = 0;
		
#if defined(TT_BUILD_DEV) // Asserts to make sure this usage for the raw data is correct.
		TT_ASSERT(tilePtr         == &p_tiles.getTileMaterial(pos));
		TT_ASSERT(tileBelowPtr    == &p_tiles.getTileMaterial(tt::math::Point2(pos.x, pos.y - 1)));
		TT_ASSERT(edgePtrVertical == &getBottomEdge(pos));
#endif
		
		// At first tile only do vertical edge
		*edgePtrVertical = getEdgeForTiles(*tileBelowPtr, *tilePtr);
		
		tileToLeftPtr = tilePtr;
		++tilePtr;
		++tileBelowPtr;
		++edgePtrVertical;
		++pos.x;
		
		for (; pos.x < levelSize.x;
		     ++pos.x, ++tilePtr, ++tileBelowPtr, ++edgePtrHorizontal, ++edgePtrVertical)
		{
#if defined(TT_BUILD_DEV) // Asserts to make sure this usage for the raw data is correct.
			TT_ASSERT(tilePtr           == &p_tiles.getTileMaterial(pos));
			TT_ASSERT(tileBelowPtr      == &p_tiles.getTileMaterial(tt::math::Point2(pos.x, pos.y - 1)));
			TT_ASSERT(edgePtrHorizontal == &getLeftEdge(pos));
			TT_ASSERT(edgePtrVertical   == &getBottomEdge(pos));
#endif
			*edgePtrVertical   = getEdgeForTiles(*tileBelowPtr,  *tilePtr);
			*edgePtrHorizontal = getEdgeForTiles(*tileToLeftPtr, *tilePtr);
			
			tileToLeftPtr = tilePtr;
		}
	}
}


void EdgeCache::handleLevelResized(s32 p_newLevelWidth, s32 p_newLevelHeight)
{
	// FIXME: Check if level size actually changed from what we have now
	
	m_horizontalSize.setValues(p_newLevelWidth - 1, p_newLevelHeight    );
	m_verticalSize  .setValues(p_newLevelWidth    , p_newLevelHeight - 1);
	
	TT_ASSERT(m_horizontalSize.x >= 0);
	TT_ASSERT(m_horizontalSize.y >= 0);
	TT_ASSERT(m_verticalSize.x   >= 0);
	TT_ASSERT(m_verticalSize.y   >= 0);
	
	tt::code::helpers::safeDelete(m_horizontalEdges);
	tt::code::helpers::safeDelete(m_verticalEdges);
	
	m_horizontalEdges = new EdgeType[m_horizontalSize.x * m_horizontalSize.y];
	m_verticalEdges   = new EdgeType[m_verticalSize.x   * m_verticalSize.y  ];
}

// Namespace end
}
}
}
