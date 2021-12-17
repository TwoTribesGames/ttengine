#if !defined(INC_TT_DLG_CHECKBOXRESOURCE_H)
#define INC_TT_DLG_CHECKBOXRESOURCE_H

#include <tt/dlg/ControlResource.h>


namespace tt {
namespace dlg {

class CheckBoxResource : public ControlResource
{
public:
	CheckBoxResource(const std::string& p_caption, int p_id);
	virtual ~CheckBoxResource();
	
private:
	CheckBoxResource(const CheckBoxResource&);
	const CheckBoxResource& operator=(const CheckBoxResource&);
	
	virtual int getStyle() const;
	virtual int getExStyle() const;
	virtual std::string getClass() const;
	
	int getTypeStyle() const;
};

// Namespace end
}
}


#endif  // !defined(INC_TT_DLG_CHECKBOXRESOURCE_H)
