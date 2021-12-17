#if !defined(INC_TT_MENU_ELEMENTS_SELECTIONCURSORTYPE_H)
#define INC_TT_MENU_ELEMENTS_SELECTIONCURSORTYPE_H


#include <string>


namespace tt {
namespace menu {
namespace elements {

enum SelectionCursorType
{
	SelectionCursorType_None,
	SelectionCursorType_Left,
	SelectionCursorType_Right,
	SelectionCursorType_Both
};


SelectionCursorType stringToSelectionCursorType(const std::string&  p_string);
std::string         selectionCursorTypeToString(SelectionCursorType p_type);

// Namespace end
}
}
}


#endif  // !defined(INC_TT_MENU_ELEMENTS_SELECTIONCURSORTYPE_H)
