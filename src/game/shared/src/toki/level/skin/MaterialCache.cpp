#include <tt/code/helpers.h>

#include <toki/level/skin/MaterialCache.h>
#include <toki/level/AttributeLayer.h>


namespace toki {
namespace level {
namespace skin {

MaterialCache::MaterialCache(s32 p_levelWidth, s32 p_levelHeight)
:
m_levelSize(p_levelWidth, p_levelHeight),
m_materialTiles(0)
{
	TT_ASSERT(m_levelSize.x > 0);
	TT_ASSERT(m_levelSize.y > 0);
	
	m_materialTiles = new TileMaterial[m_levelSize.x * m_levelSize.y];
}


MaterialCache::~MaterialCache()
{
	tt::code::helpers::safeDeleteArray(m_materialTiles);
}


void MaterialCache::update(const AttributeLayerPtr& p_layer, ThemeType p_defaultLevelTheme)
{
	TT_NULL_ASSERT(p_layer);
	TT_ASSERT(m_levelSize.x == p_layer->getWidth() );
	TT_ASSERT(m_levelSize.y == p_layer->getHeight());
	
	const u8* layerDataPtr        = p_layer->getRawData();
	TileMaterial* materialTilePtr = m_materialTiles;
	
	for (tt::math::Point2 pos(0, 0); pos.y < m_levelSize.y; ++pos.y)
	{
		for (pos.x = 0; pos.x < m_levelSize.x; ++pos.x, ++layerDataPtr, ++materialTilePtr)
		{
			const u8 value = *layerDataPtr;
			
			CollisionType    collisionType = level::getCollisionType(value);
			level::ThemeType themeType     = level::getThemeType(    value);
			
#if defined(TT_BUILD_DEV)
			// Asserts to make sure this usage for the raw data is correct.
			TT_ASSERT(collisionType   == p_layer->getCollisionType(pos));
			TT_ASSERT(themeType       == p_layer->getThemeType(pos));
			TT_ASSERT(materialTilePtr == &getTileMaterial(pos));
#endif
			
			materialTilePtr->set(collisionType, themeType, p_defaultLevelTheme);
		}
	}
}


void MaterialCache::handleLevelResized(s32 p_newLevelWidth, s32 p_newLevelHeight)
{
	if (p_newLevelWidth == m_levelSize.x && p_newLevelHeight == m_levelSize.y)
	{
		// Level size didn't change
		return;
	}
	
	m_levelSize.setValues(p_newLevelWidth, p_newLevelHeight);
	
	tt::code::helpers::safeDeleteArray(m_materialTiles);
	m_materialTiles = new TileMaterial[m_levelSize.x * m_levelSize.y];
}

// Namespace end
}
}
}
