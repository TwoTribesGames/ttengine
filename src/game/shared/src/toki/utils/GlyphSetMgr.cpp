#include <tt/engine/glyph/GlyphSet.h>
#include <tt/fs/fs.h>

#include <toki/loc/Loc.h>
#include <toki/utils/GlyphSetMgr.h>
#include <toki/AppGlobal.h>
#include <toki/cfg.h>

#if defined(TT_PLATFORM_WIN)
#pragma warning(disable : 4592)
#endif


namespace toki {
namespace utils {

static const char* const getIDName(GlyphSetID p_id)
{
	switch (p_id)
	{
	case GlyphSetID_Title:           return "title";
	case GlyphSetID_Header:          return "header";
	case GlyphSetID_Text:            return "text";
	case GlyphSetID_Notes:           return "notes";
	case GlyphSetID_EditorHelpText:  return "editor_helptext";
		
	default:
		TT_PANIC("Invalid glyph set ID: %d", p_id);
		return "";
	}
}

tt::engine::glyph::GlyphSetPtr GlyphSetMgr::ms_glyphSets[GlyphSetID_Count];


//--------------------------------------------------------------------------------------------------
// Public member functions

void GlyphSetMgr::loadAll()
{
	const std::string keyBase("toki.glyphsets.");
	
	// Allow config to specify different glyph sets
	std::string languageCode = AppGlobal::getLoc().getLanguage();
	
	// Check for languages with same glyphs and settings
	if (languageCode == "bg") languageCode = "ru"; // Bulgarian == Russian
	
	for (s32 i = 0; i < GlyphSetID_Count; ++i)
	{
		const GlyphSetID  id = static_cast<GlyphSetID>(i);
		const std::string key(keyBase + getIDName(id));
		const std::string nameKey(cfg()->getStringDirect(key + ".name"));
		
		const std::string langSpecificFilename("localization/" + nameKey + "_" + languageCode + ".glyph");
		
		// First check if language specific glyphs are available
		const std::string filename = tt::fs::fileExists(langSpecificFilename) ?
			langSpecificFilename : ("localization/" + nameKey + ".glyph");
		
		if (tt::fs::fileExists(filename) == false)
		{
			TT_PANIC("Cannot load glyph set ID '%s' (filename '%s'): file does not exist.",
			         getIDName(id), filename.c_str());
			continue;
		}
		
		// Check for language specific settings first
		const u16 characterSpacing = cfg()->hasOption(key + ".character_spacing_" + languageCode) ?
			static_cast<u16>(cfg()->getIntegerDirect(key + ".character_spacing_" + languageCode)) :
			static_cast<u16>(cfg()->getIntegerDirect(key + ".character_spacing"));
		
		const u16 wordSeparatorWidth = cfg()->hasOption(key + ".word_separator_width_" + languageCode) ?
			static_cast<u16>(cfg()->getIntegerDirect(key + ".word_separator_width_" + languageCode)) :
			static_cast<u16>(cfg()->getIntegerDirect(key + ".word_separator_width"));
		
		const u16 verticalSpacing = cfg()->hasOption(key + ".vertical_spacing_" + languageCode) ?
			static_cast<u16>(cfg()->getIntegerDirect(key + ".vertical_spacing_" + languageCode)) :
			static_cast<u16>(cfg()->getIntegerDirect(key + ".vertical_spacing"));
		
		ms_glyphSets[i].reset(new tt::engine::glyph::GlyphSet(
				filename, characterSpacing, wordSeparatorWidth, verticalSpacing));
	}
}


void GlyphSetMgr::unloadAll()
{
	for (s32 i = 0; i < GlyphSetID_Count; ++i)
	{
		ms_glyphSets[i].reset();
	}
}


const tt::engine::glyph::GlyphSetPtr& GlyphSetMgr::get(GlyphSetID p_id)
{
	TT_ASSERTMSG(p_id >= 0 && p_id < GlyphSetID_Count, "Invalid glyph set ID: %d", p_id);
	return ms_glyphSets[p_id];
}

// Namespace end
}
}
