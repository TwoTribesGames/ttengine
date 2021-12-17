#ifndef INC_TT_ENGINE_GLYPH_GLYPHSET_H
#define INC_TT_ENGINE_GLYPH_GLYPHSET_H


#include <map>
#include <set>
#include <string>
#include <vector>

#include <tt/code/fwd.h>
#include <tt/engine/glyph/Glyph.h>
#include <tt/engine/renderer/fwd.h>
#include <tt/fs/types.h>
#include <tt/math/Rect.h>
#include <tt/math/Point2.h>


namespace tt {
namespace engine {
namespace glyph {

/**
 * Unicode font wrapper
 */
class GlyphSet
{
public:
	enum Alignment
	{
		ALIGN_CENTER = 0,
		ALIGN_LEFT,
		ALIGN_RIGHT,
		ALIGN_TOP,
		ALIGN_BOTTOM
	};
	
	
	/*! \param p_filename           File containing glyph set.
	    \param p_characterSpacing   Space between characters.
	    \param p_wordSeparatorWidth Width of the word separator (space).
	    \param p_verticalSpacing    Space between lines, for multi-line text. */
	explicit GlyphSet(const std::string& p_filename,
	                  u16                p_characterSpacing   = 1,
	                  u16                p_wordSeparatorWidth = 10,
	                  u16                p_verticalSpacing    = 2);
	~GlyphSet();
	
	void drawString(
			const std::wstring& p_string,
			s32 p_x,
			s32 p_y,
			renderer::TexturePainter& p_painter,
			const renderer::ColorRGBA& p_color,
			s32 p_strlen = -1) const;
	
	s32 getStringPixelWidth(
			const std::wstring& p_string,
			s32 p_len = -1) const;
	
	// gets the minimal pixel dimensions needed to render this string.
	math::Point2 getStringPixelDimensions(
			const std::wstring& p_string,
			s32 p_len = -1) const;
	
	void setSpacing(u16 p_spacing);
	
	/*! \return Current character spacing. */
	inline u16 getSpacing() const { return m_charSpacing; }
	
	void setVerticalSpacing(u16 p_vertSpacing);
	inline u16 getVerticalSpacing() const { return m_verticalSpacing; }
	
	
	void setWordSeparatorWidth(u16 p_newwidth);
	inline u16 getWordSeparatorWidth() const { return m_wordseparatorWidth; }
	
	void drawTruncatedString(
			const std::wstring& p_string,
			renderer::TexturePainter& p_painter,
			const renderer::ColorRGBA& p_color,
			Alignment p_align,
			Alignment p_vertAlign,
			s32 p_bottomlineOffset,
			s32 p_leftMargin,
			s32 p_topMargin,
			s32 p_rightMargin,
			s32 p_bottomMargin) const;
	
	
	tt::math::PointRect drawMultiLineString(
			const std::wstring& p_string,
			renderer::TexturePainter&  p_painter,
			const renderer::ColorRGBA& p_color,
			Alignment p_align = ALIGN_CENTER,
			Alignment p_vertAlign = ALIGN_CENTER,
			s32 p_bottomlineOffset = 0,
			s32 p_leftMargin = 0,
			s32 p_topMargin = 0,
			s32 p_rightMargin = 0,
			s32 p_bottomMargin = 0) const;
	
	// TODO: Add more font metrics (see: http://ilovetypography.com/2009/01/14/inconspicuous-vertical-metrics/ )
	
	/*! \return The Cap height of this GlyphSet.
	    \note http://en.wikipedia.org/wiki/Cap_height */
	inline u16 getHeight() const { return m_heightOfCapitalX; }
	
	/*! \return For some fonts the baseline has a bit of an offset. Use the descender of the 'X' as baseline. */
	inline s16 getBaseline() const { return m_baseline; }
	
	/*! \return The height of the tallest character with this GlyphSet. (The ascender.)
	    \note Do NOT use this for the spacing between lines for multiline text.
	          For that getHeight should be used.
	          http://en.wikipedia.org/wiki/Ascender */
	inline s16 getAscenderHeight() const { return m_ascenderHeight; }
	
	//! \return The offset from baseline for bottom most pixel.
	inline s16 getDescenderHeight() const { return m_descenderHeight; }
	
	s32 getLineCount(
			const std::wstring& p_string,
			s32 p_width,
			s32 p_leftMargin,
			s32 p_rightMargin) const;
	
	/*! \param p_lineCount Number of lines in the string (you can get this via getLineCount, for example).
	    \param p_vertAlign The vertical alignment that will be used with drawMultiLineString
	                       (this has consequences for certain text rendering).
	    \return The height (in pixels) that the specified number of lines
	            would require when rendered with drawMultiLineString. */
	s32 getMultiLinePixelHeight(s32 p_lineCount, Alignment p_vertAlign = ALIGN_CENTER) const;
	
