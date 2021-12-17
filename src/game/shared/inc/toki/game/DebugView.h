#if !defined(INC_TOKI_GAME_DEBUGVIEW_H)
#define INC_TOKI_GAME_DEBUGVIEW_H

#include <tt/engine/renderer/fwd.h>
#include <tt/engine/renderer/QuadBuffer.h>

#include <toki/game/event/helpers/fwd.h>
#include <toki/game/event/Event.h>
#include <toki/game/fwd.h>


namespace toki {
namespace game {

class DebugView
{
public:
	typedef std::vector<tt::math::Point2> Point2s;
	
	struct TextInfo
	{
		TextInfo(const tt::math::Vector2& p_position, const std::string& p_text, bool p_renderShadow,
		           const tt::engine::renderer::ColorRGB& p_color, real p_lifetime, bool p_worldSpace = false)
		:
		position(p_position),
		text(p_text),
		renderShadow(p_renderShadow),
		color(p_color),
		lifetime(p_lifetime),
		worldSpace(p_worldSpace)
		{
		}
		
		tt::math::Vector2 position;
		std::string text;
		bool renderShadow;
		tt::engine::renderer::ColorRGB color;
		real lifetime;
		bool worldSpace;
	};
	
	struct CircleInfo
	{
		CircleInfo(const tt::math::Vector2& p_position, real p_radius, bool p_isSolid,
		           const tt::engine::renderer::ColorRGBA& p_color, real p_lifetime)
		:
		position(p_position),
		radius(p_radius),
		isSolid(p_isSolid),
		color(p_color),
		lifetime(p_lifetime)
		{
		}
		
		tt::math::Vector2 position;
		real radius;
		bool isSolid;
		tt::engine::renderer::ColorRGBA color;
		real lifetime;
	};
	
	struct CirclePartInfo
	{
		CirclePartInfo(const tt::math::Vector2& p_position, real p_radius, real p_startAngle, real p_endAngle,
		               const tt::engine::renderer::ColorRGBA& p_color, real p_lifetime)
		:
		position(p_position),
		radius(p_radius),
		startAngle(p_startAngle),
		endAngle(p_endAngle),
		color(p_color),
		lifetime(p_lifetime)
		{
		}
		
		tt::math::Vector2 position;
		real radius;
		real startAngle;
		real endAngle;
		tt::engine::renderer::ColorRGBA color;
		real lifetime;
	};
	
	struct LineInfo
	{
		LineInfo(const tt::math::Vector2& p_sourcePosition, const tt::math::Vector2& p_targetPosition,
		         const tt::engine::renderer::ColorRGBA& p_color, real p_lifetime)
		:
		sourcePosition(p_sourcePosition),
		targetPosition(p_targetPosition),
		color(p_color),
		lifetime(p_lifetime)
		{
		}
		
		tt::math::Vector2 sourcePosition;
		tt::math::Vector2 targetPosition;
		tt::engine::renderer::ColorRGBA color;
		real lifetime;
	};


	struct EntityLineInfo
	{
		EntityLineInfo(const entity::EntityHandle& p_fromEntity, const entity::EntityHandle& p_toEntity,
			const tt::math::Vector2& p_fromOffset, const tt::math::Vector2& p_toOffset,
		    const tt::engine::renderer::ColorRGBA& p_color, real p_lifetime)
		:
		fromEntity(p_fromEntity),
		toEntity(p_toEntity),
		fromOffset(p_fromOffset),
		toOffset(p_toOffset),
		color(p_color),
		lifetime(p_lifetime)
		{
		}
		
		entity::EntityHandle fromEntity;
		entity::EntityHandle toEntity;
		tt::math::Vector2 fromOffset;
		tt::math::Vector2 toOffset;
		tt::engine::renderer::ColorRGBA color;
		real lifetime;
	};
	
	struct TileInfo
	{
		TileInfo(const Point2s& p_tilePositions,
		         s32 p_tileIndex, real p_lifetime)
		:
		tilePositions(p_tilePositions),
		tileIndex(p_tileIndex),
		lifetime(p_lifetime)
		{
		}
	
