#if !defined(INC_TT_GWEN_VECTORCONTROL_H)
#define INC_TT_GWEN_VECTORCONTROL_H


#include <Gwen/Controls/Base.h>
#include <Gwen/Controls/Label.h>
#include <Gwen/Controls/Slider.h>
#include <Gwen/Controls/TextBox.h>

#include <tt/math/Vector3.h>
#include <tt/platform/tt_types.h>


namespace tt {
namespace gwen {


class VectorControl : public Gwen::Controls::Base
{
public:
	GWEN_CONTROL(VectorControl, Gwen::Controls::Base);
	~VectorControl() {}
	
	void setInternalValue(const math::Vector3& p_value);
	inline math::Vector3 getValue() const { return m_currentValue; }
	
	// GWEN stuff
	virtual void SetKeyboardInputEnabled( bool b );
	virtual void SetMouseInputEnabled( bool b );
	virtual void SetDisabled( bool active );
	
	Gwen::Event::Caller onValueChanged;
	
private:
	void onSliderChanged  (Gwen::Controls::Base* p_sender);
	void onSliderReleased (Gwen::Controls::Base* p_sender);
	void onValueTyped     (Gwen::Controls::Base* p_sender);
	void onValidateNumeric(Gwen::Controls::Base* p_sender);
	
	void updateNumeric(Gwen::Controls::Base* p_sender);
	void setCurrentValue(const math::Vector3& p_value);
	
	Gwen::Controls::TextBoxNumeric* m_xValueEdit;
	Gwen::Controls::TextBoxNumeric* m_yValueEdit;
	Gwen::Controls::TextBoxNumeric* m_zValueEdit;
	
	math::Vector3 m_currentValue;
	bool          m_isInteger;
};


}
}

#endif // INC_TT_GWEN_VECTORCONTROL_H
