#include <tt/undo/UndoCommand.h>


namespace tt {
namespace undo {

//--------------------------------------------------------------------------------------------------
// Public member functions

UndoCommand::UndoCommand(const std::wstring& p_displayName)
:
m_displayName(p_displayName)
{
}


UndoCommand::~UndoCommand()
{
}

// Namespace end
}
}
