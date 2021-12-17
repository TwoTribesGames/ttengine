#if !defined(INC_TT_DLG_STATICRESOURCE_H)
#define INC_TT_DLG_STATICRESOURCE_H

#include <tt/dlg/ControlResource.h>


namespace tt {
namespace dlg {

class StaticResource : public ControlResource
{
public:
	StaticResource(const std::string& p_caption, int p_id);
	virtual ~StaticResource();
	
	enum Align
	{
		Align_Left,
		Align_Right,
		Align_Center
	};
	
	void setAlignment(Align p_align);
	
private:
	StaticResource(const StaticResource&);
	const StaticResource& operator=(const StaticResource&);
	
	virtual int getStyle() const;
	virtual int getExStyle() const;
	virtual std::string getClass() const;
	
	int getAlignmentStyle() const;
	
	Align m_align;
};

// Namespace end
}
}

#endif  // !defined(INC_TT_DLG_STATICRESOURCE_H)
