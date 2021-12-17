#if !defined(INC_TOKI_GAME_EDITOR_COMMANDS_COMMANDSETLEVELTHEME_H)
#define INC_TOKI_GAME_EDITOR_COMMANDS_COMMANDSETLEVELTHEME_H


#include <tt/undo/UndoCommand.h>

#include <toki/game/editor/fwd.h>
#include <toki/level/types.h>


namespace toki {
namespace game {
namespace editor {
namespace commands {

class CommandSetLevelTheme;
typedef tt_ptr<CommandSetLevelTheme>::shared CommandSetLevelThemePtr;


class CommandSetLevelTheme : public tt::undo::UndoCommand
{
public:
	static CommandSetLevelThemePtr create(Editor*          p_editor,
	                                      level::ThemeType p_levelTheme);
	virtual ~CommandSetLevelTheme();
	
	virtual void redo();
	virtual void undo();
	
	bool hasChanges() const;
	
private:
	CommandSetLevelTheme(Editor*          p_editor,
	                     level::ThemeType p_originalTheme,
	                     level::ThemeType p_newTheme);
	
	
	Editor* const          m_editor;
	const level::ThemeType m_originalTheme;
	const level::ThemeType m_newTheme;
};

// Namespace end
}
}
}
}


#endif  // !defined(INC_TOKI_GAME_EDITOR_COMMANDS_COMMANDSETLEVELTHEME_H)
