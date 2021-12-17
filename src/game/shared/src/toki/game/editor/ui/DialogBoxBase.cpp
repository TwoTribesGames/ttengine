#include <Gwen/Controls/Label.h>

#include <tt/input/KeyList.h>
#include <tt/platform/tt_error.h>
#include <tt/str/str.h>

#include <toki/game/editor/ui/DialogBoxBase.h>


namespace toki {
namespace game {
namespace editor {
namespace ui {

//--------------------------------------------------------------------------------------------------
// Public member functions

GWEN_CONTROL_CONSTRUCTOR( DialogBoxBase )
,
m_dialogResult(Result_DialogStillOpen),
m_labelPrompt(0),
m_actionButtons()
{
	SetMinimumSize(Gwen::Point(50, 50));
	SetSize(350, 100);
	SetClosable(true);
	SetDeleteOnClose(true);
	
	// Create the prompt label with a default text
	m_labelPrompt = new Gwen::Controls::Label(this);
	m_labelPrompt->SetAlignment(Gwen::Pos::Top | Gwen::Pos::Left);
	m_labelPrompt->SetWrap(true);
	m_labelPrompt->Dock(Gwen::Pos::Fill);
}


DialogBoxBase::~DialogBoxBase()
{
}


void DialogBoxBase::handleKeyInput(const tt::input::Button* p_allKeyboardKeys)
{
	TT_NULL_ASSERT(p_allKeyboardKeys);
	if (p_allKeyboardKeys == 0) return;
	
	// No input handling if the dialog already has a result
	if (m_dialogResult != Result_DialogStillOpen)
	{
		return;
	}
	
	const bool controlDown = p_allKeyboardKeys[tt::input::Key_Control].down;
	const bool altDown     = p_allKeyboardKeys[tt::input::Key_Alt    ].down;
	const bool shiftDown   = p_allKeyboardKeys[tt::input::Key_Shift  ].down;
	const bool noModifiersDown =
			controlDown == false &&
			altDown     == false &&
			shiftDown   == false;
	
	doHandleKeyInput(p_allKeyboardKeys, controlDown, altDown, shiftDown, noModifiersDown);
}


void DialogBoxBase::setPromptText(const Gwen::TextObject& p_text)
{
	if (m_labelPrompt != 0)
	{
		m_labelPrompt->SetText(p_text);
	}
}


//--------------------------------------------------------------------------------------------------
// Protected member functions

void DialogBoxBase::doHandleKeyInput(const tt::input::Button* /*p_allKeyboardKeys*/,
                                     bool                     /*p_controlDown*/,
                                     bool                     /*p_altDown*/,
                                     bool                     /*p_shiftDown*/,
                                     bool                     /*p_noModifiersDown*/)
{
}


void DialogBoxBase::handleResultSet()
{
	// Do not allow further button presses once the dialog is dismissed
	for (ActionButtons::iterator it = m_actionButtons.begin(); it != m_actionButtons.end(); ++it)
	{
		(*it)->SetDisabled(true);
	}
}


void DialogBoxBase::setResult(Result p_result)
{
	TT_ASSERTMSG(p_result != Result_DialogStillOpen, "Must not set dialog result to 'dialog still open'.");
	TT_ASSERTMSG(m_dialogResult == Result_DialogStillOpen,
	             "Can only set the dialog result once (trying to set result to %d, but was already set to %d).",
	             p_result, m_dialogResult);
	TT_ASSERTMSG(isValidResult(p_result), "Trying to set dialog result to invalid value %d.", p_result);
	
	m_dialogResult = p_result;
	
	handleResultSet();
	
	CloseButtonPressed();
}


Gwen::Controls::Button* DialogBoxBase::addActionButton(Gwen::Controls::Base*   p_parent,
                                                       const Gwen::TextObject& p_title,
                                                       Result                  p_result)
{
	if (verifyValidCloseResult(p_result) == false)
	{
		return 0;
	}
	
	Gwen::Controls::Button* button = new Gwen::Controls::Button(p_parent);
	button->SetText(p_title);
	makeDialogCloser(button, p_result);
	m_actionButtons.push_back(button);
	return button;
}


void DialogBoxBase::makeDialogCloser(Gwen::Controls::Button* p_button, Result p_result)
{
	TT_NULL_ASSERT(p_button);
	if (p_button != 0 && verifyValidCloseResult(p_result))
	{
		p_button->UserData.Set("dialogResult", p_result);
		p_button->onPress.Add(this, &DialogBoxBase::setResultToSendersUserData);
	}
}


void DialogBoxBase::setResultToSendersUserData(Gwen::Controls::Base* p_sender)
{
	TT_NULL_ASSERT(p_sender);
	if (p_sender == 0) return;
	
	if (p_sender->UserData.Exists("dialogResult") == false)
	{
		TT_PANIC("Cannot set dialog result: GWEN '%s' control does not have 'dialogResult' user data set.",
		         p_sender->GetTypeName());
		return;
	}
	
	const Result result = p_sender->UserData.Get<Result>("dialogResult");
	if (verifyValidCloseResult(result))
	{
		setResult(result);
	}
}


bool DialogBoxBase::verifyValidCloseResult(Result p_result)
{
	const bool valid = isValidResult(p_result) && p_result != Result_DialogStillOpen;
	TT_ASSERTMSG(valid, "Invalid dialog result specified: value %d cannot be a dialog close result.", p_result);
	return valid;
}


void DialogBoxBase::destroyAutoCreatedUi()
{
	if (m_labelPrompt != 0)
	{
		m_labelPrompt->DelayedDelete();
		m_labelPrompt = 0;
	}
}

// Namespace end
}
}
}
}
