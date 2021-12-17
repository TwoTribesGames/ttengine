#if !defined(INC_TOKI_GAME_HUD_FWD_H)
#define INC_TOKI_GAME_HUD_FWD_H


#include <tt/platform/tt_types.h>


namespace toki {
namespace game {
namespace hud {

class DebugUI;
typedef tt_ptr<DebugUI>::shared DebugUIPtr;

class ListBox;
typedef tt_ptr<ListBox>::shared ListBoxPtr;

struct ListBoxColorScheme;

struct ListItem;

class ResolutionPicker;
typedef tt_ptr<ResolutionPicker>::shared ResolutionPickerPtr;

#if defined(TT_STEAM_BUILD)
class WorkshopLevelPicker;
typedef tt_ptr<WorkshopLevelPicker>::shared WorkshopLevelPickerPtr;
#endif

// Namespace end
}
}
}


#endif  // !defined(INC_TOKI_GAME_HUD_FWD_H)
