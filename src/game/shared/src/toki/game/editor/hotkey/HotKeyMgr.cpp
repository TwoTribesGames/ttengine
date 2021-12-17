#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>

#include <toki/game/editor/hotkey/HotKeyMgr.h>


namespace toki {
namespace game {
namespace editor {
namespace hotkey {

//--------------------------------------------------------------------------------------------------
// Public member functions

HotKeyMgr::HotKeyMgr()
:
m_hotKeys()
{
}


void HotKeyMgr::addHotKey(tt::input::Key    p_actionKey,
                          const Modifiers&  p_modifiers,
                          const HandlerPtr& p_handler)
{
	TT_NULL_ASSERT(p_handler);
	
#if !defined(TT_BUILD_FINAL)
	// Check for duplicate hotkeys (but do not prevent adding them)
	for (HotKeys::iterator it = m_hotKeys.begin(); it != m_hotKeys.end(); ++it)
	{
		const HotKey& hotKey(*it);
		if (hotKey.actionKey == p_actionKey && hotKey.modifiers == p_modifiers)
		{
			TT_PANIC("Hotkey '%s' is already registered!",
			         getDisplayName(p_actionKey, p_modifiers).c_str());
			break;
		}
	}
#endif
	
	m_hotKeys.push_back(HotKey(p_actionKey, p_modifiers, p_handler));
}


void HotKeyMgr::removeAllForTargetClass(void* p_targetClass)
{
	TT_NULL_ASSERT(p_targetClass);
	
	for (HotKeys::iterator it = m_hotKeys.begin(); it != m_hotKeys.end(); )
	{
		const HotKey& hotKey(*it);
		if (hotKey.actionHandler->getTargetClass() == p_targetClass)
		{
			it = m_hotKeys.erase(it);
		}
		else
		{
			++it;
		}
	}
}


void HotKeyMgr::update(const input::Controller::State::EditorState& p_editorState)
{
	// Handle the registered hotkeys
	const bool pointerDown = p_editorState.pointerLeft.down   ||
	                         p_editorState.pointerMiddle.down ||
	                         p_editorState.pointerRight.down;
	const bool ctrlDown    = p_editorState.keys[tt::input::Key_Control].down;
	const bool altDown     = p_editorState.keys[tt::input::Key_Alt    ].down;
	const bool shiftDown   = p_editorState.keys[tt::input::Key_Shift  ].down;
	
	for (HotKeys::iterator it = m_hotKeys.begin(); it != m_hotKeys.end(); ++it)
	{
		const Modifiers& m((*it).modifiers);
		
		if (p_editorState.keys[(*it).actionKey].pressed &&
		    ctrlDown  == m.checkFlag(Modifier_Ctrl )    &&
		    altDown   == m.checkFlag(Modifier_Alt  )    &&
		    shiftDown == m.checkFlag(Modifier_Shift)    &&
		    (m.checkFlag(Modifier_AllowIfPointerDown) || pointerDown == false))
		{
			(*it).actionHandler->trigger();
		}
	}
}


void HotKeyMgr::debugPrint() const
{
#if !defined(TT_BUILD_FINAL)
	TT_Printf("HotKeyMgr::debugPrint: All registered hotkeys (%u):\n", m_hotKeys.size());
	for (HotKeys::const_iterator it = m_hotKeys.begin(); it != m_hotKeys.end(); ++it)
	{
		TT_Printf("HotKeyMgr::debugPrint: - %s\n",
		          getDisplayName((*it).actionKey, (*it).modifiers).c_str());
	}
#endif
}

// Namespace end
}
}
}
}
