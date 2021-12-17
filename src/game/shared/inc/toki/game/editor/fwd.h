#if !defined(INC_TOKI_GAME_EDITOR_FWD_H)
#define INC_TOKI_GAME_EDITOR_FWD_H


#include <tt/platform/tt_types.h>


namespace toki {
namespace game {
namespace editor {

class Editor;
typedef tt_ptr<Editor>::shared EditorPtr;

struct EditorSettings;

// Namespace end
}
}
}


#endif  // !defined(INC_TOKI_GAME_EDITOR_FWD_H)
