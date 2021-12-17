#if !defined(INC_TOKI_GAME_EDITOR_HOTKEY_TYPES_H)
#define INC_TOKI_GAME_EDITOR_HOTKEY_TYPES_H


#include <string>

#include <tt/code/BitMask.h>
#include <tt/input/KeyList.h>


namespace toki {
namespace game {
namespace editor {
namespace hotkey {

enum Modifier
{
	Modifier_Ctrl,  //!< Need Ctrl  down when pressing the action key in order to trigger
	Modifier_Alt,   //!< Need Alt   down when pressing the action key in order to trigger
	Modifier_Shift, //!< Need Shift down when pressing the action key in order to trigger
	
	//! Whether to allow this hotkey if the pointer is currently down (e.g. left or right mouse button clicking)
	Modifier_AllowIfPointerDown,
	
	Modifier_Count
};

typedef tt::code::BitMask<Modifier, Modifier_Count> Modifiers;


// Helper functions to easily compose a Modifiers bitmask
inline Modifiers M()
{ return Modifiers(); }

inline Modifiers M(Modifier p_modifier1)
{ return Modifiers(p_modifier1); }

inline Modifiers M(Modifier p_modifier1, Modifier p_modifier2)
{ return Modifiers(p_modifier1) | Modifiers(p_modifier2); }

inline Modifiers M(Modifier p_modifier1, Modifier p_modifier2, Modifier p_modifier3)
{ return Modifiers(p_modifier1) | Modifiers(p_modifier2) | Modifiers(p_modifier3); }

inline Modifiers M(Modifier p_modifier1, Modifier p_modifier2, Modifier p_modifier3, Modifier p_modifier4)
{ return Modifiers(p_modifier1) | Modifiers(p_modifier2) | Modifiers(p_modifier3) | Modifiers(p_modifier4); }


std::string getDisplayName(tt::input::Key p_actionKey, const Modifiers& p_modifiers);

// Namespace end
}
}
}
}


#endif  // !defined(INC_TOKI_GAME_EDITOR_HOTKEY_TYPES_H)
