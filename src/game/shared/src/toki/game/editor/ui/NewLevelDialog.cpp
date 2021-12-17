#include <Gwen/Controls/Label.h>

#include <tt/fs/fs.h>
#include <tt/input/KeyList.h>
#include <tt/platform/tt_error.h>
#include <tt/str/str.h>

#include <toki/game/editor/ui/GenericDialogBox.h>
#include <toki/game/editor/ui/LevelNameTextBox.h>
#include <toki/game/editor/ui/NewLevelDialog.h>
#include <toki/game/editor/Editor.h>
#include <toki/game/editor/features.h>
#include <toki/game/editor/helpers.h>
#include <toki/game/StartInfo.h>


namespace toki {
namespace game {
namespace editor {
namespace ui {

//--------------------------------------------------------------------------------------------------
// Public member functions

GWEN_CONTROL_CONSTRUCTOR( NewLevelDialog )
,
m_filename(0)
{
	SetTitle     (translateString("WINDOW_NEWLEVEL_TITLE"));
	setPromptText(translateString("WINDOW_NEWLEVEL_PROMPT"));
	
	// Create the action buttons (Save, Cancel)
	Gwen::Controls::Base* buttonPanel = new Gwen::Controls::Base(this);
	buttonPanel->SetPadding(Gwen::Padding(0, 3, 0, 0));
	
	Gwen::Controls::Button* button = 0;
	
	button = addActionButton(buttonPanel, translateString("BUTTON_CANCEL"), Result_Cancel);
	button->Dock(Gwen::Pos::Right);
	
	button = new Gwen::Controls::Button(buttonPanel);
	button->SetText(translateString("BUTTON_SAVE"));
	button->SetMargin(Gwen::Margin(0, 0, 5, 0));
	button->Dock(Gwen::Pos::Right);
	button->onPress.Add(this, &NewLevelDialog::onButtonSave);
	
	buttonPanel->Dock(Gwen::Pos::Bottom);
	buttonPanel->SetHeight(25);
	
	// Text box for the filename
	m_filename = new LevelNameTextBox(this);
	m_filename->SetHeight(22);
	m_filename->Dock(Gwen::Pos::Bottom);
	m_filename->onReturnPressed.Add(this, &NewLevelDialog::onButtonSave);
	m_filename->Focus();
	
	// Closing the window using the close button is the same as picking Cancel
	m_CloseButton->onPress.RemoveHandler(this);  // Bit of a hack: disable the default close button behavior
	makeDialogCloser(m_CloseButton, Result_Cancel);
}


NewLevelDialog* NewLevelDialog::create(Editor*               p_editor,
                                       Gwen::Controls::Base* p_parent)
{
	TT_NULL_ASSERT(p_editor);
	if (p_editor == 0)
	{
		return 0;
	}
	
	NewLevelDialog* dlg = new NewLevelDialog(p_parent);
	dlg->m_editor = p_editor;
	return dlg;
}


std::string NewLevelDialog::getFilename() const
{
	return m_filename->GetText().Get();
}


//--------------------------------------------------------------------------------------------------
// Private member functions

void NewLevelDialog::doHandleKeyInput(const tt::input::Button* p_allKeyboardKeys,
                                      bool                     p_controlDown,
                                      bool                     p_altDown,
                                      bool                     p_shiftDown,
                                      bool                     p_noModifiersDown)
{
	if (p_noModifiersDown)
	{
		if (p_allKeyboardKeys[tt::input::Key_Enter].pressed)
		{
			onButtonSave();
		}
		else if (p_allKeyboardKeys[tt::input::Key_Escape].pressed)
		{
			setResult(Result_Cancel);
		}
	}
	else if (p_altDown && p_controlDown == false && p_shiftDown == false)
	{
		// Alt+key shortcuts: use the first letter of the button captions as accelerators
		if (p_allKeyboardKeys[tt::input::Key_S].pressed)
		{
			onButtonSave();
		}
		else if (p_allKeyboardKeys[tt::input::Key_C].pressed)
		{
			setResult(Result_Cancel);
		}
	}
}


void NewLevelDialog::onButtonSave()
{
	// Remove leading and trailing whitespace from the level name
	m_filename->SetText(tt::str::trim(m_filename->GetText().GetUnicode()));
	
	const std::string newName(getFilename());
	if (newName.empty())
	{
		// No filename entered: ignore
		return;
	}
	
	// Check if the entered level name already exists on disk
	const std::string path(m_editor->getCurrentLevelInfo().getLevelPath() + newName + ".ttlvl");
	
	bool newLevelExists = tt::fs::fileExists(path);
	
#if EDITOR_SUPPORTS_ASSETS_SOURCE
	// For internal builds, also check if the source file exists
	// (hard-coded relative path to the asset source files)
	if (m_editor->getCurrentLevelInfo().isUserLevel() == false)
	{
		const std::string sourcePath(getLevelsSourceDir() + newName + ".ttlvl");
		newLevelExists = newLevelExists || tt::fs::fileExists(sourcePath);
	}
#endif
	
	if (newLevelExists)
	{
		// The entered filename already exists: ask the user to confirm overwriting this file
		GenericDialogBox* confirm = m_editor->showGenericDialog(
				translateString("WINDOW_NEWLEVEL_EXISTS_TITLE"),
				translateString("WINDOW_NEWLEVEL_EXISTS_PROMPT"),
				DialogButtons_OK,
				true);
		confirm->SetSize(425, 85);
		confirm->SetPos(X() + ((Width()  - confirm->Width())  / 2),
		                Y() + ((Height() - confirm->Height()) / 2));
		confirm->onWindowClosed.Add(this, &NewLevelDialog::onConfirmationDialogClosed);
		return;
	}
	else
	{
		// Can save the level under this name: close the dialog
		setResult(Result_Save);
	}
}


void NewLevelDialog::onConfirmationDialogClosed()
{
	m_filename->Focus();
}

// Namespace end
}
}
}
}
