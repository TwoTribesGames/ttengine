#include <limits>

#include <tt/code/Buffer.h>
#include <tt/code/BufferReadContext.h>
#include <tt/code/bufferutils.h>
#include <tt/engine/glyph/CustomGlyph.h>
#include <tt/engine/glyph/Glyph.h>
#include <tt/engine/glyph/GlyphAlpha4.h>
#include <tt/engine/glyph/GlyphAlpha8.h>
#include <tt/engine/glyph/GlyphSet.h>
#include <tt/engine/renderer/Texture.h>
#include <tt/engine/renderer/TexturePainter.h>
#include <tt/engine/renderer/ColorRGBA.h>
#include <tt/fs/File.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>
#include <tt/str/str.h>
#include <tt/xml/XmlDocument.h>
#include <tt/xml/XmlNode.h>


namespace tt {
namespace engine {
namespace glyph {

GlyphSet::GlyphSet(const std::string& p_filename,
                   u16                p_characterSpacing,
                   u16                p_wordSeparatorWidth,
                   u16                p_verticalSpacing)
:
m_numGlyphs(0),
m_heightOfCapitalX(0),
m_baseline(0),
m_ascenderHeight(0),
m_descenderHeight(0),
m_pointSize(0),
m_bitDepth(0),
m_charSpacing(p_characterSpacing),
m_verticalSpacing(p_verticalSpacing),
m_wordseparatorWidth(p_wordSeparatorWidth),
m_unknownGlyphUnicode(0x003F), // defaulting to question-mark
m_glyphs(),
m_customGlyphs(),
m_spaceChars(),
m_kerningPairs()
{
	tt::fs::FilePtr file = tt::fs::open(p_filename, tt::fs::OpenMode_Read);
	if (file == 0)
	{
		TT_PANIC("Unable to open file '%s'", p_filename.c_str());
		return;
	}
	const tt::fs::size_type fileSize(file->getLength());
	tt::code::BufferPtrForCreator buffer(new tt::code::Buffer(fileSize));
	if (tt::fs::read(file, buffer->getData(), fileSize) != fileSize)
	{
		TT_PANIC("Failed to read from '%s'.", p_filename.c_str());
		return;
	}
	
	tt::code::BufferReadContext context(buffer->getReadContext());
	
	if (constructFromData(&context) == false)
	{
		// Panics are be triggered in the constructFromData method
		return;
	}
	
	// The unknown char /must/ have a glyph in the GlyphSet.
	// If it is absent, warn and set the unknown char to space
	GlyphMap::const_iterator iter =
			m_glyphs.find(getUnknownGlyphCharacterCode());
	
	if (iter == m_glyphs.end())
	{
		TT_PANIC("The glyphset constructed from file '%s' does not contain the "
		         "default unknown-char glyph 0x%04x. Unknown characters "
		         "will be rendered as space!",
		         p_filename.c_str(),
		         getUnknownGlyphCharacterCode());
		
		setUnknownGlyphCharacterCode(L' ');
	}
	
	// Add non-breaking space as space character. (work around auto-kerning code.)
	// http://en.wikipedia.org/wiki/Non-breaking_space#Encodings
	addSpaceChar(0x00A0);
}


GlyphSet::~GlyphSet()
{
	// Clean house
	for (GlyphMap::iterator pos = m_glyphs.begin(); pos != m_glyphs.end(); ++pos)
	{
		delete (*pos).second;
	}
}


/**
 * Create glyphset
 *
 * @param file
 */
bool GlyphSet::constructFromData(tt::code::BufferReadContext* p_readContext)
{
	if (p_readContext == 0)
	{
		return false;
	}
	
	namespace bu = tt::code::bufferutils;
	
	// Read num of glyphs and pointsize
	m_numGlyphs = bu::get<s16>(p_readContext);
	m_pointSize = bu::get<u16>(p_readContext);
	m_bitDepth  = bu::get<u16>(p_readContext);
	
	m_heightOfCapitalX = 0;
	m_baseline         = 0;
	m_ascenderHeight   = 0;
	m_descenderHeight  = 0;
	
	// Iterate and create glyph
	for (u16 c = 0; c < m_numGlyphs; ++c)
	{
		Glyph* curr = 0L;
		switch (m_bitDepth)
		{
		case 4:
			// FIXME: Add error checking to GlyphAlpha4 constructor
			curr = (new GlyphAlpha4(p_readContext));
			break;
			
		case 8:
			// FIXME: Add error checking to GlyphAlpha4 constructor
			curr = (new GlyphAlpha8(p_readContext));
			break;
			
		default:
			TT_PANIC("Attempt to load GlyphSet with unknown depth!");
			return false;
		}
		
		m_glyphs.insert(GlyphMap::value_type(curr->getChar(), curr));
		
		// When seeing the X character, assign its height to class member
		if (curr->getChar() == L'X')
		{
			TT_ASSERTMSG(curr->getAscenderHeight() >= 0, "Ascender Height should be >= 0; not %d", curr->getAscenderHeight());
			m_heightOfCapitalX = static_cast<u16>(curr->getAscenderHeight()); // was: curr->getHeight();
			m_baseline         = curr->getDescenderHeight();
		}
		
		if (curr->getAscenderHeight() > m_ascenderHeight)
		{
			m_ascenderHeight = curr->getAscenderHeight();
		}
		
		if (curr->getDescenderHeight() < m_descenderHeight)
		{
			m_descenderHeight = curr->getDescenderHeight();
		}
	}
	
	// Handle case where the font /has/ no capital X
	if (m_heightOfCapitalX == 0)
	{
		m_heightOfCapitalX = m_pointSize;
		TT_WARN("Glyphset does not contain glyph for Unicode 'X', defaulting "
		        "font height to be identical to pointsize.");
	}
	
	return true;
}


/**
 * Draw a string to memory organized as bytes
 *
 * The font pixel colors are organized like this:
 *
 * color 1: darkest glyph pixel
 * color 15: brightest glyph pixel
 *
 * @param p_string      unicode encoded string
 * @param p_x           X coordinate
 * @param p_y           Y coordinate of /BASELINE/ !
 * @param p_framebuffer pointer to framebuffer memory
 * @param p_fwidth      width of framebuffer in pixels
 * @param p_fheight     height of framebuffer in pixels
 * @param p_paloff      palette offset
 */
void GlyphSet::drawString( 
		const std::wstring& p_string,
		s32 p_x,
		s32 p_y,
		renderer::TexturePainter& p_painter,
		const renderer::ColorRGBA& p_color,
		s32 p_strlen)
const
{
	// Filter the string for custom glyphs
	std::wstring filtered(getCustomGlyphFilteredString(p_string));
	
	// Forward to filtered version
	drawFilteredString(
			filtered.c_str(), 
			p_x, 
			p_y, 
			p_painter,
			p_color,
			p_strlen);
}


/**
 * Translate from unicode to internal Glyph object
 *
 * @param p_char unicode char
 *
 * @return if found, Glyph object, or 0L if space
 */
Glyph* GlyphSet::getGlyph(wchar_t p_char) const
{
	if (p_char == ' ' ||
	    m_spaceChars.find(p_char) != m_spaceChars.end())
	{
		return 0;
	}
	
	GlyphMap::const_iterator iter = m_glyphs.find(p_char);

	if(iter != m_glyphs.end())
	{
		return iter->second;
	}

	// NOTE: Duplicated code path for unknown glyph
	//       previous recursive version could loop infinitely

	if (getUnknownGlyphCharacterCode() == ' ') return 0;
	
	iter = m_glyphs.find(getUnknownGlyphCharacterCode());

	if(iter != m_glyphs.end())
	{
		return iter->second;
	}

	return 0;
}


/**
 * Set character spacing
 *
 * @param p_spacing spacing
 */
void GlyphSet::setSpacing(u16 p_spacing)
{
	m_charSpacing = p_spacing;
}


//! \brief Set line spacing
void GlyphSet::setVerticalSpacing(u16 p_vertSpacing)
{
	m_verticalSpacing = p_vertSpacing;
}


/**
 * Read pixel width of string
 *
 * @param p_string
 *
 * @return
 */
s32 GlyphSet::getStringPixelWidth(const std::wstring& p_string, s32 p_len) const
{
	// Filter the strings for custom glyphs
	std::wstring filtered = getCustomGlyphFilteredString( p_string );
	
	// Forward to filtered version
	return getFilteredStringPixelWidth(filtered.c_str(), p_len);
}


math::Point2 GlyphSet::getStringPixelDimensions(const std::wstring& p_string, s32 p_len) const
{
	// Filter the strings for custom glyphs
	std::wstring filtered = getCustomGlyphFilteredString( p_string );
	
	// Forward to filtered version
	return math::Point2(getFilteredStringPixelWidth(filtered.c_str(), p_len),
	                    m_ascenderHeight - m_descenderHeight);
}


/**
 * Set pixel width of word separator (space)
 *
 * @param p_newwidth new width of space char in pixels
 */
void GlyphSet::setWordSeparatorWidth(u16 p_newwidth)
{
	m_wordseparatorWidth = p_newwidth;
}


tt::math::PointRect GlyphSet::drawMultiLineString(
		const std::wstring& p_string,
		renderer::TexturePainter& p_painter,
		const renderer::ColorRGBA& p_color,
		Alignment p_horzAlign,
		Alignment p_vertAlign,
		s32 p_bottomlineOffset,
		s32 p_leftMargin,
		s32 p_topMargin,
		s32 p_rightMargin,
		s32 p_bottomMargin )
const
{
	// Start Debug
#if 0
	{
		TT_Printf("GlyphSet::drawMultiLineString - str: '%s', p_bottomlineOffset: %d, p_leftMargin: %d, "
		          "p_topMargin: %d, p_rightMargin: %d, p_bottomMargin: %d\n",
		          tt::str::narrow(p_string).c_str(), p_bottomlineOffset, p_leftMargin, 
		          p_topMargin, p_rightMargin, p_bottomMargin);
		
		const s32 width = p_painter.getTextureWidth();
		const s32 widthBorder = width - 1;
		const s32 height = p_painter.getTextureHeight();
		const s32 heightBorder = height - 1;
		for (s32 i = 0; i < width; ++i)
		{
			p_painter.setPixel(i, 0,            tt::engine::renderer::ColorRGB::white);
			p_painter.setPixel(i, heightBorder, tt::engine::renderer::ColorRGB::white);
		}
		for (s32 i = 0; i < height; ++i)
		{
			p_painter.setPixel(0,           i, tt::engine::renderer::ColorRGB::white);
			p_painter.setPixel(widthBorder, i, tt::engine::renderer::ColorRGB::white);
		}
		const float stepY = (float)height / (float)width;
		
		for (s32 x = 0; x < width / 2; ++x)
		{
			s32 y = s32(float(stepY * x));
			p_painter.setPixel(x,               y,                tt::engine::renderer::ColorRGB::white);
			p_painter.setPixel(widthBorder - x, y,                tt::engine::renderer::ColorRGB::white);
			p_painter.setPixel(x,               heightBorder - y, tt::engine::renderer::ColorRGB::white);
			p_painter.setPixel(widthBorder - x, heightBorder - y, tt::engine::renderer::ColorRGB::white);
		}
		
		p_painter.setPixel(width / 2, height / 2, tt::engine::renderer::ColorRGB::white);
	}
#endif
	// End debug
	
	// Box in which may be drawn.
	const s32 box_width  = p_painter.getTextureWidth()  - (p_leftMargin + p_rightMargin);
	const s32 box_height = p_painter.getTextureHeight() - (p_topMargin  + p_bottomMargin);
	
	TT_ASSERTMSG(box_width > 0,
	             "No width remaining for text (%d)! Full width: %d. Left margin: %d. Right margin: %d.",
	             box_width, p_painter.getTextureWidth(), p_leftMargin, p_rightMargin);
	TT_ASSERTMSG(box_height > 0,
	             "No height remaining for text (%d)! Full height: %d. Top margin: %d. Bottom margin: %d.",
	             box_height, p_painter.getTextureHeight(), p_topMargin, p_bottomMargin);
	if (box_width <= 0 || box_height <= 0)
	{
		return tt::math::PointRect(tt::math::Point2::zero, 0, 0);
	}
	
	
	tt::math::PointRect usedPixels(tt::math::Point2::zero, 0, 0); // Rect in which all the text pixels fit.
	usedPixels.alignLeft(static_cast<s32>(p_painter.getTextureWidth()));
	
	const std::wstring filtered(getCustomGlyphFilteredString(p_string));
	
	
	// How many lines are needed for this text?
	s32 lines = getTextHeight(
			filtered.c_str(),
			static_cast<s32>(filtered.length()),
			p_horzAlign,
			p_bottomlineOffset,
			p_painter.getTextureWidth(),
			p_painter.getTextureHeight(),
			p_leftMargin,
			p_topMargin,
			p_rightMargin,
			p_bottomMargin);
	
	// A line gets the height of a capital X and between we need vertical spacing.
	s32 pixelHeightOfOneLine = (m_heightOfCapitalX + m_verticalSpacing);
	// For multiple lines we have 1 less verticalSpacing than lines.
	s32 height = (pixelHeightOfOneLine * lines) - m_verticalSpacing;
	
	// Start and end for the characters.
	s32 startindex = 0;
	s32 endindex   = static_cast<s32>(filtered.length() + 1);
	
	// The top of the current line
	s32 top = 0;
	
	//TT_Printf("GlyphSet::drawMultiLineString - lines: %d, pixelHeightOfOneLine: %d, height: %d, "
	//          "startindex: %d, endindex: %d, top: %d, box_width: %d, box_height: %d\n",
	//          lines, pixelHeightOfOneLine, height, startindex, endindex, top, box_width, box_height);
	
	TT_ASSERTMSG(p_bottomlineOffset != 4096,
	             "Legacy 'bottom line offset' value (4096) passed. This should no longer be used.");
	
	// If we add extra space at the top, the start point for drawing needs to be offset.
	s16 startOffset = 0;
	
	// Special case ignore for a single centered line.
	if (p_vertAlign == ALIGN_CENTER && lines == 1)
	{
		// For single line centered text we center around the top and bottom pixel of a capital X.
		height -= getBaseline() + 1;
	}
	else
	{
		// Take into account that some characters have pixels above 'X'.
		s16 extraHight = static_cast<s16>(getAscenderHeight() - m_heightOfCapitalX);
		if (extraHight > 0)
		{
			height      += extraHight;
			startOffset += extraHight;
		}
		// Take into account that some characters have pixels below the baseline.
		if (getDescenderHeight() < 0)
		{
			height -= getDescenderHeight();
		}
	}
	
	switch (p_vertAlign)
	{
	case ALIGN_TOP:
		{
			//TT_Printf("GlyphSet::drawMultiLineString - ALIGN_TOP - "
			//          "p_bottomlineOffset: %d, m_heightOfCapitalX: %d, getTopMostPixel(): %d\n",
			//          p_bottomlineOffset, m_heightOfCapitalX,  getTopMostPixel());
			//p_bottomlineOffset += m_heightOfCapitalX;
		}
		break;
		
	case ALIGN_CENTER:
		{
			//TT_Printf("GlyphSet::drawMultiLineString - ALIGN_CENTER (vert) - "
			//          "p_bottomlineOffset: %d, box_height: %d, height: %d, m_heightOfCapitalX: %d, getAscenderHeight(): %d\n",
			//          p_bottomlineOffset, box_height, height, m_heightOfCapitalX, getAscenderHeight());
			p_bottomlineOffset += ((box_height >> 1) - (height >> 1));// + m_heightOfCapitalX;
		}
		break;
		
	case ALIGN_BOTTOM:
		{
			//TT_Printf("GlyphSet::drawMultiLineString - ALIGN_BOTTOM - "
			//          "p_bottomlineOffset: %d, box_height: %d, height: %d, m_heightOfCapitalX: %d, getTopMostPixel(): %d\n",
			//          p_bottomlineOffset, box_height, height, m_heightOfCapitalX, getTopMostPixel());
			
			p_bottomlineOffset += (box_height - height);// + m_heightOfCapitalX;
		}
		break;
		
	default:
		TT_PANIC("Unknown/unsupported vertical text alignment: %d", p_vertAlign);
		break;
	}
	
#if 0
	{
		TT_ASSERT(top == 0);
		s32 topLine    = p_topMargin + p_bottomlineOffset;
		s32 bottomLine = topLine + height;
		s32 width = p_painter.getTextureWidth();
		for (s32 i = 0; i < width; ++i)
		{
			p_painter.setPixel(i, topLine,    tt::engine::renderer::ColorRGB::white);
			p_painter.setPixel(i, bottomLine, tt::engine::renderer::ColorRGB::white);
		}
	}
#endif
	
	usedPixels.setHeight(height);
	usedPixels.alignTop(p_topMargin + p_bottomlineOffset);
	
	
	p_bottomlineOffset += m_heightOfCapitalX + startOffset;
	
	bool           done       = false;
	const wchar_t* filter_buf = filtered.c_str();
	
	while (done == false)
	{
		const wchar_t* str = filter_buf + startindex;
		s32 alignment = 0;
		s32 len       = endindex - startindex;
		
		bool spacefound   = false;
		bool newlinefound = false;
		
		s32 newlinepos = 0;
		while (newlinepos < len && str[newlinepos] != L'\n')
		{
			newlinepos++;
		}
		
		if (newlinepos < len)
		{
			len          = newlinepos;
			newlinefound = true;
		}
		
		s32 width = getFilteredStringPixelWidth(str, len);
		
		if (width <= box_width)
		{
			if (newlinefound == false)
			{
				done = true;
			}
		}
		else
		{
			s32 newlen = 0;
			width = getFilteredStringPixelWidth(str, newlen);
			while (width <= box_width)
			{
				if (newlen < len)
				{
					newlen++;
				}
				
				width = getFilteredStringPixelWidth(str, newlen);
			}
			newlen--;
			len = newlen;
			
			while (str[newlen] != ' ' && newlen != 0)
			{
				newlen--;
			}
			
			if (newlen > 0)  // means a space has been found and a character comes before it
			{
				len = newlen;   // skip the space
				spacefound = true;
			}
			
			// Newline was found on another line; reset the flag
			if (newlinefound && newlinepos >= newlen)
			{
				newlinefound = false;
			}
			
			width = getFilteredStringPixelWidth(str, len);
		}
		
		switch (p_horzAlign)
		{
		case ALIGN_CENTER:
			alignment = (box_width - width) / 2;
			if (box_width < width)
			{
				alignment |= 0x80000000;
			}
			
			break;
			
		case ALIGN_LEFT:
			alignment = 0;
			break;
			
		case ALIGN_RIGHT:
			alignment = box_width - width;
			break;
			
		default:
			TT_PANIC("Unknown horizontal text alignment: '%d'", p_horzAlign);
			break;
		}
		
		if (p_topMargin + p_bottomlineOffset + top > 0)
		{
			//TT_Printf("GlyphSet::drawMultiLineString - p_leftMargin: %d, alignment: %d, p_topMargin: %d, "
			//          "p_bottomlineOffset: %d, top: %d\n",
			//          p_leftMargin, alignment, p_topMargin, p_bottomlineOffset, top);
			
			s32 left = p_leftMargin + alignment;
			if (width > usedPixels.getWidth())
			{
				usedPixels.setWidth(width);
			}
			if (left < usedPixels.getLeft())
			{
				usedPixels.alignLeft(left);
			}
			
			drawFilteredString(
					str,
					p_leftMargin + alignment,
					p_topMargin + p_bottomlineOffset + top,
					p_painter,
					p_color,
					len );
		}
		
		top += pixelHeightOfOneLine;
		
		if (top + pixelHeightOfOneLine > box_height)
		{
			done = true;
		}
		
		startindex += len;
		if (spacefound)
		{
			++startindex;   // skip the space
		}
		else if (newlinefound)
		{
			++startindex; // Skip newline character
		}
	}
	
	
#if 0
	{
		// Draw a rectangle around all the pixels that were used. (and a dot in the center of this rect.)
		TT_Printf("GlyphSet::drawMultiLineString - usedPixels left: %d, right: %d, top: %d, bottom: %d\n",
		          usedPixels.getLeft(), usedPixels.getRight(),
		          usedPixels.getTop(), usedPixels.getBottom());
		
		for (s32 i = usedPixels.getLeft(); i < usedPixels.getRight(); ++i)
		{
			p_painter.setPixel(i, usedPixels.getBottom(), tt::engine::renderer::ColorRGB::white);
			p_painter.setPixel(i, usedPixels.getTop(),    tt::engine::renderer::ColorRGB::white);
		}
		for (s32 i = usedPixels.getTop(); i < usedPixels.getBottom(); ++i)
		{
			p_painter.setPixel(usedPixels.getLeft(),  i, tt::engine::renderer::ColorRGB::white);
			p_painter.setPixel(usedPixels.getRight(), i, tt::engine::renderer::ColorRGB::white);
		}
		
		tt::math::Point2 centerPos = usedPixels.getCenterPosition();
		p_painter.setPixel(centerPos.x, centerPos.y, tt::engine::renderer::ColorRGB::white);
	}
#endif
	TT_WARNING(usedPixels.getWidth() <= box_width && usedPixels.getHeight() <= box_height, 
	           "Not enough pixels to render text, '%s', over %d lines."
	           " (need width: %d - height: %d, has width: %d - height: %d)", 
	           tt::str::narrow(p_string).c_str(), lines, 
	           usedPixels.getWidth(), usedPixels.getHeight(), box_width, box_height);

	
	return usedPixels;
}


void GlyphSet::drawTruncatedString(
		const std::wstring& p_string,
		renderer::TexturePainter& p_painter,
		const renderer::ColorRGBA& p_color,
		Alignment p_align,
		Alignment p_vertAlign,
		s32 p_bottomlineOffset,
		s32 p_leftMargin,
		s32 p_topMargin,
		s32 p_rightMargin,
		s32 p_bottomMargin ) const
{
	// Filter for custom glyphs
	std::wstring filtered(getCustomGlyphFilteredString(p_string));
	
	s32 stringwidth = getFilteredStringPixelWidth(filtered.c_str());
	
	s32 height = m_heightOfCapitalX;
	
	s32 box_width  = static_cast<s32>(p_painter.getTextureWidth())  - 
			(p_leftMargin + p_rightMargin);
	
	s32 box_height = static_cast<s32>(p_painter.getTextureHeight()) - 
			(p_topMargin  + p_bottomMargin);
	
	TT_ASSERTMSG(p_bottomlineOffset != 4096,
	             "Legacy 'bottom line offset' value (4096) passed. This should no longer be used.");
	
	switch (p_vertAlign)
	{
	case ALIGN_TOP:
		p_bottomlineOffset += m_heightOfCapitalX;
		break;
		
	case ALIGN_CENTER:
		p_bottomlineOffset += ((box_height >> 1) - (height >> 1)) + m_heightOfCapitalX;
		break;
		
	case ALIGN_BOTTOM:
		p_bottomlineOffset += (box_height - height) + m_heightOfCapitalX;
		break;
		
	default:
		TT_PANIC("Unknown/unsupported vertical text alignment: %d",
		            p_vertAlign);
		break;
	}
	
	if (stringwidth > box_width)
	{
		// Calculate how many characters can be shown
		
		const wchar_t* ellipsis = L"...";
		s32 dotwidth   = getFilteredStringPixelWidth(ellipsis);
		
		s32 len = static_cast<s32>(filtered.length());
		while (len > 0 && stringwidth > (box_width - dotwidth))
		{
			--len;
			stringwidth = getFilteredStringPixelWidth(filtered.c_str(), len);
		}
		
		s32 alignment = 0;
		
		switch (p_align)
		{
		case ALIGN_CENTER:
			alignment = (box_width - (stringwidth + dotwidth)) / 2;
			if (box_width < (stringwidth + dotwidth))
			{
				alignment |= 0x80000000;
			}
			break;
			
		case ALIGN_LEFT:
			alignment = 0;
			break;
			
		case ALIGN_RIGHT:
			alignment = box_width - (stringwidth + dotwidth);
			break;
			
		default:
			TT_PANIC("Unknown horizontal text alignment: '%d'", p_align);
			break;
		}
		
		drawFilteredString(
				filtered.c_str(),
				p_leftMargin + alignment,
				p_topMargin + p_bottomlineOffset,
				p_painter,
				p_color,
				len );

		drawFilteredString(
				ellipsis,
				p_leftMargin + alignment + stringwidth,
				p_topMargin + p_bottomlineOffset,
				p_painter,
				p_color,
				-1 );
	}
	else
	{
		// Entire string fits without truncation
		s32 alignment = 0;
		switch (p_align)
		{
		case ALIGN_CENTER:
			alignment = (box_width - stringwidth) / 2;
			if (box_width < stringwidth)
			{
				alignment |= 0x80000000;
			}
			break;
			
		case ALIGN_LEFT:
			alignment = 0;
			break;
			
		case ALIGN_RIGHT:
			alignment = box_width - stringwidth;
			break;
			
		default:
			TT_PANIC("Unknown horizontal text alignment: '%d'", p_align);
			break;
		}
		
		drawFilteredString(
				filtered.c_str(),
				p_leftMargin + alignment,
				p_topMargin + p_bottomlineOffset,
				p_painter,
				p_color,
				-1 );
	}
}


s32 GlyphSet::getLineCount(const std::wstring& p_string,
                           s32 p_width,
                           s32 p_leftMargin,
                           s32 p_rightMargin) const
{
	const s32 box_width = p_width - (p_leftMargin + p_rightMargin);
	if (box_width <= 0)
	{
		return 0;
	}
	
	// Filter the string for custom glyphs
	const std::wstring filtered(getCustomGlyphFilteredString(p_string));
	
	s32       startIndex = 0;
	const s32 endIndex   = static_cast<s32>(filtered.length() + 1);  // FIXME: Why is length + 1 used?
	
	s32 lineCount = 0;
	
	bool done = false;
	while (done == false)
	{
		++lineCount;
		
		const wchar_t* str = filtered.c_str() + startIndex;
		s32 len = endIndex - startIndex;
		
		bool spaceFound   = false;
		bool newlineFound = false;
		
		
		s32 newlinePos = 0;
		while (newlinePos < len && str[newlinePos] != L'\n')
		{
			++newlinePos;
		}
		
		if (newlinePos < len)
		{
			len          = newlinePos;
			newlineFound = true;
		}
		
		s32 width = getFilteredStringPixelWidth(str, len);
		
		if (width <= box_width)
		{
			if (newlineFound == false)
			{
				done = true;
			}
		}
		else
		{
			s32 newLen = 0;
			while ((width = getFilteredStringPixelWidth(str, newLen)) <= box_width)
			{
				if (newLen < len)
				{
					++newLen;
				}
			}
			--newLen;
			len = newLen;
			
			while (str[newLen] != ' ' && newLen != 0)
			{
				--newLen;
			}
			
			if (newLen > 0)  // means a space has been found and a character comes before it
			{
				len        = newLen;   // skip the space
				spaceFound = true;
			}
			
			// Newline was found on another line; reset the flag
			if (newlineFound && newlinePos >= newLen)
			{
				newlineFound = false;
			}
			
			width = getFilteredStringPixelWidth(str, len);
		}
		
		
		if (len == 0)
		{
			// Could not fit any character into this width
			return 0;
		}
		
		startIndex += len;
		if (spaceFound)
		{
			++startIndex;   // skip the space
		}
		else if (newlineFound)
		{
			++startIndex; // Skip the newline character
		}
	}
	
	return lineCount;
}


s32 GlyphSet::getMultiLinePixelHeight(s32 p_lineCount, Alignment p_vertAlign) const
{
	TT_ASSERTMSG(p_lineCount >= 0, "Invalid line count to get height for: %d", p_lineCount);
	TT_ASSERTMSG(isValidVerticalAlignment(p_vertAlign), "Invalid vertical text alignment: %d ('%s')",
	             p_vertAlign, getAlignmentName(p_vertAlign));
	
	if (p_lineCount <= 0)
	{
		return 0;
	}
	
	// FIXME: This function contains some copy&pasted code from drawMultiLineString. Refactor?
	
	// FIXME: Single-line hack (seems to fix height calculation as used in Swap)
	if (p_lineCount == 1)
	{
		return getAscenderHeight() - getDescenderHeight();
	}
	
	// A line gets the height of a capital X and between we need vertical spacing.
	const s32 pixelHeightOfOneLine = (m_heightOfCapitalX + m_verticalSpacing);
	
	// For multiple lines we have 1 less verticalSpacing than p_lineCount.
	s32 height = (pixelHeightOfOneLine * p_lineCount) - m_verticalSpacing;
	
	// Special case ignore for a single centered line.
	if (p_vertAlign == ALIGN_CENTER && p_lineCount == 1)
	{
		// For single line centered text we center around the top and bottom pixel of a capital X.
		height -= getBaseline() + 1;
	}
	else
	{
		// Take into account that some characters have pixels above 'X'.
		s32 extraHeight = getAscenderHeight() - m_heightOfCapitalX;
		if (extraHeight > 0)
		{
			height += extraHeight;
		}
		
		// Take into account that some characters have pixels below the baseline.
		if (getDescenderHeight() < 0)
		{
			height -= getDescenderHeight();
		}
	}
	
	return height;
}


std::wstring::size_type GlyphSet::getFit(
		const std::wstring& p_string,
		s32 p_width) 
const
{
	// Filter the string and forward it to getFilteredFit
	std::wstring filtered(getCustomGlyphFilteredString(p_string));
	return getFilteredFit(filtered.c_str(), p_width);
}


void GlyphSet::createLines(
		const std::wstring& p_string,
		std::vector<std::wstring>& p_lines,
		s32 p_width,
		s32 p_leftMargin,
		s32 p_rightMargin) 
const
{
	std::wstring str(getCustomGlyphFilteredString(p_string)); // scratch string to work with
	std::vector<std::wstring> newlines; // holds all newline-separated strings
	
	// First, cut the string in pieces using the newlines
	std::wstring::size_type newlinepos = str.find(L"\n");
	while (newlinepos != std::wstring::npos)
	{
		if (newlinepos == 0)
		{
			// Newline at start of string; insert empty line
			newlines.push_back(L"");
		}
		else
		{
			// Save the start of the string up to the newline marker
			newlines.push_back(str.substr(0, newlinepos));
		}
		
		// Move on to the next string
		str        = str.substr(newlinepos + 1);
		newlinepos = str.find(L"\n");
	}
	
	if (str.empty() == false)
	{
		newlines.push_back(str);
	}
	
	s32 box_width = p_width - (p_leftMargin + p_rightMargin);
	
	// Now cut all the strings in pieces so they fit a line
	while (newlines.empty() == false)
	{
		std::wstring line(newlines.front());
		newlines.erase(newlines.begin());	// remove from newlines vector
		
		if (line.empty())
		{
			// Retain empty lines
			p_lines.push_back(line);
		}
		else
		{
			while (line.empty() == false)
			{
				if (getFilteredStringPixelWidth(line.c_str()) <= box_width)
				{
					// Line fits as it is
					p_lines.push_back(line);
					line.clear();
				}
				else
				{
					std::wstring::size_type index = getFilteredFit(line.c_str(), box_width);
					
					// Now search back to find space
					std::wstring::size_type lastspace = line.rfind(L' ', index);
					if (lastspace != std::wstring::npos && lastspace != 0)
					{
						p_lines.push_back(line.substr(0, lastspace));
						line = line.substr(lastspace + 1);
					}
					else
					{
						p_lines.push_back(line.substr(0, index));
						line = line.substr(index);
					}
				}
			}
		}
	}
}


void GlyphSet::createPages(
		const std::wstring& p_string,
		std::vector<std::wstring>& p_pages,
		s32 p_width,
		s32 p_height,
		s32 p_leftMargin,
		s32 p_topMargin,
		s32 p_rightMargin,
		s32 p_bottomMargin,
		bool p_countEmptyLinesAtStartPage) const
{
	// Build initial list of pages based on the new page markers
	// (new page markers will be stripped from the initial pages)
	typedef std::vector<std::wstring> WStrings;
	WStrings initialPages;
	
	{
		std::wstring str(getCustomGlyphFilteredString(p_string)); // scratch string to work with
		
		std::wstring::size_type newPagePos = str.find(L"\\p");
		while (newPagePos != std::wstring::npos)
		{
			if (newPagePos == 0)
			{
				// New page marker at start of string; insert empty page
				initialPages.push_back(L" ");
			}
			else
			{
				// Save the start of the string up to the new page marker
				initialPages.push_back(str.substr(0, newPagePos));
			}
			
			// Move on to the next string
			str        = str.substr(newPagePos + 2);
			newPagePos = str.find(L"\\p");
		}
		
		if (str.empty() == false)
		{
			initialPages.push_back(str);
		}
	}
	
	// Determine the available space for the pages
	// s32 box_width  = p_width  - (p_leftMargin + p_rightMargin);
	s32 boxHeight = p_height - (p_topMargin  + p_bottomMargin);
	
	// Calculate the number of lines that fit on one page
	s32 linesPerPage = boxHeight / (m_heightOfCapitalX + m_verticalSpacing);
	s32 lineCount    = 0;
	
	// Go through all the initial pages
	for (WStrings::iterator it = initialPages.begin(); it != initialPages.end(); ++it)
	{
		// Parse this page into lines
		WStrings lines;
		createLines(*it,
		            lines,
		            p_width,
		            p_leftMargin,
		            p_rightMargin);
		
		// Group the lines of this page into the final pages
		std::wstring scratch; // buffer to compose page in
		while (lines.empty() == false)
		{
			// Scratch buffer not empty; add a newline
			if (scratch.empty() == false)
			{
				scratch += L"\n";
			}
			
			// Get one line of text and remove it from the working vector
			std::wstring line(lines.front());
			lines.erase(lines.begin());
			
			/*
			// Check for "new page" markers
			// NOTE: This should no longer be necessary,
			//       as these markers were stripped by
			//       the first processing step.
			wstring::size_type newpagepos = line.find(L"\\p");
			while (newpagepos != wstring::npos)
			{
				// Add the string up to the "new page" marker to the buffer
				if (newpagepos != 0)
				{
					scratch += line.substr(0, newpagepos);
				}
				
				// Create a page and clear the buffer
				p_pages.push_back(scratch);
				scratch.clear();
				lineCount = 0;
				
				// Remove the part up to the "new page" marker and find next marker
				line       = line.substr(newpagepos + 2);
				newpagepos = line.find(L"\\p");
			}
			// */
			
			// Skip line if the first line on the page is a newline or empty string
			if (scratch.empty() == false ||
			    str::isEmptyOrWhitespace(line) == false)
			{
				// Add line
				scratch += line;
				++lineCount;
			}
			else if(p_countEmptyLinesAtStartPage)
			{
				++lineCount;
			}
			
			// If the number of lines for one page is reached,
			// or the last line has been processed,
			// create a page from the current scratch string
			if (lineCount == linesPerPage || lines.empty())
			{
				p_pages.push_back(scratch);
				scratch.clear();
				lineCount = 0;
			}
		}
	}
	
	// Kill any empty pages found in the final pages vector
	for (WStrings::iterator it = p_pages.begin(); it != p_pages.end(); )
	{
		if (str::isEmptyOrWhitespace(*it))
		{
			it = p_pages.erase(it);
		}
		else
		{
			++it;
		}
	}
	
	
	/*
	TT_Printf("GlyphSet::createPages: Pages that were created:\n"
	          "--------------------------------------------------\n");
	for (WStrings::iterator it = p_pages.begin(); it != p_pages.end(); ++it)
	{
		TT_Printf("%s\n--------------------------------------------------\n",
		          str::narrow(*it).c_str());
	}
	// */
}


GlyphSet::Alignment GlyphSet::getAlignmentFromName(const std::string& p_name)
{
	for (s32 i = ALIGN_CENTER; i <= ALIGN_BOTTOM; ++i)
	{
		Alignment align = static_cast<Alignment>(i);
		if (p_name == getAlignmentName(align))
		{
			return align;
		}
	}
	
	TT_PANIC("Invalid alignment name: '%s'", p_name.c_str());
	return ALIGN_CENTER;
}


const char* GlyphSet::getAlignmentName(GlyphSet::Alignment p_alignment)
{
	switch (p_alignment)
	{
	case ALIGN_CENTER: return "center";
	case ALIGN_LEFT:   return "left";
	case ALIGN_RIGHT:  return "right";
	case ALIGN_TOP:    return "top";
	case ALIGN_BOTTOM: return "bottom";
		
	default:
		TT_PANIC("Unknown alignment: %d", p_alignment);
		break;
	}
	
	return "<unknown>";
}


bool GlyphSet::isValidHorizontalAlignment(Alignment p_alignment)
{
	return p_alignment == ALIGN_CENTER ||
	       p_alignment == ALIGN_LEFT   ||
	       p_alignment == ALIGN_RIGHT;
}


bool GlyphSet::isValidVerticalAlignment(Alignment p_alignment)
{
	return p_alignment == ALIGN_CENTER ||
	       p_alignment == ALIGN_TOP    ||
	       p_alignment == ALIGN_BOTTOM;
}


void GlyphSet::addKerningPairOffset(wchar_t p_leftChar, wchar_t p_rightChar, s32 p_offset)
{
	m_kerningPairs[p_leftChar][p_rightChar] = p_offset;
}


//------------------------------------------------------------------------------
// Private member functions

s32 GlyphSet::getTextHeight(
		const wchar_t* p_string,
		s32            p_stringLength,
		Alignment /* p_align */,
		s32 p_bottomlineOffset,
		s32 p_width,
		s32 p_height,
		s32 p_leftMargin,
		s32 p_topMargin,
		s32 p_rightMargin,
		s32 p_bottomMargin) 
const
{
	TT_ASSERTMSG(p_bottomlineOffset != 4096,
	             "Legacy 'bottom line offset' value (4096) passed. This should no longer be used.");
	
	const s32 box_width  = p_width  - (p_leftMargin + p_rightMargin);
	const s32 box_height = p_height - (p_topMargin  + p_bottomMargin);
	
	TT_ASSERTMSG(box_width > 0,
	             "No width remaining for text (%d)! Full width: %d. Left margin: %d. Right margin: %d.",
	             box_width, p_width, p_leftMargin, p_rightMargin);
	TT_ASSERTMSG(box_height > 0,
	             "No height remaining for text (%d)! Full height: %d. Top margin: %d. Bottom margin: %d.",
	             box_height, p_height, p_topMargin, p_bottomMargin);
	if (box_width <= 0 || box_height <= 0)
	{
		return 0;
	}
	
	s32 startindex = 0;
	const s32 endindex   = p_stringLength + 1;// static_cast<s32>(std::char_traits<wchar_t>::length(p_string) + 1);
	s32 top = 0;
	
	s32 lineCount = 0;
	
	bool done = false;
	
	while (done == false)
	{
		++lineCount;
		
		const wchar_t* str = p_string + startindex;
		s32 len = endindex - startindex;
		
		bool spacefound   = false;
		bool newlinefound = false;
		
		s32 newlinepos = 0;
		while (newlinepos < len && str[newlinepos] != L'\n')
		{
			++newlinepos;
		}
		
		if (newlinepos < len)
		{
			len          = newlinepos;
			newlinefound = true;
		}
		
		s32 width = getFilteredStringPixelWidth(str, len);
		
		if (width <= box_width)
		{
			if (newlinefound == false)
			{
				done = true;
			}
		}
		else
		{
			s32 newlen = 0;
			while ((width = getFilteredStringPixelWidth(str, newlen)) <= box_width)
			{
				if (newlen < len)
				{
					++newlen;
				}
			}
			--newlen;
			len = newlen;
			
			while (str[newlen] != ' ' && newlen != 0)
			{
				--newlen;
			}
			
			if (newlen > 0)  // means a space has been found and a character comes before it
			{
				len        = newlen;   // skip the space
				spacefound = true;
			}
			
			width = getFilteredStringPixelWidth(str, len);
		}
		
		const s32 pixelHeightOfOneLine = (m_heightOfCapitalX + m_verticalSpacing);
		top += pixelHeightOfOneLine;
		
		if (top + pixelHeightOfOneLine > box_height)
		{
			done = true;
		}
		
		startindex += len;
		if (spacefound)
		{
			++startindex;   // skip the space
		}
		else if (newlinefound)
		{
			++startindex;
		}
	}
	
	return lineCount;
}



// TODO: implement with more platform-generic image parameter


void GlyphSet::loadCustomGlyphs(const std::string& p_filename)
{
	tt::xml::XmlDocument customGlyphs(p_filename);

	tt::xml::XmlNode* root = customGlyphs.getRootNode();

	for(tt::xml::XmlNode* node = root->getChild(); node != 0; node = node->getSibling())
	{
		if(node->getName() == "glyph")
		{
			tt::engine::renderer::TexturePtr image =
				tt::engine::renderer::TextureCache::get(node->getAttribute("image"),
				                                        node->getAttribute("namespace"));

			if(image != 0)
			{
				addCustomGlyph(node->getAttribute("name"), image);
			}
			else
			{
				TT_WARN("Could not find image for custom glyph \"%s\"",
					node->getAttribute("name").c_str());
			}
		}
	}
}


wchar_t GlyphSet::addCustomGlyph(const std::string& p_key, const renderer::TexturePtr& p_texture)
{
    // Convert from string to wstring
	std::wstring wideKey(tt::str::widen(p_key));

    // Calculate the internal unused Unicode character
    wchar_t ucodeChar = static_cast<wchar_t>(0xE000 + m_customGlyphs.size());

    // Not bloody likely that someone will register thousands of
    // customglyphs, but still
    TT_ASSERT(isInPrivateUnicodeRange(ucodeChar));

    // Disallow binding different glyphs to same string
    TT_ASSERTMSG( (m_customGlyphs.find(wideKey) == m_customGlyphs.end() ),
        "Attempt to bind new custom glyph to existing key string!" );

    // Log the substring=>customcharacter mapping
    m_customGlyphs.insert(CustomGlyphMap::value_type(wideKey, ucodeChar));

	// Figure out center
	s32 yOffset((p_texture->getHeight() - getGlyph(L'x')->getHeight()) / 2);

    // Create a glyph from image
    Glyph* curr = new CustomGlyph(ucodeChar, p_texture, yOffset);
    m_glyphs.insert(GlyphMap::value_type(curr->getChar(), curr));

    // Return assigned unicode to caller
    return ucodeChar;
}


/**
 * Look for potential customglyphs in input string
 *
 * @param p_str             input string
 * @param p_wasmodified_OUT modification flag - set to true if a custom glyph was found
 *
 * @return the string with customglyphs substitued into it
 */
std::wstring GlyphSet::getCustomGlyphFilteredString( 
		const std::wstring& p_str ) 
const
{
	std::wstring filtered(p_str);
	
	// Iterate all the string keys
	for (CustomGlyphMap::const_iterator itr = m_customGlyphs.begin();
	     itr != m_customGlyphs.end(); ++itr)
	{
		// Grab key
		const std::wstring& customglyphname((*itr).first);
		
		// Create a string consisting of a single unicodechar
		std::wstring ucodechar(1, (*itr).second);
		
		// Find occurences of key in string
		std::wstring::size_type where = filtered.find(customglyphname);
		while ( where != std::wstring::npos )
		{
			filtered.replace( where, customglyphname.length(), ucodechar );
			where = filtered.find(customglyphname);
		}
	}
	
	return filtered;
}


s32 GlyphSet::getFilteredStringPixelWidth(
		const wchar_t* p_string,
		s32            p_len) 
const
{
	// If entire string is desired, set length to the largest s32 value possible
	if (p_len == -1)
	{
		p_len = std::numeric_limits<s32>::max();
	}

	if (*p_string == 0 || p_len == 0)
	{
		return 0;
	}
	
	// Loop through the desired section of the string
	s32 pixwidth = 0;
	const Glyph* previousGlyph = 0;
	
	for (int i = 0; *p_string != 0 && i < p_len; ++i, ++p_string)
	{
		const Glyph* gl = getGlyph(*p_string);
		
		pixwidth += kerning(previousGlyph, gl) + advance(gl);
		
		previousGlyph = gl;
	}
	
	if (previousGlyph != 0)
	{
		// The last charcter doesn't need spacing behind it.
		pixwidth -= advance(previousGlyph);
		
		// Work around glyph data bug.
		// characterWidth() might be smaller than the actuall pixels needed (getXMax(false)).
		pixwidth += previousGlyph->getWidth() - previousGlyph->getXMax(false);
	}
	
	return pixwidth;
}


void GlyphSet::drawFilteredString(
		const wchar_t* p_string,
		s32 p_x,
		s32 p_y,
		renderer::TexturePainter& p_painter,
		const renderer::ColorRGBA& p_color,
		s32 p_strlen )
const
{
	//TT_Printf("GlyphSet::drawFilteredString - string: '%s', x: %d, y: %d\n",
	//          tt::str::narrow(p_string).c_str(), p_x, p_y);
	
	// If entire string is desired, set length to the largest s32 value possible
	if (p_strlen == -1)
	{
		p_strlen = std::numeric_limits<s32>::max();
	}
	
	
	// Loop through the desired section of the string
	s32 x = p_x;
	const s32 y = p_y;
	
	const s32 texW = static_cast<s32>(p_painter.getTextureWidth());
	const s32 texH = static_cast<s32>(p_painter.getTextureHeight());
	
	const Glyph* previousGlyph = 0;
	
	for (int i = 0; *p_string != 0 && i < p_strlen; ++i, ++p_string)
	{
		const Glyph* gl = getGlyph(*p_string);
		
		if (gl != 0)
		{
			/*
			s32 kerningOffset = (previousGlyph != 0) ? previousGlyph->getKerningOffset(gl) : 0;
			x += kerningOffset;
			
			u16 total_charwidth = static_cast<u16>(gl->getCharacterWidth() + getSpacing());
			*/
			x += kerning(previousGlyph, gl);
			u16 total_charwidth = static_cast<u16>(advance(gl));
			
			s32 glyphHeight = gl->getHeight();
			
			bool within_bounds = 
					(x > (-total_charwidth)) &&
					(y > (-glyphHeight)) &&
					(x < texW) &&
					(y < texH + glyphHeight);
			
			if ( within_bounds )
			{
				gl->draw(x, y, p_painter, p_color); 
			}
			
			x += total_charwidth;
		}
		else
		{
			x += m_wordseparatorWidth;
		}
		previousGlyph = gl;
	}
}


std::wstring::size_type GlyphSet::getFilteredFit(
		const wchar_t* p_string,
		s32            p_width) 
const
{
	s32 pixwidth = 0;
	const Glyph* previousGlyph = 0;
	for (std::wstring::size_type index = 0; *p_string != 0; ++index, ++p_string)
	{
		const Glyph* gl = getGlyph(*p_string);
		pixwidth += kerning(previousGlyph, gl) + advance(gl);
		
		if (pixwidth > p_width)
		{
			// Not all characters fit; return the character
			// index at which it no longer fits
			return index;
		}
		
		previousGlyph = gl;
	}
	
	// All characters fit
	return std::wstring::npos;
}


s32 GlyphSet::kerning(const Glyph* p_previousGlyph, const Glyph* p_currentGlyph) const
{
	if (p_previousGlyph == 0 || p_currentGlyph == 0)
	{
		return 0;
	}
	//return p_previousGlyph->getKerningOffset(p_currentGlyph);
	
//#define TT_TURN_KERNING_OFF
	
#if defined(TT_TURN_KERNING_OFF)
	return 0;
#else //#if defined(TT_TURN_KERNING_OFF)
	
//#define TT_KERNING_DEBUG       // Turns on all kerning debug prints.
//#define TT_KERNING_DEBUG_LIGHT // Turns on a minimal of kerning debug prints.
	
#if defined(TT_KERNING_DEBUG) || defined(TT_KERNING_DEBUG_LIGHT)
	{
		const char left  = static_cast<char>(p_previousGlyph->getChar());
		const char right = static_cast<char>(p_currentGlyph->getChar());
		
		TT_Printf("GlyphSet::kerning left:'%c', right:'%c'\n", left, right);
	}
#endif
	
	// The previous Glyph is the left glyph.
	const s32 leftHeight  = p_previousGlyph->getHeight();
	const s32 leftStartY  = p_previousGlyph->getYMax();
	const s32 leftWidth   = p_previousGlyph->getWidth();
	// This Glyph is the right glyph 
	const s32 rightHeight = p_currentGlyph->getHeight();
	const s32 rightStartY = p_currentGlyph->getYMax();
	const s32 rightWidth  = p_currentGlyph->getWidth();
	
	const s32 startY = std::min(leftStartY, rightStartY);
	// Start with an offset were all pixels are empty.
	s32 smallestDistance = leftWidth + rightWidth;
	
	s32 leftRow  = leftStartY - startY;
	s32 rightRow = rightStartY - startY; 
	
#if defined(TT_KERNING_DEBUG)
	TT_Printf("GlyphSet::kerning startY: %d, leftStartY: %d, rightStartY: %d\n", 
	          startY, leftStartY, rightStartY);
	TT_Printf("GlyphSet::kerning leftRow: %d, rightRow: %d\n", 
	          leftRow, rightRow);
#endif
	
	
#if defined(TT_KERNING_DEBUG) || defined(TT_KERNING_DEBUG_LIGHT)
	if (leftRow >= leftHeight || rightRow >= rightHeight)
	{
		TT_Printf("GlyphSet::kerning bounding boxes don't overlap.\n");
		//return 0;
	}
#endif
	
	for (;
	     leftRow < leftHeight && rightRow < rightHeight;
	     ++leftRow, ++rightRow)
	{
		// <-- LEFT <--
		
		s32 leftOffset = p_previousGlyph->getXMax(leftRow);
		
		
#if defined(TT_KERNING_DEBUG)
		// START DEBUG
		TT_Printf("GlyphSet::kerning - in for loop - leftRow: %d, rightRow: %d\n", leftRow, rightRow);
		TT_Printf("|");
		p_previousGlyph->printPixelRow(leftRow);
		TT_Printf("|<->|");
		// END DEBUG
#endif
		// --> RIGHT -->
		
		s32 rightOffset = p_currentGlyph->getXMin(rightRow);
		
#if defined(TT_KERNING_DEBUG)
		// START DEBUG
		p_currentGlyph->printPixelRow(rightRow);
		TT_Printf("|\n");
		// END DEBUG
		
		TT_Printf("GlyphSet::kerning leftOffset: %d, leftRow: %d\n", 
		          leftOffset, leftRow);
		
		TT_Printf("GlyphSet::kerning rightOffset: %d, rightRow: %d\n", 
		          rightOffset, rightRow);
#endif
		
		s32 combinedOffset = leftOffset + rightOffset;
		
#if defined(TT_KERNING_DEBUG)
		TT_Printf("GlyphSet::kerning smallestDistance: %d, combinedOffset: %d\n",
		          smallestDistance, combinedOffset);
#endif
		
		if (combinedOffset < smallestDistance)
		{
			smallestDistance = combinedOffset;
		}
	}
	
	// The calculations within this functions were done with width.
	// But external code will use character width.
	// So this 'extra space' for the left character should be removed from the result.
	const s32 extraLeftSpace = leftWidth - p_previousGlyph->getCharacterWidth();
	s32 offset = smallestDistance - extraLeftSpace;
	
#if defined(TT_KERNING_DEBUG)
	TT_Printf("GlyphSet::kerning extraLeftSpace(%d) = leftWidth(%d) - p_previousGlyph->getCharacterWidth()(%d)\n",
	          extraLeftSpace, leftWidth, p_previousGlyph->getCharacterWidth());
	TT_Printf("GlyphSet::kerning offset(%d) = smallestDistance(%d) - extraLeftSpace(%d)\n",
	          offset, smallestDistance, extraLeftSpace);
#endif
	
	// Make sure the kerning offset doesn't move the right glyph's leftmost pixel
	// past the left glyph's rightmost pixel.
	const s32 maxOffset = (p_previousGlyph->getXMax() + p_currentGlyph->getXMin()) - 
	                      (extraLeftSpace - 1);
	
#if defined(TT_KERNING_DEBUG)
	TT_Printf("GlyphSet::kerning maxOffset(%d) = (p_previousGlyph->getXMax()(%d) + p_currentGlyph->getXMin()(%d)) - "
	          "(extraLeftSpace(%d) - 1)\n",
	          maxOffset, p_previousGlyph->getXMax(), p_currentGlyph->getXMin(), extraLeftSpace);
#endif
	
	if (offset > maxOffset)
	{
#if defined(TT_KERNING_DEBUG) || defined(TT_KERNING_DEBUG_LIGHT)
		TT_Printf("GlyphSet::kerning offset: %d > maxOffset: %d\n",
		          offset, maxOffset);
#endif
		offset = maxOffset;
	}
	
#if defined(TT_KERNING_DEBUG) || defined(TT_KERNING_DEBUG_LIGHT)
	TT_Printf("GlyphSet::kerning leftCharWidth: %d, smallestDistance: %d, "
	          "offset: %d, returns: %d\n", 
	          p_previousGlyph->getCharacterWidth(), smallestDistance, offset, 
	          ((offset > leftWidth) ? -leftWidth : -offset));
#endif
	
	
	s32 customKerningOffset = getKerningPairOffset(p_previousGlyph->getChar(), p_currentGlyph->getChar());
	
	// Make sure the right char isn't place to the left of the left character.
	// Because then the character to the left of the left charater might be overwritten.
	return ((offset > leftWidth) ? -leftWidth : -offset) + customKerningOffset;
#endif // #else defined(TT_TURN_KERNING_OFF)
}


s32 GlyphSet::advance(const Glyph* p_glyph) const
{
	if (p_glyph == 0)
	{
		return m_wordseparatorWidth;
	}
	
	return p_glyph->getCharacterWidth() + getSpacing();
}


/**
 * Set glyph which is rendered when an unknown glyph is encountered
 *
 * @param p_unknownglyph unicode of the glyph to render
 */
void GlyphSet::setUnknownGlyphCharacterCode( u16 p_unknownglyph )
{
	// Sanity-check
	TT_ASSERTMSG(
			(getGlyph( p_unknownglyph ) != 0L),
			"Attempt to set undefined glyph as undefined glyph placeholder" );
	
	// Set
	m_unknownGlyphUnicode = p_unknownglyph;
}


/**
 *
 * @return character code of currently set unknown-char code
 */
u16 GlyphSet::getUnknownGlyphCharacterCode() const
{
	return m_unknownGlyphUnicode;
}


/**
 *
 * @param p_str
 *
 * @return in-parameter string, with all private-range characters replaced with the
 *         <unknwon> character
 */
std::wstring GlyphSet::replacePrivateRange(
		const std::wstring& p_str ) const
{
	// Filtered string
	std::wstring filtered;
	
	// Iterate input string
	for (std::wstring::const_iterator it = p_str.begin();
	     it != p_str.end(); ++it)
	{
		// Readability
		wchar_t current = *it;
		
		// Append char to returnstring, if it is not in customrange
		filtered += isInPrivateUnicodeRange( current )
		            ? static_cast<wchar_t>(getUnknownGlyphCharacterCode())
		            : current;
	}
	
	// Return string
	return filtered;
}


/**
 *
 * @param p_char
 *
 * @return true if a certain character is within the private range
 */
bool GlyphSet::isInPrivateUnicodeRange( wchar_t p_char ) const
{
	return ((p_char >= 0xE000) && (p_char <= 0xF8FF));
}


/// \return bit depth of the Glyphs 
int GlyphSet::getBitDepth() const
{
	return m_bitDepth;
}


s32 GlyphSet::getKerningPairOffset(wchar_t p_leftChar, wchar_t p_rightChar) const
{
	CharCharOffset::const_iterator leftIt = m_kerningPairs.find(p_leftChar);
	if (leftIt == m_kerningPairs.end())
	{
		return 0;
	}
	CharOffset::const_iterator rightIt = leftIt->second.find(p_rightChar);
	if (rightIt == leftIt->second.end())
	{
		return 0;
	}
	return rightIt->second;
}


// Namespace end
}
}
}
