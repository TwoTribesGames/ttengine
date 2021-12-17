#if !defined(INC_TT_MENU_MENUKEYBOARD_H)
#define INC_TT_MENU_MENUKEYBOARD_H


#include <string>

#include <tt/platform/tt_types.h>


namespace tt {
namespace menu {

/*! \brief Light-weight key input class which maps real keys to virtual menu keys. */
class MenuKeyboard
{
public:
	typedef u32 MenuKey;
	
	// Virtual key mapping
	static MenuKey MENU_UP;
	static MenuKey MENU_DOWN;
	static MenuKey MENU_LEFT;
	static MenuKey MENU_RIGHT;
	static MenuKey MENU_ACCEPT;
	static MenuKey MENU_CANCEL;
	static MenuKey MENU_BACK;
	static MenuKey MENU_L_TRIGGER;
	static MenuKey MENU_R_TRIGGER;
	static MenuKey MENU_EDIT;
	static MenuKey MENU_START;
	
	
	static MenuKey MENU_NOT_A_VALID_VIRTUAL_KEY;
	
	
	/*! \brief Loads the virtual key mapping from the specified XML file. */
	static void loadVirtualKeyMapping(const std::string& p_filename);
	
	/*! \brief Translate string to key mapping value.
	    \return the MenuKey value mapped to the specified key name. */
	static MenuKey& getKeyMappingFromString(const std::string& p_key);
	
	explicit MenuKeyboard(u32 p_keys);
	
	/*! \brief Indicates whether a virtual menu key is set. */
	inline bool isKeySet(MenuKey p_key) const
	{ return (m_keys & p_key) == p_key; }
	
	/*! \brief Returns the bitmask of all virtual keys that are set. */
	inline MenuKey getKeyMask() const { return m_keys; }
	
private:
	u32 m_keys;
	
	
	static MenuKey getKeyCode(const std::string& p_realKeyName);
};

// Namespace end
}
}


#endif  // !defined(INC_TT_MENU_MENUKEYBOARD_H)
