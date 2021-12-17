#include <tt/dlg/StaticResource.h>


namespace tt {
namespace dlg {

// ------------------------------------------------------------
// Public functions

StaticResource::StaticResource(const std::string& p_caption, int p_id)
:
ControlResource(p_id),
m_align(Align_Left)
{
	setPosition(0,0);
	setDimensions(50, 14);
	setCaption(p_caption);
}


StaticResource::~StaticResource()
{
	
}


void StaticResource::setAlignment(Align p_alignment)
{
	m_align = p_alignment;
}


// ------------------------------------------------------------
// Private functions

int StaticResource::getStyle() const
{
	return getAlignmentStyle();
}


int StaticResource::getExStyle() const
{
	return 0;
}


std::string StaticResource::getClass() const
{
	return "Static";
}


int StaticResource::getAlignmentStyle() const
{
	switch (m_align)
	{
	case Align_Left:   return SS_LEFT;
	case Align_Right:  return SS_RIGHT;
	case Align_Center: return SS_CENTER;
	default:           return SS_LEFT;
	}
}

// Namespace end
}
}
