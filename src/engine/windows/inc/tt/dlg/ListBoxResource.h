#if !defined(INC_TT_DLG_LISTBOXRESOURCE_H)
#define INC_TT_DLG_LISTBOXRESOURCE_H

#include <tt/dlg/ControlResource.h>


namespace tt {
namespace dlg {

class ListBoxResource : public ControlResource
{
public:
	ListBoxResource(const std::string& p_caption, int p_id);
	virtual ~ListBoxResource();
	
private:
	ListBoxResource(const ListBoxResource&);
	const ListBoxResource& operator=(const ListBoxResource&);
	
	virtual int getStyle() const;
	virtual int getExStyle() const;
	virtual std::string getClass() const;
	
	int getTypeStyle() const;
	
};

// Namespace end
}
}

#endif  // !defined(INC_TT_DLG_LISTBOXRESOURCE_H)
