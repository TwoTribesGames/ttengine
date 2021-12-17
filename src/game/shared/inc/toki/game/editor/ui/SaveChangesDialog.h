#if !defined(INC_TOKI_GAME_EDITOR_UI_SAVECHANGESDIALOG_H)
#define INC_TOKI_GAME_EDITOR_UI_SAVECHANGESDIALOG_H


#include <toki/game/editor/ui/DialogBoxBase.h>


namespace toki {
namespace game {
namespace editor {
namespace ui {

/*! \brief Dialog box to ask the user whether to save changes. */
class SaveChangesDialog : public DialogBoxBase
{
public:
	GWEN_CONTROL(SaveChangesDialog, DialogBoxBase);
	virtual ~SaveChangesDialog() { }
	
	static SaveChangesDialog* create(const Gwen::TextObject& p_promptText, Gwen::Controls::Base* p_parent);
	
private:
	virtual void doHandleKeyInput(const tt::input::Button* p_allKeyboardKeys,
	                              bool                     p_controlDown,
	                              bool                     p_altDown,
	                              bool                     p_shiftDown,
	                              bool                     p_noModifiersDown);
};

// Namespace end
}
}
}
}


#endif  // !defined(INC_TOKI_GAME_EDITOR_UI_SAVECHANGESDIALOG_H)
