#if !defined(INC_TOKI_GAME_EDITOR_TYPES_H)
#define INC_TOKI_GAME_EDITOR_TYPES_H


#include <string>

#include <tt/platform/tt_error.h>
#include <tt/platform/tt_types.h>


namespace toki {
namespace game {
namespace editor {

enum EditCursor
{
	EditCursor_NormalArrow,
	EditCursor_OpenHand,
	EditCursor_ClosedHand,
	EditCursor_SelectionRectCreate,
	EditCursor_SelectionRectMove,
	EditCursor_SelectionContentCut,
	EditCursor_SelectionContentClone,
	EditCursor_SelectionContentMove,
	EditCursor_ResizeTopBottom,
	EditCursor_ResizeLeftRight,
	EditCursor_ResizeTopLeftBottomRight,
	EditCursor_ResizeTopRightBottomLeft,
	EditCursor_NotAllowed,
	EditCursor_Draw,
	EditCursor_DrawEntity,
	EditCursor_FloodFill,
	EditCursor_GenericMove,
	EditCursor_Erase,
	EditCursor_TilePicker,
	EditCursor_EntityPickerInvalid,  //!< Pointer is not over a valid entity to pick
	EditCursor_EntityPickerValid,    //!< Pointer is over a valid entity to pick
	EditCursor_AddNote,
	EditCursor_TextEdit,
	
	EditCursor_Count
};


enum LoadLevelFilter
{
	LoadLevelFilter_AllLevels,
	LoadLevelFilter_Slices,
	LoadLevelFilter_UserLevels,
	
	LoadLevelFilter_Count,
	LoadLevelFilter_Invalid
};


inline bool isValidLoadLevelFilter(LoadLevelFilter p_filter)
{ return p_filter >= 0 && p_filter < LoadLevelFilter_Count; }


inline const char* const getLoadLevelFilterName(LoadLevelFilter p_filter)
{
	switch (p_filter)
	{
	case LoadLevelFilter_AllLevels:  return "all_levels";
	case LoadLevelFilter_Slices:     return "slices";
	case LoadLevelFilter_UserLevels: return "user_levels";
		
	default:
		TT_PANIC("Invalid load level filter: %d", p_filter);
		return "";
	}
}


inline LoadLevelFilter getLoadLevelFilterFromName(const std::string& p_name)
{
	for (s32 i = 0; i < LoadLevelFilter_Count; ++i)
	{
		const LoadLevelFilter filter = static_cast<LoadLevelFilter>(i);
		if (getLoadLevelFilterName(filter) == p_name)
		{
			return filter;
		}
	}
	
	return LoadLevelFilter_Invalid;
}

// Namespace end
}
}
}


#endif  // !defined(INC_TOKI_GAME_EDITOR_TYPES_H)
