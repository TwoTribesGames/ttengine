#include <tt/platform/tt_error.h>

#include <toki/game/editor/commands/CommandSetLevelTheme.h>
#include <toki/game/editor/Editor.h>
#include <toki/level/LevelData.h>


namespace toki {
namespace game {
namespace editor {
namespace commands {

//--------------------------------------------------------------------------------------------------
// Public member functions

CommandSetLevelThemePtr CommandSetLevelTheme::create(
		Editor*          p_editor,
		level::ThemeType p_levelTheme)
{
	TT_NULL_ASSERT(p_editor);
	const bool validTheme = level::LevelData::isValidLevelTheme(p_levelTheme);
	
	TT_ASSERTMSG(validTheme, "Invalid level theme: %d", p_levelTheme);
	if (p_editor == 0 || validTheme == false)
	{
		return CommandSetLevelThemePtr();
	}
	
	return CommandSetLevelThemePtr(new CommandSetLevelTheme(
			p_editor,
			p_editor->getLevelData()->getLevelTheme(),
			p_levelTheme));
}


CommandSetLevelTheme::~CommandSetLevelTheme()
{
}


void CommandSetLevelTheme::redo()
{
	m_editor->getLevelData()->setLevelTheme(m_newTheme);
	m_editor->onLevelThemeChanged();
}


void CommandSetLevelTheme::undo()
{
	m_editor->getLevelData()->setLevelTheme(m_originalTheme);
	m_editor->onLevelThemeChanged();
}


bool CommandSetLevelTheme::hasChanges() const
{
	return m_newTheme != m_originalTheme;
}


//--------------------------------------------------------------------------------------------------
// Private member functions

CommandSetLevelTheme::CommandSetLevelTheme(Editor*          p_editor,
                                           level::ThemeType p_originalTheme,
                                           level::ThemeType p_newTheme)
:
tt::undo::UndoCommand(L"Set Level Theme"),
m_editor       (p_editor),
m_originalTheme(p_originalTheme),
m_newTheme     (p_newTheme)
{
}

// Namespace end
}
}
}
}
