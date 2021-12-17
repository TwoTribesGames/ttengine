#if !defined(INC_TT_DLG_EDITCONTROLRESOURCE_H)
#define INC_TT_DLG_EDITCONTROLRESOURCE_H

#include <tt/dlg/ControlResource.h>


namespace tt {
namespace dlg {

class EditControlResource : public ControlResource
{
public:
	EditControlResource(const std::string& p_caption, int p_id);
	virtual ~EditControlResource();
	
	void setMultiline(bool p_multiline);
	void setPassword(bool p_password);
	void setNumber(bool p_number);
	void setReadOnly(bool p_readOnly);
	
private:
	EditControlResource(const EditControlResource&);
	const EditControlResource& operator=(const EditControlResource&);
	
	virtual int getStyle() const;
	virtual int getExStyle() const;
	virtual std::string getClass() const;
	
	int getTypeStyle() const;
	
	bool m_multiline;
	bool m_password;
	bool m_number;
	bool m_readOnly;
};

// Namespace end
}
}

#endif  // !defined(INC_TT_DLG_EDITCONTROLRESOURCE_H)
