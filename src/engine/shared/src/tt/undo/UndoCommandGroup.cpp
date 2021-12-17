#include <algorithm>

#include <tt/platform/tt_error.h>
#include <tt/undo/UndoCommandGroup.h>


namespace tt {
namespace undo {

//--------------------------------------------------------------------------------------------------
// Public member functions

UndoCommandGroupPtr UndoCommandGroup::create(const std::wstring& p_displayName)
{
	return UndoCommandGroupPtr(new UndoCommandGroup(p_displayName));
}


UndoCommandGroup::~UndoCommandGroup()
{
}


void UndoCommandGroup::addCommand(const UndoCommandPtr& p_command)
{
	// Do not allow modification after command has been added to UndoStack
	if (m_addedToStack)
	{
		TT_PANIC("Cannot add UndoCommand to group after it has already been added to the undo stack.");
		return;
	}
	
	// Do not allow null commands
	TT_NULL_ASSERT(p_command);
	if (p_command == 0)
	{
		return;
	}
	
	// Do not allow duplicate commands
	if (std::find(m_commands.begin(), m_commands.end(), p_command) != m_commands.end())
	{
		TT_PANIC("This UndoCommand (%p) has already been added to this UndoCommandGroup.",
		         p_command.get());
		return;
	}
	
	m_commands.push_back(p_command);
}


void UndoCommandGroup::redo()
{
	// UndoStack::push calls redo(): this is a good indicator that this command has been added to an undo stack
	m_addedToStack = true;
	
	for (UndoCommands::iterator it = m_commands.begin(); it != m_commands.end(); ++it)
	{
		(*it)->redo();
	}
}


void UndoCommandGroup::undo()
{
	for (UndoCommands::iterator it = m_commands.begin(); it != m_commands.end(); ++it)
	{
		(*it)->undo();
	}
}


//--------------------------------------------------------------------------------------------------
// Private member functions

UndoCommandGroup::UndoCommandGroup(const std::wstring& p_displayName)
:
UndoCommand(p_displayName),
m_addedToStack(false),
m_commands()
{
}

// Namespace end
}
}
