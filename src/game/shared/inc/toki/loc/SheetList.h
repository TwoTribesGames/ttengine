#if !defined(INC_TOKI_LOC_SHEETLIST_H)
#define INC_TOKI_LOC_SHEETLIST_H


#include <string>


namespace toki {
namespace loc {

enum SheetID
{
	SheetID_Editor,
	SheetID_Game,
	SheetID_Achievements,
	
	SheetID_Count
};


SheetID     getSheetIDFromName(const std::string& p_name);
const char* getSheetIDName(SheetID p_sheet);

// Namespace end
}
}


#endif  // !defined(INC_TOKI_LOC_SHEETLIST_H)
