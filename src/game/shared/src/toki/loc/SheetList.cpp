#include <toki/loc/SheetList.h>

namespace toki {
namespace loc {


SheetID getSheetIDFromName(const std::string& p_name)
{
	for (int i = 0; i < SheetID_Count; ++i)
	{
		SheetID sheet = static_cast<SheetID>(i);
		if (p_name == getSheetIDName(sheet))
		{
			return sheet;
		}
	}
	
	return SheetID_Count;
}


const char* getSheetIDName(SheetID p_id)
{
	switch (p_id)
	{
	case SheetID_Editor:       return "editor";
	case SheetID_Game:         return "game";
	case SheetID_Achievements: return "achievements";
	default:                   return "<unknown>";
	}
}

// Namespace end
}
}

