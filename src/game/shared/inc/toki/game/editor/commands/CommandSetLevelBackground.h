#if !defined(INC_TOKI_GAME_EDITOR_COMMANDS_COMMANDSETLEVELBACKGROUND_H)
#define INC_TOKI_GAME_EDITOR_COMMANDS_COMMANDSETLEVELBACKGROUND_H


#include <string>

#include <tt/undo/UndoCommand.h>

#include <toki/game/editor/fwd.h>


namespace toki {
namespace game {
namespace editor {
namespace commands {

class CommandSetLevelBackground;
typedef tt_ptr<CommandSetLevelBackground>::shared CommandSetLevelBackgroundPtr;


class CommandSetLevelBackground : public tt::undo::UndoCommand
{
public:
	static CommandSetLevelBackgroundPtr create(Editor*            p_editor,
	                                           const std::string& p_newBackgroundName);
	virtual ~CommandSetLevelBackground();
	
	virtual void redo();
	virtual void undo();
	
	bool hasChanges() const;
	
private:
	CommandSetLevelBackground(Editor*            p_editor,
	                          const std::string& p_originalBackground,
	                          const std::string& p_newBackground);
	
	
	Editor* const     m_editor;
	const std::string m_originalBackground;
	const std::string m_newBackground;
};

// Namespace end
}
}
}
}


#endif  // !defined(INC_TOKI_GAME_EDITOR_COMMANDS_COMMANDSETLEVELBACKGROUND_H)
