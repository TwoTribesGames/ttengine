#include <tt/dlg/GroupBoxResource.h>


namespace tt {
namespace dlg {

// ------------------------------------------------------------
// Public functions

GroupBoxResource::GroupBoxResource(const std::string& p_caption, int p_id)
:
ControlResource(p_id)
{
	setPosition(0,0);
	setDimensions(50, 50);
	setCaption(p_caption);
}


GroupBoxResource::~GroupBoxResource()
{
	
}


// ------------------------------------------------------------
// Private functions

int GroupBoxResource::getStyle() const
{
	return BS_GROUPBOX | getTypeStyle();
}


int GroupBoxResource::getExStyle() const
{
	return 0;
}


std::string GroupBoxResource::getClass() const
{
	return "Button";
}


int GroupBoxResource::getTypeStyle() const
{
	return 0;
}

// Namespace end
}
}
