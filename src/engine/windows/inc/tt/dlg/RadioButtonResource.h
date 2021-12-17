#if !defined(INC_TT_DLG_RADIOBUTTONRESOURCE_H)
#define INC_TT_DLG_RADIOBUTTONRESOURCE_H

#include <tt/dlg/ControlResource.h>


namespace tt {
namespace dlg {

class RadioButtonResource : public ControlResource
{
public:
	RadioButtonResource(const std::string& p_caption, int p_id);
	virtual ~RadioButtonResource();
	
	static RadioButtonResource* createFirstRadioButton(const std::string& p_caption, int p_id);
	
private:
	RadioButtonResource(const RadioButtonResource&);
	const RadioButtonResource& operator=(const RadioButtonResource&);
	
	virtual int getStyle() const;
	virtual int getExStyle() const;
	virtual std::string getClass() const;
	
	int getTypeStyle() const;
};

// Namespace end
}
}

#endif  // !defined(INC_TT_DLG_RADIOBUTTONRESOURCE_H)
