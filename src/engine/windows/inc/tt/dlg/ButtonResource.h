#if !defined(INC_TT_DLG_BUTTONRESOURCE_H)
#define INC_TT_DLG_BUTTONRESOURCE_H

#include <tt/dlg/ControlResource.h>


namespace tt {
namespace dlg {

class ButtonResource : public ControlResource
{
public:
	ButtonResource(const std::string& p_caption, int p_id);
	virtual ~ButtonResource();
	
	void setDefault(bool p_default);
	
	static ButtonResource* createOkButton();
	static ButtonResource* createCancelButton();
	static ButtonResource* createAbortButton();
	static ButtonResource* createRetryButton();
	static ButtonResource* createIgnoreButton();
	static ButtonResource* createYesButton();
	static ButtonResource* createNoButton();
	
private:
	ButtonResource(const ButtonResource&);
	const ButtonResource& operator=(const ButtonResource&);
	
	virtual int getStyle() const;
	virtual int getExStyle() const;
	virtual std::string getClass() const;
	
	bool m_default;
	
};

// Namespace end
}
}

#endif  // !defined(INC_TT_DLG_BUTTONRESOURCE_H)
