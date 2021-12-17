#include <tt/platform/tt_error.h>
#include <tt/menu/elements/SelectionCursorType.h>


namespace tt {
namespace menu {
namespace elements {

SelectionCursorType stringToSelectionCursorType(const std::string&  p_string)
{
	if (p_string == "no" || p_string == "false" || p_string == "none")
	{
		return SelectionCursorType_None;
	}
	
	if (p_string == "left")
	{
		return SelectionCursorType_Left;
	}
	
	if (p_string == "right")
	{
		return SelectionCursorType_Right;
	}
	
	if (p_string == "yes" || p_string == "true" || p_string == "both")
	{
		return SelectionCursorType_Both;
	}
	
	TT_PANIC("Selection cursor type '%s' is not a valid value.",
	         p_string.c_str());
	
	return SelectionCursorType_Both;
}


std::string selectionCursorTypeToString(SelectionCursorType p_type)
{
	switch (p_type)
	{
	case SelectionCursorType_None:  return "none";
	case SelectionCursorType_Left:  return "left";
	case SelectionCursorType_Right: return "right";
	case SelectionCursorType_Both:  return "both";
	}
	
	TT_PANIC("Invalid selection cursor type: %d", p_type);
	return "both";
};

// Namespace end
}
}
}
