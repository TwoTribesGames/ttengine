#include <tt/platform/tt_error.h>

#include <toki/game/editor/commands/CommandSetLevelBackground.h>
#include <toki/game/editor/Editor.h>
#include <toki/level/LevelData.h>


namespace toki {
namespace game {
namespace editor {
namespace commands {

//--------------------------------------------------------------------------------------------------
// Public member functions

CommandSetLevelBackgroundPtr CommandSetLevelBackground::create(
		Editor*            p_editor,
		const std::string& p_newBackgroundName)
{
	TT_NULL_ASSERT(p_editor);
	if (p_editor == 0)
	{
		return CommandSetLevelBackgroundPtr();
	}
	
	return CommandSetLevelBackgroundPtr(new CommandSetLevelBackground(
			p_editor,
			p_editor->getLevelData()->getLevelBackground(),
			p_newBackgroundName));
}


CommandSetLevelBackground::~CommandSetLevelBackground()
{
}


void CommandSetLevelBackground::redo()
{
	m_editor->getLevelData()->setLevelBackground(m_newBackground);
	m_editor->onLevelBackgroundChanged();
}


void CommandSetLevelBackground::undo()
{
	m_editor->getLevelData()->setLevelBackground(m_originalBackground);
	m_editor->onLevelBackgroundChanged();
}


bool CommandSetLevelBackground::hasChanges() const
{
	return m_newBackground != m_originalBackground;
}


//--------------------------------------------------------------------------------------------------
// Private member functions

CommandSetLevelBackground::CommandSetLevelBackground(Editor*            p_editor,
                                                     const std::string& p_originalBackground,
                                                     const std::string& p_newBackground)
:
tt::undo::UndoCommand(L"Set Level Background"),
m_editor            (p_editor),
m_originalBackground(p_originalBackground),
m_newBackground     (p_newBackground)
{
}

// Namespace end
}
}
}
}
