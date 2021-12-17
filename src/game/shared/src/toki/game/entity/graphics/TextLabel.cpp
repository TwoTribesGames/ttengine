#include <tt/code/bufferutils.h>
#include <tt/engine/glyph/GlyphSet.h>
#include <tt/engine/renderer/QuadSprite.h>
#include <tt/engine/renderer/TextureHardware.h>
#include <tt/engine/renderer/TexturePainter.h>
#include <tt/engine/renderer/Texture.h>
#include <tt/loc/LocStr.h>
#include <tt/str/StringFormatter.h>

#include <toki/game/entity/graphics/TextLabel.h>
#include <toki/game/entity/EntityMgr.h>
#include <toki/game/Game.h>
#include <toki/loc/Loc.h>
#include <toki/AppGlobal.h>


namespace toki {
namespace game {
namespace entity {
namespace graphics {

//--------------------------------------------------------------------------------------------------
// Public member functions

TextLabel::TextLabel(const CreationParams&  p_creationParams,
                     const TextLabelHandle& p_ownHandle)
:
m_ownHandle(p_ownHandle),
m_creationParams(p_creationParams),
m_width(0.0f),
m_height(0.0f),
m_scale(1.0f),
m_color(tt::engine::renderer::ColorRGB::black),
m_renderedText(),
m_verticalAlignment(VerticalAlignment_Top),
m_horizontalAlignment(HorizontalAlignment_Left),
m_presentationOverlayRefCount(0),
m_numberOfVisibleCharacters(-1),
m_dropShadowOffset(),
m_dropShadowColor(tt::engine::renderer::ColorRGB::black),
m_quad(),
m_dropShadowQuad(),
m_textNeedsRepaint(true),
m_usedSize()
#if !defined(TT_BUILD_FINAL)
, m_renderTextBorders(false)
#endif
{
	using namespace tt::engine::renderer;
	m_quad = QuadSprite::createQuad(TexturePtr(), m_color);
}


void TextLabel::update(real p_elapsedTime)
{
	(void)p_elapsedTime;
	// Update location based on parent entity.
	
	const entity::Entity* parent(m_creationParams.parentEntity.getPtr());
	if (parent != 0)
	{
		const tt::math::Vector2 pos(parent->getCenterPosition());
		
		if (m_quad != 0)
		{
			m_quad->setPosition(pos.x, -pos.y, 0.0f);
			
			if (m_dropShadowQuad != 0)
			{
				const tt::math::Vector2 dropShadowPos(pos + getScaledDropShadowOffset());
				m_dropShadowQuad->setPosition(dropShadowPos.x, -dropShadowPos.y, 0.0f);//(pos.x + m_dropShadowOffset.x, -(pos.y + m_dropShadowOffset.y), 0.0f);
			}
		}
	}
	
	if (m_textNeedsRepaint)
	{
		// render text
		
		if (m_quad == 0 || m_quad->getTexture() == 0)
		{
			return;
		}
		
		tt::engine::glyph::GlyphSetPtr glyphSet(utils::GlyphSetMgr::get(m_creationParams.glyphSet));
		TT_NULL_ASSERT(glyphSet);
		if (glyphSet == 0)
		{
			return;
		}
		
		tt::engine::renderer::TexturePtr tex(m_quad->getTexture());
		const tt::math::Point2 sizeInPixels(static_cast<s32>(m_width  * TextPixelsPerWorldUnit),
		                                    static_cast<s32>(m_height * TextPixelsPerWorldUnit));
		
		tt::engine::renderer::TexturePainter painter(tex->lock());
		painter.clear();
		const std::wstring text(m_renderedText.substr(0, m_numberOfVisibleCharacters));
		if (text.empty() == false)
		{
			using tt::engine::glyph::GlyphSet;
			GlyphSet::Alignment verticalAlignment   = GlyphSet::ALIGN_TOP;
			switch (m_verticalAlignment)
			{
			case VerticalAlignment_Top:    verticalAlignment = GlyphSet::ALIGN_TOP;    break;
			case VerticalAlignment_Center: verticalAlignment = GlyphSet::ALIGN_CENTER; break;
			case VerticalAlignment_Bottom: verticalAlignment = GlyphSet::ALIGN_BOTTOM; break;
			default: TT_PANIC("Unknown HorizontalAlignment: %d\n", m_verticalAlignment);
			}
			GlyphSet::Alignment horizontalAlignment = GlyphSet::ALIGN_LEFT;
			switch (m_horizontalAlignment)
			{
			case HorizontalAlignment_Left:   horizontalAlignment = GlyphSet::ALIGN_LEFT;   break;
			case HorizontalAlignment_Center: horizontalAlignment = GlyphSet::ALIGN_CENTER; break;
			case HorizontalAlignment_Right:  horizontalAlignment = GlyphSet::ALIGN_RIGHT;  break;
			default: TT_PANIC("Unknown HorizontalAlignment: %d\n", m_horizontalAlignment);
			}
			
			tt::math::Point2 margin(0,0);
			if (m_scale > 1.0f)
			{
				const tt::math::Vector2 sizeInPixelsVec(sizeInPixels);
				const tt::math::Vector2 marginVec( ((m_scale - 1.0f) * (sizeInPixelsVec / m_scale)) * 0.5f);
				
				margin = tt::math::Point2(marginVec);
			}
			
			const tt::math::PointRect usedPixels = glyphSet->drawMultiLineString(
					text,
					painter,
					tt::engine::renderer::ColorRGB::white,
					horizontalAlignment,
					verticalAlignment,
					0,
					margin.x,
					margin.y,
					margin.x + static_cast<s32>(tex->getWidth() - sizeInPixels.x),
					margin.y);
			
			// Convert used pixels to world units
			m_usedSize.setValues(
				tt::math::Vector2(static_cast<real>(usedPixels.getPosition().x) / TextPixelsPerWorldUnit,
					              static_cast<real>(usedPixels.getPosition().y) / TextPixelsPerWorldUnit),
				static_cast<real>(usedPixels.getWidth()) / TextPixelsPerWorldUnit,
				static_cast<real>(usedPixels.getHeight()) / TextPixelsPerWorldUnit
			);
		}
		
#if !defined(TT_BUILD_FINAL)
		if (m_renderTextBorders)
		{
			// Debug helper: render texture borders to find alignment issues
			for (s32 y = 0; y < painter.getTextureHeight(); ++y)
			{
				painter.setPixel(0,                             y, tt::engine::renderer::ColorRGB::white);
				painter.setPixel(painter.getTextureWidth() - 1, y, tt::engine::renderer::ColorRGB::white);
				painter.setPixel(sizeInPixels.x - 1,            y, tt::engine::renderer::ColorRGB::yellow);
			}
			for (s32 x = 0; x < painter.getTextureWidth(); ++x)
			{
				painter.setPixel(x, 0,                              tt::engine::renderer::ColorRGB::white);
				painter.setPixel(x, painter.getTextureHeight() - 1, tt::engine::renderer::ColorRGB::white);
				painter.setPixel(x, sizeInPixels.y - 1,             tt::engine::renderer::ColorRGB::yellow);
			}
		}
#endif
		
		m_textNeedsRepaint = false;
	}
	
	if (m_quad != 0)
	{
		m_quad->update();
		
		if (m_dropShadowQuad != 0)
		{
			m_dropShadowQuad->update();
		}
	}
}


void TextLabel::updateForRender()
{
	
}


void TextLabel::render() const
{
	if (m_quad != 0 && isUsedAsPresentationOverlay() == false)
	{
		if (m_dropShadowQuad != 0)
		{
			m_dropShadowQuad->render();
		}
		m_quad->render();
	}
}


void TextLabel::setShowTextBorders(bool p_show)
{
#if defined(TT_BUILD_FINAL)
	(void) p_show;
#else
	if (m_renderTextBorders == p_show)
	{
		return;
	}
	
	m_renderTextBorders = p_show;
	m_textNeedsRepaint  = true;
#endif
}


void TextLabel::setColor(const tt::engine::renderer::ColorRGBA& p_color)
{
	if (m_quad != 0)
	{
		m_quad->setColor(p_color);
	}
	m_color = p_color;
}


void TextLabel::setText(const std::wstring& p_text)
{
	if (p_text != m_renderedText)
	{
		m_renderedText              = p_text;
		m_textNeedsRepaint          = true;
		m_numberOfVisibleCharacters = static_cast<s32>(m_renderedText.size());
	}
}


void TextLabel::setTextLocalized(const std::string& p_localizationKey)
{
	if (AppGlobal::getLoc().hasLocStr(loc::SheetID_Game) == false)
	{
		TT_PANIC("Game localization sheet not loaded!");
		setText(tt::str::widen(p_localizationKey));
		return;
	}
	
	setText(AppGlobal::getLoc().getLocStr(loc::SheetID_Game).getString(p_localizationKey));
}


void TextLabel::setTextLocalizedAndFormatted(const std::string& p_localizationKey, const tt::str::Strings& p_vars)
{
	if (AppGlobal::getLoc().hasLocStr(loc::SheetID_Game) == false)
	{
		TT_PANIC("Game localization sheet not loaded!");
		setText(tt::str::widen(p_localizationKey));
		return;
	}
	
	tt::str::StringFormatter formatter(AppGlobal::getLoc().getLocStr(loc::SheetID_Game).getString(p_localizationKey));
	for (tt::str::Strings::const_iterator it = p_vars.begin(); it != p_vars.end(); ++it)
	{
		formatter << *it;
	}
	setText(formatter.getResult());
}


void TextLabel::setNumberOfVisibleCharacters(s32 p_numberOfVisibleCharacters)
{
	if (p_numberOfVisibleCharacters == m_numberOfVisibleCharacters)
	{
		return;
	}
	
	m_textNeedsRepaint = true;
	
	if (p_numberOfVisibleCharacters >= 0 && p_numberOfVisibleCharacters < static_cast<s32>(m_renderedText.size()))
	{
		m_numberOfVisibleCharacters = p_numberOfVisibleCharacters;
	}
	else
	{
		m_numberOfVisibleCharacters = static_cast<s32>(m_renderedText.size());
	}
}


const tt::math::VectorRect& TextLabel::getOrCalculateUsedSize()
{
	if (m_textNeedsRepaint)
	{
		update(0.0f);
	}
	TT_ASSERT(m_textNeedsRepaint == false);
	return m_usedSize;
}


void TextLabel::setSize(real p_width, real p_height)
{
	m_width  = p_width;
	m_height = p_height;
	
	if (m_quad != 0)
	{
		tt::math::Point2 sizeInPixels(static_cast<s32>(m_width  * TextPixelsPerWorldUnit),
		                              static_cast<s32>(m_height * TextPixelsPerWorldUnit));
		using namespace tt::engine::renderer;
		
		if (sizeInPixels.x <= 0 || sizeInPixels.y <= 0)
		{
			m_quad->setTexture(TexturePtr());
			if (m_dropShadowQuad != 0)
			{
				m_dropShadowQuad->setTexture(TexturePtr());
			}
			return;
		}
		
		TexturePtr tex(m_quad->getTexture());
		if (tex == 0 || tex->getWidth() < sizeInPixels.x || tex->getHeight() < sizeInPixels.y)
		{
			// Current texture isn't large enough for the quad: create a new one
			tex = Texture::createForText(sizeInPixels, true);
			m_quad->setTexture(tex);
			
			if (m_dropShadowQuad != 0)
			{
				m_dropShadowQuad->setTexture(tex);
			}
		}
		
		m_quad->setFrame (sizeInPixels.x, sizeInPixels.y);
		m_quad->setWidth (m_width  * m_scale);
		m_quad->setHeight(m_height * m_scale);
		
		if (m_dropShadowQuad != 0)
		{
			m_dropShadowQuad->setFrame (sizeInPixels.x, sizeInPixels.y);
			m_dropShadowQuad->setWidth (m_width  * m_scale);
			m_dropShadowQuad->setHeight(m_height * m_scale);
		}
		m_textNeedsRepaint = true;
	}
}


void TextLabel::setScale(real p_scale)
{
	m_scale = p_scale;
	setSize(m_width, m_height); // refresh sizes with new scale.
}


void TextLabel::addDropShadow(const tt::math::Vector2& p_offset, const tt::engine::renderer::ColorRGBA& p_color)
{
	if (m_quad != 0)
	{
		using namespace tt::engine::renderer;
		m_dropShadowQuad   = QuadSprite::createQuad(m_quad->getTexture(), p_color);
		m_dropShadowColor  = p_color;
		m_dropShadowOffset = p_offset;
	
		if (m_dropShadowQuad != 0)
		{
			// Force update, not the most elegant way though
			setSize(m_width, m_height);
		}
	}
}


void TextLabel::removeDropShadow()
{
	m_dropShadowQuad.reset();
}


void TextLabel::fadeIn( real p_time)
{
	if (m_quad != 0)
	{
		m_quad->fadeIn(p_time);
		
		if (m_dropShadowQuad != 0)
		{
			m_dropShadowQuad->fadeIn(p_time);
		}
	}
}


void TextLabel::fadeOut(real p_time)
{
	if (m_quad != 0)
	{
		m_quad->fadeOut(p_time);
		
		if (m_dropShadowQuad != 0)
		{
			m_dropShadowQuad->fadeOut(p_time);
		}
	}
}


void TextLabel::serializeCreationParams(tt::code::BufferWriteContext* p_context) const
{
	TT_NULL_ASSERT(p_context);
	
	namespace bu = tt::code::bufferutils;
	
	bu::putHandle(  m_creationParams.parentEntity   , p_context);
	bu::putEnum<u8>(m_creationParams.glyphSet       , p_context);
}


TextLabel::CreationParams TextLabel::unserializeCreationParams(
		tt::code::BufferReadContext* p_context)
{
	TT_NULL_ASSERT(p_context);
	
	namespace bu = tt::code::bufferutils;
	
	const entity::EntityHandle parentEntity    = bu::getHandle<entity::Entity     >(p_context);
	const utils::GlyphSetID    glyphSet        = bu::getEnum<u8, utils::GlyphSetID>(p_context);
	
	return CreationParams(parentEntity, glyphSet);
}


void TextLabel::serialize(tt::code::BufferWriteContext* p_context) const
{
	TT_NULL_ASSERT(p_context);
	
	namespace bu = tt::code::bufferutils;
	
	bu::put(m_width,                       p_context);
	bu::put(m_height,                      p_context);
	bu::put(m_scale,                       p_context);
	bu::put(m_color,                       p_context);
	bu::put(m_renderedText,                p_context);
	bu::putEnum<u8>(m_verticalAlignment  , p_context);
	bu::putEnum<u8>(m_horizontalAlignment, p_context);
	bu::put(m_presentationOverlayRefCount, p_context);
	bu::put(m_numberOfVisibleCharacters,   p_context);
	
	const bool dropShadow = hasDropShadow();
	bu::put(dropShadow,                    p_context);
	if (dropShadow)
	{
		bu::put(m_dropShadowOffset,        p_context);
		bu::put(m_dropShadowColor,         p_context);
	}
	
	bool visible = true;
	if (m_quad != 0)
	{
		using tt::engine::renderer::QuadSprite;
		visible = m_quad->checkFlag(QuadSprite::Flag_Visible) &&
		          m_quad->checkFlag(QuadSprite::Flag_FadingOut) == false;
	}
	bu::put(visible, p_context);
}


void TextLabel::unserialize(tt::code::BufferReadContext* p_context)
{
	TT_NULL_ASSERT(p_context);
	
	namespace bu = tt::code::bufferutils;
	
	m_width        = bu::get<real                           >(p_context);
	m_height       = bu::get<real                           >(p_context);
	m_scale        = bu::get<real                           >(p_context);
	m_color        = bu::get<tt::engine::renderer::ColorRGBA>(p_context);
	m_renderedText = bu::get<std::wstring                   >(p_context);
	m_verticalAlignment   = bu::getEnum<u8, VerticalAlignment  >(p_context);
	m_horizontalAlignment = bu::getEnum<u8, HorizontalAlignment>(p_context);
	m_presentationOverlayRefCount = bu::get<s32                >(p_context);
	m_numberOfVisibleCharacters   = bu::get<s32                >(p_context);
	
	const bool dropShadow = bu::get<bool>(p_context);
	if (dropShadow)
	{
		m_dropShadowOffset = bu::get<tt::math::Vector2>(p_context);
		m_dropShadowColor  = bu::get<tt::engine::renderer::ColorRGBA>(p_context);
		addDropShadow(m_dropShadowOffset, m_dropShadowColor);
	}
	
	const bool visible = bu::get<bool>(p_context);
	
	// Make sure to push all these values to m_quad.
	setSize(m_width, m_height);
	setColor(m_color);
	if (visible == false)
	{
		fadeOut(0.0f);
	}
	m_textNeedsRepaint = true;
}


TextLabel* TextLabel::getPointerFromHandle(const TextLabelHandle& p_handle)
{
	if (AppGlobal::hasGame()                 == false ||
	    AppGlobal::getGame()->hasEntityMgr() == false)
	{
		return 0;
	}
	
	return AppGlobal::getGame()->getEntityMgr().getTextLabelMgr().getTextLabel(p_handle);
}


tt::engine::renderer::TexturePtr TextLabel::getTexture() const
{
	return (m_quad != 0) ? m_quad->getTexture() : tt::engine::renderer::TexturePtr();
}


//--------------------------------------------------------------------------------------------------
// Private member functions


// Namespace end
}
}
}
}
