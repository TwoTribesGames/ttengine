#include <tt/platform/tt_error.h>

#include <toki/game/editor/commands/CommandReplaceLevel.h>
#include <toki/game/editor/Editor.h>
#include <toki/level/LevelData.h>


namespace toki {
namespace game {
namespace editor {
namespace commands {

//--------------------------------------------------------------------------------------------------
// Public member functions

CommandReplaceLevelPtr CommandReplaceLevel::create(
		Editor*                    p_editor,
		const level::LevelDataPtr& p_replacementLevel)
{
	TT_NULL_ASSERT(p_editor);
	TT_NULL_ASSERT(p_replacementLevel);
	if (p_editor == 0 || p_replacementLevel == 0)
	{
		return CommandReplaceLevelPtr();
	}
	
	if (p_editor->getLevelData() == p_replacementLevel)
	{
		TT_PANIC("Cannot replace the editor's level with the same level.");
		return CommandReplaceLevelPtr();
	}
	
	return CommandReplaceLevelPtr(new CommandReplaceLevel(
			p_editor, p_editor->getLevelData(), p_replacementLevel));
}


CommandReplaceLevel::~CommandReplaceLevel()
{
}


void CommandReplaceLevel::redo()
{
	if (m_canRedo == false)
	{
		// Cannot currently perform the redo step
		return;
	}
	
	m_originalLevel->swap(m_replacementLevel);
	
	m_canRedo = false;
	m_canUndo = true;
	
	m_editor->handleLevelResized();
}


void CommandReplaceLevel::undo()
{
	if (m_canUndo == false)
	{
		// Cannot currently perform the undo step
		return;
	}
	
	// Swapping target and resized again restores the resized level data to m_resizedLevel
	m_originalLevel->swap(m_replacementLevel);
	
	m_canRedo = true;
	m_canUndo = false;
	
	m_editor->handleLevelResized();
}


//--------------------------------------------------------------------------------------------------
// Private member functions

CommandReplaceLevel::CommandReplaceLevel(Editor*                    p_editor,
                                         const level::LevelDataPtr& p_originalLevel,
                                         const level::LevelDataPtr& p_replacementLevel)
:
tt::undo::UndoCommand(L"Replace Level"),
m_editor          (p_editor),
m_originalLevel   (p_originalLevel),
m_replacementLevel(p_replacementLevel),
m_canRedo(true),
m_canUndo(false)
{
}

// Namespace end
}
}
}
}
