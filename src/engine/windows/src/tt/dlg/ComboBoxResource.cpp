#include <tt/dlg/ComboBoxResource.h>


namespace tt {
namespace dlg {

// ------------------------------------------------------------
// Public functions

ComboBoxResource::ComboBoxResource(const std::string& p_caption, int p_id)
:
ControlResource(p_id),
m_editable(false)
{
	setPosition(0,0);
	setDimensions(50, 14);
	setCaption(p_caption);
	setTabStop(true);
	setVScroll(true);
}


ComboBoxResource::~ComboBoxResource()
{
	
}


void ComboBoxResource::setEditable(bool p_editable)
{
	m_editable = p_editable;
}


// ------------------------------------------------------------
// Private functions

int ComboBoxResource::getStyle() const
{
	return CBS_SORT | getTypeStyle();
}


int ComboBoxResource::getExStyle() const
{
	return 0;
}


std::string ComboBoxResource::getClass() const
{
	return "ComboBox";
}


int ComboBoxResource::getTypeStyle() const
{
	if (m_editable)
	{
		return CBS_DROPDOWN;
	}
	else
	{
		return CBS_DROPDOWNLIST;
	}
}

// Namespace end
}
}
