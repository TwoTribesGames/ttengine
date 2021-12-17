#include <Gwen/Controls/Label.h>

#include <tt/input/KeyList.h>
#include <tt/platform/tt_error.h>

#include <toki/game/editor/ui/GenericDialogBox.h>
#include <toki/game/editor/helpers.h>


namespace toki {
namespace game {
namespace editor {
namespace ui {

//--------------------------------------------------------------------------------------------------
// Public member functions

GWEN_CONTROL_CONSTRUCTOR( GenericDialogBox )
,
m_buttonsPreset(DialogButtons_OK)
{
}


GenericDialogBox* GenericDialogBox::create(const Gwen::TextObject& p_title,
                                           const Gwen::TextObject& p_promptText,
                                           DialogButtons           p_buttons,
                                           Gwen::Controls::Base*   p_parent)
{
	if (isValidDialogButtons(p_buttons) == false)
	{
		TT_PANIC("Invalid dialog buttons preset: %d", p_buttons);
		return 0;
	}
	
	GenericDialogBox* dlg = new GenericDialogBox(p_parent);
	dlg->m_buttonsPreset = p_buttons;
	dlg->createUi(p_title, p_promptText);
	return dlg;
}


//--------------------------------------------------------------------------------------------------
// Private member functions

void GenericDialogBox::createUi(const Gwen::TextObject& p_title,
                                const Gwen::TextObject& p_promptText)
{
	SetTitle(p_title);
	setPromptText(p_promptText);
	
	Result closeButtonResult = Result_OK;
	
	// Create the action buttons
	Gwen::Controls::Base* buttonPanel = new Gwen::Controls::Base(this);
	buttonPanel->SetPadding(Gwen::Padding(0, 3, 0, 0));
	
	Gwen::Controls::Button* button = 0;
	switch (m_buttonsPreset)
	{
	default:
		TT_PANIC("Unsupported dialog buttons preset: %d", m_buttonsPreset);
		// Intentional fall-through, so that at least an OK button is created
		
	case DialogButtons_OK:
		button = addActionButton(buttonPanel, translateString("BUTTON_OK"), Result_OK);
		button->Dock(Gwen::Pos::Right);
		break;
		
	case DialogButtons_OKCancel:
		button = addActionButton(buttonPanel, translateString("BUTTON_CANCEL"), Result_Cancel);
		button->SetMargin(Gwen::Margin(5, 0, 0, 0));
		button->Dock(Gwen::Pos::Right);
		
		button = addActionButton(buttonPanel, translateString("BUTTON_OK"), Result_OK);
		button->Dock(Gwen::Pos::Right);
		
		closeButtonResult = Result_Cancel;
		break;
		
	case DialogButtons_RetryCancel:
		button = addActionButton(buttonPanel, translateString("BUTTON_CANCEL"), Result_Cancel);
		button->SetMargin(Gwen::Margin(5, 0, 0, 0));
		button->Dock(Gwen::Pos::Right);
		
		button = addActionButton(buttonPanel, translateString("BUTTON_RETRY"), Result_Retry);
		button->Dock(Gwen::Pos::Right);
		
		closeButtonResult = Result_Cancel;
		break;
		
	case DialogButtons_YesNo:
		button = addActionButton(buttonPanel, translateString("BUTTON_NO"), Result_No);
		button->SetMargin(Gwen::Margin(5, 0, 0, 0));
		button->Dock(Gwen::Pos::Right);
		
		button = addActionButton(buttonPanel, translateString("BUTTON_YES"), Result_Yes);
		button->Dock(Gwen::Pos::Right);
		
		closeButtonResult = Result_No;
		break;
	}
	
	buttonPanel->Dock(Gwen::Pos::Bottom);
	buttonPanel->SetHeight(25);
	
	// Closing the window using the close button is the same as picking Cancel
	m_CloseButton->onPress.RemoveHandler(this);  // Bit of a hack: disable the default close button behavior
	makeDialogCloser(m_CloseButton, closeButtonResult);
}


void GenericDialogBox::doHandleKeyInput(const tt::input::Button* p_allKeyboardKeys,
                                        bool                     p_controlDown,
                                        bool                     p_altDown,
                                        bool                     p_shiftDown,
                                        bool                     p_noModifiersDown)
{
	if (p_noModifiersDown)
	{
		if (p_allKeyboardKeys[tt::input::Key_Enter].pressed)
		{
			switch (m_buttonsPreset)
			{
			case DialogButtons_OK:
			case DialogButtons_OKCancel:
				setResult(Result_OK);
				break;
				
			case DialogButtons_RetryCancel:
				setResult(Result_Retry);
				break;
				
			case DialogButtons_YesNo:
				setResult(Result_Yes);
				break;
				
			default: break;
			}
		}
		else if (p_allKeyboardKeys[tt::input::Key_Escape].pressed)
		{
			switch (m_buttonsPreset)
			{
			case DialogButtons_OK:
				setResult(Result_OK);
				break;
				
			case DialogButtons_OKCancel:
			case DialogButtons_RetryCancel:
				setResult(Result_Cancel);
				break;
				
			case DialogButtons_YesNo:
				setResult(Result_No);
				break;
				
			default: break;
			}
		}
	}
	else if (p_altDown && p_controlDown == false && p_shiftDown == false)
	{
		// Alt+key shortcuts: use the first letter of the button captions as accelerators
		// FIXME: This isn't really localization friendly
		switch (m_buttonsPreset)
		{
		case DialogButtons_OKCancel:
			if (p_allKeyboardKeys[tt::input::Key_C].pressed)
			{
				setResult(Result_Cancel);
				break;
			}
			// Intentional fall-through for the OK shortcut
			
		case DialogButtons_OK:
			if (p_allKeyboardKeys[tt::input::Key_O].pressed)
			{
				setResult(Result_OK);
				break;
			}
			break;
			
		case DialogButtons_RetryCancel:
			if (p_allKeyboardKeys[tt::input::Key_R].pressed)
			{
				setResult(Result_Retry);
				break;
			}
			else if (p_allKeyboardKeys[tt::input::Key_C].pressed)
			{
				setResult(Result_Cancel);
				break;
			}
			break;
			
		case DialogButtons_YesNo:
			if (p_allKeyboardKeys[tt::input::Key_Y].pressed)
			{
				setResult(Result_Yes);
			}
			else if (p_allKeyboardKeys[tt::input::Key_N].pressed)
			{
				setResult(Result_No);
			}
			break;
			
		default: break;
		}
	}
}

// Namespace end
}
}
}
}
