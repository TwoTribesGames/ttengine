#include <tt/platform/tt_error.h>

#include <toki/game/editor/commands/CommandChangeNoteRect.h>
#include <toki/level/Note.h>


namespace toki {
namespace game {
namespace editor {
namespace commands {

//--------------------------------------------------------------------------------------------------
// Public member functions

CommandChangeNoteRectPtr CommandChangeNoteRect::create(const level::NotePtr& p_noteToChange)
{
	TT_NULL_ASSERT(p_noteToChange);
	if (p_noteToChange == 0)
	{
		return CommandChangeNoteRectPtr();
	}
	
	return CommandChangeNoteRectPtr(new CommandChangeNoteRect(p_noteToChange));
}


CommandChangeNoteRect::~CommandChangeNoteRect()
{
}


void CommandChangeNoteRect::redo()
{
	// UndoStack::push calls redo(): this is a good indicator that this command has been added to an undo stack
	m_addedToStack = true;
	
	m_note->setWorldRect(m_newRect);
}


void CommandChangeNoteRect::undo()
{
	m_note->setWorldRect(m_originalRect);
}


void CommandChangeNoteRect::setPosition(const tt::math::Vector2& p_pos)
{
	if (m_addedToStack)
	{
		TT_PANIC("Should not call setPosition after this command has already been added to an undo stack.");
		return;
	}
	
	m_newRect.setPosition(p_pos);
	m_note->setPosition(p_pos);
}


void CommandChangeNoteRect::setRect(const tt::math::VectorRect& p_rect)
{
	if (m_addedToStack)
	{
		TT_PANIC("Should not call setRect after this command has already been added to an undo stack.");
		return;
	}
	
	m_newRect = p_rect;
	m_note->setWorldRect(p_rect);
}


//--------------------------------------------------------------------------------------------------
// Private member functions

CommandChangeNoteRect::CommandChangeNoteRect(const level::NotePtr& p_noteToChange)
:
tt::undo::UndoCommand(L"Change Note Rect"),
m_addedToStack(false),
m_note(p_noteToChange),
m_originalRect(p_noteToChange->getWorldRect()),
m_newRect(p_noteToChange->getWorldRect())
{
}

// Namespace end
}
}
}
}
