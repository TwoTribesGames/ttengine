#include <tt/platform/tt_error.h>

#include <toki/game/editor/commands/CommandSetDefaultMission.h>
#include <toki/game/editor/Editor.h>
#include <toki/level/LevelData.h>


namespace toki {
namespace game {
namespace editor {
namespace commands {

//--------------------------------------------------------------------------------------------------
// Public member functions

CommandSetDefaultMissionPtr CommandSetDefaultMission::create(Editor*            p_editor,
                                                             const std::string& p_defaultMission)
{
	return CommandSetDefaultMissionPtr(new CommandSetDefaultMission(
			p_editor,
			p_editor->getLevelData()->getDefaultMission(),
			p_defaultMission));
}


CommandSetDefaultMission::~CommandSetDefaultMission()
{
}


void CommandSetDefaultMission::redo()
{
	m_editor->getLevelData()->setDefaultMission(m_newDefaultMission);
	m_editor->onLevelDefaultMissionChanged();
}


void CommandSetDefaultMission::undo()
{
	m_editor->getLevelData()->setDefaultMission(m_originalDefaultMission);
	m_editor->onLevelDefaultMissionChanged();
}


bool CommandSetDefaultMission::hasChanges() const
{
	return m_newDefaultMission != m_originalDefaultMission;
}


//--------------------------------------------------------------------------------------------------
// Private member functions

CommandSetDefaultMission::CommandSetDefaultMission(Editor*            p_editor,
                                                   const std::string& p_originalDefaultMission,
                                                   const std::string& p_newDefaultMission)
:
tt::undo::UndoCommand   (L"Set Default Mission"),
m_editor                (p_editor),
m_originalDefaultMission(p_originalDefaultMission),
m_newDefaultMission     (p_newDefaultMission)
{
}

// Namespace end
}
}
}
}
