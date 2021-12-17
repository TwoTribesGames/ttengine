#if !defined(INC_TOKI_GAME_EDITOR_HOTKEY_HOTKEYMGR_H)
#define INC_TOKI_GAME_EDITOR_HOTKEY_HOTKEYMGR_H


#include <vector>

#include <toki/game/editor/hotkey/Handler.h>
#include <toki/game/editor/hotkey/types.h>
#include <toki/input/Controller.h>


namespace toki {
namespace game {
namespace editor {
namespace hotkey {

class HotKeyMgr
{
public:
	HotKeyMgr();
	
	void addHotKey(tt::input::Key    p_actionKey,
	               const Modifiers&  p_modifiers,
	               const HandlerPtr& p_handler);
	void removeAllForTargetClass(void* p_targetClass);
	
	void update(const input::Controller::State::EditorState& p_editorState);
	
	void debugPrint() const;
	
private:
	struct HotKey
	{
		tt::input::Key actionKey;
		Modifiers      modifiers;
		HandlerPtr     actionHandler;
		
		inline explicit HotKey(
				tt::input::Key    p_actionKey     = tt::input::Key_Space,
				const Modifiers&  p_modifiers     = Modifiers(),
				const HandlerPtr& p_actionHandler = HandlerPtr())
		:
		actionKey(p_actionKey),
		modifiers(p_modifiers),
		actionHandler(p_actionHandler)
		{ }
	};
	typedef std::vector<HotKey> HotKeys;
	
	
	HotKeys m_hotKeys;
};

// Namespace end
}
}
}
}


#endif  // !defined(INC_TOKI_GAME_EDITOR_HOTKEY_HOTKEYMGR_H)
