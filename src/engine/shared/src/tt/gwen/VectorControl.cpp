#include <tt/gwen/VectorControl.h>

#include <iomanip>
#include <limits>
#include <sstream>
#include <Gwen/Controls/HorizontalSlider.h>

#include <tt/platform/tt_error.h>


namespace tt {
namespace gwen {


static const int defaultWidth  = 50;
static const int defaultHeight = 20;


GWEN_CONTROL_CONSTRUCTOR( VectorControl )
,
m_xValueEdit  (0),
m_yValueEdit  (0),
m_zValueEdit  (0),
m_currentValue(0),
m_isInteger   (false)
{
	SetMouseInputEnabled(true);
	SetSize(defaultWidth, defaultHeight);
	
	m_xValueEdit = new Gwen::Controls::TextBoxNumeric(this);
	m_xValueEdit->SetText("0.0");
	m_xValueEdit->SetWidth(defaultWidth);
	m_xValueEdit->Dock(Gwen::Pos::Left);
	m_xValueEdit->onTextChanged.Add  (this, &VectorControl::onValueTyped);
	m_xValueEdit->onReturnPressed.Add(this, &VectorControl::onValidateNumeric);
	
	m_yValueEdit = new Gwen::Controls::TextBoxNumeric(this);
	m_yValueEdit->SetText("0.0");
	m_yValueEdit->SetWidth(defaultWidth);
	m_yValueEdit->SetPos(m_xValueEdit->Right(), m_xValueEdit->GetPos().y);
	m_yValueEdit->Dock(Gwen::Pos::Left);
	m_yValueEdit->onTextChanged.Add  (this, &VectorControl::onValueTyped);
	m_yValueEdit->onReturnPressed.Add(this, &VectorControl::onValidateNumeric);
	
	m_zValueEdit = new Gwen::Controls::TextBoxNumeric(this);
	m_zValueEdit->SetText("0.0");
	m_zValueEdit->SetWidth(defaultWidth);
	m_zValueEdit->SetPos(m_yValueEdit->Right(), m_yValueEdit->GetPos().y);
	m_zValueEdit->Dock(Gwen::Pos::Left);
	m_zValueEdit->onTextChanged.Add  (this, &VectorControl::onValueTyped);
	m_zValueEdit->onReturnPressed.Add(this, &VectorControl::onValidateNumeric);
	
	SizeToChildren();
}


void VectorControl::setInternalValue(const math::Vector3& p_value)
{
	setCurrentValue(p_value);
	updateNumeric(m_xValueEdit);
	updateNumeric(m_yValueEdit);
	updateNumeric(m_zValueEdit);
	onValueChanged.Call(this);
}


void VectorControl::SetKeyboardInputEnabled(bool p_enabled)
{
	BaseClass::SetKeyboardInputEnabled(p_enabled);
	
	if(m_xValueEdit != 0) m_xValueEdit->SetKeyboardInputEnabled(p_enabled);
	if(m_yValueEdit != 0) m_yValueEdit->SetKeyboardInputEnabled(p_enabled);
	if(m_zValueEdit != 0) m_zValueEdit->SetKeyboardInputEnabled(p_enabled);
}


void VectorControl::SetMouseInputEnabled(bool p_enabled)
{
	BaseClass::SetMouseInputEnabled(p_enabled);
	
	if(m_xValueEdit != 0) m_xValueEdit->SetMouseInputEnabled(p_enabled);
	if(m_yValueEdit != 0) m_yValueEdit->SetMouseInputEnabled(p_enabled);
	if(m_zValueEdit != 0) m_zValueEdit->SetMouseInputEnabled(p_enabled);
}


void VectorControl::SetDisabled(bool p_disabled)
{
	BaseClass::SetDisabled(p_disabled);
	
	if(m_xValueEdit != 0) m_xValueEdit->SetDisabled(p_disabled);
	if(m_yValueEdit != 0) m_yValueEdit->SetDisabled(p_disabled);
	if(m_zValueEdit != 0) m_zValueEdit->SetDisabled(p_disabled);
}


//--------------------------------------------------------------------------------------------------
// Private member functions


void VectorControl::onValueTyped(Gwen::Controls::Base*)
{
	math::Vector3 currentValue(m_currentValue);
	currentValue.x = m_xValueEdit->GetFloatFromText();
	currentValue.y = m_yValueEdit->GetFloatFromText();
	currentValue.z = m_zValueEdit->GetFloatFromText();
	
	setCurrentValue(currentValue);
	onValueChanged.Call(this);
}


void VectorControl::onValidateNumeric(Gwen::Controls::Base* p_sender)
{
	updateNumeric(p_sender);
}


void VectorControl::updateNumeric(Gwen::Controls::Base* p_sender)
{
	Gwen::Controls::TextBoxNumeric* textBox = gwen_cast<Gwen::Controls::TextBoxNumeric>(p_sender);
	std::stringstream s;
	real currentValue(0.0f);
	if (textBox == m_xValueEdit) currentValue = m_currentValue.x;
	if (textBox == m_yValueEdit) currentValue = m_currentValue.y;
	if (textBox == m_zValueEdit) currentValue = m_currentValue.z;
	
	if (m_isInteger)
	{
		s32 value = static_cast<s32>(currentValue);
		s << value;
	}
	else
	{
		const std::streamsize numPrecision(2);
		s << std::fixed << std::setprecision(numPrecision) << currentValue;
	}
	textBox->SetText(s.str(), false);
}


void VectorControl::setCurrentValue(const math::Vector3& p_value)
{
	m_currentValue = p_value;
	
	if (m_isInteger)
	{
		m_currentValue.x = math::round(m_currentValue.x);
		m_currentValue.y = math::round(m_currentValue.y);
		m_currentValue.z = math::round(m_currentValue.z);
	}
}

}
}

