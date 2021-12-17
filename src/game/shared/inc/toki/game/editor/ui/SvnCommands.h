#if !defined(INC_TOKI_GAME_EDITOR_UI_SVNCOMMANDS_H)
#define INC_TOKI_GAME_EDITOR_UI_SVNCOMMANDS_H


#include <string>

#include <Gwen/Controls/MenuItem.h>
#include <Gwen/Events.h>

#include <toki/game/editor/ui/fwd.h>
#include <toki/game/editor/hotkey/types.h>
#include <toki/game/editor/features.h>
#include <toki/game/editor/fwd.h>


namespace toki {
namespace game {
namespace editor {
namespace ui {

/*! \brief Provides Subversion commands for the current level file.
           Only supported in internal builds on Windows. */
class SvnCommands : public Gwen::Event::Handler
{
public:
	static SvnCommandsPtr create(Editor* p_editor, Gwen::Controls::MenuItem* p_targetMenu);
	
private:
	SvnCommands(Editor* p_editor, Gwen::Controls::MenuItem* p_targetMenu);
	
	// No copying
	SvnCommands(const SvnCommands&);
	SvnCommands& operator=(const SvnCommands&);
	
#if EDITOR_SUPPORTS_SUBVERSION
	typedef void (SvnCommands::*Action)();
	
	void addMenuItem(Gwen::Controls::Menu* p_targetMenu,
	                 const std::string&    p_displayName,
	                 Action                p_actionFunc);
	void addMenuItem(Gwen::Controls::Menu*    p_targetMenu,
	                 const std::string&       p_displayName,
	                 Action                   p_actionFunc,
	                 Editor*                  p_editor,
	                 tt::input::Key           p_actionKey,
	                 const hotkey::Modifiers& p_modifiers);
	
	void runTortoiseProc(const std::string& p_command,
	                     const std::string& p_extraArguments = std::string());
	
	// UI handlers
	void onCommandLock();
	void onCommandUnlock();
	void onCommandCommit();
	void onCommandUpdate();
	void onCommandShowLog();
	
	
	Editor* m_editor;
#endif
};

// Namespace end
}
}
}
}


#endif  // !defined(INC_TOKI_GAME_EDITOR_UI_SVNCOMMANDS_H)
