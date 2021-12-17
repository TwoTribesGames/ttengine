#if !defined(INC_TOKI_GAME_EDITOR_UI_SAVEASDIALOG_H)
#define INC_TOKI_GAME_EDITOR_UI_SAVEASDIALOG_H


#include <Gwen/Controls/TextBox.h>

#include <toki/game/editor/ui/DialogBoxBase.h>
#include <toki/game/editor/fwd.h>


namespace toki {
namespace game {
namespace editor {
namespace ui {

/*! \brief Dialog box to ask the user for a new name to save the level under. */
class SaveAsDialog : public DialogBoxBase
{
public:
	GWEN_CONTROL(SaveAsDialog, DialogBoxBase);
	virtual ~SaveAsDialog() { }
	
	static SaveAsDialog* create(Editor*               p_editor,
	                            const std::string&    p_initialFilename,
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
	void onConfirmationDialogClosed(Gwen::Controls::Base* p_sender);
	
	
	Editor*                  m_editor;
	Gwen::Controls::TextBox* m_filename;
};

// Namespace end
}
}
}
}


#endif  // !defined(INC_TOKI_GAME_EDITOR_UI_SAVEASDIALOG_H)
