#if !defined(INC_TOKI_GAME_ATTRIBUTEDEBUGVIEW_H)
#define INC_TOKI_GAME_ATTRIBUTEDEBUGVIEW_H


#include <tt/engine/renderer/fwd.h>
#include <tt/engine/renderer/QuadBuffer.h>
#include <tt/math/Vector3.h>

#include <toki/game/fluid/types.h>
#include <toki/level/fwd.h>
#include <toki/level/TileChangedObserver.h>
#include <toki/level/types.h>


namespace toki {
namespace game {

class AttributeDebugView;
typedef tt_ptr<AttributeDebugView>::shared AttributeDebugViewPtr;


/*! \brief Visualizes the attribute layer. */
class AttributeDebugView : public level::TileChangedObserver
{
public:
	enum ViewMode
	{
		ViewMode_CollisionType,
		ViewMode_ThemeType,
		ViewMode_Fluids
	};
	
	
	/*! \return Null pointer in case of error, pointer to layer otherwise. */
	static AttributeDebugViewPtr create(const level::AttributeLayerPtr& p_attributes,
	                                    ViewMode                        p_mode,
	                                    const std::string&              p_tilesetID        = "attributeview",
	                                    const std::string&              p_tilesetNamespace = "textures");
	
	~AttributeDebugView();
	
	void update();
	void render();
	
	void setAttributeLayer(const level::AttributeLayerPtr& p_attributes);
	
	void setVertexColor(const tt::engine::renderer::ColorRGBA& p_color);
	
	inline bool isVisible() const          { return m_visible;      }
	inline void setVisible(bool p_visible) { m_visible = p_visible; }
	
	inline void                     setPosition(const tt::math::Vector3& p_pos) { m_position = p_pos; }
	inline const tt::math::Vector3& getPosition() const                         { return m_position;  }
	
	inline const tt::engine::renderer::TexturePtr& getTexture() const { return m_tileSet; }
	void setTexture(const tt::engine::renderer::TexturePtr& p_texture);
	
	virtual void onTileChange(const tt::math::Point2&) { } // don't care about tile changes
	virtual void onTileLayerDirty();
	
private:
	enum TexTile
	{
		TexTile_Width  = 32, //!< The width of one tile in the texture, in pixels
		TexTile_Height = 32  //!< The height of one tile in the texture, in pixels
	};
	
	typedef u8 (*TileIndexTranslator)(u8);
	
	
	AttributeDebugView(const level::AttributeLayerPtr&         p_layerData,
	                   const tt::engine::renderer::TexturePtr& p_tileSet,
	                   ViewMode                                p_mode);
	tt::engine::renderer::BatchQuad createQuadForTile(s32 p_x, s32 p_y, u8 p_tile) const;
	void rebuildQuadBuffer();
	
	static u8 getTileIndexCollisionType(u8 p_attrib);
	static u8 getTileIndexThemeType    (u8 p_attrib);
	static u8 getTileIndexFluids       (u8 p_attrib);
	
	// No copying
	AttributeDebugView(const AttributeDebugView&);
	AttributeDebugView& operator=(const AttributeDebugView&);
	
	
	tt_ptr<AttributeDebugView>::weak m_this;
	bool                             m_visible;
	const ViewMode                   m_viewMode;
	TileIndexTranslator              m_tileIndexFunc;
	tt::math::Vector3                m_position;
	tt::engine::renderer::TexturePtr m_tileSet;       //!< The tile set.
	level::AttributeLayerPtr         m_attribs;       //!< For attribute view: the attribute data.
	bool                             m_tilesAreDirty; //!< Whether the tiles have been changed (view needs to update).
	const s32                        m_tileSetTilesX; //!< The number of tiles in the tile set along the X axis.
	const s32                        m_tileSetTilesY; //!< The number of tiles in the tile set along the Y axis.
	const real                       m_tileTexW;      //!< Width of one tile in texture units.
	const real                       m_tileTexH;      //!< Height of one tile in texture units.
	bool                             m_bufferNeedsUpdate;
	
	tt::engine::renderer::ColorRGBA     m_vertexColor;
	tt::engine::renderer::QuadBufferPtr m_tileQuads;
};

// Namespace end
}
}


#endif  // !defined(INC_TOKI_GAME_ATTRIBUTEDEBUGVIEW_H)
