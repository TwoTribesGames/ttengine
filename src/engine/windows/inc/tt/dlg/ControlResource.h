#if !defined(INC_TT_DLG_CONTROLRESOURCE_H)
#define INC_TT_DLG_CONTROLRESOURCE_H

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <string>


namespace tt {
namespace dlg {

class ControlResource
{
public:
	virtual ~ControlResource();
	
	void setPosition(int p_x, int p_y);
	int getX() const;
	int getY() const;
	
	void setDimensions(int p_width, int p_height);
	int getWidth() const;
	int getHeight() const;
	
	void setVisible(bool p_visible);
	void setGroup(bool p_group);
	void setTabStop(bool p_tabStop);
	void setBorder(bool p_border);
	void setVScroll(bool p_vScroll);
	void setHScroll(bool p_hScroll);
	
	size_t getSize() const;
	LPWORD createBuffer() const;
	
protected:
	ControlResource(int p_id);
	
	int getId() const;
	
	void setCaption(const std::string& p_caption);
	std::string getCaption() const;
	
	virtual int getStyle() const = 0;
	virtual int getExStyle() const = 0;
	virtual std::string getClass() const = 0;
	
private:
	ControlResource(const ControlResource&);
	const ControlResource& operator=(const ControlResource&);
	
	int getBaseStyle() const;
	
	struct DLGITEMTEMPLATEEX
	{
		DWORD helpID;
		DWORD exStyle;
		DWORD style;
		short x;
		short y;
		short cx;
		short cy;
		DWORD id;
		
		DLGITEMTEMPLATEEX()
		:
		helpID(0),
		exStyle(0),
		style(0),
		x(0),
		y(0),
		cx(0),
		cy(0),
		id(0)
		{}
	};
	
	int m_x;      //!< X position in dialog box units
	int m_y;      //!< Y position in dialog box units
	int m_width;  //!< Width in dialog box units
	int m_height; //!< Height in dialog box units
	
	int m_id; //!< Identifier of the control
	
	std::string m_caption; //!< Caption of the control
	
	bool m_tabStop;
	bool m_group;
	bool m_visible;
	bool m_border;
	bool m_vScroll;
	bool m_hScroll;
};

// Namespace end
}
}

#endif  // !defined(INC_TT_DLG_CONTROLRESOURCE_H)
