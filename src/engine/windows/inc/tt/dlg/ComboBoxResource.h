#if !defined(INC_TT_DLG_COMBOBOXRESOURCE_H)
#define INC_TT_DLG_COMBOBOXRESOURCE_H

#include <tt/dlg/ControlResource.h>


namespace tt {
namespace dlg {

class ComboBoxResource : public ControlResource
{
public:
	ComboBoxResource(const std::string& p_caption, int p_id);
	virtual ~ComboBoxResource();
	
	void setEditable(bool p_editable);
	
private:
	ComboBoxResource(const ComboBoxResource&);
	const ComboBoxResource& operator=(const ComboBoxResource&);
	
	virtual int getStyle() const;
	virtual int getExStyle() const;
	virtual std::string getClass() const;
	
	int getTypeStyle() const;
	
	bool m_editable;
};

// Namespace end
}
}

#endif  // !defined(INC_TT_DLG_COMBOBOXRESOURCE_H)
