#include <tt/platform/tt_error.h>

#include <toki/game/editor/commands/CommandChangeNoteText.h>
#include <toki/level/Note.h>


namespace toki {
namespace game {
namespace editor {
namespace commands {

//--------------------------------------------------------------------------------------------------
// Public member functions

CommandChangeNoteTextPtr CommandChangeNoteText::create(const level::NotePtr& p_noteToChange)
{
	TT_NULL_ASSERT(p_noteToChange);
	if (p_noteToChange == 0)
	{
		return CommandChangeNoteTextPtr();
	}
	
	return CommandChangeNoteTextPtr(new CommandChangeNoteText(p_noteToChange));
}


CommandChangeNoteText::~CommandChangeNoteText()
{
}


void CommandChangeNoteText::redo()
{
	// UndoStack::push calls redo(): this is a good indicator that this command has been added to an undo stack
	m_addedToStack = true;
	
	m_note->setText(m_newText);
}


void CommandChangeNoteText::undo()
{
	m_note->setText(m_originalText);
}


void CommandChangeNoteText::setText(const std::wstring& p_text)
{
	if (m_addedToStack)
	{
		TT_PANIC("Should not call setText after this command has already been added to an undo stack.");
		return;
	}
	
	m_newText = p_text;
	m_note->setText(p_text);
}


bool CommandChangeNoteText::isTextChanged() const
{
	return m_newText != m_originalText;
}


//--------------------------------------------------------------------------------------------------
// Private member functions

CommandChangeNoteText::CommandChangeNoteText(const level::NotePtr& p_noteToChange)
:
tt::undo::UndoCommand(L"Change Note Text"),
m_addedToStack(false),
m_note(p_noteToChange),
m_originalText(p_noteToChange->getText()),
m_newText(p_noteToChange->getText())
{
}

// Namespace end
}
}
}
}
