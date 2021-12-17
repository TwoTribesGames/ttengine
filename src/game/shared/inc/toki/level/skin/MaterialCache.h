#if !defined(INC_TOKI_LEVEL_SKIN_MATERIALCACHE_H)
#define INC_TOKI_LEVEL_SKIN_MATERIALCACHE_H

#include <tt/math/Point2.h>

#include <toki/level/skin/fwd.h>
#include <toki/level/skin/types.h>
#include <toki/level/skin/TileMaterial.h>
#include <toki/level/fwd.h>

namespace toki {
namespace level {
namespace skin {


class MaterialCache
{
public:
	MaterialCache(s32 p_levelWidth, s32 p_levelHeight);
	~MaterialCache();
	
	void update(const AttributeLayerPtr& p_layer, ThemeType p_defaultLevelTheme);
	void handleLevelResized(s32 p_newLevelWidth, s32 p_newLevelHeight);
	
	inline const TileMaterial& getTileMaterial(const tt::math::Point2& p_position) const
	{
		if (p_position.x < 0 || p_position.x >= m_levelSize.x ||
		    p_position.y < 0 || p_position.y >= m_levelSize.y)
		{
			static TileMaterial emptyTile;
			return emptyTile;
		}
		return m_materialTiles[p_position.x + (p_position.y * m_levelSize.x)];
	}
	
	inline TileMaterial& getTileMaterial(const tt::math::Point2& p_position)
	{
		if (p_position.x < 0 || p_position.x >= m_levelSize.x ||
		    p_position.y < 0 || p_position.y >= m_levelSize.y)
		{
			static TileMaterial emptyTile;
			return emptyTile;
		}
		return m_materialTiles[p_position.x + (p_position.y * m_levelSize.x)];
	}
	
	inline const tt::math::Point2& getSize() const { return m_levelSize; }
	
	inline const TileMaterial* getRawTiles() const { return m_materialTiles; }
	inline const TileMaterial* getRawTiles(const tt::math::Point2& p_position) const
	{
		return &getTileMaterial(p_position);
	}
	
private:
	tt::math::Point2 m_levelSize;
	TileMaterial*    m_materialTiles;
	
	MaterialCache(const MaterialCache&);                  // Copy disabled
	const MaterialCache& operator=(const MaterialCache&); // Assigment disabled.
};

// Namespace end
}
}
}


#endif  // !defined(INC_TOKI_LEVEL_SKIN_MATERIALCACHE_H)
