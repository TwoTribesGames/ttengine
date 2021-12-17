#if !defined(INC_TOKI_GAME_EDITOR_COMMANDS_COMMANDSETDEFAULTMISSION_H)
#define INC_TOKI_GAME_EDITOR_COMMANDS_COMMANDSETDEFAULTMISSION_H


#include <tt/undo/UndoCommand.h>

#include <toki/game/editor/fwd.h>
#include <toki/level/types.h>


namespace toki {
namespace game {
namespace editor {
namespace commands {

class CommandSetDefaultMission;
typedef tt_ptr<CommandSetDefaultMission>::shared CommandSetDefaultMissionPtr;


class CommandSetDefaultMission : public tt::undo::UndoCommand
{
public:
	static CommandSetDefaultMissionPtr create(Editor*            p_editor,
	                                          const std::string& p_defaultMission);
	virtual ~CommandSetDefaultMission();
	
	virtual void redo();
	virtual void undo();
	
	bool hasChanges() const;
	
private:
	CommandSetDefaultMission(Editor*             p_editor,
	                         const std::string& p_originalDefaultMission,
	                         const std::string& p_newDefaultMission);
	
	
	Editor* const     m_editor;
	const std::string m_originalDefaultMission;
	const std::string m_newDefaultMission;
};

// Namespace end
}
}
}
}


#endif  // !defined(INC_TOKI_GAME_EDITOR_COMMANDS_COMMANDSETDEFAULTMISSION_H)
