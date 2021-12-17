#include <tt/dlg/ButtonResource.h>


namespace tt {
namespace dlg {


// ------------------------------------------------------------
// Public functions

ButtonResource::ButtonResource(const std::string& p_caption, int p_id)
:
ControlResource(p_id),
m_default(false)
{
	setPosition(0,0);
	setDimensions(50, 14);
	setCaption(p_caption);
}


ButtonResource::~ButtonResource()
{
	
}


void ButtonResource::setDefault(bool p_default)
{
	m_default = p_default;
}


ButtonResource* ButtonResource::createOkButton()
{
	return new ButtonResource("Ok", IDOK);
}

ButtonResource* ButtonResource::createCancelButton()
{
	return new ButtonResource("Cancel", IDCANCEL);
}

ButtonResource* ButtonResource::createAbortButton()
{
	return new ButtonResource("Abort", IDABORT);
}

ButtonResource* ButtonResource::createRetryButton()
{
	return new ButtonResource("Retry", IDRETRY);
}

ButtonResource* ButtonResource::createIgnoreButton()
{
	return new ButtonResource("Ignore", IDIGNORE);
}

ButtonResource* ButtonResource::createYesButton()
{
	return new ButtonResource("Yes", IDYES);
}

ButtonResource* ButtonResource::createNoButton()
{
	return new ButtonResource("No", IDNO);
}


// ------------------------------------------------------------
// Private functions

int ButtonResource::getStyle() const
{
	return m_default ? BS_DEFPUSHBUTTON : BS_PUSHBUTTON;
}


int ButtonResource::getExStyle() const
{
	return 0;
}


std::string ButtonResource::getClass() const
{
	return "Button";
}

// Namespace end
}
}
