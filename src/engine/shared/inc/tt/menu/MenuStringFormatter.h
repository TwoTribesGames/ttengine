#if !defined(INC_TT_MENU_MENUSTRINGFORMATTER_H)
#define INC_TT_MENU_MENUSTRINGFORMATTER_H


#include <tt/str/StringFormatter.h>


namespace tt {
namespace menu {

/*! \brief Utility class for formatting localization strings
           using the menu localization sheet. */
class MenuStringFormatter : public str::StringFormatter
{
public:
	MenuStringFormatter(const std::string& p_localizationID,
	                    bool p_numberedParams = false);
	virtual ~MenuStringFormatter();
	
private:
	// No copying or assigning
	MenuStringFormatter(const MenuStringFormatter&);
	const MenuStringFormatter& operator=(const MenuStringFormatter&);
};

// Namespace end
}
}


#endif  // !defined(INC_TT_MENU_MENUSTRINGFORMATTER_H)
