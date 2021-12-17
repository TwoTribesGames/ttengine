#include <tt/code/helpers.h>
#include <tt/engine/glyph/GlyphSet.h>
#include <tt/engine/renderer/QuadSprite.h>
#include <tt/engine/renderer/Texture.h>
#include <tt/engine/renderer/TextureHardware.h>
#include <tt/engine/renderer/TexturePainter.h>

#include <toki/level/Note.h>
#include <toki/utils/GlyphSetMgr.h>


namespace toki {
namespace level {

//--------------------------------------------------------------------------------------------------
// Public member functions

NotePtr Note::create(const tt::math::VectorRect& p_rect,
                     const std::wstring&         p_text,
                     VisualType                  p_visualType)
{
	return NotePtr(new Note(p_rect, p_text, p_visualType));
}


NotePtr Note::create(const tt::math::Vector2& p_pos,
                     const std::wstring&      p_text,
                     VisualType               p_visualType)
{
	// Calculate which size we'll need for the text.
	tt::engine::glyph::GlyphSetPtr glyphSet(utils::GlyphSetMgr::get(utils::GlyphSetID_Notes));
	const s32 maxWidthInTiles  = 7;
	const s32 maxWidthInPixels = maxWidthInTiles * pixelsInTiles;
	const s32  linesNeeded = glyphSet->getLineCount(p_text, maxWidthInPixels, textMargin, textMargin);
	const s32 pixelsNeeded = glyphSet->getMultiLinePixelHeight(linesNeeded) + (textMargin * 2);
	const real tilesNeeded = static_cast<real>(pixelsNeeded) / static_cast<real>(pixelsInTiles);
	
	return create(tt::math::VectorRect(p_pos, static_cast<real>(maxWidthInTiles), tilesNeeded),
	              p_text, p_visualType);
}


Note::~Note()
{
	destroyVisual();
}


void Note::update(real p_deltaTime)
{
	createVisual();
	m_visual->update(p_deltaTime, m_visualType);
}


void Note::render()
{
	createVisual();
	m_visual->render();
}


void Note::destroyVisual()
{
	tt::code::helpers::safeDelete(m_visual);
}


NotePtr Note::clone() const
{
	// Create a new note with the same data as this instance, but without the visual
	return NotePtr(new Note(m_worldRect, m_text, m_visualType));
}


void Note::setPosition(const tt::math::Vector2& p_pos)
{
	if (p_pos != m_worldRect.getPosition())
	{
		m_worldRect.setPosition(p_pos);
		handleNoteChanged(NoteVisual::Change_Pos);
	}
}


void Note::setWorldRect(const tt::math::VectorRect& p_rect)
{
	if (p_rect != m_worldRect)
	{
		if (p_rect.getPosition() != m_worldRect.getPosition())
		{
			handleNoteChanged(NoteVisual::Change_Pos);
		}
		
		if (tt::math::realEqual(p_rect.getWidth(),  m_worldRect.getWidth())  == false ||
		    tt::math::realEqual(p_rect.getHeight(), m_worldRect.getHeight()) == false )
		{
			handleNoteChanged(NoteVisual::Change_Size);
		}
		
		m_worldRect = p_rect;
	}
}


void Note::setText(const std::wstring& p_text)
{
	m_text = p_text;
	handleNoteChanged(NoteVisual::Change_Text);
}


//--------------------------------------------------------------------------------------------------
// Private member functions

Note::NoteVisual::NoteVisual(Note* p_note, VisualType p_visualType)
:
m_note(p_note),
m_changes(),
m_backgroundQuad(tt::engine::renderer::QuadSprite::createQuad(
		tt::engine::renderer::TextureCache::get((p_visualType == VisualType_Note) ? "note" : "warning", "textures.editor.ui"),
		tt::engine::renderer::ColorRGBA(255, 255, 255, 230))),
m_textTex(),
m_textQuad(tt::engine::renderer::QuadSprite::createQuad(
		tt::engine::renderer::TexturePtr(),
		tt::engine::renderer::ColorRGBA(255, 255, 255, 255))),
m_textAvailablePixelArea(0, 0),
m_editCaretVisible(false),
m_appendCaretToText(false),
m_caretToggleTime(0.0f)
{
	TT_NULL_ASSERT(m_note);
	
	// Mark everything as changed
	m_changes.setAllFlags();
}


void Note::NoteVisual::update(real p_deltaTime, VisualType p_visualType)
{
	// Handle the caret blinking "animation"
	bool textRefreshBecauseOfCaret = false;
	
	const bool caretVisibleNow = m_note->isEditCaretVisible();
	if (caretVisibleNow != m_editCaretVisible)
	{
		m_caretToggleTime   = 0.0f;
		m_appendCaretToText = caretVisibleNow;
		m_changes.setFlag(Change_Text);
		textRefreshBecauseOfCaret = true;
	}
	else if (caretVisibleNow)
	{
		m_caretToggleTime += p_deltaTime;
		if (m_caretToggleTime > 0.5f)
		{
			m_caretToggleTime   = 0.0f;
			m_appendCaretToText = (m_appendCaretToText == false);
			m_changes.setFlag(Change_Text);
			textRefreshBecauseOfCaret = true;
		}
	}
	
	m_editCaretVisible = caretVisibleNow;
	
	// Check if size requires updating
	if (m_changes.checkFlag(Change_Size))
	{
		m_changes.resetFlag(Change_Size);
		
		const tt::math::VectorRect& noteRect(m_note->getWorldRect());
		
		m_backgroundQuad->setWidth (noteRect.getWidth());
		m_backgroundQuad->setHeight(noteRect.getHeight());
		
		tt::math::Point2 texSize(static_cast<s32>(noteRect.getWidth()  * static_cast<real>(pixelsInTiles)),
		                         static_cast<s32>(noteRect.getHeight() * static_cast<real>(pixelsInTiles)));
		
		if (m_textTex == 0 || texSize != m_textAvailablePixelArea)
		{
			//TT_Printf("NoteVisual::update: Available text area: %d x %d\n",
			//          m_textAvailablePixelArea.x, m_textAvailablePixelArea.y);
			
			m_textAvailablePixelArea = texSize;
			
			texSize = tt::engine::renderer::TextureHardware::getRequirements().correctDimension(texSize);
			
			if (m_textTex == 0                     ||
			    texSize.x != m_textTex->getWidth() ||
			    texSize.y != m_textTex->getHeight())
			{
				//TT_Printf("NoteVisual::update: Creating texture of size %d x %d\n",
				//          texSize.x, texSize.y);
				m_textTex = tt::engine::renderer::Texture::createForText(
						static_cast<s16>(texSize.x), static_cast<s16>(texSize.y), true);
				m_textQuad->setTexture(m_textTex);
			}
			
			m_textQuad->setFrame(m_textAvailablePixelArea.x, m_textAvailablePixelArea.y);
			
			m_changes.setFlag(Change_Text);
		}
		
		m_textQuad->setWidth (noteRect.getWidth());
		m_textQuad->setHeight(noteRect.getHeight());
		
		m_changes.setFlag(Change_Pos);  // size change also means center point changed
	}
	
	// Check if position requires updating
	if (m_changes.checkFlag(Change_Pos))
	{
		m_changes.resetFlag(Change_Pos);
		
		const tt::math::Vector2& notePos(m_note->getWorldRect().getPosition());
		
		m_backgroundQuad->setPosition(  notePos.x + (m_backgroundQuad->getWidth()  * 0.5f),
		                              -(notePos.y + (m_backgroundQuad->getHeight() * 0.5f)),
		                              0.0f);
		m_textQuad->setPosition(  notePos.x + (m_textQuad->getWidth()  * 0.5f),
		                        -((notePos.y + m_note->getWorldRect().getHeight()) - (m_textQuad->getHeight() * 0.5f)),
		                        0.0f);
	}
	
	// Check if text requires updating
	if (m_changes.checkFlag(Change_Text))
	{
		m_changes.resetFlag(Change_Text);
		
		if (textRefreshBecauseOfCaret == false && caretVisibleNow)
		{
			// Typing causes the caret to become visible
			m_appendCaretToText = true;
			m_caretToggleTime   = 0.0f;
		}
		
		//TT_Printf("NoteVisual::update: Note text changed.\n");
		
		// (Re-)render the text
		TT_NULL_ASSERT(m_textTex);
		tt::engine::renderer::TexturePainter painter(m_textTex->lock());
		painter.clear();
		
		tt::engine::glyph::GlyphSetPtr glyphSet(utils::GlyphSetMgr::get(utils::GlyphSetID_Notes));
		TT_NULL_ASSERT(glyphSet);
		if (glyphSet != 0)
		{
			static const tt::engine::glyph::GlyphSet::Alignment alignHorz = tt::engine::glyph::GlyphSet::ALIGN_LEFT;
			static const tt::engine::glyph::GlyphSet::Alignment alignVert = tt::engine::glyph::GlyphSet::ALIGN_TOP;
			
			const std::wstring text(m_note->getText() + (m_appendCaretToText ? L"|" : L""));
			
			const s32 marginLeft  = textMargin;
			const s32 marginRight = (m_textTex->getWidth() - m_textAvailablePixelArea.x) + textMargin;
			
			/*
			// FIXME: Early out for too small text
			
			// FIXME: Switch from creating texture for note dimensions to creating texture based on
			//        requirements of text (with width limit)
			
			const s32 lineCount   = glyphSet->getLineCount(text, painter.getTextureWidth(), marginLeft, marginRight);
			const s32 pixelHeight = glyphSet->getMultiLinePixelHeight(lineCount, alignVert);
			
			TT_Printf("NoteVisual::update: Width %d, margin left %d, margin right %d: %d lines, %d pixels high\n",
			          painter.getTextureWidth(), marginLeft, marginRight, lineCount, pixelHeight);
			// */
			
			glyphSet->drawMultiLineString(
					text,
					painter,
					(p_visualType == VisualType_Note) ? tt::engine::renderer::ColorRGB::black : tt::engine::renderer::ColorRGB::white,
					alignHorz,
					alignVert,
					0,
					marginLeft,   // Left margin
					textMargin,       // Top margin
					marginRight,  // Right margin
					(m_textTex->getHeight() - m_textAvailablePixelArea.y) + textMargin); // Bottom margin
		}
	}
	
	m_backgroundQuad->update();
	m_textQuad->update();
}


void Note::NoteVisual::render()
{
	m_backgroundQuad->render();
	m_textQuad->render();
}


void Note::NoteVisual::handleNoteChanged(Change p_change)
{
	m_changes.setFlag(p_change);
}


Note::Note(const tt::math::VectorRect& p_rect,
           const std::wstring&         p_text,
           VisualType                  p_visualType)
:
m_worldRect(p_rect),
m_text(p_text),
m_visual(0),
m_visualType(p_visualType),
m_editCaretVisible(false)
{
}


void Note::createVisual()
{
	if (m_visual == 0)
	{
		m_visual = new NoteVisual(this, m_visualType);
	}
}


void Note::handleNoteChanged(NoteVisual::Change p_change)
{
	if (m_visual != 0)
	{
		m_visual->handleNoteChanged(p_change);
	}
}

// Namespace end
}
}
