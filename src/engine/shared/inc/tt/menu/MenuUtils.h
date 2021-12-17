#if !defined(INC_TT_MENU_MENUUTILS_H)
#define INC_TT_MENU_MENUUTILS_H


#include <string>

#include <tt/platform/tt_types.h>


namespace tt {
namespace menu {

/*! \brief Menu related helper/utility functions. */
class MenuUtils
{
public:
	/*! \brief Converts a wide string to a hex-encoded ASCII string. */
	static std::string wideStringToHex(const std::wstring& p_string);
	
	/*! \brief Converts a hex-encoded ASCII string to a wide string. */
	static std::wstring hexToWideString(const std::string& p_string);
	
private:
	// Static class, can't create instance (or copy)
	MenuUtils();
	MenuUtils(const MenuUtils&);
	~MenuUtils();
	const MenuUtils& operator=(const MenuUtils&);
	
	static wchar_t asciiToHex(char p_char);
};

// Namespace end
}
}


#endif  // !defined(INC_TT_MENU_MENUUTILS_H)