	std::wstring::size_type getFit(const std::wstring& p_string, s32 p_width) const;
	
	void createLines(
			const std::wstring& p_string,
			std::vector<std::wstring>& p_pages,
			s32 p_width = 256,
			s32 p_leftMargin = 0,
			s32 p_rightMargin = 0) const;
	
	// (Note: I (Eelke) removed the default parameters,
	//  so we are force to check the value of the new parameter p_countEmptyLinesAtStartPage!)
	void createPages(
			const std::wstring& p_string,
			std::vector<std::wstring>& p_pages,
			s32 p_width,        // was: = 256,
			s32 p_height,       // was: = 192,
			s32 p_leftMargin,   // was: = 0,
			s32 p_topMargin,    // was: = 0,
			s32 p_rightMargin,  // was: = 0,
			s32 p_bottomMargin, // was: = 0
			bool p_countEmptyLinesAtStartPage // New paramter, get old behavior by passing true.
			) const;
	
	void loadCustomGlyphs(const std::string& p_filename);
	wchar_t addCustomGlyph(const std::string& p_key, const renderer::TexturePtr& p_texture);
	
	u16 getUnknownGlyphCharacterCode() const;
	void setUnknownGlyphCharacterCode(u16 p_unknownglyph);
	
	std::wstring replacePrivateRange(const std::wstring& p_str) const;
	
	int getBitDepth() const;
	
	static Alignment getAlignmentFromName(const std::string& p_name);
	static const char* getAlignmentName(Alignment p_alignment);
	
	static bool isValidHorizontalAlignment(Alignment p_alignment);
	static bool isValidVerticalAlignment(Alignment p_alignment);
	
	Glyph* getGlyph(wchar_t p_char) const;
	
	inline void addSpaceChar(wchar_t p_char) { m_spaceChars.insert(p_char); };
	
	void addKerningPairOffset(wchar_t p_leftChar, wchar_t p_rightChar, s32 p_offset);
	
private:
	typedef std::map<wchar_t, Glyph*>       GlyphMap;
	typedef std::map<std::wstring, wchar_t> CustomGlyphMap;
	typedef std::map<wchar_t, s32>          CharOffset;
	typedef std::map<wchar_t, CharOffset>   CharCharOffset;
	typedef std::set<wchar_t>               CharSet;
	
	
	/*! \brief Returns how many *lines* high the specified string is (not pixels!). */
	s32 getTextHeight(
			const wchar_t* p_string,
			s32            p_stringLength,
			Alignment      p_align,
			s32            p_bottomlineOffset,
			s32            p_width,
			s32            p_height,
			s32            p_leftMargin,
			s32            p_topMargin,
			s32            p_rightMargin,
			s32            p_bottomMargin) const;
	
	bool constructFromData(tt::code::BufferReadContext* p_readContext);
	
	std::wstring getCustomGlyphFilteredString(const std::wstring& p_str) const;
	
	
	// Fast implementations of the public counterparts,
	// which do no custom glyph filtering or make unnecessary string copies
	
	s32 getFilteredStringPixelWidth(
			const wchar_t* p_string,
			s32            p_len = -1) const;
	
	void drawFilteredString(
			const wchar_t* p_str,
			s32 p_x,
			s32 p_y,
			renderer::TexturePainter& p_painter,
			const renderer::ColorRGBA& p_color,
			s32 p_strlen  = -1 ) const;
	
	std::wstring::size_type getFilteredFit(
			const wchar_t* p_string,
			s32            p_width) const;
	
	s32 kerning(const Glyph* p_previousGlyph, const Glyph* p_currentGlyph) const;
	s32 advance(const Glyph* p_glyph) const;
	
	bool isInPrivateUnicodeRange(wchar_t p_char) const;
	
	s32 getKerningPairOffset(wchar_t p_leftChar, wchar_t p_rightChar) const;
	
	// No copying or assignment
	GlyphSet(const GlyphSet&);
	GlyphSet& operator=(const GlyphSet&);
	
	
	u16 m_numGlyphs;
	u16 m_heightOfCapitalX;
	s16 m_baseline;
	s16 m_ascenderHeight;   // Offset from baseline for top most pixel.
	s16 m_descenderHeight;  // Offset from baseline for bottom most pixel.
	u16 m_pointSize;
	u16 m_bitDepth;
	u16 m_charSpacing;
	u16 m_verticalSpacing;
	
	u16 m_wordseparatorWidth;
	u16 m_unknownGlyphUnicode;
	
	GlyphMap       m_glyphs;
	CustomGlyphMap m_customGlyphs;
	CharSet        m_spaceChars; // Characters which should be treated like a space ' '.
	
	CharCharOffset m_kerningPairs;
};


// Namespace end
}
}
}


#endif  // !defined(INC_TT_ENGINE_GLYPH_GLYPHSET_H)
