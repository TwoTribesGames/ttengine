#if !defined(INC_TT_GWEN_ADAPTIVESLIDER_H)
#define INC_TT_GWEN_ADAPTIVESLIDER_H


#include <Gwen/Controls/Base.h>
#include <Gwen/Controls/Label.h>
#include <Gwen/Controls/Slider.h>
#include <Gwen/Controls/TextBox.h>

#include <tt/math/Point2.h>
#include <tt/platform/tt_types.h>


namespace tt {
namespace gwen {


class AdaptiveSlider : public Gwen::Controls::Base
{
public:
	GWEN_CONTROL(AdaptiveSlider, Gwen::Controls::Base);
	~AdaptiveSlider() {}

	void update();

	void setInternalValue(real p_value);
	inline real getValue() const { return m_currentValue; }

	inline void useAsInteger() { m_isInteger = true; updateNumeric(); }

	inline void setLegalRange(real p_min, real p_max) { m_minLegalValue = p_min; m_maxLegalValue = p_max; }
	inline void setPositiveValuesOnly() { m_minLegalValue = 0; }
	inline void setMaxUpdateStep(real p_updateStep) { m_maxUpdateStep = p_updateStep; }

	// GWEN stuff
	virtual void SetKeyboardInputEnabled( bool b );
	virtual void SetMouseInputEnabled( bool b );
	virtual void SetDisabled( bool active );
	virtual void SetWidth (int p_width);

	Gwen::Event::Caller onValueChanged;

private:
	void onSliderChanged  (Gwen::Controls::Base* p_sender);
	void onSliderReleased (Gwen::Controls::Base* p_sender);
	void onValueTyped     (Gwen::Controls::Base* p_sender);
	void onValidateNumeric(Gwen::Controls::Base* p_sender);

	void updateNumeric();
	void setCurrentValue(real p_value);


	Gwen::Controls::Slider*         m_slider;
	Gwen::Controls::TextBoxNumeric* m_sliderValueEdit;

	bool m_active;
	bool m_isInteger;
	real m_currentValue;
	real m_currentUpdateStep;
	real m_maxUpdateStep;

	real m_minLegalValue;
	real m_maxLegalValue;
};


}
}

#endif // INC_TT_GWEN_ADAPTIVESLIDER_H