		Point2s tilePositions;
		s32 tileIndex;
		real lifetime;
	};
	
	/*! \return Null pointer in case of error, pointer to layer otherwise. */
	static DebugViewPtr create(const std::string& p_tileTextureName);
	
	~DebugView();
	
	void update(real p_deltaTime);
	void render() const;
	
	inline bool isVisible() const          { return m_visible;      }
	inline void setVisible(bool p_visible) { m_visible = p_visible; }
	
	void registerText(const TextInfo& p_textInfo);
	void registerCircle(const CircleInfo& p_circleInfo);
	void registerCirclePart(const CirclePartInfo& p_circlePartInfo);
	void registerLine(const LineInfo& p_lineInfo);
	void registerEntityLine(const EntityLineInfo& p_lineInfo);
	void registerTiles(const TileInfo& p_tileInfo);
	
	void clear();
	
	void renderText(const std::string& p_message, s32 p_x, s32 p_y,
	                const tt::engine::renderer::ColorRGB& p_color) const;
	
	void renderLine(const tt::engine::renderer::ColorRGBA& p_color,
	                const tt::math::Vector2& p_start, const tt::math::Vector2& p_end) const;
	
	void renderCircle(const tt::engine::renderer::ColorRGBA& p_color,
	                  const tt::math::Vector2& p_pos, real p_radius) const;
	
	inline void renderSolidCircle(const tt::engine::renderer::ColorRGBA& p_color, 
	                              const tt::math::Vector2& p_pos, real p_radius) const
	{
		renderSolidCircle(p_color, p_color, p_pos, p_radius);
	}
	
	void renderSolidCircle(const tt::engine::renderer::ColorRGBA& p_centerColor, 
	                       const tt::engine::renderer::ColorRGBA& p_edgeColor,
	                       const tt::math::Vector2& p_pos, real p_radius) const;
	
	void renderCirclePart(const tt::engine::renderer::ColorRGBA& p_color,
	                      const tt::math::Vector2& p_pos, real p_radius, real p_startAngle, real p_endAngle) const;
	
private:
	enum TexTile
	{
		TexTile_Width  = 32, //!< The width of one tile in the texture, in pixels
		TexTile_Height = 32  //!< The height of one tile in the texture, in pixels
	};
	
	DebugView(const tt::engine::renderer::TexturePtr& p_tileSet);
	
	tt::engine::renderer::BatchQuad createQuad(s32 p_x, s32 p_y, s32 p_tileIndex) const;
	
	// No copying
	DebugView(const DebugView&);
	DebugView& operator=(const DebugView&);
	
	typedef std::vector<TextInfo>       TextCollection;
	typedef std::vector<CircleInfo>     CircleCollection;
	typedef std::vector<CirclePartInfo> CirclePartCollection;
	typedef std::vector<LineInfo>       LineCollection;
	typedef std::vector<EntityLineInfo> EntityLineCollection;
	typedef std::vector<TileInfo>       TileCollection;
	
	TextCollection                   m_texts;
	CircleCollection                 m_circles;
	CirclePartCollection             m_circleParts;
	LineCollection                   m_lines;
	EntityLineCollection             m_entityLines;
	TileCollection                   m_tiles;
	
	bool                             m_visible;
	tt::engine::renderer::TexturePtr m_tileSet;       //!< The tile set.
	const s32                        m_tileSetTilesX; //!< The number of tiles in the tile set along the X axis.
	const s32                        m_tileSetTilesY; //!< The number of tiles in the tile set along the Y axis.
	const real                       m_tileTexW;      //!< Width of one tile in texture units.
	const real                       m_tileTexH;      //!< Height of one tile in texture units.
	
	tt::engine::renderer::QuadBufferPtr m_tileQuads;
	bool m_tilesDirty;
	
	static const s32 circlePrimitiveCount = 32; // (segments for solid = -2; outline = -1)
};

// Namespace end
}
}


#endif  // !defined(INC_TOKI_GAME_DEBUGVIEW_H)
