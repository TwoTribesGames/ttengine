#define NOMINMAX
#include <windows.h>

#include <tt/dlg/ControlResource.h>
#include <tt/dlg/DialogResource.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>


namespace tt {
namespace dlg {

// ------------------------------------------------------------
// Public functions

DialogResource::DialogResource(const std::string& p_caption)
:
m_lpdte(0),
m_data(0),
m_offset(0),
m_size(0)
{
	DLGTEMPLATEEX dte;
	dte.style = DS_SETFONT | DS_MODALFRAME | DS_FIXEDSYS | WS_POPUP | WS_CAPTION | WS_SYSMENU;
	dte.cx = 316;
	dte.cy = 185;
	addData(reinterpret_cast<LPWORD>(&dte), sizeof(DLGTEMPLATEEX) / sizeof(WORD));
	m_lpdte = reinterpret_cast<DLGTEMPLATEEX*>(m_data);
	
	WORD data = 0;
	addData(&data, 1); // Default window class
	addString(p_caption); // window caption
	
	if ((dte.style & DS_SETFONT) == DS_SETFONT)
	{
		data = 8; // point size
		addData(&data, 1);
		
		data = 400; // weight
		addData(&data, 1);
		
		data = HIWORD(0) | LOWORD(0); // italic | character set
		addData(&data, 1);
		
		addString("MS Shell Dlg"); // typeface name
	}
}


DialogResource::~DialogResource()
{
	delete[] m_data;
}


void DialogResource::setPosition(int p_x, int p_y)
{
	m_lpdte->x = static_cast<short>(p_x);
	m_lpdte->y = static_cast<short>(p_y);
}


void DialogResource::setDimensions(int p_width, int p_height)
{
	m_lpdte->cx = static_cast<short>(p_width);
	m_lpdte->cy = static_cast<short>(p_height);
}


void DialogResource::setCenter(bool p_center)
{
	if (p_center)
	{
		m_lpdte->style |= DS_CENTER;
	}
	else
	{
		m_lpdte->style &= ~DS_CENTER;
	}
}


bool DialogResource::addControl(const ControlResource* p_control)
{
	if (p_control == 0)
	{
		return false;
	}
	
	align();
	
	size_t size = p_control->getSize();
	LPWORD buffer = p_control->createBuffer();
	addData(buffer, size);
	delete[] buffer;
	
	++m_lpdte->cDlgItems;
	
	return true;
}


HWND DialogResource::create(HINSTANCE p_hInstance, HWND p_hWndParent, DLGPROC p_lpDialogFunc)
{
	HGLOBAL hGbl = GlobalAlloc(GMEM_ZEROINIT, m_offset * sizeof(WORD));
	if (hGbl == 0)
	{
		return 0;
	}
	LPVOID buffer = GlobalLock(hGbl);
	memcpy(buffer, m_data, m_offset * sizeof(WORD));
	GlobalUnlock(hGbl);
	
	HWND ret = CreateDialogIndirect(p_hInstance, reinterpret_cast<LPDLGTEMPLATE>(hGbl), p_hWndParent, p_lpDialogFunc);
	
	GlobalFree(hGbl);
	return ret;
}


INT_PTR DialogResource::instantiate(HINSTANCE p_hInstance, HWND p_hWndParent, DLGPROC p_lpDialogFunc)
{
	HGLOBAL hGbl = GlobalAlloc(GMEM_ZEROINIT, m_offset * sizeof(WORD));
	if (hGbl == 0)
	{
		return -1;
	}
	LPVOID buffer = GlobalLock(hGbl);
	memcpy(buffer, m_data, m_offset * sizeof(WORD));
	GlobalUnlock(hGbl);
	
	INT_PTR ret = DialogBoxIndirect(p_hInstance, reinterpret_cast<LPDLGTEMPLATE>(hGbl), p_hWndParent, p_lpDialogFunc);
	TT_ASSERT(ret != -1);
	
	GlobalFree(hGbl);
	return ret;
}


void DialogResource::dump() const
{
#ifndef TT_BUILD_FINAL
	LPBYTE data = reinterpret_cast<LPBYTE>(m_data);
	for (size_t i = 0; i < m_offset * 2; ++i)
	{
		TT_Printf("%02X ", data[i]);
		if ((i & 7) == 7)
		{
			TT_Printf(" ");
		}
		if ((i & 15) == 15)
		{
			TT_Printf("\n");
		}
	}
	TT_Printf("\n");
#endif
}


// ------------------------------------------------------------
// Private functions

void DialogResource::reserve(size_t p_size)
{
	while (m_offset + p_size > m_size)
	{
		LPWORD data = new WORD[m_size + 1024];
		memset(data, 0, (m_size + 1024) * sizeof(WORD));
		memcpy(data, m_data, m_offset * sizeof(WORD));
		delete[] m_data;
		m_data = data;
		m_size += 1024;
	}
}


void DialogResource::addData(LPWORD p_data, size_t p_size)
{
	reserve(p_size);
	memcpy(m_data + m_offset, p_data, p_size * sizeof(WORD));
	m_offset += p_size;
}


void DialogResource::addString(const std::string& p_string)
{
	int length = MultiByteToWideChar(CP_ACP, 0, p_string.c_str(), -1, 0, 0);
	LPWSTR str = new WCHAR[length];
	MultiByteToWideChar(CP_ACP, 0, p_string.c_str(), -1, str, length);
	addData(reinterpret_cast<LPWORD>(str), (length * sizeof(WCHAR)) / sizeof(WORD));
	delete[] str;
}


void DialogResource::align()
{
	++m_offset;
	m_offset >>= 1;
	m_offset <<= 1;
}

// Namespace end
}
}
