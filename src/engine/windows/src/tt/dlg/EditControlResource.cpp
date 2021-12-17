#include <tt/dlg/EditControlResource.h>


namespace tt {
namespace dlg {

// ------------------------------------------------------------
// Public functions

EditControlResource::EditControlResource(const std::string& p_caption, int p_id)
:
ControlResource(p_id),
m_multiline(false),
m_password(false),
m_number(false),
m_readOnly(false)
{
	setPosition(0,0);
	setDimensions(50, 14);
	setCaption(p_caption);
	setTabStop(true);
	setBorder(true);
}


EditControlResource::~EditControlResource()
{
	
}


void EditControlResource::setMultiline(bool p_multiline)
{
	m_multiline = p_multiline;
}


void EditControlResource::setPassword(bool p_password)
{
	m_password = p_password;
}


void EditControlResource::setNumber(bool p_number)
{
	m_number = p_number;
}


void EditControlResource::setReadOnly(bool p_readOnly)
{
	m_readOnly = p_readOnly;
}


// ------------------------------------------------------------
// Private functions

int EditControlResource::getStyle() const
{
	return ES_AUTOHSCROLL | getTypeStyle();
}


int EditControlResource::getExStyle() const
{
	return 0;
}


std::string EditControlResource::getClass() const
{
	return "Edit";
}


int EditControlResource::getTypeStyle() const
{
	int style = 0;
	if (m_multiline) style |= ES_MULTILINE;
	if (m_password)  style |= ES_PASSWORD;
	if (m_number)    style |= ES_NUMBER;
	if (m_readOnly)  style |= ES_READONLY;
	
	return style;
}

// Namespace end
}
}
