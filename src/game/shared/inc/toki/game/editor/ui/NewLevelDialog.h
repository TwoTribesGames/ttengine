#if !defined(INC_TOKI_GAME_EDITOR_UI_NEWLEVELDIALOG_H)
#define INC_TOKI_GAME_EDITOR_UI_NEWLEVELDIALOG_H


#include <Gwen/Controls/TextBox.h>

#include <toki/game/editor/ui/DialogBoxBase.h>
#include <toki/game/editor/fwd.h>


namespace toki {
namespace game {
namespace editor {
namespace ui {

/*! \brief Dialog box that allows the user to create a new level. */
class NewLevelDialog : public DialogBoxBase
{
public:
	GWEN_CONTROL(NewLevelDialog, DialogBoxBase);
	virtual ~NewLevelDialog() { }
	
	static NewLevelDialog* create(Editor*               p_editor,
	                              Gwen::Controls::Base* p_parent);
	
	/*! \brief Returns the filename that the user entered. */
	std::string getFilename() const;
	
private:
	virtual void doHandleKeyInput(const tt::input::Button* p_allKeyboardKeys,
	                              bool                     p_controlDown,
	                              bool                     p_altDown,
	                              bool                     p_shiftDown,
	                              bool                     p_noModifiersDown);
	
	void onButtonSave();
	void onConfirmationDialogClosed();
	
	
	Editor*                  m_editor;
	Gwen::Controls::TextBox* m_filename;
};

// Namespace end
}
}
}
}


#endif  // !defined(INC_TOKI_GAME_EDITOR_UI_NEWLEVELDIALOG_H)
