#include <tt/dlg/ListBoxResource.h>


namespace tt {
namespace dlg {

// ------------------------------------------------------------
// Public functions

ListBoxResource::ListBoxResource(const std::string& p_caption, int p_id)
:
ControlResource(p_id)
{
	setPosition(0,0);
	setDimensions(50, 14);
	setCaption(p_caption);
	setTabStop(true);
	setVScroll(true);
	setBorder(true);
}


ListBoxResource::~ListBoxResource()
{
	
}


// ------------------------------------------------------------
// Private functions

int ListBoxResource::getStyle() const
{
	return LBS_SORT | LBS_NOINTEGRALHEIGHT | getTypeStyle();
}


int ListBoxResource::getExStyle() const
{
	return 0;
}


std::string ListBoxResource::getClass() const
{
	return "ListBox";
}


int ListBoxResource::getTypeStyle() const
{
	return 0;
}

// Namespace end
}
}
