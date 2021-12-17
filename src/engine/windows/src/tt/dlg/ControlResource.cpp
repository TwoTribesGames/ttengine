#include <tt/dlg/ControlResource.h>
#include <tt/platform/tt_printf.h>


namespace tt {
namespace dlg {

// ------------------------------------------------------------
// Public functions

ControlResource::~ControlResource()
{
	
}


void ControlResource::setPosition(int p_x, int p_y)
{
	m_x = p_x;
	m_y = p_y;
}


int ControlResource::getX() const
{
	return m_x;
}


int ControlResource::getY() const
{
	return m_y;
}


void ControlResource::setDimensions(int p_width, int p_height)
{
	m_width = p_width;
	m_height = p_height;
}


int ControlResource::getWidth() const
{
	return m_width;
}


int ControlResource::getHeight() const
{
	return m_height;
}


void ControlResource::setVisible(bool p_visible)
{
	m_visible = p_visible;
}


void ControlResource::setTabStop(bool p_tabStop)
{
	m_tabStop = p_tabStop;
}


void ControlResource::setGroup(bool p_group)
{
	m_group = p_group;
}


void ControlResource::setBorder(bool p_border)
{
	m_border = p_border;
}


void ControlResource::setVScroll(bool p_vScroll)
{
	m_vScroll = p_vScroll;
}


void ControlResource::setHScroll(bool p_hScroll)
{
	m_hScroll = p_hScroll;
}


size_t ControlResource::getSize() const
{
	int captionSize = MultiByteToWideChar(CP_ACP, 0, m_caption.c_str(), -1, 0, 0) * sizeof(WCHAR);
	int classSize = MultiByteToWideChar(CP_ACP, 0, getClass().c_str(), -1, 0, 0) * sizeof(WCHAR);
	return (sizeof(DLGITEMTEMPLATEEX) + static_cast<size_t>(captionSize + classSize) + sizeof(WORD)) / sizeof(WORD);
}


LPWORD ControlResource::createBuffer() const
{
	size_t size = getSize();
	LPWORD buffer = new WORD[size];
	size *= sizeof(WORD);
	LPWORD scratch = buffer;
	
	DLGITEMTEMPLATEEX dite;
	dite.style = static_cast<DWORD>(getBaseStyle() | getStyle());
	dite.exStyle = static_cast<DWORD>(getExStyle());
	dite.x = static_cast<short>(m_x);
	dite.y = static_cast<short>(m_y);
	dite.cx = static_cast<short>(m_width);
	dite.cy = static_cast<short>(m_height);
	dite.id = static_cast<DWORD>(m_id);
	
	memcpy(scratch, &dite, sizeof(DLGITEMTEMPLATEEX));
	size -= sizeof(DLGITEMTEMPLATEEX);
	scratch += sizeof(DLGITEMTEMPLATEEX) / sizeof(WORD);
	
	// write class
	int length = MultiByteToWideChar(CP_ACP, 0, getClass().c_str(), -1, 0, 0);
	size -= MultiByteToWideChar(CP_ACP, 0, getClass().c_str(), -1, reinterpret_cast<LPWSTR>(scratch), length) * sizeof(WCHAR);
	scratch += (length * sizeof(WCHAR)) / sizeof(WORD);
	
	// write caption
	length = MultiByteToWideChar(CP_ACP, 0, m_caption.c_str(), -1, 0, 0);
	size -= MultiByteToWideChar(CP_ACP, 0, m_caption.c_str(), -1, reinterpret_cast<LPWSTR>(scratch), length) * sizeof(WCHAR);
	scratch += (length * sizeof(WCHAR)) / sizeof(WORD);
	
	// write creation data
	*scratch = 0x0000; // no data
	
	return buffer;
}


// ------------------------------------------------------------
// Protected functions

ControlResource::ControlResource(int p_id)
:
m_x(0),
m_y(0),
m_width(0),
m_height(0),
m_id(p_id),
m_tabStop(false),
m_group(false),
m_visible(true),
m_border(false),
m_vScroll(false),
m_hScroll(false)
{
	
}


int ControlResource::getId() const
{
	return m_id;
}


void ControlResource::setCaption(const std::string& p_caption)
{
	m_caption = p_caption;
}


std::string ControlResource::getCaption() const
{
	return m_caption;
}


// ------------------------------------------------------------
// Private functions

int ControlResource::getBaseStyle() const
{
	int style = WS_CHILD;
	if (m_visible) style |= WS_VISIBLE;
	if (m_tabStop) style |= WS_TABSTOP;
	if (m_group)   style |= WS_GROUP;
	if (m_border)  style |= WS_BORDER;
	if (m_vScroll) style |= WS_VSCROLL;
	if (m_hScroll) style |= WS_HSCROLL;
	return style;
}

// Namespace end
}
}
