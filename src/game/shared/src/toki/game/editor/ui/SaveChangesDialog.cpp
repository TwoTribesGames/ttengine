#include <Gwen/Controls/Label.h>

#include <tt/input/KeyList.h>
#include <tt/platform/tt_error.h>
#include <tt/str/str.h>

#include <toki/game/editor/ui/SaveChangesDialog.h>
#include <toki/game/editor/helpers.h>


namespace toki {
namespace game {
namespace editor {
namespace ui {

//--------------------------------------------------------------------------------------------------
// Public member functions

GWEN_CONTROL_CONSTRUCTOR( SaveChangesDialog )
{
	const std::wstring title(translateString("WINDOW_UNSAVED_CHANGES_TITLE"));
	SetTitle     (title);
	setPromptText(title);
	
	// Create the action buttons (Save, Discard, Cancel)
	Gwen::Controls::Base* buttonPanel = new Gwen::Controls::Base(this);
	buttonPanel->SetPadding(Gwen::Padding(0, 3, 0, 0));
	
	Gwen::Controls::Button* button = 0;
	
	button = addActionButton(buttonPanel, translateString("BUTTON_SAVE"), Result_Save);
	button->Dock(Gwen::Pos::Left);
	
	button = addActionButton(buttonPanel, translateString("BUTTON_DISCARD"), Result_Discard);
	button->SetMargin(Gwen::Margin(10, 0, 10, 0));
	button->Dock(Gwen::Pos::Fill);
	
	button = addActionButton(buttonPanel, translateString("BUTTON_CANCEL"), Result_Cancel);
	button->Dock(Gwen::Pos::Right);
	
	buttonPanel->Dock(Gwen::Pos::Bottom);
	buttonPanel->SetHeight(25);
	
	// Closing the window using the close button is the same as picking Cancel
	m_CloseButton->onPress.RemoveHandler(this);  // Bit of a hack: disable the default close button behavior
	makeDialogCloser(m_CloseButton, Result_Cancel);
}


SaveChangesDialog* SaveChangesDialog::create(const Gwen::TextObject& p_promptText,
                                             Gwen::Controls::Base*   p_parent)
{
	SaveChangesDialog* dlg = new SaveChangesDialog(p_parent);
	dlg->setPromptText(p_promptText);
	return dlg;
}


//--------------------------------------------------------------------------------------------------
// Private member functions

void SaveChangesDialog::doHandleKeyInput(const tt::input::Button* p_allKeyboardKeys,
                                         bool                     p_controlDown,
                                         bool                     p_altDown,
                                         bool                     p_shiftDown,
                                         bool                     p_noModifiersDown)
{
	if (p_noModifiersDown)
	{
		if (p_allKeyboardKeys[tt::input::Key_Enter].pressed)
		{
			setResult(Result_Save);
		}
		else if (p_allKeyboardKeys[tt::input::Key_Escape].pressed)
		{
			setResult(Result_Cancel);
		}
	}
	else if (p_altDown && p_controlDown == false && p_shiftDown == false)
	{
		// Alt+key shortcuts: use the first letter of the button captions as accelerators
		// FIXME: This isn't really localization friendly
		if (p_allKeyboardKeys[tt::input::Key_S].pressed)
		{
			setResult(Result_Save);
		}
		else if (p_allKeyboardKeys[tt::input::Key_D].pressed)
		{
			setResult(Result_Discard);
		}
		else if (p_allKeyboardKeys[tt::input::Key_C].pressed)
		{
			setResult(Result_Cancel);
		}
	}
}

// Namespace end
}
}
}
}
