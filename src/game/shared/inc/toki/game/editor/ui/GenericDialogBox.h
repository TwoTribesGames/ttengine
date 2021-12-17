#if !defined(INC_TOKI_GAME_EDITOR_UI_GENERICDIALOGBOX_H)
#define INC_TOKI_GAME_EDITOR_UI_GENERICDIALOGBOX_H


#include <toki/game/editor/ui/DialogBoxBase.h>
#include <toki/game/editor/ui/types.h>


namespace toki {
namespace game {
namespace editor {
namespace ui {

/*! \brief Generic dialog box: just displays some text with an OK button. */
class GenericDialogBox : public DialogBoxBase
{
public:
	GWEN_CONTROL(GenericDialogBox, DialogBoxBase);
	virtual ~GenericDialogBox() { }
	
	static GenericDialogBox* create(const Gwen::TextObject& p_title,
	                                const Gwen::TextObject& p_promptText,
	                                DialogButtons           p_buttons,
	                                Gwen::Controls::Base*   p_parent);
	
private:
	void createUi(const Gwen::TextObject& p_title,
	              const Gwen::TextObject& p_promptText);
	
	virtual void doHandleKeyInput(const tt::input::Button* p_allKeyboardKeys,
	                              bool                     p_controlDown,
	                              bool                     p_altDown,
	                              bool                     p_shiftDown,
	                              bool                     p_noModifiersDown);
	
	
	DialogButtons m_buttonsPreset;
};

// Namespace end
}
}
}
}


#endif  // !defined(INC_TOKI_GAME_EDITOR_UI_GENERICDIALOGBOX_H)
