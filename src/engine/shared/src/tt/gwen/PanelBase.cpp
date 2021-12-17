#include <tt/gwen/PanelBase.h>

#include <Gwen/Controls/CheckBox.h>
#include <Gwen/Controls/HorizontalSlider.h>
#include <Gwen/Controls/Label.h>
#include <Gwen/Controls/TextBox.h>

#include <tt/platform/tt_error.h>
#include <tt/str/toStr.h>

#include <tt/gwen/AdaptiveSlider.h>


namespace tt {
namespace gwen {


GWEN_CONTROL_CONSTRUCTOR( PanelBase )
,
m_sliders(),
m_root(0),
m_userInterface(0),
m_currentPosition(0,0),
m_updateTime(0),
m_updateFrequency(1/60.0f),
m_rightMargin(5)
{
	SetAutoHideBars(true);
	SetMargin(Gwen::Margin(1, 1, 1, 1));
}


void PanelBase::createRoot()
{
	// Create root control so we can destroy all children through it
	m_root = new Gwen::Controls::Base(this);
	m_currentPosition = tt::math::Point2::zero;
}


void PanelBase::destroyRoot()
{
	m_sliders.clear();

	if(m_root != 0)
	{
		m_root->DelayedDelete();
		m_root = 0;
	}
	Clear();
}


void PanelBase::updateSliders(real p_elapsedTime)
{
	m_updateTime += p_elapsedTime;
	
	while (m_updateTime > m_updateFrequency)
	{
		for (AdaptiveSliders::iterator it = m_sliders.begin(); it != m_sliders.end(); ++it)
		{
			(*it)->update();
		}
		m_updateTime -= m_updateFrequency;
	}
}


Gwen::Controls::Label* PanelBase::addLabel(const tt::math::Point2& p_position,
                                           const tt::math::Point2& p_size,
                                           const std::string&      p_text)
{
	return addLabel(p_position, p_size, p_text, m_root);
}


Gwen::Controls::Label* PanelBase::addLabel(const tt::math::Point2& p_position,
                                           const tt::math::Point2& p_size,
                                           const std::string&      p_text,
                                           Gwen::Controls::Base*   p_root)
{
	TT_NULL_ASSERT(p_root);
	
	Gwen::Controls::Label* label = new Gwen::Controls::Label(p_root);
	
	label->SetText(p_text);
	label->SetBounds(p_position.x, p_position.y, p_size.x, p_size.y);
	label->SetAlignment(Gwen::Pos::Left);
	
	return label;
}


Gwen::Controls::TextBox* PanelBase::addTextBox(const tt::math::Point2& p_position,
                                               const tt::math::Point2& p_size,
                                               const std::string&      p_text)
{
	TT_NULL_ASSERT(m_root);
	
	Gwen::Controls::TextBox* textBox = new Gwen::Controls::TextBox(m_root);
	
	textBox->SetText(p_text);
	textBox->SetBounds(p_position.x, p_position.y, p_size.x, p_size.y);
	
	return textBox;
}


Gwen::Controls::TextBoxNumeric* PanelBase::addLabeledIntBox(const std::string&      p_text,
                                                            const tt::math::Point2& p_labelSize,
                                                            s32                     p_value)
{
	TT_NULL_ASSERT(m_root);
	addLabel(m_currentPosition, p_labelSize, p_text);
	
	Gwen::Controls::TextBoxNumeric* textBox = new Gwen::Controls::TextBoxNumeric(m_root);
	
	const int panelWidth = m_root->GetBounds().w;
	
	textBox->SetBounds(m_currentPosition.x + p_labelSize.x, m_currentPosition.y,
		panelWidth - p_labelSize.x - m_rightMargin, p_labelSize.y);
	textBox->SetText(tt::str::toStr(p_value));
	
	m_currentPosition.y += p_labelSize.y;
	
	return textBox;
}


Gwen::Controls::TextBox* PanelBase::addLabeledTextBox(const std::string&      p_text,
                                                      const tt::math::Point2& p_labelSize,
                                                      const std::string&      p_value)
{
	TT_NULL_ASSERT(m_root);
	
	addLabel(m_currentPosition, p_labelSize, p_text);
	
	Gwen::Controls::TextBox* textBox = new Gwen::Controls::TextBox(m_root);
	
	const int panelWidth = m_root->GetBounds().w;
	
	textBox->SetBounds(m_currentPosition.x + p_labelSize.x, m_currentPosition.y,
		panelWidth - p_labelSize.x - m_rightMargin, p_labelSize.y);
	
	textBox->SetText(p_value);
	textBox->SetKeyboardInputEnabled(true);
	
	m_currentPosition.y += p_labelSize.y;
	
	return textBox;
}


Gwen::Controls::ComboBox* PanelBase::addLabeledComboBox(const std::string&      p_text,
                                                        const tt::math::Point2& p_labelSize)
{
	TT_NULL_ASSERT(m_root);

	addLabel(m_currentPosition, p_labelSize, p_text);
	
	Gwen::Controls::ComboBox* comboBox = new Gwen::Controls::ComboBox(m_root);
	
	const int panelWidth = m_root->GetBounds().w;
	
	comboBox->SetBounds(m_currentPosition.x + p_labelSize.x, m_currentPosition.y,
		panelWidth - p_labelSize.x - m_rightMargin, p_labelSize.y);

	m_currentPosition.y += p_labelSize.y;
	
	return comboBox;
}


Gwen::Controls::CheckBoxWithLabel* PanelBase::addLabeledCheckBox(const std::string&      p_text,
                                                                 const tt::math::Point2& p_size,
                                                                 bool p_checkState)
{
	TT_NULL_ASSERT(m_root);
	
	Gwen::Controls::CheckBoxWithLabel* checkBox = new Gwen::Controls::CheckBoxWithLabel(m_root);
	
	checkBox->SetPos(m_currentPosition.x, m_currentPosition.y);
	checkBox->SetHeight(p_size.y);
	checkBox->Checkbox()->SetChecked(p_checkState);
	checkBox->Label()->SetText(p_text);
	
	m_currentPosition.y += p_size.y;
	
	return checkBox;
}


Gwen::Controls::GroupBox* PanelBase::addGroupBox(const std::string&      p_title,
                                                 const tt::math::Point2& p_itemSize,
                                                 s32                     p_items,
                                                 s32                     p_margin,
                                                 const Gwen::Color&      p_textColor)
{
	TT_NULL_ASSERT(m_root);
	
	m_currentPosition.x = 0;
	
	Gwen::Controls::GroupBox* group = new Gwen::Controls::GroupBox(m_root);
	
	group->SetText(p_title);
	group->SetTextColor(p_textColor);
	
	group->SetBounds(m_currentPosition.x, m_currentPosition.y, p_itemSize.x, (p_items + 1) * p_itemSize.y);
	
	m_currentPosition.y += p_itemSize.y;
	m_currentPosition.x = p_margin;

	return group;
}


ColorButton* PanelBase::addLabeledColorButton(const std::string&      p_text,
                                              const tt::math::Point2& p_labelSize,
                                              const tt::math::Point2& p_buttonSize,
                                              const std::string&      p_name)
{
	TT_NULL_ASSERT(m_root);
	addLabel(m_currentPosition, p_labelSize, p_text);
	ColorButton* button = new ColorButton(m_root);
	button->SetBounds(p_labelSize.x, m_currentPosition.y, p_buttonSize.x, p_buttonSize.y);
	button->SetName(p_name);
	
	m_currentPosition.y += p_labelSize.y;
	
	return button;
}


AdaptiveSlider* PanelBase::addLabeledSlider(const std::string&      p_text,
                                            const tt::math::Point2& p_labelSize,
                                            real                    p_value)
{
	return addLabeledSlider(p_text, p_labelSize, p_value, m_root);
}


AdaptiveSlider* PanelBase::addLabeledSlider(const std::string& p_text,
                                            const tt::math::Point2& p_labelSize,
                                            real p_value,
                                            Gwen::Controls::Base* p_root)
{
	TT_NULL_ASSERT(p_root);
	
	addLabel(m_currentPosition, p_labelSize, p_text, p_root);
	
	AdaptiveSlider* slider = new AdaptiveSlider(p_root);
	slider->setInternalValue(p_value);
	
	const int panelWidth = m_root->GetBounds().w;
	slider->SetBounds(m_currentPosition.x + p_labelSize.x, m_currentPosition.y,
	                  panelWidth - p_labelSize.x - m_rightMargin, p_labelSize.y);
	
	m_currentPosition.y += p_labelSize.y;
	
	m_sliders.push_back(slider);
	
	return slider;
}


void PanelBase::setControlEnabled(Gwen::Controls::Base* p_control, bool p_enabled)
{
	TT_NULL_ASSERT(p_control);

	p_control->SetKeyboardInputEnabled(p_enabled);
	p_control->SetMouseInputEnabled(p_enabled);
	p_control->SetDisabled(p_enabled == false);
}


}
}
