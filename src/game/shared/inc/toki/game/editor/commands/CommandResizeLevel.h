#if !defined(INC_TOKI_GAME_EDITOR_COMMANDS_COMMANDRESIZELEVEL_H)
#define INC_TOKI_GAME_EDITOR_COMMANDS_COMMANDRESIZELEVEL_H


#include <tt/math/Rect.h>
#include <tt/undo/UndoCommand.h>

#include <toki/game/editor/fwd.h>
#include <toki/level/fwd.h>


namespace toki {
namespace game {
namespace editor {
namespace commands {

class CommandResizeLevel;
typedef tt_ptr<CommandResizeLevel>::shared CommandResizeLevelPtr;


class CommandResizeLevel : public tt::undo::UndoCommand
{
public:
	static CommandResizeLevelPtr create(Editor*                    p_editorToNotify,
	                                    const level::LevelDataPtr& p_targetLevel,
	                                    const tt::math::PointRect& p_newRect);
	virtual ~CommandResizeLevel();
	
	virtual void redo();
	virtual void undo();
	
private:
	CommandResizeLevel(Editor*                    p_editorToNotify,
	                   const level::LevelDataPtr& p_targetLevel,
	                   const level::LevelDataPtr& p_resizedLevel);
	
	
	Editor*             m_editorToNotify;
	level::LevelDataPtr m_targetLevel;
	level::LevelDataPtr m_resizedLevel;
	bool                m_canRedo;
	bool                m_canUndo;
};

// Namespace end
}
}
}
}


#endif  // !defined(INC_TOKI_GAME_EDITOR_COMMANDS_COMMANDRESIZELEVEL_H)
