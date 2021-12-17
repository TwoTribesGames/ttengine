#include <tt/platform/tt_error.h>
#include <tt/undo/UndoCommand.h>
#include <tt/undo/UndoStack.h>


namespace tt {
namespace undo {

//--------------------------------------------------------------------------------------------------
// Public member functions

UndoStack::UndoStack(s32 p_undoLimit)
:
m_undoLimit(p_undoLimit),
m_currentIndex(0),
m_cleanIndex(0),
m_commands()
{
}


UndoStack::~UndoStack()
{
}


bool UndoStack::canUndo() const
{
	return m_commands.empty() == false && m_currentIndex > 0;
}


bool UndoStack::canRedo() const
{
	return m_commands.empty() == false && m_currentIndex < getCommandCount();
}


void UndoStack::undo()
{
	if (m_currentIndex == 0)
	{
		// Reached start of stack: no more commands to undo
		return;
	}
	
	--m_currentIndex;
	internalGetCommand(m_currentIndex)->undo();
}


void UndoStack::redo()
{
	if (m_currentIndex == getCommandCount())
	{
		// Reached end of stack: no more commands to redo
		return;
	}
	
	internalGetCommand(m_currentIndex)->redo();
	++m_currentIndex;
}


void UndoStack::clear()
{
	m_commands.clear();
	m_currentIndex = 0;
	m_cleanIndex   = 0;
}


ConstUndoCommandPtr UndoStack::getCommand(s32 p_index) const
{
	if (p_index < 0 || p_index >= getCommandCount())
	{
		TT_PANIC("Invalid undo command index: %d. Must be in range 0 - %d.", p_index, getCommandCount());
		return ConstUndoCommandPtr();
	}
	
	return m_commands.at(static_cast<UndoCommands::size_type>(p_index));
}


s32 UndoStack::getCommandCount() const
{
	return static_cast<s32>(m_commands.size());
}


void UndoStack::push(const UndoCommandPtr& p_command)
{
	if (p_command == 0)
	{
		TT_PANIC("Invalid undo command pointer passed.");
		return;
	}
	
	p_command->redo();
	
	// Remove all commands after the current index (as they are being overwritten with the new command)
	while (m_currentIndex < getCommandCount())
	{
		m_commands.pop_back();
	}
	
	// If the command at the clean index was removed, the stack is no longer clean
	if (m_cleanIndex > m_currentIndex)
	{
		m_cleanIndex = -1;
	}
	
	// Add the new command to the stack
	m_commands.push_back(p_command);
	enforceUndoLimit();
	++m_currentIndex;
}


void UndoStack::setCurrentIndex(s32 p_index)
{
	// Index must be valid
	if (p_index < 0 || p_index > getCommandCount())
	{
		TT_PANIC("Invalid stack index specified: %d. Must be in range 0 - %d.",
		         p_index, getCommandCount());
		return;
	}
	
	while (m_currentIndex < p_index)
	{
		internalGetCommand(m_currentIndex)->redo();
		++m_currentIndex;
	}
	
	while (m_currentIndex > p_index)
	{
		--m_currentIndex;
		internalGetCommand(m_currentIndex)->undo();
	}
}


void UndoStack::setClean()
{
	m_cleanIndex = m_currentIndex;
}


//--------------------------------------------------------------------------------------------------
// Private member functions

UndoCommandPtr UndoStack::internalGetCommand(s32 p_index)
{
	return m_commands.at(static_cast<UndoCommands::size_type>(p_index));
}


void UndoStack::enforceUndoLimit()
{
	if (m_undoLimit <= 0 || getCommandCount() <= m_undoLimit)
	{
		// Nothing to do: stack still obeys limit
		return;
	}
	
	const s32 commandsToDelete = getCommandCount() - m_undoLimit;
	for (s32 i = 0; i < commandsToDelete; ++i)
	{
		m_commands.erase(m_commands.begin());
	}
	
	m_currentIndex -= commandsToDelete;
	
	// Ensure the clean index is kept up to date
	if (m_cleanIndex != -1)
	{
		if (m_cleanIndex < commandsToDelete)
		{
			m_cleanIndex = -1;
		}
		else
		{
			m_cleanIndex -= commandsToDelete;
		}
	}
}

// Namespace end
}
}
