#include <tt/platform/tt_error.h>

#include <toki/game/editor/commands/CommandResizeLevel.h>
#include <toki/game/editor/Editor.h>
#include <toki/level/LevelData.h>


namespace toki {
namespace game {
namespace editor {
namespace commands {

//--------------------------------------------------------------------------------------------------
// Public member functions

CommandResizeLevelPtr CommandResizeLevel::create(
		Editor*                    p_editorToNotify,
		const level::LevelDataPtr& p_targetLevel,
		const tt::math::PointRect& p_newRect)
{
	TT_NULL_ASSERT(p_editorToNotify);
	TT_NULL_ASSERT(p_targetLevel);
	TT_ASSERTMSG(p_newRect.hasArea(),
	             "Invalid new level dimensions: %d x %d. Must be at least 1 x 1.",
	             p_newRect.getWidth(), p_newRect.getHeight());
	if (p_editorToNotify == 0 || p_targetLevel == 0 || p_newRect.hasArea() == false)
	{
		return CommandResizeLevelPtr();
	}
	
	if (p_newRect == p_targetLevel->getLevelRect())
	{
		TT_PANIC("Level rectangle did not change: this is not a resize.");
		return CommandResizeLevelPtr();
	}
	
	level::LevelDataPtr resizedLevel(p_targetLevel->clone());
	resizedLevel->resizeTo(p_newRect);
	
	return CommandResizeLevelPtr(new CommandResizeLevel(p_editorToNotify, p_targetLevel, resizedLevel));
}


CommandResizeLevel::~CommandResizeLevel()
{
}


void CommandResizeLevel::redo()
{
	if (m_canRedo == false)
	{
		// Cannot currently perform the redo step
		return;
	}
	
	m_targetLevel->swap(m_resizedLevel);
	
	m_canRedo = false;
	m_canUndo = true;
	
	m_editorToNotify->handleLevelResized();
}


void CommandResizeLevel::undo()
{
	if (m_canUndo == false)
	{
		// Cannot currently perform the undo step
		return;
	}
	
	// Swapping target and resized again restores the resized level data to m_resizedLevel
	m_targetLevel->swap(m_resizedLevel);
	
	m_canRedo = true;
	m_canUndo = false;
	
	m_editorToNotify->handleLevelResized();
}


//--------------------------------------------------------------------------------------------------
// Private member functions

CommandResizeLevel::CommandResizeLevel(Editor*                    p_editorToNotify,
                                       const level::LevelDataPtr& p_targetLevel,
                                       const level::LevelDataPtr& p_resizedLevel)
:
tt::undo::UndoCommand(L"Resize Level"),
m_editorToNotify(p_editorToNotify),
m_targetLevel(p_targetLevel),
m_resizedLevel(p_resizedLevel),
m_canRedo(true),
m_canUndo(false)
{
}

// Namespace end
}
}
}
}
