#include <tt/gwen/AdaptiveSlider.h>

#include <iomanip>
#include <limits>
#include <sstream>
#include <Gwen/Controls/HorizontalSlider.h>

#include <tt/platform/tt_error.h>


namespace tt {
namespace gwen {


// NOTE: For some reason tt::math::clamp doesn't work with numeric_limits<float>::min() / FLT_MIN
static const real minFloatValue(-100000.0f);
static const real maxFloatValue( 100000.0f);
static const int defaultWidth  = 50;
static const int defaultHeight = 20;


GWEN_CONTROL_CONSTRUCTOR( AdaptiveSlider )
, 
m_slider(0),
m_sliderValueEdit   (0),
m_active            (false),
m_isInteger         (false),
m_currentValue      (0),
m_currentUpdateStep (0),
m_maxUpdateStep     (0.5f),
m_minLegalValue     (minFloatValue),
m_maxLegalValue     (maxFloatValue)
{
	SetMouseInputEnabled(true);
	SetSize(defaultWidth, defaultHeight);

	m_sliderValueEdit = new Gwen::Controls::TextBoxNumeric(this);
	m_sliderValueEdit->SetText("0.0");
	m_sliderValueEdit->SetWidth(defaultWidth);
	m_sliderValueEdit->Dock(Gwen::Pos::Left);
	m_sliderValueEdit->onTextChanged.Add  (this, &AdaptiveSlider::onValueTyped);
	m_sliderValueEdit->onReturnPressed.Add(this, &AdaptiveSlider::onValidateNumeric);

	m_slider = new Gwen::Controls::HorizontalSlider(this);
	m_slider->SetPos(m_sliderValueEdit->Right(), 0);
	m_slider->Dock(Gwen::Pos::Left);

	m_slider->SetRange(-1.0f, 1.0f);
	m_slider->SetWidth(defaultWidth);
	m_slider->SetFloatValue(0.0f);

	// HACK: Slider is not set to correct position
	m_slider->SetWidth(defaultWidth + 5);

	m_slider->onValueChanged.Add       (this, &AdaptiveSlider::onSliderChanged);
	m_slider->onValueChangeFinished.Add(this, &AdaptiveSlider::onSliderReleased);

	SizeToChildren();
}


void AdaptiveSlider::update()
{
	if(m_active)
	{
		setCurrentValue(m_currentValue + m_currentUpdateStep);
		updateNumeric();
		onValueChanged.Call(this);
	}
}


void AdaptiveSlider::setInternalValue(real p_value)
{
	setCurrentValue(p_value);
	updateNumeric();
	onValueChanged.Call(this);
}


void AdaptiveSlider::SetKeyboardInputEnabled(bool p_enabled)
{
	BaseClass::SetKeyboardInputEnabled(p_enabled);

	if(m_slider != 0)          m_slider->SetKeyboardInputEnabled(p_enabled);
	if(m_sliderValueEdit != 0) m_sliderValueEdit->SetKeyboardInputEnabled(p_enabled);
}


void AdaptiveSlider::SetMouseInputEnabled(bool p_enabled)
{
	BaseClass::SetMouseInputEnabled(p_enabled);
	if(m_slider != 0)          m_slider->SetMouseInputEnabled(p_enabled);
	if(m_sliderValueEdit != 0) m_sliderValueEdit->SetMouseInputEnabled(p_enabled);
}


void AdaptiveSlider::SetDisabled(bool p_disabled)
{
	BaseClass::SetDisabled(p_disabled);
	if(m_slider != 0)          m_slider->SetDisabled(p_disabled);
	if(m_sliderValueEdit != 0) m_sliderValueEdit->SetDisabled(p_disabled);
}


void AdaptiveSlider::SetWidth(int p_width)
{
	if (m_slider != 0) m_slider->SetWidth(p_width - defaultWidth);
	BaseClass::SetWidth(p_width);
}


//--------------------------------------------------------------------------------------------------
// Private member functions

void AdaptiveSlider::onSliderChanged(Gwen::Controls::Base* p_sender)
{
	// Update label
	m_active = true;
	float value = static_cast<Gwen::Controls::Slider*>(p_sender)->GetFloatValue();

	m_currentUpdateStep = (value * value * value) * m_maxUpdateStep;
}


void AdaptiveSlider::onSliderReleased(Gwen::Controls::Base* /*p_sender*/)
{
	m_slider->SetFloatValue(0.0f);
	m_currentUpdateStep = 0;
	m_active = false;
	
	// FIXME: for particle editor to use this, need new way to keep track of unsaved changes
}


void AdaptiveSlider::onValueTyped(Gwen::Controls::Base*)
{
	setCurrentValue(m_sliderValueEdit->GetFloatFromText());
	onValueChanged.Call(this);
	
	// FIXME: for particle editor to use this, need new way to keep track of unsaved changes
}


void AdaptiveSlider::onValidateNumeric(Gwen::Controls::Base*)
{
	updateNumeric();
}


void AdaptiveSlider::updateNumeric()
{
	std::stringstream s;

	if(m_isInteger)
	{
		s32 value = static_cast<s32>(m_currentValue);
		s << value;
	}
	else
	{
		int numPrecision(2);
		if(m_currentValue > -1.0f && m_currentValue < 1.0f)
		{
			numPrecision = 4;
		}
		s << std::fixed << std::setprecision(numPrecision) << m_currentValue;
	}
	m_sliderValueEdit->SetText(s.str(), false);
}


void AdaptiveSlider::setCurrentValue(real p_value)
{
	m_currentValue = p_value;

	tt::math::clamp<real>(m_currentValue, m_minLegalValue, m_maxLegalValue);

	if(m_isInteger)
	{
		m_currentValue = tt::math::round(m_currentValue);
	}
}

}
}

