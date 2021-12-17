#if !defined(INC_TOKI_GAME_EDITOR_UI_TYPES_H)
#define INC_TOKI_GAME_EDITOR_UI_TYPES_H


namespace toki {
namespace game {
namespace editor {
namespace ui {

// Preset dialog buttons for the generic dialog box
enum DialogButtons
{
	DialogButtons_OK,
	DialogButtons_OKCancel,
	DialogButtons_RetryCancel,
	DialogButtons_YesNo,
	
	DialogButtons_Count
};


inline bool isValidDialogButtons(DialogButtons p_buttons)
{
	return p_buttons >= 0 && p_buttons < DialogButtons_Count;
}

// Namespace end
}
}
}
}


#endif  // !defined(INC_TOKI_GAME_EDITOR_UI_TYPES_H)
