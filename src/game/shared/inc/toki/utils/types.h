#if !defined(INC_TOKI_UTILS_TYPES_H)
#define INC_TOKI_UTILS_TYPES_H


#include <string>
#include <utility>
#include <vector>


namespace toki  /*! */ {
namespace utils /*! */ {

/*! \brief GlyphSetID */
enum GlyphSetID
{
	GlyphSetID_Title,           //!< Font for titles.
	GlyphSetID_Header,          //!< Font for header.
	GlyphSetID_Text,            //!< Font for text.
	GlyphSetID_Notes,           //!< Font used for editor and in-game notes.
	GlyphSetID_EditorHelpText,  //!< Font used for editor help text (for e.g. note edit, entity pick).
	
	GlyphSetID_Count
};


typedef std::pair<std::string, std::string> StringPair;
typedef std::vector<StringPair>             StringPairs;

// Namespace end
}
}


#endif  // !defined(INC_TOKI_UTILS_TYPES_H)
