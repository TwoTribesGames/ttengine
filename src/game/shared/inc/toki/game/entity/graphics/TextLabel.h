#if !defined(INC_TOKI_GAME_ENTITY_GRAPHICS_TEXTLABEL_H)
#define INC_TOKI_GAME_ENTITY_GRAPHICS_TEXTLABEL_H

#include <tt/engine/renderer/ColorRGBA.h>
#include <tt/engine/renderer/fwd.h>
#include <tt/str/str_types.h>

#include <toki/game/entity/graphics/fwd.h>
#include <toki/game/entity/graphics/types.h>
#include <toki/game/entity/fwd.h>
#include <toki/utils/GlyphSetMgr.h>


namespace toki {
namespace game {
namespace entity {
namespace graphics {

class TextLabel
{
public:
	struct CreationParams
	{
		inline CreationParams(entity::EntityHandle p_parentEntity,
		                      utils::GlyphSetID    p_glyphSet)
		:
		parentEntity(   p_parentEntity),
		glyphSet(       p_glyphSet)
		{ }
		
		entity::EntityHandle parentEntity;
		utils::GlyphSetID    glyphSet;
	};
	typedef const CreationParams& ConstructorParamType;
	
	TextLabel(const CreationParams&  p_creationParams,
	          const TextLabelHandle& p_ownHandle);
	
	void update(real p_elapsedTime);
	void updateForRender();
	void render() const;
	
	void setShowTextBorders(bool p_show);
	void setColor(const tt::engine::renderer::ColorRGBA& p_color);
	inline const tt::engine::renderer::ColorRGBA& getColor() const { return m_color; }
	void setText(const std::wstring& p_text);
	void setTextLocalized(const std::string& p_localizationKey);
	void setTextLocalizedAndFormatted(const std::string& p_localizationKey, const tt::str::Strings& p_vars);
	inline std::wstring getText() const { return m_renderedText; }
	inline s32 getTextLength() const { return static_cast<s32>(m_renderedText.size()); }
	inline s32 getNumberOfVisibleCharacters() const { return m_numberOfVisibleCharacters; }
	void setNumberOfVisibleCharacters(s32 p_numberOfVisibleCharacters);
	const tt::math::VectorRect& getOrCalculateUsedSize();
	void setSize(real p_width, real p_height);
	void setScale(real p_scale);
	inline void setVerticalAlignment(  VerticalAlignment   p_verticalAlignment)   { m_verticalAlignment   = p_verticalAlignment;   }
	inline void setHorizontalAlignment(HorizontalAlignment p_horizontalAlignment) { m_horizontalAlignment = p_horizontalAlignment; }
	
	// DropShadow methods
	void addDropShadow(const tt::math::Vector2& p_offset, const tt::engine::renderer::ColorRGBA& p_color);
	void removeDropShadow();
	inline bool hasDropShadow() const { return m_dropShadowQuad != 0; }
	inline const tt::engine::renderer::ColorRGBA& getDropShadowColor() const { return m_dropShadowColor; }
	inline tt::math::Vector2 getScaledDropShadowOffset() const { return m_dropShadowOffset * m_scale; }
	
	void fadeIn( real p_time);
	void fadeOut(real p_time);
	
	inline const TextLabelHandle& getHandle() const { return m_ownHandle; }
	
	void                  serializeCreationParams  (tt::code::BufferWriteContext* p_context) const;
	static CreationParams unserializeCreationParams(tt::code::BufferReadContext*  p_context);
	
	void serialize  (tt::code::BufferWriteContext* p_context) const;
	void unserialize(tt::code::BufferReadContext*  p_context);
	
	static TextLabel* getPointerFromHandle(const TextLabelHandle& p_handle);
	void invalidateTempCopy() { m_quad.reset(); }
	
	tt::engine::renderer::TexturePtr getTexture() const;
	inline void incRefPresentationOverlay()     { ++m_presentationOverlayRefCount; }
	inline void decRefPresentationOverlay()     { --m_presentationOverlayRefCount; TT_ASSERT(m_presentationOverlayRefCount >= 0); }
	inline bool isUsedAsPresentationOverlay() const { return m_presentationOverlayRefCount > 0; }
	
private:
	static const s32 TextPixelsPerWorldUnit = 64;
	
	TextLabelHandle m_ownHandle;
	CreationParams  m_creationParams;
	
	real                            m_width;
	real                            m_height;
	real                            m_scale;
	tt::engine::renderer::ColorRGBA m_color;
	std::wstring                    m_renderedText;
	VerticalAlignment               m_verticalAlignment;
	HorizontalAlignment             m_horizontalAlignment;
	s32                             m_presentationOverlayRefCount;
	s32                             m_numberOfVisibleCharacters;
	tt::math::Vector2               m_dropShadowOffset;
	tt::engine::renderer::ColorRGBA m_dropShadowColor;
	
	// Don't serialize the following.
	tt::engine::renderer::QuadSpritePtr m_quad;
	tt::engine::renderer::QuadSpritePtr m_dropShadowQuad;
	bool                                m_textNeedsRepaint;
	tt::math::VectorRect                m_usedSize;
	
#if !defined(TT_BUILD_FINAL)
	bool m_renderTextBorders;
#endif
};

// Namespace end
}
}
}
}


#endif  // !defined(INC_TOKI_GAME_ENTITY_GRAPHICS_TEXTLABEL_H)
