#if !defined(INC_TT_DLG_DIALOGRESOURCE_H)
#define INC_TT_DLG_DIALOGRESOURCE_H

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>


namespace tt {
namespace dlg {

class ControlResource;

class DialogResource
{
public:
	DialogResource(const std::string& p_caption);
	~DialogResource();
	
	void setPosition(int p_x, int p_y);
	void setDimensions(int p_width, int p_height);
	void setCenter(bool p_center);
	
	bool addControl(const ControlResource* p_control);
	
	HWND create(HINSTANCE p_hInstance, HWND p_hWndParent, DLGPROC p_lpDialogFunc);
	INT_PTR instantiate(HINSTANCE p_hInstance, HWND p_hWndParent, DLGPROC p_lpDialogFunc);
	
	void dump() const;
	
private:
	DialogResource(const DialogResource&);
	const DialogResource& operator=(const DialogResource&);
	
	void reserve(size_t p_size);
	void addData(LPWORD p_data, size_t p_size);
	void addString(const std::string& p_string);
	void align();
	
	struct DLGTEMPLATEEX
	{
		WORD dlgVer;
		WORD signature;
		DWORD helpID;
		DWORD exStyle;
		DWORD style;
		WORD cDlgItems;
		short x;
		short y;
		short cx;
		short cy;
		WORD menu;
		
		DLGTEMPLATEEX()
		:
		dlgVer(1),
		signature(0xFFFF),
		helpID(0),
		exStyle(0),
		style(0),
		cDlgItems(0),
		x(0),
		y(0),
		cx(0),
		cy(0),
		menu(0)
		{}
	};
	
	DLGTEMPLATEEX* m_lpdte;
	LPWORD m_data;
	size_t m_offset;
	size_t m_size;
};

// Namespace end
}
}

#endif  // !defined(INC_TT_DLG_DIALOGRESOURCE_H)
