#if !defined(INC_TT_DLG_GROUPBOXRESOURCE_H)
#define INC_TT_DLG_GROUPBOXRESOURCE_H

#include <tt/dlg/ControlResource.h>


namespace tt {
namespace dlg {

class GroupBoxResource : public ControlResource
{
public:
	GroupBoxResource(const std::string& p_caption, int p_id);
	virtual ~GroupBoxResource();
	
private:
	GroupBoxResource(const GroupBoxResource&);
	const GroupBoxResource& operator=(const GroupBoxResource&);
	
	virtual int getStyle() const;
	virtual int getExStyle() const;
	virtual std::string getClass() const;
	
	int getTypeStyle() const;
};

// Namespace end
}
}

#endif  // !defined(INC_TT_DLG_GROUPBOXRESOURCE_H)
