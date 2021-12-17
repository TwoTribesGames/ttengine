#if !defined(INC_TOKI_GAME_EDITOR_UI_FWD_H)
#define INC_TOKI_GAME_EDITOR_UI_FWD_H


#include <tt/platform/tt_types.h>


namespace toki {
namespace game {
namespace editor {
namespace ui {

class DialogBoxBase;
class EntityPropertyControl;
class EntityPropertyList;
class GenericDialogBox;
class LevelNameTextBox;
class NewLevelDialog;
#if defined(TT_STEAM_BUILD)
class PublishedLevelBrowser;
class PublishToWorkshopDialog;
#endif
class SaveAsDialog;
class SaveChangesDialog;
class SetThemeColorUI;
class SvnCommands;

typedef tt_ptr<SvnCommands>::shared SvnCommandsPtr;

// Namespace end
}
}
}
}


#endif  // !defined(INC_TOKI_GAME_EDITOR_UI_FWD_H)
