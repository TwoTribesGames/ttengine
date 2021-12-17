#include <tt/dlg/CheckBoxResource.h>


namespace tt {
namespace dlg {

// ------------------------------------------------------------
// Public functions

CheckBoxResource::CheckBoxResource(const std::string& p_caption, int p_id)
:
ControlResource(p_id)
{
	setPosition(0,0);
	setDimensions(50, 14);
	setCaption(p_caption);
	setTabStop(true);
}


CheckBoxResource::~CheckBoxResource()
{
	
}


// ------------------------------------------------------------
// Private functions

int CheckBoxResource::getStyle() const
{
	return BS_AUTOCHECKBOX | getTypeStyle();
}


int CheckBoxResource::getExStyle() const
{
	return 0;
}


std::string CheckBoxResource::getClass() const
{
	return "Button";
}


int CheckBoxResource::getTypeStyle() const
{
	return 0;
}

// Namespace end
}
}
