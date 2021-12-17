#include <tt/dlg/RadioButtonResource.h>


namespace tt {
namespace dlg {

// ------------------------------------------------------------
// Public functions

RadioButtonResource::RadioButtonResource(const std::string& p_caption, int p_id)
:
ControlResource(p_id)
{
	setPosition(0,0);
	setDimensions(50, 14);
	setCaption(p_caption);
}


RadioButtonResource::~RadioButtonResource()
{
	
}


RadioButtonResource* RadioButtonResource::createFirstRadioButton(const std::string& p_caption, int p_id)
{
	RadioButtonResource* ret = new RadioButtonResource(p_caption, p_id);
	ret->setGroup(true);
	ret->setTabStop(true);
	return ret;
}


// ------------------------------------------------------------
// Private functions

int RadioButtonResource::getStyle() const
{
	return BS_AUTORADIOBUTTON | getTypeStyle();
}


int RadioButtonResource::getExStyle() const
{
	return 0;
}


std::string RadioButtonResource::getClass() const
{
	return "Button";
}


int RadioButtonResource::getTypeStyle() const
{
	return 0;
}

// Namespace end
}
}
