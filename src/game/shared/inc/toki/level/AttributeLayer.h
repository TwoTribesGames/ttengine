#if !defined(INC_TOKI_LEVEL_ATTRIBUTELAYER_H)
#define INC_TOKI_LEVEL_ATTRIBUTELAYER_H


#include <vector>

#include <tt/math/Point2.h>
#include <tt/math/Rect.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_types.h>

#include <toki/game/fluid/types.h>
#include <toki/level/fwd.h>
#include <toki/level/types.h>


namespace toki {
namespace level {

class AttributeLayer;
typedef tt_ptr<AttributeLayer>::shared AttributeLayerPtr;

/*! \brief Manages attribute data for an entire layer. */
class AttributeLayer
{
public:
	static AttributeLayerPtr create(s32 p_width, s32 p_height);
	~AttributeLayer();
	
	/*! \brief Reverses the rows of tile data (flipping the level along the Y axis). */
	void flipRows();
	
	/*! \brief Clears the attribute layer by setting all tiles to 0. */
	void clear();
	
	/*! \param p_newRect New rectangle for the layer, relative to the existing data. */
	void resizeTo(const tt::math::PointRect& p_newRect);
	
	/*! \brief Swap the contents of this object with another one. */
	void swap(const AttributeLayerPtr& p_other);
	
	/*! \brief Returns whether this object has the same content as another one */
	bool equals(const AttributeLayerPtr& p_other) const;
	
	AttributeLayerPtr clone() const;
	
	inline s32 getLength() const { return m_width * m_height; }
	inline s32 getWidth()  const { return m_width;            }
	inline s32 getHeight() const { return m_height;           }
	
	/*! \brief Indicates whether the specified tile position is within the layer boundaries. */
	inline bool contains(const tt::math::Point2& p_pos) const
	{
		return p_pos.x >= 0 && p_pos.x < m_width && p_pos.y >= 0 && p_pos.y < m_height;
	}
	
	inline const u8* getRawData() const { return m_attributes; }
	inline u8*       getRawData()       { return m_attributes; }
	
	// Convenience functions for attribute checks
	
	inline CollisionType getCollisionType(const tt::math::Point2& p_pos) const
	{ return level::getCollisionType(getAttribute(p_pos)); }
	
	inline bool isSolid(const tt::math::Point2& p_pos) const
	{
		// Always consider positions outside the layer solid
		if (contains(p_pos) == false) return true;
		return level::isAttributeSolid(getAttribute(p_pos));
	}
	
	void setCollisionType(const tt::math::Point2& p_pos, CollisionType p_type);
	
	
	inline ThemeType getThemeType(const tt::math::Point2& p_pos) const
	{ return level::getThemeType(getAttribute(p_pos)); }
	void setThemeType(const tt::math::Point2& p_pos, ThemeType p_type);
	
	
	inline game::fluid::FluidType getFluidType(const tt::math::Point2& p_pos) const
	{
		if (contains(p_pos) == false)
		{
			TT_PANIC("Tile coordinates (%d, %d) out of bounds (max (%d, %d))!",
			         p_pos.x, p_pos.y, m_width - 1, m_height - 1);
			return game::fluid::FluidType_Invalid;
		}
		return game::fluid::getFluidType(getAttribute(p_pos));
	}
	
	
	inline void setFluidType(const tt::math::Point2& p_pos, game::fluid::FluidType p_type)
	{
		if (contains(p_pos) == false)
		{
			TT_PANIC("Tile coordinates (%d, %d) out of bounds (max (%d, %d))!",
			         p_pos.x, p_pos.y, m_width - 1, m_height - 1);
			return;
		}
		
		const s32 index = (p_pos.y * m_width) + p_pos.x;
		game::fluid::setFluidType(m_attributes[index], p_type);
		makeDirty();
	}
	
	
	inline game::fluid::FluidFlowType getFluidFlowType(const tt::math::Point2& p_pos) const
	{
		if (contains(p_pos) == false)
		{
			TT_PANIC("Tile coordinates (%d, %d) out of bounds (max (%d, %d))!",
			         p_pos.x, p_pos.y, m_width - 1, m_height - 1);
			return game::fluid::FluidFlowType_Invalid;
		}
		return game::fluid::getFluidFlowType(getAttribute(p_pos));
	}
	
	
	inline void setFluidFlowType(const tt::math::Point2& p_pos, game::fluid::FluidFlowType p_type)
	{
		if (contains(p_pos) == false)
		{
			TT_PANIC("Tile coordinates (%d, %d) out of bounds (max (%d, %d))!",
			         p_pos.x, p_pos.y, m_width - 1, m_height - 1);
			return;
		}
		
		const s32 index = (p_pos.y * m_width) + p_pos.x;
		game::fluid::setFluidFlowType(m_attributes[index], p_type);
		makeDirty();
	}
	
	inline bool isWarpTile(const tt::math::Point2& p_pos)
	{
		return game::fluid::isWarpTile(getAttribute(p_pos));
	}
	
	inline void setWarpTile(const tt::math::Point2& p_pos, bool p_isWarpTile)
	{
		if (contains(p_pos) == false)
		{
			TT_PANIC("Tile coordinates (%d, %d) out of bounds (max (%d, %d))!",
			         p_pos.x, p_pos.y, m_width - 1, m_height - 1);
			return;
		}
		
		game::fluid::setWarpTile(m_attributes[(p_pos.y * m_width) + p_pos.x], p_isWarpTile);
		makeDirty();
	}
	
	void registerObserver  (const TileChangedObserverPtr& p_observer);
	void unregisterObserver(const TileChangedObserverPtr& p_observer);
	inline void clearObservers() { m_onTileChangedObservers.clear(); }
	
	// Debugging
	void print() const;
	
private:
	/*! \brief Returns the attribute at the specified tile position. */
	inline u8 getAttribute(const tt::math::Point2& p_pos) const
	{
		// Higher level code must make sure that it doesn't reference tiles outside the layer
		TT_ASSERTMSG(contains(p_pos), "Tile coordinates (%d, %d) out of bounds (max (%d, %d))!",
			         p_pos.x, p_pos.y, m_width - 1, m_height - 1);

		return m_attributes[(p_pos.y * m_width) + p_pos.x];
	}
	
	AttributeLayer(s32 p_width, s32 p_height);
	
	void makeDirty();
	void notifyTileChanged(const tt::math::Point2& p_tilePos);
	
	// No copying
	AttributeLayer(const AttributeLayer&);
	AttributeLayer& operator=(const AttributeLayer&);
	
	
	typedef std::vector<TileChangedObserverWeakPtr> TileChangedObservers;
	
	u8*  m_attributes; //!< The attribute tiles in the layer.
	s32  m_width;      //!< The width of the attribute layer, in tiles.
	s32  m_height;     //!< The height of the attribute layer, in tiles.
	TileChangedObservers m_onTileChangedObservers; //!< The onTileChange observers
};

// Namespace end
}
}


#endif  // !defined(INC_TOKI_LEVEL_ATTRIBUTELAYER_H)
