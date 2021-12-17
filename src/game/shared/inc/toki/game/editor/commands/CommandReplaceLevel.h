#if !defined(INC_TOKI_GAME_EDITOR_COMMANDS_COMMANDREPLACELEVEL_H)
#define INC_TOKI_GAME_EDITOR_COMMANDS_COMMANDREPLACELEVEL_H


#include <tt/undo/UndoCommand.h>

#include <toki/game/editor/fwd.h>
#include <toki/level/fwd.h>


namespace toki {
namespace game {
namespace editor {
namespace commands {

class CommandReplaceLevel;
typedef tt_ptr<CommandReplaceLevel>::shared CommandReplaceLevelPtr;


/*! \brief Replaces the editor's level data with different level data.
           Useful for making massive level changes without requiring all kinds of separate undo steps. */
class CommandReplaceLevel : public tt::undo::UndoCommand
{
public:
	static CommandReplaceLevelPtr create(Editor*                   p_editor,
	                                    const level::LevelDataPtr& p_replacementLevel);
	virtual ~CommandReplaceLevel();
	
	virtual void redo();
	virtual void undo();
	
private:
	CommandReplaceLevel(Editor*                    p_editor,
	                    const level::LevelDataPtr& p_originalLevel,
	                    const level::LevelDataPtr& p_replacementLevel);
	
	
	Editor* const             m_editor;
	const level::LevelDataPtr m_originalLevel;
	const level::LevelDataPtr m_replacementLevel;
	bool                      m_canRedo;
	bool                      m_canUndo;
};

// Namespace end
}
}
}
}


#endif  // !defined(INC_TOKI_GAME_EDITOR_COMMANDS_COMMANDREPLACELEVEL_H)
