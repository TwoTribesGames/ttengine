#if !defined(INC_TOKI_LEVEL_SKIN_EDGECACHE_H)
#define INC_TOKI_LEVEL_SKIN_EDGECACHE_H


#include <tt/math/Point2.h>

#include <toki/level/skin/fwd.h>
#include <toki/level/skin/types.h>
#include <toki/level/fwd.h>

namespace toki {
namespace level {
namespace skin {


class EdgeCache
{
public:
	EdgeCache(const tt::math::Point2& p_levelSize);
	~EdgeCache();
	
	inline const EdgeType& getHorizontalEdge(const tt::math::Point2& p_position) const
	{
		if (p_position.x < 0 || p_position.x >= m_horizontalSize.x ||
		    p_position.y < 0 || p_position.y >= m_horizontalSize.y)
		{
			static EdgeType emptyEdge = EdgeType_None;
			return emptyEdge;
		}
		return m_horizontalEdges[p_position.x + (p_position.y * m_horizontalSize.x)];
	}
	inline const EdgeType& getLeftEdge(const tt::math::Point2& p_position) const
	{
		return getHorizontalEdge(tt::math::Point2(p_position.x - 1, p_position.y));
	}
	inline const EdgeType& getRightEdge(const tt::math::Point2& p_position) const
	{
		return getHorizontalEdge(p_position);
	}
	
	inline const EdgeType& getVerticalEdge(const tt::math::Point2& p_position) const
	{
		if (p_position.x < 0 || p_position.x >= m_verticalSize.x ||
		    p_position.y < 0 || p_position.y >= m_verticalSize.y)
		{
			static EdgeType emptyEdge = EdgeType_None;
			return emptyEdge;
		}
		return m_verticalEdges[p_position.x + (p_position.y * m_verticalSize.x)];
	}
	inline const EdgeType& getBottomEdge(const tt::math::Point2& p_position) const
	{
		return getVerticalEdge(tt::math::Point2(p_position.x, p_position.y - 1));
	}
	inline const EdgeType& getTopEdge(const tt::math::Point2& p_position) const
	{
		return getVerticalEdge(p_position);
	}
	
	
	void update(const MaterialCache& p_layer);
	void handleLevelResized(s32 p_newLevelWidth, s32 p_newLevelHeight);
	
private:
	tt::math::Point2 m_horizontalSize;
	tt::math::Point2 m_verticalSize;
	EdgeType*        m_horizontalEdges;
	EdgeType*        m_verticalEdges;
	
	EdgeCache(const EdgeCache&);                  // Copy disabled
	const EdgeCache& operator=(const EdgeCache&); // Assigment disabled.
};

// Namespace end
}
}
}


#endif  // !defined(INC_TOKI_LEVEL_SKIN_EDGECACHE_H)
